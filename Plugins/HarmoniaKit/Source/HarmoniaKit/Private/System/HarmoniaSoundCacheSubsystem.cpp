// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaSoundCacheSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogHarmoniaSoundCache, Log, All);

void UHarmoniaSoundCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Initializing Harmonia Sound Cache Subsystem"));

	if (bPreloadAllSounds)
	{
		LoadAndCacheSounds();

		if (bPreloadAssets)
		{
			PreloadSoundAssets(false);
		}
	}

	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Sound Cache initialized with %d sounds"), SoundCache.Num());
}

void UHarmoniaSoundCacheSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Deinitializing Harmonia Sound Cache Subsystem"));

	// Stop all playing sounds
	for (auto& Pair : PlayingSounds)
	{
		for (TWeakObjectPtr<UAudioComponent>& AudioCompPtr : Pair.Value)
		{
			if (UAudioComponent* AudioComp = AudioCompPtr.Get())
			{
				AudioComp->Stop();
			}
		}
	}

	ClearCache();

	Super::Deinitialize();
}

const FHarmoniaSoundData* UHarmoniaSoundCacheSubsystem::GetSoundData(FGameplayTag SoundTag) const
{
	if (!SoundTag.IsValid())
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("GetSoundData called with invalid tag"));
		return nullptr;
	}

	if (const FHarmoniaSoundData* Data = SoundCache.Find(SoundTag))
	{
		return Data;
	}

	if (!FailedLoadTags.Contains(SoundTag))
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("Sound data not found for tag: %s"), *SoundTag.ToString());
	}

	return nullptr;
}

const FHarmoniaSoundData* UHarmoniaSoundCacheSubsystem::GetSoundDataLazy(FGameplayTag SoundTag, bool bForceLoad)
{
	if (!SoundTag.IsValid())
	{
		return nullptr;
	}

	if (const FHarmoniaSoundData* Data = SoundCache.Find(SoundTag))
	{
		return Data;
	}

	if (bForceLoad && !FailedLoadTags.Contains(SoundTag))
	{
		if (LoadSoundFromDataTables(SoundTag))
		{
			return SoundCache.Find(SoundTag);
		}

		FailedLoadTags.Add(SoundTag);
	}

	return nullptr;
}

bool UHarmoniaSoundCacheSubsystem::HasSoundData(FGameplayTag SoundTag) const
{
	return SoundTag.IsValid() && SoundCache.Contains(SoundTag);
}

TArray<FGameplayTag> UHarmoniaSoundCacheSubsystem::GetAllSoundTags() const
{
	TArray<FGameplayTag> Tags;
	SoundCache.GetKeys(Tags);
	return Tags;
}

TArray<FHarmoniaSoundData> UHarmoniaSoundCacheSubsystem::GetSoundsByTag(FGameplayTag ParentTag, bool bExactMatch) const
{
	TArray<FHarmoniaSoundData> MatchingSounds;

	if (!ParentTag.IsValid())
	{
		return MatchingSounds;
	}

	for (const auto& Pair : SoundCache)
	{
		const FGameplayTag& SoundTag = Pair.Key;
		const FHarmoniaSoundData& SoundData = Pair.Value;

		bool bMatches = bExactMatch ? (SoundTag == ParentTag) : SoundTag.MatchesTag(ParentTag);

		if (bMatches)
		{
			MatchingSounds.Add(SoundData);
		}
	}

	return MatchingSounds;
}

TArray<FHarmoniaSoundData> UHarmoniaSoundCacheSubsystem::GetSoundsByGameplayTags(FGameplayTagContainer GameplayTags, bool bMatchAll) const
{
	TArray<FHarmoniaSoundData> MatchingSounds;

	if (GameplayTags.IsEmpty())
	{
		return MatchingSounds;
	}

	for (const auto& Pair : SoundCache)
	{
		const FHarmoniaSoundData& SoundData = Pair.Value;

		bool bMatches = bMatchAll ?
			SoundData.GameplayTags.HasAll(GameplayTags) :
			SoundData.GameplayTags.HasAny(GameplayTags);

		if (bMatches)
		{
			MatchingSounds.Add(SoundData);
		}
	}

	return MatchingSounds;
}

