// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaSoundCacheSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HarmoniaLoadManager.h"
#include "Engine/Engine.h"

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

	// Register tick for debug display and looping sound management
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime) -> bool
	{
		CleanupFinishedSounds();
		DisplayActiveSoundsDebug();
		return true; // Keep ticking
	}), 0.1f); // Update every 0.1 seconds

	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Sound Cache initialized with %d sounds"), SoundCache.Num());
}

void UHarmoniaSoundCacheSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Deinitializing Harmonia Sound Cache Subsystem"));

	// Unregister tick
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
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

	USoundBase* Sound = GetSoundBaseFromData(SoundData);
	if (!Sound)
	{
		return nullptr;
	}

	// Play 2D sound with context overrides
	UAudioComponent* AudioComp = UGameplayStatics::CreateSound2D(
		WorldContext,
		Sound,
		Context.VolumeMultiplier,
		Context.PitchMultiplier,
		0.0f, // StartTime
		nullptr, // Concurrency settings
		false, // Don't persist across level transitions
		true // AutoDestroy
	);

	if (AudioComp)
	{
		// Apply fade in if requested
		if (Context.FadeInDuration > 0.0f)
		{
			AudioComp->FadeIn(Context.FadeInDuration, Context.VolumeMultiplier);
		}
		else
		{
			AudioComp->Play();
		}

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

	USoundBase* Sound = GetSoundBaseFromData(SoundData);
	if (!Sound)
	{
		return nullptr;
	}

	// Play at location (attenuation is configured in SoundCue)
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
		WorldContext,
		Sound,
		Location,
		Rotation,
		Context.VolumeMultiplier,
		Context.PitchMultiplier,
		0.0f, // StartTime
		nullptr, // Attenuation (use SoundCue settings)
		nullptr, // Concurrency settings
		true // AutoDestroy
	);

	if (AudioComp)
	{
		// Apply fade in if requested
		if (Context.FadeInDuration > 0.0f)
		{
			AudioComp->FadeIn(Context.FadeInDuration, Context.VolumeMultiplier);
		}

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

	USoundBase* Sound = GetSoundBaseFromData(SoundData);
	if (!Sound)
	{
		return nullptr;
	}

	// Play attached (attenuation is configured in SoundCue)
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
		Sound,
		AttachComponent,
		AttachPointName,
		Location,
		Rotation,
		EAttachLocation::KeepRelativeOffset,
		false, // Don't stop when attached to destroyed
		Context.VolumeMultiplier,
		Context.PitchMultiplier,
		0.0f, // StartTime
		nullptr, // Attenuation (use SoundCue settings)
		nullptr, // Concurrency settings
		true // AutoDestroy
	);

	if (AudioComp)
	{
		// Apply fade in if requested
		if (Context.FadeInDuration > 0.0f)
		{
			AudioComp->FadeIn(Context.FadeInDuration, Context.VolumeMultiplier);
		}

		UE_LOG(LogHarmoniaSoundCache, Verbose, TEXT("Playing sound attached: %s"), *SoundTag.ToString());
	}

	return AudioComp;
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
	int32 TotalSoundsLoaded = 0;

	// 1. HarmoniaLoadManager에서 Sound 테이블 로드 시도
	if (UHarmoniaLoadManager* LoadManager = UHarmoniaLoadManager::Get())
	{
		UDataTable* SoundDataTable = LoadManager->GetDataTableByKey(TEXT("Sound"));
		if (SoundDataTable)
		{
			if (SoundDataTable->GetRowStruct() == FHarmoniaSoundData::StaticStruct())
			{
				int32 SoundsLoaded = LoadSoundsFromDataTable(SoundDataTable);
				TotalSoundsLoaded += SoundsLoaded;
				LoadedDataTables.Add(SoundDataTable);
				UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Loaded %d sounds from LoadManager DataTable: %s"),
					SoundsLoaded, *SoundDataTable->GetName());
			}
			else
			{
				UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("Sound DataTable from LoadManager has wrong row structure"));
			}
		}
	}

	// 2. Config 경로에서 추가 테이블 로드 (폴백/추가 테이블용)
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

		// 이미 LoadManager에서 로드했으면 스킵
		if (LoadedDataTables.Contains(DataTable))
		{
			continue;
		}

		int32 SoundsLoaded = LoadSoundsFromDataTable(DataTable);
		TotalSoundsLoaded += SoundsLoaded;

		LoadedDataTables.Add(DataTable);

		UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Loaded %d sounds from DataTable: %s"),
			SoundsLoaded, *DataTable->GetName());
	}

	if (TotalSoundsLoaded == 0)
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("No sounds loaded. Check HarmoniaLoadManager registry or SoundDataTablePaths config."));
	}
	else
	{
		UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Total sounds loaded: %d"), TotalSoundsLoaded);
	}
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
	if (!SoundData || SoundData->SoundCue.IsNull())
	{
		return nullptr;
	}

	return SoundData->SoundCue.LoadSynchronous();
}

// ============================================================================
// Priority-Based Sound Management Implementation
// ============================================================================

