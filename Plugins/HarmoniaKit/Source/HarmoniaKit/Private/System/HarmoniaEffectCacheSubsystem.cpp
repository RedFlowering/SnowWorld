// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaEffectCacheSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogHarmoniaEffectCache, Log, All);

void UHarmoniaEffectCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Initializing Harmonia Effect Cache Subsystem"));

	if (bPreloadAllEffects)
	{
		LoadAndCacheEffects();

		if (bPreloadAssets)
		{
			PreloadEffectAssets(false);
		}
	}

	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Effect Cache initialized with %d effects"), EffectCache.Num());
}

void UHarmoniaEffectCacheSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Deinitializing Harmonia Effect Cache Subsystem"));

	ClearCache();

	Super::Deinitialize();
}

const FHarmoniaAnimationEffectData* UHarmoniaEffectCacheSubsystem::GetEffectData(FGameplayTag EffectTag) const
{
	if (!EffectTag.IsValid())
	{
		UE_LOG(LogHarmoniaEffectCache, Warning, TEXT("GetEffectData called with invalid tag"));
		return nullptr;
	}

	if (const FHarmoniaAnimationEffectData* Data = EffectCache.Find(EffectTag))
	{
		return Data;
	}

	// Log warning only if not already in failed set
	if (!FailedLoadTags.Contains(EffectTag))
	{
		UE_LOG(LogHarmoniaEffectCache, Warning, TEXT("Effect data not found for tag: %s"), *EffectTag.ToString());
	}

	return nullptr;
}

const FHarmoniaAnimationEffectData* UHarmoniaEffectCacheSubsystem::GetEffectDataLazy(FGameplayTag EffectTag, bool bForceLoad)
{
	if (!EffectTag.IsValid())
	{
		return nullptr;
	}

	// Check cache first
	if (const FHarmoniaAnimationEffectData* Data = EffectCache.Find(EffectTag))
	{
		return Data;
	}

	// Try lazy loading if requested
	if (bForceLoad && !FailedLoadTags.Contains(EffectTag))
	{
		if (LoadEffectFromDataTables(EffectTag))
		{
			return EffectCache.Find(EffectTag);
		}

		// Mark as failed to avoid repeated attempts
		FailedLoadTags.Add(EffectTag);
	}

	return nullptr;
}

bool UHarmoniaEffectCacheSubsystem::HasEffectData(FGameplayTag EffectTag) const
{
	return EffectTag.IsValid() && EffectCache.Contains(EffectTag);
}

TArray<FGameplayTag> UHarmoniaEffectCacheSubsystem::GetAllEffectTags() const
{
	TArray<FGameplayTag> Tags;
	EffectCache.GetKeys(Tags);
	return Tags;
}

TArray<FHarmoniaAnimationEffectData> UHarmoniaEffectCacheSubsystem::GetEffectsByTag(FGameplayTag ParentTag, bool bExactMatch) const
{
	TArray<FHarmoniaAnimationEffectData> MatchingEffects;

	if (!ParentTag.IsValid())
	{
		return MatchingEffects;
	}

	for (const auto& Pair : EffectCache)
	{
		const FGameplayTag& EffectTag = Pair.Key;
		const FHarmoniaAnimationEffectData& EffectData = Pair.Value;

		bool bMatches = false;
		if (bExactMatch)
		{
			bMatches = (EffectTag == ParentTag);
		}
		else
		{
			bMatches = EffectTag.MatchesTag(ParentTag);
		}

		if (bMatches)
		{
			MatchingEffects.Add(EffectData);
		}
	}

	return MatchingEffects;
}

void UHarmoniaEffectCacheSubsystem::ReloadEffectCache()
{
	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Reloading effect cache..."));

	ClearCache();
	LoadAndCacheEffects();

	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Effect cache reloaded with %d effects"), EffectCache.Num());
}

void UHarmoniaEffectCacheSubsystem::ClearCache()
{
	EffectCache.Empty();
	LoadedDataTables.Empty();
	FailedLoadTags.Empty();
}

void UHarmoniaEffectCacheSubsystem::PreloadEffectAssets(bool bAsync)
{
	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Preloading effect assets (Async: %s)..."), bAsync ? TEXT("true") : TEXT("false"));

	TArray<FSoftObjectPath> AssetsToLoad;

	// Collect all asset references
	for (const auto& Pair : EffectCache)
	{
		const FHarmoniaAnimationEffectData& EffectData = Pair.Value;

		if (!EffectData.NiagaraSystem.IsNull())
		{
			AssetsToLoad.Add(EffectData.NiagaraSystem.ToSoftObjectPath());
		}

		if (!EffectData.ParticleSystem.IsNull())
		{
			AssetsToLoad.Add(EffectData.ParticleSystem.ToSoftObjectPath());
		}

		if (!EffectData.Sound.IsNull())
		{
			AssetsToLoad.Add(EffectData.Sound.ToSoftObjectPath());
		}
	}

	if (AssetsToLoad.Num() == 0)
	{
		UE_LOG(LogHarmoniaEffectCache, Log, TEXT("No assets to preload"));
		return;
	}

	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Preloading %d assets..."), AssetsToLoad.Num());

	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		FStreamableManager& StreamableManager = AssetManager->GetStreamableManager();

		if (bAsync)
		{
			StreamableManager.RequestAsyncLoad(
				AssetsToLoad,
				FStreamableDelegate::CreateLambda([AssetsToLoad]()
				{
					UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Async asset preload completed (%d assets)"), AssetsToLoad.Num());
				})
			);
		}
		else
		{
			StreamableManager.RequestSyncLoad(AssetsToLoad);
			UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Sync asset preload completed (%d assets)"), AssetsToLoad.Num());
		}
	}
	else
	{
		UE_LOG(LogHarmoniaEffectCache, Warning, TEXT("AssetManager not initialized, cannot preload assets"));
	}
}