UAudioComponent* UHarmoniaSoundCacheSubsystem::PlaySound2DByTag(
	UObject* WorldContext,
	FGameplayTag SoundTag,
	FHarmoniaSoundPlaybackContext Context
)
{
	if (!WorldContext)
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("PlaySound2DByTag called with null WorldContext"));
		return nullptr;
	}

	const FHarmoniaSoundData* SoundData = GetSoundData(SoundTag);
	if (!SoundData)
	{
		SoundData = GetSoundDataLazy(SoundTag, true);
		if (!SoundData)
		{
			return nullptr;
		}
	}

	// Check concurrency
	if (!CheckConcurrency(SoundTag, SoundData))
	{
		return nullptr;
	}

	USoundBase* Sound = GetSoundBaseFromData(SoundData);
	if (!Sound)
	{
		return nullptr;
	}

	// Calculate final volume and pitch
	float FinalVolume = Context.bOverrideVolume ? Context.VolumeOverride : SoundData->VolumeMultiplier;
	float FinalPitch = Context.bOverridePitch ? Context.PitchOverride : SoundData->PitchMultiplier;
	ApplyRandomization(SoundData, FinalVolume, FinalPitch);

	// Play 2D sound
	UAudioComponent* AudioComp = UGameplayStatics::CreateSound2D(
		WorldContext,
		Sound,
		FinalVolume,
		FinalPitch,
		SoundData->StartTime,
		nullptr, // Concurrency settings
		false, // Don't persist across level transitions
		SoundData->bAutoDestroy
	);

	if (AudioComp)
	{
		// Apply fade in if requested
		if (Context.FadeInDuration > 0.0f)
		{
			AudioComp->FadeIn(Context.FadeInDuration, FinalVolume);
		}
		else
		{
			AudioComp->Play();
		}

		RegisterPlayingSound(SoundTag, AudioComp);

		UE_LOG(LogHarmoniaSoundCache, Verbose, TEXT("Playing 2D sound: %s"), *SoundTag.ToString());
	}

	return AudioComp;
}

UAudioComponent* UHarmoniaSoundCacheSubsystem::PlaySoundAtLocationByTag(
	UObject* WorldContext,
	FGameplayTag SoundTag,
	FVector Location,
	FRotator Rotation,
	FHarmoniaSoundPlaybackContext Context
)
{
	if (!WorldContext)
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("PlaySoundAtLocationByTag called with null WorldContext"));
		return nullptr;
	}

	const FHarmoniaSoundData* SoundData = GetSoundData(SoundTag);
	if (!SoundData)
	{
		SoundData = GetSoundDataLazy(SoundTag, true);
		if (!SoundData)
		{
			return nullptr;
		}
	}

	// Check concurrency
	if (!CheckConcurrency(SoundTag, SoundData))
	{
		return nullptr;
	}

	USoundBase* Sound = GetSoundBaseFromData(SoundData);
	if (!Sound)
	{
		return nullptr;
	}

	// Calculate final volume and pitch
	float FinalVolume = Context.bOverrideVolume ? Context.VolumeOverride : SoundData->VolumeMultiplier;
	float FinalPitch = Context.bOverridePitch ? Context.PitchOverride : SoundData->PitchMultiplier;
	ApplyRandomization(SoundData, FinalVolume, FinalPitch);

	// Load attenuation settings if available
	USoundAttenuation* Attenuation = nullptr;
	if (!SoundData->AttenuationSettings.IsNull())
	{
		Attenuation = SoundData->AttenuationSettings.LoadSynchronous();
	}

	// Play at location
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
		WorldContext,
		Sound,
		Location,
		Rotation,
		FinalVolume,
		FinalPitch,
		SoundData->StartTime,
		Attenuation,
		nullptr, // Concurrency settings
		SoundData->bAutoDestroy
	);

	if (AudioComp)
	{
		// Apply fade in if requested
		if (Context.FadeInDuration > 0.0f)
		{
			AudioComp->FadeIn(Context.FadeInDuration, FinalVolume);
		}

		RegisterPlayingSound(SoundTag, AudioComp);

		UE_LOG(LogHarmoniaSoundCache, Verbose, TEXT("Playing sound at location: %s"), *SoundTag.ToString());
	}

	return AudioComp;
}

