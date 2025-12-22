// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "GameplayTagContainer.h"
#include "Components/AudioComponent.h"
#include "Definitions/HarmoniaSoundDataDefinitions.h"
#include "HarmoniaSoundCacheSubsystem.generated.h"

/**
 * Active Sound - Tracks currently playing sounds for priority management
 */
struct FActiveSound
{
	FGameplayTag SoundTag;
	TWeakObjectPtr<UAudioComponent> AudioComponent;
	bool bPaused = false;
	bool bLooping = false;
	float FadeInDuration = 0.0f;
	float FadeOutDuration = 0.0f;
	float VolumeMultiplier = 1.0f;
	
	FActiveSound() = default;
	FActiveSound(FGameplayTag InTag, UAudioComponent* InComp, bool bInLooping, float InFadeIn, float InFadeOut, float InVolume)
		: SoundTag(InTag)
		, AudioComponent(InComp)
		, bPaused(false)
		, bLooping(bInLooping)
		, FadeInDuration(InFadeIn)
		, FadeOutDuration(InFadeOut)
		, VolumeMultiplier(InVolume)
	{
	}
};

/**
 * Harmonia Sound Cache Subsystem
 * Loads and caches sound data from DataTables at game startup
 *
 * Features:
 * - Fast tag-based sound lookup
 * - Centralized sound management
 * - Priority-based concurrent sound management
 * - Automatic pause/resume for priority conflicts
 *
 * Usage:
 * 1. Create DataTable(s) with FHarmoniaSoundData
 * 2. Set DataTablePaths in Project Settings or Config
 * 3. Query and play sounds by tag
 */
