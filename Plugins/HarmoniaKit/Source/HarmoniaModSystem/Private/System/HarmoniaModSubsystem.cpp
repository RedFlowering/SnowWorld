// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaModSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogHarmoniaModSystem);

void UHarmoniaModSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Initializing Harmonia Mod System"));

	bIsInitialized = true;

	// Set default configuration if not set
	if (ModSearchPaths.Num() == 0)
	{
		ModSearchPaths.Add(FPaths::ProjectDir() + TEXT("Mods/"));
		ModSearchPaths.Add(FPaths::ProjectSavedDir() + TEXT("Mods/"));
	}

	if (MaxModCount <= 0)
	{
		MaxModCount = 256; // Default max
	}

	// Discover and load mods if auto-load is enabled
	if (bAutoLoadModsOnStartup)
	{
		DiscoverMods();

		TArray<FName> FailedMods;
		int32 LoadedCount = LoadAllMods(FailedMods);

		UE_LOG(LogHarmoniaModSystem, Log, TEXT("Auto-loaded %d mods (%d failed)"), LoadedCount, FailedMods.Num());

		if (FailedMods.Num() > 0)
		{
			for (const FName& ModId : FailedMods)
			{
				UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Failed to load mod: %s"), *ModId.ToString());
			}
		}
	}
}

void UHarmoniaModSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Deinitializing Harmonia Mod System"));

	// Unload all mods
	UnloadAllMods();

	DiscoveredMods.Empty();
	LoadedMods.Empty();
	DetectedConflicts.Empty();

	bIsInitialized = false;

	Super::Deinitialize();
}

int32 UHarmoniaModSubsystem::DiscoverMods()
{
	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Discovering mods..."));

	DiscoveredMods.Empty();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	int32 DiscoveredCount = 0;

	for (const FString& SearchPath : ModSearchPaths)
	{
		if (!PlatformFile.DirectoryExists(*SearchPath))
		{
			UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Mod search path does not exist: %s"), *SearchPath);
			continue;
		}

		// Find all ModInfo.json files
		TArray<FString> ModDirectories;
		PlatformFile.IterateDirectory(*SearchPath, [&ModDirectories](const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (bIsDirectory)
			{
				ModDirectories.Add(FilenameOrDirectory);
			}
			return true;
		});

		for (const FString& ModDir : ModDirectories)
		{
			FString ManifestPath = ModDir / TEXT("ModInfo.json");
			if (PlatformFile.FileExists(*ManifestPath))
			{
				FHarmoniaModInfo ModInfo;
				if (LoadModManifest(ManifestPath, ModInfo))
				{
					ModInfo.ModRootPath = ModDir;
					DiscoveredMods.Add(ModInfo.ModId, ModInfo);
					DiscoveredCount++;

					UE_LOG(LogHarmoniaModSystem, Log, TEXT("Discovered mod: %s (%s) v%s by %s"),
						*ModInfo.ModId.ToString(),
						*ModInfo.DisplayName.ToString(),
						*ModInfo.Version,
						*ModInfo.Author);
				}
				else
				{
					UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Failed to load manifest: %s"), *ManifestPath);
				}
			}
		}
	}

	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Discovered %d mods"), DiscoveredCount);
	return DiscoveredCount;
}

