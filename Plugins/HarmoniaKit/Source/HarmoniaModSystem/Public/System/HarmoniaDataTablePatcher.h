// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaModSystemDefinitions.h"
#include "Engine/DataTable.h"
#include "HarmoniaDataTablePatcher.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaDataTablePatcher, Log, All);

/**
 * Wrapper struct for TArray<FString> to be used as TMap value
 */
USTRUCT()
struct FHarmoniaPatchKeyArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> PatchKeys;
};

/**
 * Patch operation result
 */
USTRUCT(BlueprintType)
struct FHarmoniaPatchResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly)
	FString OldValue;

	UPROPERTY(BlueprintReadOnly)
	FString NewValue;
};

/**
 * Data table patcher subsystem
 * Handles runtime modification of data tables (item stats, monster abilities, etc.)
 */
UCLASS()
class HARMONIAMODSYSTEM_API UHarmoniaDataTablePatcher : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	/**
	 * Apply a data table patch
	 * @param Patch - Patch information
	 * @param ModId - ID of the mod applying this patch
	 * @param OutResult - Result of the patch operation
	 * @return True if patch applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Data Table Patcher")
	bool ApplyPatch(const FHarmoniaDataTablePatch& Patch, FName ModId, FHarmoniaPatchResult& OutResult);

	/**
	 * Apply multiple patches
	 * @param Patches - Array of patches to apply
	 * @param ModId - ID of the mod applying these patches
	 * @return Number of patches successfully applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Data Table Patcher")
	int32 ApplyPatches(const TArray<FHarmoniaDataTablePatch>& Patches, FName ModId);

	/**
	 * Revert a specific patch
	 * @param Patch - Patch to revert
	 * @param ModId - ID of the mod that applied this patch
	 * @return True if patch reverted successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Data Table Patcher")
	bool RevertPatch(const FHarmoniaDataTablePatch& Patch, FName ModId);

	/**
	 * Revert all patches from a mod
	 * @param ModId - ID of the mod
	 * @return Number of patches reverted
	 */
	UFUNCTION(BlueprintCallable, Category = "Data Table Patcher")
	int32 RevertAllModPatches(FName ModId);

	/**
	 * Revert all patches
	 */
	UFUNCTION(BlueprintCallable, Category = "Data Table Patcher")
	void RevertAllPatches();

	/**
	 * Get original value before patch
	 * @param TablePath - Data table path
	 * @param RowName - Row name
	 * @param PropertyName - Property name
	 * @param OutOriginalValue - Original value (as JSON string)
	 * @return True if original value found
	 */
	UFUNCTION(BlueprintPure, Category = "Data Table Patcher")
	bool GetOriginalValue(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName, FString& OutOriginalValue) const;

	/**
	 * Check if a value has been patched
	 * @param TablePath - Data table path
	 * @param RowName - Row name
	 * @param PropertyName - Property name
	 * @return True if value has been patched
	 */
	UFUNCTION(BlueprintPure, Category = "Data Table Patcher")
	bool IsValuePatched(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName) const;

	/**
	 * Get all active patches
	 * @return Map of patch keys to patches
	 */
	UFUNCTION(BlueprintPure, Category = "Data Table Patcher")
	TMap<FString, FHarmoniaDataTablePatch> GetAllActivePatches() const;

	/**
	 * Get patches from a specific mod
	 * @param ModId - ID of the mod
	 * @return Array of patches from this mod
	 */
	UFUNCTION(BlueprintPure, Category = "Data Table Patcher")
	TArray<FHarmoniaDataTablePatch> GetModPatches(FName ModId) const;

protected:
	/**
	 * Validate a patch
	 * @param Patch - Patch to validate
	 * @param OutErrorMessage - Error message if validation fails
	 * @return True if patch is valid
	 */
	bool ValidatePatch(const FHarmoniaDataTablePatch& Patch, FString& OutErrorMessage) const;

	/**
	 * Apply patch operation to a property
	 * @param Property - Property to modify
	 * @param DataPtr - Pointer to the data
	 * @param Patch - Patch information
	 * @param OutResult - Result of the operation
	 * @return True if operation succeeded
	 */
	bool ApplyPatchOperation(FProperty* Property, void* DataPtr, const FHarmoniaDataTablePatch& Patch, FHarmoniaPatchResult& OutResult);

	/**
	 * Get property value as string
	 * @param Property - Property to read
	 * @param DataPtr - Pointer to the data
	 * @return String representation of the value
	 */
	FString GetPropertyValueAsString(FProperty* Property, const void* DataPtr) const;

	/**
	 * Set property value from string
	 * @param Property - Property to write
	 * @param DataPtr - Pointer to the data
	 * @param Value - String value to set
	 * @return True if value set successfully
	 */
	bool SetPropertyValueFromString(FProperty* Property, void* DataPtr, const FString& Value) const;

	/**
	 * Generate patch key
	 * @param TablePath - Data table path
	 * @param RowName - Row name
	 * @param PropertyName - Property name
	 * @return Unique key for this patch
	 */
	FString GeneratePatchKey(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName) const;

private:
	/** Backup of original values (PatchKey -> OriginalValue) */
	UPROPERTY()
	TMap<FString, FString> OriginalValues;

	/** Active patches (PatchKey -> Patch) */
	UPROPERTY()
	TMap<FString, FHarmoniaDataTablePatch> ActivePatches;

	/** Patches by mod (ModId -> PatchKeys) */
	UPROPERTY()
	TMap<FName, FHarmoniaPatchKeyArray> PatchesByMod;

	/** Loaded data table cache */
	UPROPERTY()
	TMap<FSoftObjectPath, TObjectPtr<UDataTable>> DataTableCache;
};
