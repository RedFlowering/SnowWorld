// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaAnimationDataDefinitions.h"
#include "HarmoniaAnimationCacheSubsystem.generated.h"

/**
 * Harmonia Animation Cache Subsystem
 * Loads and caches animation data from DataTables at game startup
 *
 * Features:
 * - Fast tag-based animation lookup
 * - Centralized animation management
 * - Support for AnimSequence, AnimMontage, BlendSpace
 * - Play animations by tag with optional overrides
 * - Query animations by gameplay tags
 *
 * Usage:
 * 1. Create DataTable(s) with FHarmoniaAnimationData
 * 2. Set DataTablePaths in Project Settings or Config
 * 3. Query and play animations by tag
 *
 * Example:
 * auto* AnimCache = GetGameInstance()->GetSubsystem<UHarmoniaAnimationCacheSubsystem>();
 * AnimCache->PlayAnimationByTag(Character, FGameplayTag::RequestGameplayTag("Anim.Player.Attack.Sword.Light"));
 */
UCLASS(Config=Game)
class HARMONIAKIT_API UHarmoniaAnimationCacheSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	// ============================================================================
	// Animation Data Access
	// ============================================================================

	/**
	 * Get animation data by tag
	 * @param AnimationTag - Gameplay tag identifying the animation
	 * @return Pointer to cached animation data, or nullptr if not found
	 * NOTE: C++ only - use GetAnimationsByTag for Blueprint
	 */
	const FHarmoniaAnimationData* GetAnimationData(FGameplayTag AnimationTag) const;

	/**
	 * Get animation data by tag with lazy loading
	 * @param AnimationTag - Gameplay tag identifying the animation
	 * @param bForceLoad - If true, attempts to load from DataTable if not cached
	 * @return Pointer to cached animation data, or nullptr if not found
	 */
	const FHarmoniaAnimationData* GetAnimationDataLazy(FGameplayTag AnimationTag, bool bForceLoad = false);

	/**
	 * Check if animation exists in cache
	 * @param AnimationTag - Gameplay tag to check
	 * @return True if animation is cached
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Animations")
	bool HasAnimationData(FGameplayTag AnimationTag) const;

	/**
	 * Get all cached animation tags
	 * @return Array of all cached animation tags
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Animations")
	TArray<FGameplayTag> GetAllAnimationTags() const;

	/**
	 * Get animations matching a parent tag (e.g., all "Anim.Player.Attack.*")
	 * @param ParentTag - Parent tag to match
	 * @param bExactMatch - If true, requires exact match; if false, matches children
	 * @return Array of matching animation data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	TArray<FHarmoniaAnimationData> GetAnimationsByTag(FGameplayTag ParentTag, bool bExactMatch = false) const;

	/**
	 * Get animations by gameplay tags
	 * @param GameplayTags - Tags to match against animation's GameplayTags field
	 * @param bMatchAll - If true, must match all tags; if false, match any
	 * @return Array of matching animation data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	TArray<FHarmoniaAnimationData> GetAnimationsByGameplayTags(FGameplayTagContainer GameplayTags, bool bMatchAll = false) const;

	// ============================================================================
	// Animation Playback
	// ============================================================================

	/**
	 * Play animation montage by tag
	 * @param Character - Character to play animation on
	 * @param AnimationTag - Tag identifying the animation
	 * @param Context - Optional playback context for overrides
	 * @return Duration of the animation, or 0 if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	float PlayAnimationByTag(
		ACharacter* Character,
		FGameplayTag AnimationTag,
		FHarmoniaAnimationPlaybackContext Context = FHarmoniaAnimationPlaybackContext()
	);

	/**
	 * Stop animation montage by tag
	 * @param Character - Character to stop animation on
	 * @param AnimationTag - Tag identifying the animation
	 * @param BlendOutTime - Time to blend out (negative = use default)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	void StopAnimationByTag(
		ACharacter* Character,
		FGameplayTag AnimationTag,
		float BlendOutTime = -1.0f
	);

	/**
	 * Stop all animations in a tag category
	 * @param Character - Character to stop animations on
	 * @param ParentTag - Parent tag (e.g., "Anim.Player.Attack")
	 * @param BlendOutTime - Time to blend out
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	void StopAnimationsByParentTag(
		ACharacter* Character,
		FGameplayTag ParentTag,
		float BlendOutTime = 0.25f
	);

	// ============================================================================
	// Cache Management
	// ============================================================================

	/**
	 * Reload all animation data from DataTables
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	void ReloadAnimationCache();

	/**
	 * Clear all cached animation data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	void ClearCache();

	/**
	 * Get cache statistics
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Animations")
	int32 GetCachedAnimationCount() const { return AnimationCache.Num(); }

	/**
	 * Preload all assets referenced by cached animations
	 * @param bAsync - If true, loads asynchronously
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Animations")
	void PreloadAnimationAssets(bool bAsync = false);

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * DataTable paths to load animation data from
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	TArray<FSoftObjectPath> AnimationDataTablePaths;

	/**
	 * Whether to load all data tables at startup
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	bool bPreloadAllAnimations = true;

	/**
	 * Whether to preload all referenced assets at startup
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	bool bPreloadAssets = false;

private:
	/**
	 * Load and cache animations from all configured DataTables
	 */
	void LoadAndCacheAnimations();

	/**
	 * Load animations from a specific DataTable
	 */
	int32 LoadAnimationsFromDataTable(UDataTable* DataTable);

	/**
	 * Load a single animation from DataTable by tag
	 */
	bool LoadAnimationFromDataTables(FGameplayTag AnimationTag);

	/** Cache of all loaded animations, indexed by tag */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FHarmoniaAnimationData> AnimationCache;

	/** Loaded DataTable references (kept alive) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDataTable>> LoadedDataTables;

	/** Set of tags that failed to load */
	TSet<FGameplayTag> FailedLoadTags;
};