bool UHarmoniaModSubsystem::LoadModManifest(const FString& ManifestPath, FHarmoniaModInfo& OutModInfo)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *ManifestPath))
	{
		UE_LOG(LogHarmoniaModSystem, Error, TEXT("Failed to read manifest file: %s"), *ManifestPath);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogHarmoniaModSystem, Error, TEXT("Failed to parse JSON in manifest: %s"), *ManifestPath);
		return false;
	}

	// Parse basic info
	OutModInfo.ModId = FName(*JsonObject->GetStringField(TEXT("ModId")));
	OutModInfo.DisplayName = FText::FromString(JsonObject->GetStringField(TEXT("DisplayName")));
	OutModInfo.Author = JsonObject->GetStringField(TEXT("Author"));
	OutModInfo.Version = JsonObject->GetStringField(TEXT("Version"));
	OutModInfo.Description = FText::FromString(JsonObject->GetStringField(TEXT("Description")));

	// Parse optional fields
	if (JsonObject->HasField(TEXT("LoadPriority")))
	{
		int32 Priority = JsonObject->GetIntegerField(TEXT("LoadPriority"));
		OutModInfo.LoadPriority = static_cast<EHarmoniaModLoadPriority>(Priority);
	}

	if (JsonObject->HasField(TEXT("Homepage")))
	{
		OutModInfo.Homepage = JsonObject->GetStringField(TEXT("Homepage"));
	}

	if (JsonObject->HasField(TEXT("WorkshopItemId")))
	{
		OutModInfo.WorkshopItemId = static_cast<int64>(JsonObject->GetNumberField(TEXT("WorkshopItemId")));
	}

	if (JsonObject->HasField(TEXT("SandboxMode")))
	{
		OutModInfo.bSandboxMode = JsonObject->GetBoolField(TEXT("SandboxMode"));
	}

	if (JsonObject->HasField(TEXT("HotReloadEnabled")))
	{
		OutModInfo.bHotReloadEnabled = JsonObject->GetBoolField(TEXT("HotReloadEnabled"));
	}

	// Parse dependencies
	if (JsonObject->HasField(TEXT("Dependencies")))
	{
		const TArray<TSharedPtr<FJsonValue>>& DepsArray = JsonObject->GetArrayField(TEXT("Dependencies"));
		for (const TSharedPtr<FJsonValue>& DepValue : DepsArray)
		{
			if (DepValue->Type == EJson::Object)
			{
				TSharedPtr<FJsonObject> DepObj = DepValue->AsObject();
				FHarmoniaModDependency Dep;
				Dep.ModId = FName(*DepObj->GetStringField(TEXT("ModId")));
				Dep.MinVersion = DepObj->GetStringField(TEXT("MinVersion"));

				if (DepObj->HasField(TEXT("MaxVersion")))
				{
					Dep.MaxVersion = DepObj->GetStringField(TEXT("MaxVersion"));
				}

				if (DepObj->HasField(TEXT("Optional")))
				{
					Dep.bOptional = DepObj->GetBoolField(TEXT("Optional"));
				}

				OutModInfo.Dependencies.Add(Dep);
			}
		}
	}

	// Parse incompatibilities
	if (JsonObject->HasField(TEXT("Incompatibilities")))
	{
		const TArray<TSharedPtr<FJsonValue>>& IncompatArray = JsonObject->GetArrayField(TEXT("Incompatibilities"));
		for (const TSharedPtr<FJsonValue>& IncompatValue : IncompatArray)
		{
			if (IncompatValue->Type == EJson::Object)
			{
				TSharedPtr<FJsonObject> IncompatObj = IncompatValue->AsObject();
				FHarmoniaModIncompatibility Incompat;
				Incompat.ModId = FName(*IncompatObj->GetStringField(TEXT("ModId")));
				Incompat.Reason = IncompatObj->GetStringField(TEXT("Reason"));

				if (IncompatObj->HasField(TEXT("VersionRange")))
				{
					Incompat.VersionRange = IncompatObj->GetStringField(TEXT("VersionRange"));
				}

				OutModInfo.Incompatibilities.Add(Incompat);
			}
		}
	}

	OutModInfo.LoadState = EHarmoniaModLoadState::Unloaded;

	return true;
}

