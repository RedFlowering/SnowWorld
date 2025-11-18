// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaModSystemDefinitions.h"
#include "HarmoniaModSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaModSystem, Log, All);

/**
 * Delegate fired when a mod is loaded
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModLoaded, const FHarmoniaModInfo&, ModInfo);

/**
 * Delegate fired when a mod fails to load
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModLoadFailed, const FHarmoniaModInfo&, ModInfo, const FString&, ErrorMessage);

/**
 * Delegate fired when mods are reloaded (hot-reload)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModsReloaded);

/**
 * Delegate fired when a conflict is detected
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModConflictDetected, const FHarmoniaModConflict&, Conflict);

/**
 * Main mod system subsystem
 * Handles mod discovery, loading, dependency resolution, and lifecycle management
 */
UCLASS(Config=Game)
class HARMONIAMODSYSTEM_API UHarmoniaModSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	// ========================================
	// Configuration
	// ========================================

	/** Directories to search for mods */
	UPROPERTY(Config, EditAnywhere, Category = "Mod System|Configuration")
	TArray<FString> ModSearchPaths;

	/** Auto-load mods on startup? */
	UPROPERTY(Config, EditAnywhere, Category = "Mod System|Configuration")
	bool bAutoLoadModsOnStartup;

	/** Enable hot-reload support? */
	UPROPERTY(Config, EditAnywhere, Category = "Mod System|Configuration")
	bool bEnableHotReload;

	/** Enable conflict detection? */
	UPROPERTY(Config, EditAnywhere, Category = "Mod System|Configuration")
	bool bEnableConflictDetection;

	/** Enable Steam Workshop integration? */
	UPROPERTY(Config, EditAnywhere, Category = "Mod System|Configuration")
	bool bEnableSteamWorkshop;

	/** Fail load if critical conflicts detected? */
	UPROPERTY(Config, EditAnywhere, Category = "Mod System|Configuration")
	bool bFailOnCriticalConflicts;

	/** Maximum number of mods to load */
	UPROPERTY(Config, EditAnywhere, Category = "Mod System|Configuration")
	int32 MaxModCount;

	// ========================================
	// Mod Discovery and Loading
	// ========================================

	/**
	 * Discover all available mods in configured search paths
	 * @return Number of mods discovered
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	int32 DiscoverMods();

	/**
	 * Load a specific mod by ID
	 * @param ModId - Unique identifier of the mod to load
	 * @param OutErrorMessage - Error message if loading fails
	 * @return True if mod loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	bool LoadMod(FName ModId, FString& OutErrorMessage);

	/**
	 * Load all discovered mods (respecting dependencies and priority)
	 * @param OutFailedMods - List of mods that failed to load
	 * @return Number of mods successfully loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	int32 LoadAllMods(TArray<FName>& OutFailedMods);

	/**
	 * Unload a specific mod
	 * @param ModId - Unique identifier of the mod to unload
	 * @return True if mod unloaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	bool UnloadMod(FName ModId);

	/**
	 * Unload all loaded mods
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	void UnloadAllMods();

	/**
	 * Reload all mods (hot-reload)
	 * @return Number of mods successfully reloaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	int32 ReloadAllMods();

	/**
	 * Reload a specific mod (hot-reload)
	 * @param ModId - Unique identifier of the mod to reload
	 * @return True if mod reloaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	bool ReloadMod(FName ModId);

	// ========================================
	// Mod Queries
	// ========================================

	/**
	 * Get information about a specific mod
	 * @param ModId - Unique identifier of the mod
	 * @param OutModInfo - Output mod information
	 * @return True if mod exists
	 */
	UFUNCTION(BlueprintPure, Category = "Mod System")
	bool GetModInfo(FName ModId, FHarmoniaModInfo& OutModInfo) const;

	/**
	 * Get all discovered mods
	 * @return Array of all mod information
	 */
	UFUNCTION(BlueprintPure, Category = "Mod System")
	TArray<FHarmoniaModInfo> GetAllMods() const;

	/**
	 * Get all loaded mods
	 * @return Array of loaded mod information
	 */
	UFUNCTION(BlueprintPure, Category = "Mod System")
	TArray<FHarmoniaModInfo> GetLoadedMods() const;

	/**
	 * Check if a mod is loaded
	 * @param ModId - Unique identifier of the mod
	 * @return True if mod is loaded
	 */
	UFUNCTION(BlueprintPure, Category = "Mod System")
	bool IsModLoaded(FName ModId) const;

	/**
	 * Get mods by tag
	 * @param Tag - Gameplay tag to filter by
	 * @return Array of mods with the specified tag
	 */
	UFUNCTION(BlueprintPure, Category = "Mod System")
	TArray<FHarmoniaModInfo> GetModsByTag(FGameplayTag Tag) const;

	/**
	 * Get total number of loaded mods
	 * @return Number of loaded mods
	 */
	UFUNCTION(BlueprintPure, Category = "Mod System")
	int32 GetLoadedModCount() const;

	// ========================================
	// Dependency and Conflict Management
	// ========================================

	/**
	 * Check dependencies for a mod
	 * @param ModId - Unique identifier of the mod
	 * @param OutMissingDependencies - Output list of missing dependencies
	 * @return True if all dependencies are satisfied
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	bool CheckDependencies(FName ModId, TArray<FHarmoniaModDependency>& OutMissingDependencies) const;

	/**
	 * Detect conflicts between loaded mods
	 * @return Array of detected conflicts
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	TArray<FHarmoniaModConflict> DetectConflicts() const;

	/**
	 * Check if two mods are incompatible
	 * @param ModA - First mod ID
	 * @param ModB - Second mod ID
	 * @return True if mods are incompatible
	 */
	UFUNCTION(BlueprintPure, Category = "Mod System")
	bool AreModsIncompatible(FName ModA, FName ModB) const;

	/**
	 * Get load order for all mods (sorted by priority and dependencies)
	 * @return Array of mod IDs in load order
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod System")
	TArray<FName> GetLoadOrder() const;

	// ========================================
	// Events
	// ========================================

	/** Event fired when a mod is successfully loaded */
	UPROPERTY(BlueprintAssignable, Category = "Mod System|Events")
	FOnModLoaded OnModLoaded;

	/** Event fired when a mod fails to load */
	UPROPERTY(BlueprintAssignable, Category = "Mod System|Events")
	FOnModLoadFailed OnModLoadFailed;

	/** Event fired when mods are reloaded */
	UPROPERTY(BlueprintAssignable, Category = "Mod System|Events")
	FOnModsReloaded OnModsReloaded;

	/** Event fired when a conflict is detected */
	UPROPERTY(BlueprintAssignable, Category = "Mod System|Events")
	FOnModConflictDetected OnModConflictDetected;