void UHarmoniaEffectCacheSubsystem::LoadAndCacheEffects()
{
	if (EffectDataTablePaths.Num() == 0)
	{
		UE_LOG(LogHarmoniaEffectCache, Warning, TEXT("No effect DataTable paths configured. Set EffectDataTablePaths in Project Settings or DefaultGame.ini"));
		return;
	}

	int32 TotalEffectsLoaded = 0;

	for (const FSoftObjectPath& TablePath : EffectDataTablePaths)
	{
		if (TablePath.IsNull())
		{
			continue;
		}

		// Load DataTable
		UDataTable* DataTable = Cast<UDataTable>(TablePath.TryLoad());
		if (!DataTable)
		{
			UE_LOG(LogHarmoniaEffectCache, Error, TEXT("Failed to load DataTable: %s"), *TablePath.ToString());
			continue;
		}

		// Check row structure
		if (DataTable->GetRowStruct() != FHarmoniaAnimationEffectData::StaticStruct())
		{
			UE_LOG(LogHarmoniaEffectCache, Error, TEXT("DataTable has wrong row structure: %s (expected FHarmoniaAnimationEffectData)"),
				*TablePath.ToString());
			continue;
		}

		// Load effects from table
		int32 EffectsLoaded = LoadEffectsFromDataTable(DataTable);
		TotalEffectsLoaded += EffectsLoaded;

		// Keep DataTable alive
		LoadedDataTables.Add(DataTable);

		UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Loaded %d effects from DataTable: %s"),
			EffectsLoaded, *DataTable->GetName());
	}

	UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Total effects loaded: %d"), TotalEffectsLoaded);
}

int32 UHarmoniaEffectCacheSubsystem::LoadEffectsFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return 0;
	}

	int32 EffectsLoaded = 0;

	TArray<FHarmoniaAnimationEffectData*> AllRows;
	DataTable->GetAllRows<FHarmoniaAnimationEffectData>(TEXT("LoadEffectsFromDataTable"), AllRows);

	for (FHarmoniaAnimationEffectData* RowData : AllRows)
	{
		if (!RowData)
		{
			continue;
		}

		// Validate effect tag
		if (!RowData->EffectTag.IsValid())
		{
			UE_LOG(LogHarmoniaEffectCache, Warning, TEXT("Effect data has invalid tag in DataTable: %s"),
				*DataTable->GetName());
			continue;
		}

		// Check for duplicates
		if (EffectCache.Contains(RowData->EffectTag))
		{
			UE_LOG(LogHarmoniaEffectCache, Warning, TEXT("Duplicate effect tag found: %s (overwriting)"),
				*RowData->EffectTag.ToString());
		}

		// Add to cache
		EffectCache.Add(RowData->EffectTag, *RowData);
		EffectsLoaded++;
	}

	return EffectsLoaded;
}

bool UHarmoniaEffectCacheSubsystem::LoadEffectFromDataTables(FGameplayTag EffectTag)
{
	if (!EffectTag.IsValid())
	{
		return false;
	}

	// Search through all loaded DataTables
	for (UDataTable* DataTable : LoadedDataTables)
	{
		if (!DataTable)
		{
			continue;
		}

		// Try to find row by tag name
		FName RowName = FName(*EffectTag.ToString());
		FHarmoniaAnimationEffectData* RowData = DataTable->FindRow<FHarmoniaAnimationEffectData>(
			RowName,
			TEXT("LoadEffectFromDataTables")
		);

		if (RowData && RowData->EffectTag == EffectTag)
		{
			EffectCache.Add(EffectTag, *RowData);
			UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Lazy loaded effect: %s"), *EffectTag.ToString());
			return true;
		}

		// Also search all rows if row name doesn't match tag
		TArray<FHarmoniaAnimationEffectData*> AllRows;
		DataTable->GetAllRows<FHarmoniaAnimationEffectData>(TEXT("LoadEffectFromDataTables"), AllRows);

		for (FHarmoniaAnimationEffectData* Row : AllRows)
		{
			if (Row && Row->EffectTag == EffectTag)
			{
				EffectCache.Add(EffectTag, *Row);
				UE_LOG(LogHarmoniaEffectCache, Log, TEXT("Lazy loaded effect: %s"), *EffectTag.ToString());
				return true;
			}
		}
	}

	return false;
}
