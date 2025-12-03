// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaAssetOverrideSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

DEFINE_LOG_CATEGORY(LogHarmoniaAssetOverride);

void UHarmoniaAssetOverrideSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaAssetOverride, Log, TEXT("Initializing Harmonia Asset Override Subsystem"));

	ActiveOverrides.Empty();
	OverridesByMod.Empty();
	OriginalAssetCache.Empty();
	OverrideAssetCache.Empty();
}

void UHarmoniaAssetOverrideSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaAssetOverride, Log, TEXT("Deinitializing Harmonia Asset Override Subsystem"));

	ClearAllOverrides();

	Super::Deinitialize();
}

bool UHarmoniaAssetOverrideSubsystem::RegisterAssetOverride(const FHarmoniaAssetOverride& Override, FName ModId)
{
	if (!ValidateOverride(Override))
	{
		UE_LOG(LogHarmoniaAssetOverride, Error, TEXT("Invalid override for mod '%s'"), *ModId.ToString());
		return false;
	}

	// Check if already overridden
	if (ActiveOverrides.Contains(Override.OriginalAssetPath))
	{
		UE_LOG(LogHarmoniaAssetOverride, Warning, TEXT("Asset '%s' is already overridden, replacing..."),
			*Override.OriginalAssetPath.ToString());
	}

	// Preload override asset
	if (!PreloadOverrideAsset(Override))
	{
		UE_LOG(LogHarmoniaAssetOverride, Error, TEXT("Failed to preload override asset: %s"),
			*Override.OverrideAssetPath.ToString());
		return false;
	}

	// Register override
	ActiveOverrides.Add(Override.OriginalAssetPath, Override);

	// Track by mod
	if (!OverridesByMod.Contains(ModId))
	{
		OverridesByMod.Add(ModId, FHarmoniaSoftObjectPathArray());
	}
	OverridesByMod[ModId].Paths.AddUnique(Override.OriginalAssetPath);

	UE_LOG(LogHarmoniaAssetOverride, Log, TEXT("Registered asset override: %s -> %s (Mod: %s)"),
		*Override.OriginalAssetPath.ToString(),
		*Override.OverrideAssetPath.ToString(),
		*ModId.ToString());

	return true;
}

bool UHarmoniaAssetOverrideSubsystem::UnregisterAssetOverride(const FSoftObjectPath& OriginalAssetPath, FName ModId)
{
	if (!ActiveOverrides.Contains(OriginalAssetPath))
	{
		UE_LOG(LogHarmoniaAssetOverride, Warning, TEXT("No override registered for asset: %s"),
			*OriginalAssetPath.ToString());
		return false;
	}

	// Remove from active overrides
	ActiveOverrides.Remove(OriginalAssetPath);

	// Remove from mod tracking
	if (OverridesByMod.Contains(ModId))
	{
		OverridesByMod[ModId].Paths.Remove(OriginalAssetPath);

		if (OverridesByMod[ModId].Paths.Num() == 0)
		{
			OverridesByMod.Remove(ModId);
		}
	}

	// Clear cache
	OverrideAssetCache.Remove(OriginalAssetPath);

	UE_LOG(LogHarmoniaAssetOverride, Log, TEXT("Unregistered asset override: %s (Mod: %s)"),
		*OriginalAssetPath.ToString(), *ModId.ToString());

	return true;
}

void UHarmoniaAssetOverrideSubsystem::UnregisterAllModOverrides(FName ModId)
{
	if (!OverridesByMod.Contains(ModId))
	{
		return;
	}

	TArray<FSoftObjectPath> OverridePaths = OverridesByMod[ModId].Paths;

	for (const FSoftObjectPath& Path : OverridePaths)
	{
		ActiveOverrides.Remove(Path);
		OverrideAssetCache.Remove(Path);
	}

	OverridesByMod.Remove(ModId);

	UE_LOG(LogHarmoniaAssetOverride, Log, TEXT("Unregistered all overrides for mod: %s (%d overrides)"),
		*ModId.ToString(), OverridePaths.Num());
}

bool UHarmoniaAssetOverrideSubsystem::GetOverriddenAssetPath(const FSoftObjectPath& OriginalAssetPath, FSoftObjectPath& OutOverridePath) const
{
	if (ActiveOverrides.Contains(OriginalAssetPath))
	{
		OutOverridePath = ActiveOverrides[OriginalAssetPath].OverrideAssetPath;
		return true;
	}

	return false;
}