UAudioComponent* UHarmoniaSoundCacheSubsystem::PlayManagedSound(UObject* WorldContext, FGameplayTag SoundTag, float VolumeMultiplier)
{
	if (!SoundTag.IsValid())
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("PlayManagedSound called with invalid tag"));
		return nullptr;
	}

	// Get sound data from cache
	const FHarmoniaSoundData* SoundData = GetSoundData(SoundTag);
	if (!SoundData)
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("Sound not found in cache: %s"), *SoundTag.ToString());
		return nullptr;
	}

	// Clean up finished sounds
	CleanupFinishedSounds();

	// Check if we need to pause lower priority sounds
	int32 ActiveCount = 0;
	for (const FActiveSound& Sound : ActiveSounds)
	{
		if (!Sound.bPaused && Sound.AudioComponent.IsValid())
		{
			ActiveCount++;
		}
	}

	if (ActiveCount >= MaxConcurrentSounds)
	{
		// Find if new sound has higher priority than any playing sound
		int32 LowestPriorityIndex = FindLowestPrioritySound();
		if (LowestPriorityIndex >= 0)
		{
			const FHarmoniaSoundData* LowestData = GetSoundData(ActiveSounds[LowestPriorityIndex].SoundTag);
			if (LowestData && LowestData->Priority < SoundData->Priority)
			{
				PauseLowestPrioritySound();
			}
			else
			{
				UE_LOG(LogHarmoniaSoundCache, Verbose, TEXT("Cannot play %s: max concurrent sounds reached and priority not high enough"), *SoundTag.ToString());
				return nullptr;
			}
		}
	}

	// Load and play the sound
	USoundBase* Sound = GetSoundBaseFromData(SoundData);
	if (!Sound)
	{
		UE_LOG(LogHarmoniaSoundCache, Warning, TEXT("Failed to load sound: %s"), *SoundTag.ToString());
		return nullptr;
	}

	UAudioComponent* AudioComp = UGameplayStatics::CreateSound2D(
		WorldContext,
		Sound,
		VolumeMultiplier,
		1.0f,
		0.0f,
		nullptr,
		false,
		false
	);

	if (AudioComp)
	{
		// Apply loop setting from data table
		if (SoundData->bShouldLoop)
		{
			AudioComp->bIsUISound = false;
			AudioComp->bAutoDestroy = false;
		}
		
		// Apply fade in or direct play
		if (SoundData->FadeInDuration > 0.0f)
		{
			AudioComp->FadeIn(SoundData->FadeInDuration, VolumeMultiplier);
		}
		else
		{
			AudioComp->Play();
		}
		
		ActiveSounds.Add(FActiveSound(SoundTag, AudioComp, SoundData->bShouldLoop, SoundData->FadeInDuration, SoundData->FadeOutDuration, VolumeMultiplier));
		UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Playing managed sound: %s (Priority: %d, Loop: %s, FadeIn: %.1fs, Active: %d)"), 
			*SoundTag.ToString(), SoundData->Priority, SoundData->bShouldLoop ? TEXT("Yes") : TEXT("No"), SoundData->FadeInDuration, ActiveSounds.Num());
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
				FString::Printf(TEXT("▶ Sound: %s (Loop: %s, FadeIn: %.1fs)"), *SoundTag.ToString(), SoundData->bShouldLoop ? TEXT("Yes") : TEXT("No"), SoundData->FadeInDuration));
		}
	}

	return AudioComp;
}

void UHarmoniaSoundCacheSubsystem::StopManagedSound(FGameplayTag SoundTag, float FadeOutDuration)
{
	for (int32 i = ActiveSounds.Num() - 1; i >= 0; --i)
	{
		if (ActiveSounds[i].SoundTag == SoundTag && ActiveSounds[i].AudioComponent.IsValid())
		{
			// Use parameter if provided, otherwise use stored fade out duration
			float ActualFadeOut = (FadeOutDuration > 0.0f) ? FadeOutDuration : ActiveSounds[i].FadeOutDuration;
			
			if (ActualFadeOut > 0.0f)
			{
				ActiveSounds[i].AudioComponent->FadeOut(ActualFadeOut, 0.0f);
				UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Fading out sound: %s (%.1fs)"), *SoundTag.ToString(), ActualFadeOut);
			}
			else
			{
				ActiveSounds[i].AudioComponent->Stop();
			}
			ActiveSounds.RemoveAt(i);
		}
	}

	// Try to resume paused sounds
	ResumePausedSounds();
}

void UHarmoniaSoundCacheSubsystem::StopAllManagedSounds(float FadeOutDuration)
{
	for (FActiveSound& Sound : ActiveSounds)
	{
		if (Sound.AudioComponent.IsValid())
		{
			if (FadeOutDuration > 0.0f)
			{
				Sound.AudioComponent->FadeOut(FadeOutDuration, 0.0f);
			}
			else
			{
				Sound.AudioComponent->Stop();
			}
		}
	}
	ActiveSounds.Empty();
}

