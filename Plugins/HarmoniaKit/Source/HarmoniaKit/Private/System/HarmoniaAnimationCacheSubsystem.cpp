// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaAnimationCacheSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogHarmoniaAnimationCache, Log, All);

void UHarmoniaAnimationCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Initializing Harmonia Animation Cache Subsystem"));

	if (bPreloadAllAnimations)
	{
		LoadAndCacheAnimations();

		if (bPreloadAssets)
		{
			PreloadAnimationAssets(false);
		}
	}

	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Animation Cache initialized with %d animations"), AnimationCache.Num());
}

void UHarmoniaAnimationCacheSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Deinitializing Harmonia Animation Cache Subsystem"));

	ClearCache();

	Super::Deinitialize();
}

const FHarmoniaAnimationData* UHarmoniaAnimationCacheSubsystem::GetAnimationData(FGameplayTag AnimationTag) const
{
	if (!AnimationTag.IsValid())
	{
		UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("GetAnimationData called with invalid tag"));
		return nullptr;
	}

	if (const FHarmoniaAnimationData* Data = AnimationCache.Find(AnimationTag))
	{
		return Data;
	}

	if (!FailedLoadTags.Contains(AnimationTag))
	{
		UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("Animation data not found for tag: %s"), *AnimationTag.ToString());
	}

	return nullptr;
}

const FHarmoniaAnimationData* UHarmoniaAnimationCacheSubsystem::GetAnimationDataLazy(FGameplayTag AnimationTag, bool bForceLoad)
{
	if (!AnimationTag.IsValid())
	{
		return nullptr;
	}

	// Check cache first
	if (const FHarmoniaAnimationData* Data = AnimationCache.Find(AnimationTag))
	{
		return Data;
	}

	// Try lazy loading if requested
	if (bForceLoad && !FailedLoadTags.Contains(AnimationTag))
	{
		if (LoadAnimationFromDataTables(AnimationTag))
		{
			return AnimationCache.Find(AnimationTag);
		}

		FailedLoadTags.Add(AnimationTag);
	}

	return nullptr;
}

bool UHarmoniaAnimationCacheSubsystem::HasAnimationData(FGameplayTag AnimationTag) const
{
	return AnimationTag.IsValid() && AnimationCache.Contains(AnimationTag);
}

TArray<FGameplayTag> UHarmoniaAnimationCacheSubsystem::GetAllAnimationTags() const
{
	TArray<FGameplayTag> Tags;
	AnimationCache.GetKeys(Tags);
	return Tags;
}

TArray<FHarmoniaAnimationData> UHarmoniaAnimationCacheSubsystem::GetAnimationsByTag(FGameplayTag ParentTag, bool bExactMatch) const
{
	TArray<FHarmoniaAnimationData> MatchingAnimations;

	if (!ParentTag.IsValid())
	{
		return MatchingAnimations;
	}

	for (const auto& Pair : AnimationCache)
	{
		const FGameplayTag& AnimTag = Pair.Key;
		const FHarmoniaAnimationData& AnimData = Pair.Value;

		bool bMatches = false;
		if (bExactMatch)
		{
			bMatches = (AnimTag == ParentTag);
		}
		else
		{
			bMatches = AnimTag.MatchesTag(ParentTag);
		}

		if (bMatches)
		{
			MatchingAnimations.Add(AnimData);
		}
	}

	return MatchingAnimations;
}

TArray<FHarmoniaAnimationData> UHarmoniaAnimationCacheSubsystem::GetAnimationsByGameplayTags(FGameplayTagContainer GameplayTags, bool bMatchAll) const
{
	TArray<FHarmoniaAnimationData> MatchingAnimations;

	if (GameplayTags.IsEmpty())
	{
		return MatchingAnimations;
	}

	for (const auto& Pair : AnimationCache)
	{
		const FHarmoniaAnimationData& AnimData = Pair.Value;

		bool bMatches = false;
		if (bMatchAll)
		{
			bMatches = AnimData.GameplayTags.HasAll(GameplayTags);
		}
		else
		{
			bMatches = AnimData.GameplayTags.HasAny(GameplayTags);
		}

		if (bMatches)
		{
			MatchingAnimations.Add(AnimData);
		}
	}

	return MatchingAnimations;
}

