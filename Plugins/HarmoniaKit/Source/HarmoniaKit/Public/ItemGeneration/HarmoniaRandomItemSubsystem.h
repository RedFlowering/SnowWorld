// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaRandomItemSystemDefinitions.h"
#include "HarmoniaRandomItemSubsystem.generated.h"

class UHarmoniaRandomItemConfigDataAsset;

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemGenerated, const FGeneratedItemData&, GeneratedItem);

/**
 * Harmonia Random Item Subsystem
 *
 * Generates random items with affixes (prefixes/suffixes).
 * Similar to Diablo/Path of Exile style loot generation.
 *
 * Features:
 * - Affix pool management
 * - Rarity-based generation
 * - Weighted random selection
 * - Stat modifier rolling
 * - Name generation
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaRandomItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================================
	// Configuration
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	void SetConfigDataAsset(UHarmoniaRandomItemConfigDataAsset* InConfig);

	// ============================================================================
	// Affix Management
	// ============================================================================

	/** Register an affix */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	void RegisterAffix(const FHarmoniaAffixData& Affix);

	/** Unregister an affix */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	void UnregisterAffix(FName AffixID);

	/** Get affix definition */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	bool GetAffix(FName AffixID, FHarmoniaAffixData& OutAffix) const;

	/** Get all affixes */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	TArray<FHarmoniaAffixData> GetAllAffixes() const;

	/** Get affixes by type */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	TArray<FHarmoniaAffixData> GetAffixesByType(EAffixType Type) const;

	// ============================================================================
	// Item Generation
	// ============================================================================

	/** Generate a random item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	FGeneratedItemData GenerateItem(const FItemGenerationParams& Settings);

	/** Generate item with specific rarity */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	FGeneratedItemData GenerateItemWithRarity(const FItemGenerationParams& Settings, EItemRarity Rarity);

	/** Generate multiple items */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	TArray<FGeneratedItemData> GenerateItems(const FItemGenerationParams& Settings, int32 Count);

	/** Re-roll affixes on existing item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	bool RerollAffixes(UPARAM(ref) FGeneratedItemData& Item, bool bKeepRarity = true);

	/** Add random affix to item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	bool AddRandomAffix(UPARAM(ref) FGeneratedItemData& Item, EAffixType Type);

	/** Remove affix from item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|RandomItem")
	bool RemoveAffix(UPARAM(ref) FGeneratedItemData& Item, int32 AffixIndex, EAffixType Type);

	// ============================================================================
	// Rarity
	// ============================================================================

	/** Roll rarity based on weights */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	EItemRarity RollRarity(float RarityBonus = 0.0f) const;

	/** Get rarity color */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	FLinearColor GetRarityColor(EItemRarity Rarity) const;

	/** Get max affixes for rarity */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	void GetMaxAffixesForRarity(EItemRarity Rarity, int32& OutMaxPrefixes, int32& OutMaxSuffixes) const;

	/** Get affix count for rarity */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	int32 GetAffixCountForRarity(EItemRarity Rarity) const;

	// ============================================================================
	// Utility
	// ============================================================================

	/** Get item's combined modifiers */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	TArray<FAffixStatModifier> GetCombinedModifiers(const FGeneratedItemData& Item) const;

	/** Generate item name from affixes */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	FText GenerateItemName(const FGeneratedItemData& Item, const FText& BaseName) const;

	/** Get affix description text */
	UFUNCTION(BlueprintPure, Category = "Harmonia|RandomItem")
	FText GetAffixDescription(const FAppliedAffix& Affix) const;

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|RandomItem")
	FOnItemGenerated OnItemGenerated;

protected:
	/** Roll affix modifiers */
	FAppliedAffix RollAffix(const FHarmoniaAffixData& Affix) const;

	/** Get valid affixes for slot and level */
	TArray<FHarmoniaAffixData> GetValidAffixes(EAffixType Type, EAffixType Slot, 
		int32 ItemLevel, const FGameplayTagContainer& ExcludedTags) const;

	/** Select weighted random affix */
	const FHarmoniaAffixData* SelectWeightedAffix(const TArray<FHarmoniaAffixData>& ValidAffixes) const;

	/** Update item's combined modifiers */
	void UpdateCombinedModifiers(FGeneratedItemData& Item) const;

private:
	/** Registered affixes */
	UPROPERTY()
	TMap<FName, FHarmoniaAffixData> Affixes;

	/** Config data asset */
	UPROPERTY()
	TObjectPtr<UHarmoniaRandomItemConfigDataAsset> ConfigAsset;
};
