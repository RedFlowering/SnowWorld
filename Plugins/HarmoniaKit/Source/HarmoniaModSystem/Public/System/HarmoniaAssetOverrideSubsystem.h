// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaModSystemDefinitions.h"
#include "HarmoniaAssetOverrideSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaAssetOverride, Log, All);

/**
 * Wrapper struct for TArray<FSoftObjectPath> to be used as TMap value
 */
USTRUCT()
struct FHarmoniaSoftObjectPathArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSoftObjectPath> Paths;
};

/**
 * Asset override subsystem
 * Handles runtime asset replacement (models, textures, sounds, etc.)
 */
UCLASS()
class HARMONIAMODSYSTEM_API UHarmoniaAssetOverrideSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	/**
	 * Register an asset override
	 * @param Override - Asset override information
	 * @param ModId - ID of the mod registering this override
	 * @return True if override registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Asset Override")
	bool RegisterAssetOverride(const FHarmoniaAssetOverride& Override, FName ModId);

	/**
	 * Unregister an asset override
	 * @param OriginalAssetPath - Path to the original asset
	 * @param ModId - ID of the mod that registered the override
	 * @return True if override unregistered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Asset Override")
	bool UnregisterAssetOverride(const FSoftObjectPath& OriginalAssetPath, FName ModId);

	/**
	 * Unregister all overrides from a specific mod
	 * @param ModId - ID of the mod
	 */
	UFUNCTION(BlueprintCallable, Category = "Asset Override")
	void UnregisterAllModOverrides(FName ModId);

	/**
	 * Get the overridden asset path (if any)
	 * @param OriginalAssetPath - Path to the original asset
	 * @param OutOverridePath - Output overridden asset path
	 * @return True if an override exists
	 */
	UFUNCTION(BlueprintPure, Category = "Asset Override")
	bool GetOverriddenAssetPath(const FSoftObjectPath& OriginalAssetPath, FSoftObjectPath& OutOverridePath) const;

	/**
	 * Load an asset with override support
	 * @param AssetPath - Path to the asset (will be overridden if applicable)
	 * @return Loaded asset object (or nullptr if failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Asset Override")
	UObject* LoadAssetWithOverride(const FSoftObjectPath& AssetPath);

	/**
	 * Check if an asset has an override
	 * @param AssetPath - Path to the asset
	 * @return True if asset has an override
	 */
	UFUNCTION(BlueprintPure, Category = "Asset Override")
	bool HasOverride(const FSoftObjectPath& AssetPath) const;

	/**
	 * Get all active overrides
	 * @return Map of original paths to override info
	 */
	UFUNCTION(BlueprintPure, Category = "Asset Override")
	TMap<FSoftObjectPath, FHarmoniaAssetOverride> GetAllOverrides() const;

	/**
	 * Clear all overrides
	 */
	UFUNCTION(BlueprintCallable, Category = "Asset Override")
	void ClearAllOverrides();

	/**
	 * Apply all overrides from a mod
	 * @param ModId - ID of the mod
	 * @param Overrides - Array of overrides to apply
	 * @return Number of overrides successfully applied
	 */
	int32 ApplyModOverrides(FName ModId, const TArray<FHarmoniaAssetOverride>& Overrides);

	/**
	 * Revert all overrides from a mod
	 * @param ModId - ID of the mod
	 */
	void RevertModOverrides(FName ModId);

protected:
	/**
	 * Validate an asset override
	 * @param Override - Override to validate
	 * @return True if override is valid
	 */
	bool ValidateOverride(const FHarmoniaAssetOverride& Override) const;

	/**
	 * Preload override asset
	 * @param Override - Override containing the asset path
	 * @return True if asset loaded successfully
	 */
	bool PreloadOverrideAsset(const FHarmoniaAssetOverride& Override);

private:
	/** Active asset overrides (OriginalPath -> Override) */
	UPROPERTY()
	TMap<FSoftObjectPath, FHarmoniaAssetOverride> ActiveOverrides;

	/** Overrides by mod (ModId -> OriginalPaths) */
	UPROPERTY()
	TMap<FName, FHarmoniaSoftObjectPathArray> OverridesByMod;

	/** Original asset cache for restoration */
	UPROPERTY()
	TMap<FSoftObjectPath, TObjectPtr<UObject>> OriginalAssetCache;

	/** Override asset cache */
	UPROPERTY()
	TMap<FSoftObjectPath, TObjectPtr<UObject>> OverrideAssetCache;
};