UAudioComponent* UHarmoniaSoundCacheSubsystem::PlaySoundAttachedByTag(
	FGameplayTag SoundTag,
	USceneComponent* AttachComponent,
	FName AttachPointName,
	FVector Location,
	FRotator Rotation,
	FHarmoniaSoundPlaybackContext Context
)
{
	if (!AttachComponent)
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("PlaySoundAttachedByTag called with null AttachComponent"));
		return nullptr;
	}

	const FHarmoniaSoundData* SoundData = GetSoundData(SoundTag);
	if (!SoundData)
	{
		SoundData = GetSoundDataLazy(SoundTag, true);
		if (!SoundData)
		{
			return nullptr;
		}
	}

	// Check concurrency
	if (!CheckConcurrency(SoundTag, SoundData))
	{
		return nullptr;
	}

	USoundBase* Sound = GetSoundBaseFromData(SoundData);
	if (!Sound)
	{
		return nullptr;
	}

	// Calculate final volume and pitch
	float FinalVolume = Context.bOverrideVolume ? Context.VolumeOverride : SoundData->VolumeMultiplier;
	float FinalPitch = Context.bOverridePitch ? Context.PitchOverride : SoundData->PitchMultiplier;
	ApplyRandomization(SoundData, FinalVolume, FinalPitch);

	// Load attenuation settings if available
	USoundAttenuation* Attenuation = nullptr;
	if (!SoundData->AttenuationSettings.IsNull())
	{
		Attenuation = SoundData->AttenuationSettings.LoadSynchronous();
	}

	// Play attached
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
		Sound,
		AttachComponent,
		AttachPointName,
		Location,
		Rotation,
		EAttachLocation::KeepRelativeOffset,
		false, // Don't stop when attached to destroyed
		FinalVolume,
		FinalPitch,
		SoundData->StartTime,
		Attenuation,
		nullptr, // Concurrency settings
		SoundData->bAutoDestroy
	);

	if (AudioComp)
	{
		// Apply fade in if requested
		if (Context.FadeInDuration > 0.0f)
		{
			AudioComp->FadeIn(Context.FadeInDuration, FinalVolume);
		}

		RegisterPlayingSound(SoundTag, AudioComp);

		UE_LOG(LogHarmoniaSoundCache, Verbose, TEXT("Playing sound attached: %s"), *SoundTag.ToString());
	}

	return AudioComp;
}

void UHarmoniaSoundCacheSubsystem::StopSoundsByTag(FGameplayTag SoundTag, float FadeOutDuration)
{
	if (!PlayingSounds.Contains(SoundTag))
	{
		return;
	}

	TArray<TWeakObjectPtr<UAudioComponent>>& AudioComps = PlayingSounds[SoundTag];
	for (TWeakObjectPtr<UAudioComponent>& AudioCompPtr : AudioComps)
	{
		if (UAudioComponent* AudioComp = AudioCompPtr.Get())
		{
			if (FadeOutDuration > 0.0f)
			{
				AudioComp->FadeOut(FadeOutDuration, 0.0f);
			}
			else
			{
				AudioComp->Stop();
			}
		}
	}

	AudioComps.Empty();
}

void UHarmoniaSoundCacheSubsystem::StopSoundsByParentTag(FGameplayTag ParentTag, float FadeOutDuration)
{
	TArray<FGameplayTag> TagsToStop;

	for (const auto& Pair : PlayingSounds)
	{
		if (Pair.Key.MatchesTag(ParentTag))
		{
			TagsToStop.Add(Pair.Key);
		}
	}

	for (const FGameplayTag& Tag : TagsToStop)
	{
		StopSoundsByTag(Tag, FadeOutDuration);
	}
}

void UHarmoniaSoundCacheSubsystem::ReloadSoundCache()
{
	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Reloading sound cache..."));

	ClearCache();
	LoadAndCacheSounds();

	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Sound cache reloaded with %d sounds"), SoundCache.Num());
}

void UHarmoniaSoundCacheSubsystem::ClearCache()
{
	SoundCache.Empty();
	LoadedDataTables.Empty();
	FailedLoadTags.Empty();
	PlayingSounds.Empty();
	LastPlayTimes.Empty();
}