UObject* UHarmoniaAssetOverrideSubsystem::LoadAssetWithOverride(const FSoftObjectPath& AssetPath)
{
	FSoftObjectPath ActualPath = AssetPath;

	// Check for override
	FSoftObjectPath OverridePath;
	if (GetOverriddenAssetPath(AssetPath, OverridePath))
	{
		ActualPath = OverridePath;
		UE_LOG(LogHarmoniaAssetOverride, Verbose, TEXT("Using override: %s -> %s"),
			*AssetPath.ToString(), *OverridePath.ToString());
	}

	// Check cache first
	if (OverrideAssetCache.Contains(ActualPath))
	{
		return OverrideAssetCache[ActualPath];
	}

	// Load asset
	UObject* LoadedAsset = ActualPath.TryLoad();

	if (LoadedAsset)
	{
		OverrideAssetCache.Add(ActualPath, LoadedAsset);
		UE_LOG(LogHarmoniaAssetOverride, Verbose, TEXT("Loaded asset: %s"), *ActualPath.ToString());
	}
	else
	{
		UE_LOG(LogHarmoniaAssetOverride, Warning, TEXT("Failed to load asset: %s"), *ActualPath.ToString());
	}

	return LoadedAsset;
}

bool UHarmoniaAssetOverrideSubsystem::HasOverride(const FSoftObjectPath& AssetPath) const
{
	return ActiveOverrides.Contains(AssetPath);
}

TMap<FSoftObjectPath, FHarmoniaAssetOverride> UHarmoniaAssetOverrideSubsystem::GetAllOverrides() const
{
	return ActiveOverrides;
}

void UHarmoniaAssetOverrideSubsystem::ClearAllOverrides()
{
	UE_LOG(LogHarmoniaAssetOverride, Log, TEXT("Clearing all asset overrides"));

	ActiveOverrides.Empty();
	OverridesByMod.Empty();
	OverrideAssetCache.Empty();
	OriginalAssetCache.Empty();
}

int32 UHarmoniaAssetOverrideSubsystem::ApplyModOverrides(FName ModId, const TArray<FHarmoniaAssetOverride>& Overrides)
{
	int32 AppliedCount = 0;

	for (const FHarmoniaAssetOverride& Override : Overrides)
	{
		if (RegisterAssetOverride(Override, ModId))
		{
			AppliedCount++;
		}
	}

	UE_LOG(LogHarmoniaAssetOverride, Log, TEXT("Applied %d/%d overrides for mod: %s"),
		AppliedCount, Overrides.Num(), *ModId.ToString());

	return AppliedCount;
}

void UHarmoniaAssetOverrideSubsystem::RevertModOverrides(FName ModId)
{
	UnregisterAllModOverrides(ModId);
}

bool UHarmoniaAssetOverrideSubsystem::ValidateOverride(const FHarmoniaAssetOverride& Override) const
{
	// Check original path is valid
	if (!Override.OriginalAssetPath.IsValid())
	{
		UE_LOG(LogHarmoniaAssetOverride, Error, TEXT("Original asset path is invalid"));
		return false;
	}

	// Check override path is valid
	if (!Override.OverrideAssetPath.IsValid())
	{
		UE_LOG(LogHarmoniaAssetOverride, Error, TEXT("Override asset path is invalid"));
		return false;
	}

	// Check they're not the same
	if (Override.OriginalAssetPath == Override.OverrideAssetPath)
	{
		UE_LOG(LogHarmoniaAssetOverride, Error, TEXT("Original and override paths are the same"));
		return false;
	}

	return true;
}

bool UHarmoniaAssetOverrideSubsystem::PreloadOverrideAsset(const FHarmoniaAssetOverride& Override)
{
	// Try to load the override asset
	UObject* OverrideAsset = Override.OverrideAssetPath.TryLoad();

	if (!OverrideAsset)
	{
		UE_LOG(LogHarmoniaAssetOverride, Error, TEXT("Failed to load override asset: %s"),
			*Override.OverrideAssetPath.ToString());
		return false;
	}

	// Cache the override asset
	OverrideAssetCache.Add(Override.OverrideAssetPath, OverrideAsset);

	// Also cache the original asset if not already cached
	if (!OriginalAssetCache.Contains(Override.OriginalAssetPath))
	{
		UObject* OriginalAsset = Override.OriginalAssetPath.TryLoad();
		if (OriginalAsset)
		{
			OriginalAssetCache.Add(Override.OriginalAssetPath, OriginalAsset);
		}
	}

	return true;
}