void UHarmoniaSoundCacheSubsystem::CleanupFinishedSounds()
{
	for (int32 i = ActiveSounds.Num() - 1; i >= 0; --i)
	{
		FActiveSound& ActiveSound = ActiveSounds[i];
		
		if (!ActiveSound.AudioComponent.IsValid())
		{
			ActiveSounds.RemoveAt(i);
			continue;
		}
		
		// Check if sound finished playing
		if (!ActiveSound.bPaused && !ActiveSound.AudioComponent->IsPlaying())
		{
			// If looping, restart the sound (Random nodes will pick new sound)
			if (ActiveSound.bLooping)
			{
				// Apply fade in for loop restart if configured
				if (ActiveSound.FadeInDuration > 0.0f)
				{
					ActiveSound.AudioComponent->FadeIn(ActiveSound.FadeInDuration, ActiveSound.VolumeMultiplier);
				}
				else
				{
					ActiveSound.AudioComponent->Play();
				}
				
				UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Restarting looping sound: %s (FadeIn: %.1fs)"), 
					*ActiveSound.SoundTag.ToString(), ActiveSound.FadeInDuration);
				
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, 
						FString::Printf(TEXT("🔄 Loop: %s"), *ActiveSound.SoundTag.ToString()));
				}
			}
			else
			{
				// Non-looping sound finished, remove it
				ActiveSounds.RemoveAt(i);
			}
		}
	}
}

int32 UHarmoniaSoundCacheSubsystem::FindLowestPrioritySound() const
{
	int32 LowestIndex = -1;
	int32 LowestPriority = INT32_MAX;

	for (int32 i = 0; i < ActiveSounds.Num(); ++i)
	{
		if (ActiveSounds[i].bPaused || !ActiveSounds[i].AudioComponent.IsValid())
		{
			continue;
		}

		const FHarmoniaSoundData* SoundData = GetSoundData(ActiveSounds[i].SoundTag);
		if (SoundData && SoundData->Priority < LowestPriority)
		{
			LowestPriority = SoundData->Priority;
			LowestIndex = i;
		}
	}

	return LowestIndex;
}

void UHarmoniaSoundCacheSubsystem::PauseLowestPrioritySound()
{
	int32 LowestIndex = FindLowestPrioritySound();
	if (LowestIndex >= 0 && ActiveSounds[LowestIndex].AudioComponent.IsValid())
	{
		ActiveSounds[LowestIndex].AudioComponent->SetPaused(true);
		ActiveSounds[LowestIndex].bPaused = true;
		UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Paused sound: %s"), *ActiveSounds[LowestIndex].SoundTag.ToString());
	}
}

void UHarmoniaSoundCacheSubsystem::ResumePausedSounds()
{
	CleanupFinishedSounds();

	int32 ActiveCount = 0;
	for (const FActiveSound& Sound : ActiveSounds)
	{
		if (!Sound.bPaused && Sound.AudioComponent.IsValid())
		{
			ActiveCount++;
		}
	}

	// Resume highest priority paused sounds first
	while (ActiveCount < MaxConcurrentSounds)
	{
		int32 HighestPausedIndex = -1;
		int32 HighestPriority = -1;

		for (int32 i = 0; i < ActiveSounds.Num(); ++i)
		{
			if (!ActiveSounds[i].bPaused || !ActiveSounds[i].AudioComponent.IsValid())
			{
				continue;
			}

			const FHarmoniaSoundData* SoundData = GetSoundData(ActiveSounds[i].SoundTag);
			if (SoundData && SoundData->Priority > HighestPriority)
			{
				HighestPriority = SoundData->Priority;
				HighestPausedIndex = i;
			}
		}

		if (HighestPausedIndex < 0)
		{
			break;
		}

		ActiveSounds[HighestPausedIndex].AudioComponent->SetPaused(false);
		ActiveSounds[HighestPausedIndex].bPaused = false;
		ActiveCount++;
		UE_LOG(LogHarmoniaSoundCache, Log, TEXT("Resumed sound: %s"), *ActiveSounds[HighestPausedIndex].SoundTag.ToString());
	}
}

void UHarmoniaSoundCacheSubsystem::DisplayActiveSoundsDebug()
{
	if (!bShowActiveSoundsDebug || !GEngine)
	{
		return;
	}

	// Build the debug string
	FString DebugText = TEXT("=== Active Sounds ===\n");
	
	if (ActiveSounds.Num() == 0)
	{
		DebugText += TEXT("(No sounds playing)");
	}
	else
	{
		for (int32 i = 0; i < ActiveSounds.Num(); ++i)
		{
			const FActiveSound& Sound = ActiveSounds[i];
			FString Status;
			
			if (Sound.bPaused)
			{
				Status = TEXT("[PAUSED]");
			}
			else if (Sound.bLooping)
			{
				Status = TEXT("[LOOP]");
			}
			else
			{
				Status = TEXT("[PLAY]");
			}
			
			DebugText += FString::Printf(TEXT("%s %s\n"), *Status, *Sound.SoundTag.ToString());
		}
	}
	
	DebugText += FString::Printf(TEXT("--- Total: %d/%d ---"), ActiveSounds.Num(), MaxConcurrentSounds);
	
	// Use a fixed key (100) so the message updates in place instead of stacking
	GEngine->AddOnScreenDebugMessage(100, 0.15f, FColor::Yellow, DebugText);
}