void UHarmoniaSoundCacheSubsystem::PreloadSoundAssets(bool bAsync)
{
	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Preloading sound assets (Async: %s)..."), bAsync ? TEXT("true") : TEXT("false"));

	TArray<FSoftObjectPath> AssetsToLoad;

	for (const auto& Pair : SoundCache)
	{
		const FHarmoniaSoundData& SoundData = Pair.Value;

		if (!SoundData.SoundCue.IsNull())
		{
			AssetsToLoad.Add(SoundData.SoundCue.ToSoftObjectPath());
		}

		if (!SoundData.SoundWave.IsNull())
		{
			AssetsToLoad.Add(SoundData.SoundWave.ToSoftObjectPath());
		}

		if (!SoundData.SoundBase.IsNull())
		{
			AssetsToLoad.Add(SoundData.SoundBase.ToSoftObjectPath());
		}
	}

	if (AssetsToLoad.Num() == 0)
	{
		return;
	}

	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		FStreamableManager& StreamableManager = AssetManager->GetStreamableManager();

		if (bAsync)
		{
			StreamableManager.RequestAsyncLoad(AssetsToLoad);
		}
		else
		{
			StreamableManager.RequestSyncLoad(AssetsToLoad);
		}
	}
}

void UHarmoniaSoundCacheSubsystem::LoadAndCacheSounds()
{
	if (SoundDataTablePaths.Num() == 0)
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("No sound DataTable paths configured"));
		return;
	}

	int32 TotalSoundsLoaded = 0;

	for (const FSoftObjectPath& TablePath : SoundDataTablePaths)
	{
		if (TablePath.IsNull())
		{
			continue;
		}

		UDataTable* DataTable = Cast<UDataTable>(TablePath.TryLoad());
		if (!DataTable)
		{
			UE_LOG(LogHarmoniaSoundCache, Error, TEXT("Failed to load DataTable: %s"), *TablePath.ToString());
			continue;
		}

		if (DataTable->GetRowStruct() != FHarmoniaSoundData::StaticStruct())
		{
			UE_LOG(LogHarmoniaSoundCache, Error, TEXT("DataTable has wrong row structure: %s"), *TablePath.ToString());
			continue;
		}

		int32 SoundsLoaded = LoadSoundsFromDataTable(DataTable);
		TotalSoundsLoaded += SoundsLoaded;

		LoadedDataTables.Add(DataTable);

		UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Loaded %d sounds from DataTable: %s"),
			SoundsLoaded, *DataTable->GetName());
	}

	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Total sounds loaded: %d"), TotalSoundsLoaded);
}

int32 UHarmoniaSoundCacheSubsystem::LoadSoundsFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return 0;
	}

	int32 SoundsLoaded = 0;

	TArray<FHarmoniaSoundData*> AllRows;
	DataTable->GetAllRows<FHarmoniaSoundData>(TEXT("LoadSoundsFromDataTable"), AllRows);

	for (FHarmoniaSoundData* RowData : AllRows)
	{
		if (!RowData || !RowData->SoundTag.IsValid())
		{
			continue;
		}

		if (SoundCache.Contains(RowData->SoundTag))
		{
			UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("Duplicate sound tag found: %s"), *RowData->SoundTag.ToString());
		}

		SoundCache.Add(RowData->SoundTag, *RowData);
		SoundsLoaded++;
	}

	return SoundsLoaded;
}

bool UHarmoniaSoundCacheSubsystem::LoadSoundFromDataTables(FGameplayTag SoundTag)
{
	if (!SoundTag.IsValid())
	{
		return false;
	}

	for (UDataTable* DataTable : LoadedDataTables)
	{
		if (!DataTable)
		{
			continue;
		}

		FName RowName = FName(*SoundTag.ToString());
		FHarmoniaSoundData* RowData = DataTable->FindRow<FHarmoniaSoundData>(RowName, TEXT("LoadSoundFromDataTables"));

		if (RowData && RowData->SoundTag == SoundTag)
		{
			SoundCache.Add(SoundTag, *RowData);
			return true;
		}
	}

	return false;
}

