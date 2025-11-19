// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaSoundDataDefinitions.h"
#include "HarmoniaSoundCacheSubsystem.generated.h"

/**
 * Harmonia Sound Cache Subsystem
 * Loads and caches sound data from DataTables at game startup
 *
 * Features:
 * - Fast tag-based sound lookup
 * - Centralized sound management
 * - Support for SoundCue, SoundWave, SoundBase
 * - Play sounds by tag with optional overrides
 * - Concurrency management
 * - Spatial sound support
 *
 * Usage:
 * 1. Create DataTable(s) with FHarmoniaSoundData
 * 2. Set DataTablePaths in Project Settings or Config
 * 3. Query and play sounds by tag
 *
 * Example:
 * auto* SoundCache = GetGameInstance()->GetSubsystem<UHarmoniaSoundCacheSubsystem>();
 * SoundCache->PlaySoundByTag(GetWorld(), FGameplayTag::RequestGameplayTag("Sound.SFX.Hit.Metal"), GetActorLocation());
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

	/**
	 * Get sounds by gameplay tags
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	TArray<FHarmoniaSoundData> GetSoundsByGameplayTags(FGameplayTagContainer GameplayTags, bool bMatchAll = false) const;

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
	// Sound Control
	// ============================================================================

	/**
	 * Stop all sounds with a specific tag
	 * @param SoundTag - Tag identifying the sounds to stop
	 * @param FadeOutDuration - Time to fade out (0 = immediate stop)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	void StopSoundsByTag(FGameplayTag SoundTag, float FadeOutDuration = 0.0f);

	/**
	 * Stop all sounds matching a parent tag
	 * @param ParentTag - Parent tag (e.g., "Sound.SFX.Weapon")
	 * @param FadeOutDuration - Time to fade out
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Sounds")
	void StopSoundsByParentTag(FGameplayTag ParentTag, float FadeOutDuration = 0.0f);

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

	/**
	 * Apply randomization to volume and pitch
	 */
	void ApplyRandomization(const FHarmoniaSoundData* SoundData, float& OutVolume, float& OutPitch) const;

	/**
	 * Check concurrency limits
	 */
	bool CheckConcurrency(FGameplayTag SoundTag, const FHarmoniaSoundData* SoundData);

	/**
	 * Register playing sound for concurrency tracking
	 */
	void RegisterPlayingSound(FGameplayTag SoundTag, UAudioComponent* AudioComponent);

	/**
	 * Clean up finished audio components
	 */
	void CleanupFinishedSounds();

	/** Cache of all loaded sounds, indexed by tag */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FHarmoniaSoundData> SoundCache;

	/** Loaded DataTable references (kept alive) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDataTable>> LoadedDataTables;

	/** Set of tags that failed to load */
	TSet<FGameplayTag> FailedLoadTags;

	/** Currently playing sounds for concurrency management (not exposed to Blueprint) */
	TMap<FGameplayTag, TArray<TWeakObjectPtr<UAudioComponent>>> PlayingSounds;

	/** Last play time for each sound tag (for MinTimeBetweenPlays) */
	TMap<FGameplayTag, double> LastPlayTimes;
};