bool UHarmoniaModSubsystem::LoadMod(FName ModId, FString& OutErrorMessage)
{
	if (!DiscoveredMods.Contains(ModId))
	{
		OutErrorMessage = FString::Printf(TEXT("Mod '%s' not found"), *ModId.ToString());
		UE_LOG(LogHarmoniaModSystem, Error, TEXT("%s"), *OutErrorMessage);
		return false;
	}

	FHarmoniaModInfo& ModInfo = DiscoveredMods[ModId];

	// Check if already loaded
	if (LoadedMods.Contains(ModId))
	{
		UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Mod '%s' is already loaded"), *ModId.ToString());
		return true;
	}

	// Update state
	ModInfo.LoadState = EHarmoniaModLoadState::Loading;

	// Validate mod
	if (!ValidateMod(ModInfo, OutErrorMessage))
	{
		ModInfo.LoadState = EHarmoniaModLoadState::Failed;
		OnModLoadFailed.Broadcast(ModInfo, OutErrorMessage);
		return false;
	}

	// Check dependencies
	TArray<FHarmoniaModDependency> MissingDeps;
	if (!CheckDependencies(ModId, MissingDeps))
	{
		// Try to load missing dependencies
		for (const FHarmoniaModDependency& Dep : MissingDeps)
		{
			if (!Dep.bOptional)
			{
				FString DepError;
				if (!LoadMod(Dep.ModId, DepError))
				{
					OutErrorMessage = FString::Printf(TEXT("Failed to load dependency '%s': %s"),
						*Dep.ModId.ToString(), *DepError);
					ModInfo.LoadState = EHarmoniaModLoadState::Failed;
					OnModLoadFailed.Broadcast(ModInfo, OutErrorMessage);
					return false;
				}
			}
		}
	}

	// Check incompatibilities
	for (const FHarmoniaModIncompatibility& Incompat : ModInfo.Incompatibilities)
	{
		if (IsModLoaded(Incompat.ModId))
		{
			OutErrorMessage = FString::Printf(TEXT("Mod '%s' is incompatible with loaded mod '%s': %s"),
				*ModId.ToString(), *Incompat.ModId.ToString(), *Incompat.Reason);
			ModInfo.LoadState = EHarmoniaModLoadState::Conflict;

			FHarmoniaModConflict Conflict;
			Conflict.ModA = ModId;
			Conflict.ModB = Incompat.ModId;
			Conflict.Severity = EHarmoniaModConflictSeverity::Critical;
			Conflict.Description = Incompat.Reason;
			OnModConflictDetected.Broadcast(Conflict);

			return false;
		}
	}

	// Apply mod content
	if (!ApplyModContent(ModInfo))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to apply content for mod '%s'"), *ModId.ToString());
		ModInfo.LoadState = EHarmoniaModLoadState::Failed;
		OnModLoadFailed.Broadcast(ModInfo, OutErrorMessage);
		return false;
	}

	// Mark as loaded
	ModInfo.LoadState = EHarmoniaModLoadState::Loaded;
	LoadedMods.Add(ModId, ModInfo);

	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Successfully loaded mod: %s"), *ModId.ToString());
	OnModLoaded.Broadcast(ModInfo);

	return true;
}

int32 UHarmoniaModSubsystem::LoadAllMods(TArray<FName>& OutFailedMods)
{
	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Loading all discovered mods..."));

	OutFailedMods.Empty();
	int32 LoadedCount = 0;

	// Get all mods and sort by priority
	TArray<FHarmoniaModInfo*> ModsToLoad;
	for (auto& Pair : DiscoveredMods)
	{
		ModsToLoad.Add(&Pair.Value);
	}

	SortModsByPriority(ModsToLoad);

	// Load mods in order
	for (FHarmoniaModInfo* ModInfo : ModsToLoad)
	{
		FString ErrorMessage;
		if (LoadMod(ModInfo->ModId, ErrorMessage))
		{
			LoadedCount++;
		}
		else
		{
			OutFailedMods.Add(ModInfo->ModId);
		}
	}

	// Detect conflicts if enabled
	if (bEnableConflictDetection)
	{
		DetectedConflicts = DetectConflicts();
		if (DetectedConflicts.Num() > 0)
		{
			UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Detected %d mod conflicts"), DetectedConflicts.Num());
		}
	}

	return LoadedCount;
}

bool UHarmoniaModSubsystem::UnloadMod(FName ModId)
{
	if (!LoadedMods.Contains(ModId))
	{
		UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Mod '%s' is not loaded"), *ModId.ToString());
		return false;
	}

	FHarmoniaModInfo& ModInfo = LoadedMods[ModId];

	// Unapply mod content
	UnapplyModContent(ModInfo);

	// Update state
	ModInfo.LoadState = EHarmoniaModLoadState::Unloaded;

	// Remove from loaded mods
	LoadedMods.Remove(ModId);

	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Unloaded mod: %s"), *ModId.ToString());

	return true;
}

void UHarmoniaModSubsystem::UnloadAllMods()
{
	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Unloading all mods..."));

	TArray<FName> ModIds;
	LoadedMods.GetKeys(ModIds);

	for (const FName& ModId : ModIds)
	{
		UnloadMod(ModId);
	}

	LoadedMods.Empty();
}