float UHarmoniaAnimationCacheSubsystem::PlayAnimationByTag(
	ACharacter* Character,
	FGameplayTag AnimationTag,
	FHarmoniaAnimationPlaybackContext Context
)
{
	if (!Character)
	{
		UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("PlayAnimationByTag called with null Character"));
		return 0.0f;
	}

	const FHarmoniaAnimationData* AnimData = GetAnimationData(AnimationTag);
	if (!AnimData)
	{
		// Try lazy loading
		AnimData = GetAnimationDataLazy(AnimationTag, true);
		if (!AnimData)
		{
			UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("Failed to play animation: %s"), *AnimationTag.ToString());
			return 0.0f;
		}
	}

	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("Character has no SkeletalMeshComponent"));
		return 0.0f;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("Character has no AnimInstance"));
		return 0.0f;
	}

	// Calculate final play rate
	float FinalPlayRate = Context.bOverridePlayRate ? Context.PlayRateOverride : AnimData->PlayRate;
	float FinalBlendInTime = Context.bOverrideBlendInTime ? Context.BlendInTimeOverride : AnimData->BlendInTime;

	// Stop all montages if requested
	if (Context.bStopAllMontages)
	{
		AnimInstance->Montage_Stop(FinalBlendInTime);
	}

	// Try to play montage first (preferred)
	if (!AnimData->AnimMontage.IsNull())
	{
		UAnimMontage* Montage = AnimData->AnimMontage.LoadSynchronous();
		if (Montage)
		{
			float Duration = AnimInstance->Montage_Play(
				Montage,
				FinalPlayRate,
				EMontagePlayReturnType::MontageLength,
				AnimData->StartPosition,
				Context.bStopAllMontages
			);

			if (Duration > 0.0f)
			{
				UE_LOG(LogHarmoniaAnimationCache, Verbose, TEXT("Playing animation montage: %s (Duration: %.2f)"),
					*AnimationTag.ToString(), Duration);
				return Duration;
			}
		}
	}

	// Fallback to sequence (note: less control)
	if (!AnimData->AnimSequence.IsNull())
	{
		UAnimSequence* Sequence = AnimData->AnimSequence.LoadSynchronous();
		if (Sequence)
		{
			// For sequences, we'd need to use slot node in AnimBP
			// This is a simplified version - in practice, you'd want to create a montage
			UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("Direct AnimSequence playback not fully supported. Consider using AnimMontage for: %s"),
				*AnimationTag.ToString());
			return Sequence->GetPlayLength() / FinalPlayRate;
		}
	}

	UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("No playable animation found for tag: %s"), *AnimationTag.ToString());
	return 0.0f;
}

void UHarmoniaAnimationCacheSubsystem::StopAnimationByTag(
	ACharacter* Character,
	FGameplayTag AnimationTag,
	float BlendOutTime
)
{
	if (!Character)
	{
		return;
	}

	const FHarmoniaAnimationData* AnimData = GetAnimationData(AnimationTag);
	if (!AnimData)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MeshComp)
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// Use default blend out time if not specified
	float FinalBlendOutTime = BlendOutTime >= 0.0f ? BlendOutTime : AnimData->BlendOutTime;

	// Stop montage if it's playing
	if (!AnimData->AnimMontage.IsNull())
	{
		UAnimMontage* Montage = AnimData->AnimMontage.LoadSynchronous();
		if (Montage && AnimInstance->Montage_IsPlaying(Montage))
		{
			AnimInstance->Montage_Stop(FinalBlendOutTime, Montage);
		}
	}
}

void UHarmoniaAnimationCacheSubsystem::StopAnimationsByParentTag(
	ACharacter* Character,
	FGameplayTag ParentTag,
	float BlendOutTime
)
{
	if (!Character)
	{
		return;
	}

	TArray<FHarmoniaAnimationData> MatchingAnims = GetAnimationsByTag(ParentTag, false);
	for (const FHarmoniaAnimationData& AnimData : MatchingAnims)
	{
		StopAnimationByTag(Character, AnimData.AnimationTag, BlendOutTime);
	}
}

void UHarmoniaAnimationCacheSubsystem::ReloadAnimationCache()
{
	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Reloading animation cache..."));

	ClearCache();
	LoadAndCacheAnimations();

	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Animation cache reloaded with %d animations"), AnimationCache.Num());
}

void UHarmoniaAnimationCacheSubsystem::ClearCache()
{
	AnimationCache.Empty();
	LoadedDataTables.Empty();
	FailedLoadTags.Empty();
}

