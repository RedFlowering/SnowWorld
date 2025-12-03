// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaClassData.h
 * @brief Character class data asset for class definitions, awakening, and prestige
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaClassData.generated.h"

/**
 * @class UHarmoniaClassData
 * @brief Class data asset
 * 
 * Manages character class definitions, awakening, and prestige data.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaClassData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ Class Definitions
	
	/** All class definitions list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class Data")
	TArray<FHarmoniaClassDefinition> ClassDefinitions;

	//~ Awakening Definitions
	
	/** Awakening tier definitions list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening Data")
	TArray<FHarmoniaAwakeningDefinition> AwakeningDefinitions;

	//~ Prestige Definitions
	
	/** Prestige level definitions list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige Data")
	TArray<FHarmoniaPrestigeDefinition> PrestigeDefinitions;

	//~ Utility Functions
	
	/** Find specific class definition (Blueprint) */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Class Definition")
	bool GetClassDefinitionBP(EHarmoniaCharacterClass ClassType, FHarmoniaClassDefinition& OutClassDef) const;

	/** Find specific class definition (C++) */
	const FHarmoniaClassDefinition* GetClassDefinition(EHarmoniaCharacterClass ClassType) const;

	/** Find specific awakening tier definition (Blueprint) */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Awakening Definition")
	bool GetAwakeningDefinitionBP(EHarmoniaAwakeningTier Tier, FHarmoniaAwakeningDefinition& OutAwakeningDef) const;

	/** Find specific awakening tier definition (C++) */
	const FHarmoniaAwakeningDefinition* GetAwakeningDefinition(EHarmoniaAwakeningTier Tier) const;

	/** Find specific prestige level definition (Blueprint) */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Prestige Definition")
	bool GetPrestigeDefinitionBP(int32 PrestigeLevel, FHarmoniaPrestigeDefinition& OutPrestigeDef) const;

	/** Find specific prestige level definition (C++) */
	const FHarmoniaPrestigeDefinition* GetPrestigeDefinition(int32 PrestigeLevel) const;

	/** Get starter classes list */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetStarterClasses() const;

	/** Get advancement options for specific class */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetAdvancementOptions(EHarmoniaCharacterClass BaseClass) const;

	/** Check if advancement requirements are met */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool CanAdvanceToClass(EHarmoniaCharacterClass CurrentClass, EHarmoniaCharacterClass TargetClass, int32 PlayerLevel) const;

	/** Check if class is advanced class */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool IsAdvancedClass(EHarmoniaCharacterClass ClassType) const;

	/** Get class tier */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	int32 GetClassTier(EHarmoniaCharacterClass ClassType) const;
};
