// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaAnimationEffectDefinitions.h"
#include "HarmoniaEffectCacheSubsystem.generated.h"

/**
 * Harmonia Effect Cache Subsystem
 * Loads and caches animation effect data from DataTables at game startup
 *
 * Features:
 * - Fast tag-based lookup (O(log n) or O(1))
 * - Lazy loading support
 * - Memory efficient (single copy per effect)
 * - Automatic asset loading
 *
 * Usage:
 * 1. Create DataTable(s) with FHarmoniaAnimationEffectData
 * 2. Set DataTablePaths in Project Settings or Config
 * 3. Query effects by tag in AnimNotifies
 *
 * Example:
 * auto* Subsystem = GetGameInstance()->GetSubsystem<UHarmoniaEffectCacheSubsystem>();
 * if (const FHarmoniaAnimationEffectData* Data = Subsystem->GetEffectData(MyTag))
 * {
 *     // Use cached effect data
 * }
 */
UCLASS(Config=Game)
class HARMONIAKIT_API UHarmoniaEffectCacheSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	// ============================================================================
	// Effect Data Access
	// ============================================================================

	/**
	 * Get effect data by tag (const version)
	 * @param EffectTag - Gameplay tag identifying the effect
	 * @return Pointer to cached effect data, or nullptr if not found
	 * NOTE: C++ only - use GetEffectsByTag for Blueprint
	 */
	const FHarmoniaAnimationEffectData* GetEffectData(FGameplayTag EffectTag) const;

	/**
	 * Get effect data by tag with lazy loading
	 * @param EffectTag - Gameplay tag identifying the effect
	 * @param bForceLoad - If true, attempts to load from DataTable if not cached
	 * @return Pointer to cached effect data, or nullptr if not found
	 */
	const FHarmoniaAnimationEffectData* GetEffectDataLazy(FGameplayTag EffectTag, bool bForceLoad = false);

	/**
	 * Check if effect exists in cache
	 * @param EffectTag - Gameplay tag to check
	 * @return True if effect is cached
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Effects")
	bool HasEffectData(FGameplayTag EffectTag) const;

	/**
	 * Get all cached effect tags
	 * @return Array of all cached effect tags
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Effects")
	TArray<FGameplayTag> GetAllEffectTags() const;

	/**
	 * Get effects matching a parent tag (e.g., all "Effect.Hit.*")
	 * @param ParentTag - Parent tag to match
	 * @param bExactMatch - If true, requires exact match; if false, matches children
	 * @return Array of matching effect data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Effects")
	TArray<FHarmoniaAnimationEffectData> GetEffectsByTag(FGameplayTag ParentTag, bool bExactMatch = false) const;

	// ============================================================================
	// Cache Management
	// ============================================================================

	/**
	 * Reload all effect data from DataTables
	 * Useful for hot-reloading in editor
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Effects")
	void ReloadEffectCache();

	/**
	 * Clear all cached effect data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Effects")
	void ClearCache();

	/**
	 * Get cache statistics
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Effects")
	int32 GetCachedEffectCount() const { return EffectCache.Num(); }

	/**
	 * Preload all assets referenced by cached effects
	 * @param bAsync - If true, loads asynchronously
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Effects")
	void PreloadEffectAssets(bool bAsync = false);

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * DataTable paths to load effect data from
	 * Can be set in DefaultGame.ini under [/Script/HarmoniaKit.HarmoniaEffectCacheSubsystem]
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	TArray<FSoftObjectPath> EffectDataTablePaths;

	/**
	 * Whether to load all data tables at startup
	 * If false, uses lazy loading
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	bool bPreloadAllEffects = true;

	/**
	 * Whether to preload all referenced assets at startup
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
	bool bPreloadAssets = false;

	// ============================================================================
	// Internal
	// ============================================================================

private:
	/**
	 * Load and cache effects from all configured DataTables
	 */
	void LoadAndCacheEffects();

	/**
	 * Load effects from a specific DataTable
	 * @param DataTable - DataTable to load from
	 * @return Number of effects loaded
	 */
	int32 LoadEffectsFromDataTable(UDataTable* DataTable);

	/**
	 * Load a single effect from DataTable by tag
	 * @param EffectTag - Tag to search for
	 * @return True if effect was found and loaded
	 */
	bool LoadEffectFromDataTables(FGameplayTag EffectTag);

	/** Cache of all loaded effects, indexed by tag */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FHarmoniaAnimationEffectData> EffectCache;

	/** Loaded DataTable references (kept alive) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDataTable>> LoadedDataTables;

	/** Set of tags that failed to load (to avoid repeated warnings) */
	TSet<FGameplayTag> FailedLoadTags;
};