void UHarmoniaAnimationCacheSubsystem::PreloadAnimationAssets(bool bAsync)
{
	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Preloading animation assets (Async: %s)..."), bAsync ? TEXT("true") : TEXT("false"));

	TArray<FSoftObjectPath> AssetsToLoad;

	// Collect all asset references
	for (const auto& Pair : AnimationCache)
	{
		const FHarmoniaAnimationData& AnimData = Pair.Value;

		if (!AnimData.AnimMontage.IsNull())
		{
			AssetsToLoad.Add(AnimData.AnimMontage.ToSoftObjectPath());
		}

		if (!AnimData.AnimSequence.IsNull())
		{
			AssetsToLoad.Add(AnimData.AnimSequence.ToSoftObjectPath());
		}

		if (!AnimData.BlendSpace.IsNull())
		{
			AssetsToLoad.Add(AnimData.BlendSpace.ToSoftObjectPath());
		}
	}

	if (AssetsToLoad.Num() == 0)
	{
		UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("No assets to preload"));
		return;
	}

	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Preloading %d assets..."), AssetsToLoad.Num());

	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		FStreamableManager& StreamableManager = AssetManager->GetStreamableManager();

		if (bAsync)
		{
			StreamableManager.RequestAsyncLoad(
				AssetsToLoad,
				FStreamableDelegate::CreateLambda([AssetsToLoad]()
				{
					UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Async asset preload completed (%d assets)"), AssetsToLoad.Num());
				})
			);
		}
		else
		{
			StreamableManager.RequestSyncLoad(AssetsToLoad);
			UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Sync asset preload completed (%d assets)"), AssetsToLoad.Num());
		}
	}
}

void UHarmoniaAnimationCacheSubsystem::LoadAndCacheAnimations()
{
	if (AnimationDataTablePaths.Num() == 0)
	{
		UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("No animation DataTable paths configured"));
		return;
	}

	int32 TotalAnimationsLoaded = 0;

	for (const FSoftObjectPath& TablePath : AnimationDataTablePaths)
	{
		if (TablePath.IsNull())
		{
			continue;
		}

		UDataTable* DataTable = Cast<UDataTable>(TablePath.TryLoad());
		if (!DataTable)
		{
			UE_LOG(LogHarmoniaAnimationCache, Error, TEXT("Failed to load DataTable: %s"), *TablePath.ToString());
			continue;
		}

		if (DataTable->GetRowStruct() != FHarmoniaAnimationData::StaticStruct())
		{
			UE_LOG(LogHarmoniaAnimationCache, Error, TEXT("DataTable has wrong row structure: %s (expected FHarmoniaAnimationData)"),
				*TablePath.ToString());
			continue;
		}

		int32 AnimationsLoaded = LoadAnimationsFromDataTable(DataTable);
		TotalAnimationsLoaded += AnimationsLoaded;

		LoadedDataTables.Add(DataTable);

		UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Loaded %d animations from DataTable: %s"),
			AnimationsLoaded, *DataTable->GetName());
	}

	UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Total animations loaded: %d"), TotalAnimationsLoaded);
}

int32 UHarmoniaAnimationCacheSubsystem::LoadAnimationsFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return 0;
	}

	int32 AnimationsLoaded = 0;

	TArray<FHarmoniaAnimationData*> AllRows;
	DataTable->GetAllRows<FHarmoniaAnimationData>(TEXT("LoadAnimationsFromDataTable"), AllRows);

	for (FHarmoniaAnimationData* RowData : AllRows)
	{
		if (!RowData)
		{
			continue;
		}

		if (!RowData->AnimationTag.IsValid())
		{
			UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("Animation data has invalid tag in DataTable: %s"),
				*DataTable->GetName());
			continue;
		}

		if (AnimationCache.Contains(RowData->AnimationTag))
		{
			UE_LOG(LogHarmoniaAnimationCache, Warning, TEXT("Duplicate animation tag found: %s (overwriting)"),
				*RowData->AnimationTag.ToString());
		}

		AnimationCache.Add(RowData->AnimationTag, *RowData);
		AnimationsLoaded++;
	}

	return AnimationsLoaded;
}

bool UHarmoniaAnimationCacheSubsystem::LoadAnimationFromDataTables(FGameplayTag AnimationTag)
{
	if (!AnimationTag.IsValid())
	{
		return false;
	}

	for (UDataTable* DataTable : LoadedDataTables)
	{
		if (!DataTable)
		{
			continue;
		}

		FName RowName = FName(*AnimationTag.ToString());
		FHarmoniaAnimationData* RowData = DataTable->FindRow<FHarmoniaAnimationData>(
			RowName,
			TEXT("LoadAnimationFromDataTables")
		);

		if (RowData && RowData->AnimationTag == AnimationTag)
		{
			AnimationCache.Add(AnimationTag, *RowData);
			UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Lazy loaded animation: %s"), *AnimationTag.ToString());
			return true;
		}

		// Also search all rows
		TArray<FHarmoniaAnimationData*> AllRows;
		DataTable->GetAllRows<FHarmoniaAnimationData>(TEXT("LoadAnimationFromDataTables"), AllRows);

		for (FHarmoniaAnimationData* Row : AllRows)
		{
			if (Row && Row->AnimationTag == AnimationTag)
			{
				AnimationCache.Add(AnimationTag, *Row);
				UE_LOG(LogHarmoniaAnimationCache, Log, TEXT("Lazy loaded animation: %s"), *AnimationTag.ToString());
				return true;
			}
		}
	}

	return false;
}