int32 UHarmoniaModSubsystem::ReloadAllMods()
{
	if (!bEnableHotReload)
	{
		UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Hot-reload is disabled"));
		return 0;
	}

	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Hot-reloading all mods..."));

	// Save current loaded mods
	TArray<FName> LoadedModIds;
	LoadedMods.GetKeys(LoadedModIds);

	// Unload all
	UnloadAllMods();

	// Reload all
	TArray<FName> FailedMods;
	int32 ReloadedCount = 0;

	for (const FName& ModId : LoadedModIds)
	{
		FString ErrorMessage;
		if (LoadMod(ModId, ErrorMessage))
		{
			ReloadedCount++;
		}
		else
		{
			FailedMods.Add(ModId);
		}
	}

	OnModsReloaded.Broadcast();

	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Reloaded %d mods (%d failed)"), ReloadedCount, FailedMods.Num());

	return ReloadedCount;
}

bool UHarmoniaModSubsystem::ReloadMod(FName ModId)
{
	if (!bEnableHotReload)
	{
		UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Hot-reload is disabled"));
		return false;
	}

	if (!LoadedMods.Contains(ModId))
	{
		UE_LOG(LogHarmoniaModSystem, Warning, TEXT("Mod '%s' is not loaded, cannot reload"), *ModId.ToString());
		return false;
	}

	// Unload
	UnloadMod(ModId);

	// Reload
	FString ErrorMessage;
	if (LoadMod(ModId, ErrorMessage))
	{
		UE_LOG(LogHarmoniaModSystem, Log, TEXT("Successfully reloaded mod: %s"), *ModId.ToString());
		return true;
	}

	return false;
}

bool UHarmoniaModSubsystem::GetModInfo(FName ModId, FHarmoniaModInfo& OutModInfo) const
{
	if (DiscoveredMods.Contains(ModId))
	{
		OutModInfo = DiscoveredMods[ModId];
		return true;
	}

	return false;
}

TArray<FHarmoniaModInfo> UHarmoniaModSubsystem::GetAllMods() const
{
	TArray<FHarmoniaModInfo> AllMods;
	DiscoveredMods.GenerateValueArray(AllMods);
	return AllMods;
}

TArray<FHarmoniaModInfo> UHarmoniaModSubsystem::GetLoadedMods() const
{
	TArray<FHarmoniaModInfo> Loaded;
	LoadedMods.GenerateValueArray(Loaded);
	return Loaded;
}

bool UHarmoniaModSubsystem::IsModLoaded(FName ModId) const
{
	return LoadedMods.Contains(ModId);
}

TArray<FHarmoniaModInfo> UHarmoniaModSubsystem::GetModsByTag(FGameplayTag Tag) const
{
	TArray<FHarmoniaModInfo> FilteredMods;

	for (const auto& Pair : DiscoveredMods)
	{
		if (Pair.Value.ModTags.HasTag(Tag))
		{
			FilteredMods.Add(Pair.Value);
		}
	}

	return FilteredMods;
}

int32 UHarmoniaModSubsystem::GetLoadedModCount() const
{
	return LoadedMods.Num();
}

bool UHarmoniaModSubsystem::CheckDependencies(FName ModId, TArray<FHarmoniaModDependency>& OutMissingDependencies) const
{
	OutMissingDependencies.Empty();

	if (!DiscoveredMods.Contains(ModId))
	{
		return false;
	}

	const FHarmoniaModInfo& ModInfo = DiscoveredMods[ModId];

	for (const FHarmoniaModDependency& Dep : ModInfo.Dependencies)
	{
		if (!IsModLoaded(Dep.ModId))
		{
			OutMissingDependencies.Add(Dep);
		}
	}

	return OutMissingDependencies.Num() == 0;
}

TArray<FHarmoniaModConflict> UHarmoniaModSubsystem::DetectConflicts() const
{
	TArray<FHarmoniaModConflict> Conflicts;

	// Check explicit incompatibilities
	for (const auto& PairA : LoadedMods)
	{
		const FHarmoniaModInfo& ModA = PairA.Value;

		for (const FHarmoniaModIncompatibility& Incompat : ModA.Incompatibilities)
		{
			if (IsModLoaded(Incompat.ModId))
			{
				FHarmoniaModConflict Conflict;
				Conflict.ModA = ModA.ModId;
				Conflict.ModB = Incompat.ModId;
				Conflict.Severity = EHarmoniaModConflictSeverity::Critical;
				Conflict.Description = Incompat.Reason;
				Conflicts.Add(Conflict);
			}
		}

		// Check for asset override conflicts
		for (const auto& PairB : LoadedMods)
		{
			if (PairA.Key == PairB.Key)
			{
				continue;
			}

			const FHarmoniaModInfo& ModB = PairB.Value;

			// Check if both mods override the same assets
			for (const FHarmoniaAssetOverride& OverrideA : ModA.AssetOverrides)
			{
				for (const FHarmoniaAssetOverride& OverrideB : ModB.AssetOverrides)
				{
					if (OverrideA.OriginalAssetPath == OverrideB.OriginalAssetPath)
					{
						FHarmoniaModConflict Conflict;
						Conflict.ModA = ModA.ModId;
						Conflict.ModB = ModB.ModId;
						Conflict.Severity = EHarmoniaModConflictSeverity::Warning;
						Conflict.Description = FString::Printf(TEXT("Both mods override asset: %s"),
							*OverrideA.OriginalAssetPath.ToString());
						Conflict.ConflictingResources.Add(OverrideA.OriginalAssetPath.ToString());
						Conflicts.Add(Conflict);
					}
				}
			}
		}
	}

	return Conflicts;
}

