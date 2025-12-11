// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaRowIncludes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaLoadManager.h"

#include "HarmoniaDataTableBFL.generated.h"

// This file is auto-generated. Do not modify manually.
// Any manual changes will be overwritten by the code generator.

UCLASS()
class HARMONIALOADMANAGER_API UHarmoniaDataTableBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetItemDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetItemStatDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetEquipmentDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetSpellDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetComboAttackDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetComboAttackDataDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetHitReactionDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCraftingRecipeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCraftingStationDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCraftingCategoryDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetItemGradeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetItemGradeConfigDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCookingRecipeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetEnhancementLevelDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetGemDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetReforgeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetTranscendenceDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetRepairDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetRepairKitDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetEnchantmentDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetRarityWeightDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetAffixDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetItemTemplateDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetShopDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetShopItemDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCurrencyDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetQuestDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetGatheringResourceDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetFishDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCropDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetLootTableDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetMountDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetClimbingDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetParkourDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetSwimmingDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetWaypointDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetAchievementDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetTitleDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetLeaderboardDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCosmeticDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCollectionDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetSkillNodeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetSkillTreeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetBuildingPartsDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetWeaponUpgradeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetAnimationDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetAnimationEffectDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetSoundDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetMusicSheetDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetGameplayTagsDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetAttributeDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetLocalizationDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetModConfigDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetBuffDataTable();

};