USoundBase* UHarmoniaSoundCacheSubsystem::GetSoundBaseFromData(const FHarmoniaSoundData* SoundData)
{
	if (!SoundData)
	{
		return nullptr;
	}

	// Priority: SoundCue > SoundBase > SoundWave
	if (!SoundData->SoundCue.IsNull())
	{
		return SoundData->SoundCue.LoadSynchronous();
	}

	if (!SoundData->SoundBase.IsNull())
	{
		return SoundData->SoundBase.LoadSynchronous();
	}

	if (!SoundData->SoundWave.IsNull())
	{
		return SoundData->SoundWave.LoadSynchronous();
	}

	return nullptr;
}

void UHarmoniaSoundCacheSubsystem::ApplyRandomization(const FHarmoniaSoundData* SoundData, float& OutVolume, float& OutPitch) const
{
	if (!SoundData)
	{
		return;
	}

	// Apply volume randomization
	if (SoundData->VolumeRange.X != SoundData->VolumeRange.Y)
	{
		float RandomVolume = FMath::RandRange(SoundData->VolumeRange.X, SoundData->VolumeRange.Y);
		OutVolume *= RandomVolume;
	}

	// Apply pitch randomization
	if (SoundData->PitchRange.X != SoundData->PitchRange.Y)
	{
		float RandomPitch = FMath::RandRange(SoundData->PitchRange.X, SoundData->PitchRange.Y);
		OutPitch *= RandomPitch;
	}
}

bool UHarmoniaSoundCacheSubsystem::CheckConcurrency(FGameplayTag SoundTag, const FHarmoniaSoundData* SoundData)
{
	if (!SoundData)
	{
		return false;
	}

	// Clean up finished sounds first
	CleanupFinishedSounds();

	// Check minimum time between plays
	if (SoundData->MinTimeBetweenPlays > 0.0f)
	{
		if (double* LastPlayTime = LastPlayTimes.Find(SoundTag))
		{
			double CurrentTime = FPlatformTime::Seconds();
			if (CurrentTime - *LastPlayTime < SoundData->MinTimeBetweenPlays)
			{
				UE_LOG(LogHarmoniaSoundCache, Verbose, TEXT("Sound %s played too recently, skipping"), *SoundTag.ToString());
				return false;
			}
		}
	}

	// Check max concurrent instances
	if (PlayingSounds.Contains(SoundTag))
	{
		TArray<TWeakObjectPtr<UAudioComponent>>& AudioComps = PlayingSounds[SoundTag];
		int32 ActiveCount = 0;

		for (const TWeakObjectPtr<UAudioComponent>& AudioCompPtr : AudioComps)
		{
			if (AudioCompPtr.IsValid() && AudioCompPtr->IsPlaying())
			{
				ActiveCount++;
			}
		}

		if (ActiveCount >= SoundData->MaxConcurrentInstances)
		{
			if (SoundData->bStopOldestInstance && AudioComps.Num() > 0)
			{
				// Stop oldest instance
				if (UAudioComponent* OldestComp = AudioComps[0].Get())
				{
					OldestComp->Stop();
				}
				AudioComps.RemoveAt(0);
			}
			else
			{
				UE_LOG(LogHarmoniaSoundCache, Verbose, TEXT("Max concurrent instances reached for: %s"), *SoundTag.ToString());
				return false;
			}
		}
	}

	// Update last play time
	LastPlayTimes.Add(SoundTag, FPlatformTime::Seconds());

	return true;
}

void UHarmoniaSoundCacheSubsystem::RegisterPlayingSound(FGameplayTag SoundTag, UAudioComponent* AudioComponent)
{
	if (!AudioComponent)
	{
		return;
	}

	if (!PlayingSounds.Contains(SoundTag))
	{
		PlayingSounds.Add(SoundTag, TArray<TWeakObjectPtr<UAudioComponent>>());
	}

	PlayingSounds[SoundTag].Add(AudioComponent);
}

void UHarmoniaSoundCacheSubsystem::CleanupFinishedSounds()
{
	for (auto& Pair : PlayingSounds)
	{
		Pair.Value.RemoveAll([](const TWeakObjectPtr<UAudioComponent>& AudioCompPtr)
		{
			return !AudioCompPtr.IsValid() || !AudioCompPtr->IsPlaying();
		});
	}
}