protected:
	/**
	 * Load mod manifest file (ModInfo.json)
	 * @param ManifestPath - Path to the manifest file
	 * @param OutModInfo - Output mod information
	 * @return True if manifest loaded successfully
	 */
	bool LoadModManifest(const FString& ManifestPath, FHarmoniaModInfo& OutModInfo);

	/**
	 * Validate mod against game version and dependencies
	 * @param ModInfo - Mod information to validate
	 * @param OutErrorMessage - Error message if validation fails
	 * @return True if mod is valid
	 */
	bool ValidateMod(const FHarmoniaModInfo& ModInfo, FString& OutErrorMessage) const;

	/**
	 * Sort mods by load priority and dependencies
	 * @param Mods - Array of mods to sort (modified in place)
	 */
	void SortModsByPriority(TArray<FHarmoniaModInfo*>& Mods);

	/**
	 * Apply mod content (assets, data tables, etc.)
	 * @param ModInfo - Mod to apply
	 * @return True if content applied successfully
	 */
	bool ApplyModContent(FHarmoniaModInfo& ModInfo);

	/**
	 * Unapply mod content (restore original assets)
	 * @param ModInfo - Mod to unapply
	 */
	void UnapplyModContent(FHarmoniaModInfo& ModInfo);

private:
	/** All discovered mods (ModId -> ModInfo) */
	UPROPERTY()
	TMap<FName, FHarmoniaModInfo> DiscoveredMods;

	/** Currently loaded mods (ModId -> ModInfo) */
	UPROPERTY()
	TMap<FName, FHarmoniaModInfo> LoadedMods;

	/** Detected conflicts */
	UPROPERTY()
	TArray<FHarmoniaModConflict> DetectedConflicts;

	/** Is system initialized? */
	bool bIsInitialized;
};