UCLASS(Config=Game)
class HARMONIAKIT_API UHarmoniaSoundCacheSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	// ============================================================================
	// Sound Data Access
	// ============================================================================

	/**
	 * Get sound data by tag
	 * @param SoundTag - Gameplay tag identifying the sound
	 * @return Pointer to cached sound data, or nullptr if not found
	 * NOTE: C++ only - use GetSoundsByTag for Blueprint
	 */
	const FHarmoniaSoundData* GetSoundData(FGameplayTag SoundTag) const;

	/**
	 * Get sound data by tag with lazy loading
	 */
	const FHarmoniaSoundData* GetSoundDataLazy(FGameplayTag SoundTag, bool bForceLoad = false);

	/**
	 * Check if sound exists in cache
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Sounds")
	bool HasSoundData(FGameplayTag SoundTag) const;

	/**
	 * Get all cached sound tags
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Sounds")
	TArray<FGameplayTag> GetAllSoundTags() const;

	/**
	 * Get sounds matching a parent tag (e.g., all "Sound.SFX.Hit.*")
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	TArray<FHarmoniaSoundData> GetSoundsByTag(FGameplayTag ParentTag, bool bExactMatch = false) const;

	// ============================================================================
	// Sound Playback (2D)
	// ============================================================================

	/**
	 * Play 2D sound by tag
	 * @param WorldContext - World context object
	 * @param SoundTag - Tag identifying the sound
	 * @param Context - Optional playback context for overrides
	 * @return Audio component for the playing sound, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds", meta = (WorldContext = "WorldContext"))
	UAudioComponent* PlaySound2DByTag(
		UObject* WorldContext,
		FGameplayTag SoundTag,
		FHarmoniaSoundPlaybackContext Context = FHarmoniaSoundPlaybackContext()
	);

	// ============================================================================
	// Sound Playback (3D)
	// ============================================================================

	/**
	 * Play sound at location by tag
	 * @param WorldContext - World context object
	 * @param SoundTag - Tag identifying the sound
	 * @param Location - World location to play sound at
	 * @param Rotation - World rotation
	 * @param Context - Optional playback context for overrides
	 * @return Audio component for the playing sound, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds", meta = (WorldContext = "WorldContext"))
	UAudioComponent* PlaySoundAtLocationByTag(
		UObject* WorldContext,
		FGameplayTag SoundTag,
		FVector Location,
		FRotator Rotation = FRotator::ZeroRotator,
		FHarmoniaSoundPlaybackContext Context = FHarmoniaSoundPlaybackContext()
	);

	/**
	 * Play sound attached to component by tag
	 * @param SoundTag - Tag identifying the sound
	 * @param AttachComponent - Component to attach sound to
	 * @param AttachPointName - Socket/bone name to attach to
	 * @param Location - Location offset
	 * @param Rotation - Rotation offset
	 * @param Context - Optional playback context for overrides
	 * @return Audio component for the playing sound, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	UAudioComponent* PlaySoundAttachedByTag(
		FGameplayTag SoundTag,
		USceneComponent* AttachComponent,
		FName AttachPointName = NAME_None,
		FVector Location = FVector::ZeroVector,
		FRotator Rotation = FRotator::ZeroRotator,
		FHarmoniaSoundPlaybackContext Context = FHarmoniaSoundPlaybackContext()
	);

	// ============================================================================
	// Priority-Based Sound Management
	// ============================================================================

	/**
	 * Play a managed sound with priority support
	 * Automatically handles concurrent sound limits and pause/resume
	 * @param WorldContext - World context object
	 * @param SoundTag - Tag identifying the sound
	 * @param VolumeMultiplier - Volume multiplier (default 1.0)
	 * @return Audio component for the playing sound, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds", meta = (WorldContext = "WorldContext"))
	UAudioComponent* PlayManagedSound(UObject* WorldContext, FGameplayTag SoundTag, float VolumeMultiplier = 1.0f);

	/**
	 * Stop a managed sound by tag
	 * @param SoundTag - Tag identifying the sound to stop
	 * @param FadeOutDuration - Fade out duration (0 = immediate)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	void StopManagedSound(FGameplayTag SoundTag, float FadeOutDuration = 0.0f);

	/**
	 * Stop all managed sounds
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	void StopAllManagedSounds(float FadeOutDuration = 0.0f);

	/**
	 * Get number of currently active managed sounds
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Sounds")
	int32 GetActiveSoundCount() const { return ActiveSounds.Num(); }

	/**
	 * Maximum concurrent managed sounds (configurable)
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Sounds")
	int32 MaxConcurrentSounds = 8;

	// ============================================================================
	// Cache Management
	// ============================================================================

	/**
	 * Reload all sound data from DataTables
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	void ReloadSoundCache();

	/**
	 * Clear all cached sound data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	void ClearCache();

	/**
	 * Get cache statistics
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Sounds")
	int32 GetCachedSoundCount() const { return SoundCache.Num(); }

	/**
	 * Preload all assets referenced by cached sounds
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	void PreloadSoundAssets(bool bAsync = false);

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * DataTable paths to load sound data from
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	TArray<FSoftObjectPath> SoundDataTablePaths;

	/**
	 * Whether to load all data tables at startup
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	bool bPreloadAllSounds = true;

	/**
	 * Whether to preload all referenced assets at startup
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	bool bPreloadAssets = false;

private:
	/**
	 * Load and cache sounds from all configured DataTables
	 */
	void LoadAndCacheSounds();

	/**
	 * Load sounds from a specific DataTable
	 */
	int32 LoadSoundsFromDataTable(UDataTable* DataTable);

	/**
	 * Load a single sound from DataTable by tag
	 */
	bool LoadSoundFromDataTables(FGameplayTag SoundTag);

	/**
	 * Get sound base from sound data (handles Cue, Wave, Base priority)
	 */
	USoundBase* GetSoundBaseFromData(const FHarmoniaSoundData* SoundData);

	/** Cache of all loaded sounds, indexed by tag */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FHarmoniaSoundData> SoundCache;

	/** Loaded DataTable references (kept alive) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDataTable>> LoadedDataTables;

	/** Set of tags that failed to load */
	TSet<FGameplayTag> FailedLoadTags;

	/** Ticker handle for cleanup - must be removed on deinitialize */
	FTSTicker::FDelegateHandle CleanupTickerHandle;

	/** Currently active managed sounds */
	TArray<FActiveSound> ActiveSounds;

	// ============================================================================
	// Priority Management Helpers
	// ============================================================================

	/** Clean up finished sounds from ActiveSounds */
	void CleanupFinishedSounds();

	/** Find lowest priority active sound */
	int32 FindLowestPrioritySound() const;

	/** Pause lowest priority sound to make room for new one */
	void PauseLowestPrioritySound();

	/** Resume paused sounds if there's room */
	void ResumePausedSounds();
};