bool UHarmoniaModSubsystem::AreModsIncompatible(FName ModA, FName ModB) const
{
	if (!DiscoveredMods.Contains(ModA) || !DiscoveredMods.Contains(ModB))
	{
		return false;
	}

	const FHarmoniaModInfo& InfoA = DiscoveredMods[ModA];
	const FHarmoniaModInfo& InfoB = DiscoveredMods[ModB];

	// Check A's incompatibilities
	for (const FHarmoniaModIncompatibility& Incompat : InfoA.Incompatibilities)
	{
		if (Incompat.ModId == ModB)
		{
			return true;
		}
	}

	// Check B's incompatibilities
	for (const FHarmoniaModIncompatibility& Incompat : InfoB.Incompatibilities)
	{
		if (Incompat.ModId == ModA)
		{
			return true;
		}
	}

	return false;
}

TArray<FName> UHarmoniaModSubsystem::GetLoadOrder() const
{
	TArray<FHarmoniaModInfo*> AllMods;
	for (const auto& Pair : DiscoveredMods)
	{
		AllMods.Add(const_cast<FHarmoniaModInfo*>(&Pair.Value));
	}

	const_cast<UHarmoniaModSubsystem*>(this)->SortModsByPriority(AllMods);

	TArray<FName> LoadOrder;
	for (const FHarmoniaModInfo* ModInfo : AllMods)
	{
		LoadOrder.Add(ModInfo->ModId);
	}

	return LoadOrder;
}

bool UHarmoniaModSubsystem::ValidateMod(const FHarmoniaModInfo& ModInfo, FString& OutErrorMessage) const
{
	// Check mod ID is valid
	if (ModInfo.ModId == NAME_None)
	{
		OutErrorMessage = TEXT("Mod ID is invalid");
		return false;
	}

	// Check version format
	if (ModInfo.Version.IsEmpty())
	{
		OutErrorMessage = TEXT("Mod version is empty");
		return false;
	}

	// Check max mod count
	if (LoadedMods.Num() >= MaxModCount)
	{
		OutErrorMessage = FString::Printf(TEXT("Maximum mod count reached (%d)"), MaxModCount);
		return false;
	}

	return true;
}

void UHarmoniaModSubsystem::SortModsByPriority(TArray<FHarmoniaModInfo*>& Mods)
{
	// Topological sort based on dependencies and priority
	Mods.Sort([](const FHarmoniaModInfo& A, const FHarmoniaModInfo& B)
	{
		// First sort by priority
		if (A.LoadPriority != B.LoadPriority)
		{
			return A.LoadPriority < B.LoadPriority;
		}

		// Then by alphabetical order
		return A.ModId.LexicalLess(B.ModId);
	});
}

bool UHarmoniaModSubsystem::ApplyModContent(FHarmoniaModInfo& ModInfo)
{
	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Applying content for mod: %s"), *ModInfo.ModId.ToString());

	// Asset overrides will be handled by HarmoniaAssetOverrideSubsystem
	// Data table patches will be handled by HarmoniaModDataTablePatcher
	// Custom rulesets will be handled by HarmoniaModRulesetSubsystem

	// For now, just mark as successful
	// Actual implementation will be added in specialized subsystems

	return true;
}

void UHarmoniaModSubsystem::UnapplyModContent(FHarmoniaModInfo& ModInfo)
{
	UE_LOG(LogHarmoniaModSystem, Log, TEXT("Unapplying content for mod: %s"), *ModInfo.ModId.ToString());

	// Revert asset overrides, data table patches, etc.
	// Will be implemented in specialized subsystems
}
