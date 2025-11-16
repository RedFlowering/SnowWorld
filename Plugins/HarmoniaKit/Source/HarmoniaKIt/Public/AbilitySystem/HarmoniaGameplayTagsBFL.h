// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystem/Definitions/HarmoniaGameplayTagsDefinitions.h"
#include "HarmoniaGameplayTagsBFL.generated.h"

/**
 * Blueprint Function Library for accessing Harmonia Gameplay Tags DataTables
 * Provides convenient access to gameplay tags and combo attack configurations
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayTagsBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the Gameplay Tags DataTable
	 * @return The DataTable containing gameplay tag configurations
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameplayTags")
	static UDataTable* GetGameplayTagsDataTable();

	/**
	 * Get the Combo Attack DataTable
	 * @return The DataTable containing combo attack configurations
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameplayTags")
	static UDataTable* GetComboAttackDataTable();

	/**
	 * Get gameplay tag data by tag ID
	 * @param TagId The unique identifier for the tag
	 * @param OutTagData The retrieved tag data
	 * @return True if tag was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameplayTags")
	static bool GetGameplayTagData(FName TagId, FHarmoniaGameplayTagData& OutTagData);

	/**
	 * Get combo attack data by combo index
	 * @param ComboIndex The index of the combo attack (0, 1, 2, etc.)
	 * @param OutComboData The retrieved combo data
	 * @return True if combo data was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameplayTags")
	static bool GetComboAttackData(int32 ComboIndex, FHarmoniaComboAttackData& OutComboData);

	/**
	 * Get all combo attack data sorted by combo index
	 * @param OutComboDataArray Array of all combo attack configurations
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameplayTags")
	static void GetAllComboAttackData(TArray<FHarmoniaComboAttackData>& OutComboDataArray);

	/**
	 * Get the maximum combo index available
	 * @return The highest combo index in the combo attack data table
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameplayTags")
	static int32 GetMaxComboIndex();

private:
	// Cached references to DataTables
	static UDataTable* CachedGameplayTagsDataTable;
	static UDataTable* CachedComboAttackDataTable;

	// Helper function to load DataTables
	static UDataTable* LoadDataTable(const FString& AssetPath, UDataTable*& CachedDataTable);
};
