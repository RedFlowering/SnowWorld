// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaClassData.generated.h"

/**
 * Data asset containing all class definitions
 * Defines class progression, stats, abilities, and advancement paths
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaClassData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All class definitions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classes")
	TArray<FHarmoniaClassDefinition> ClassDefinitions;

	/** Awakening tier definitions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
	TArray<FHarmoniaAwakeningDefinition> AwakeningDefinitions;

	/** Prestige tier definitions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prestige")
	TArray<FHarmoniaPrestigeDefinition> PrestigeDefinitions;

	//~ Queries (C++ only - use through ProgressionComponent for Blueprint)

	/** Get class definition by type */
	const FHarmoniaClassDefinition* GetClassDefinition(EHarmoniaCharacterClass ClassType) const;

	/** Get awakening definition by tier */
	const FHarmoniaAwakeningDefinition* GetAwakeningDefinition(EHarmoniaAwakeningTier Tier) const;

	/** Get prestige definition by level */
	const FHarmoniaPrestigeDefinition* GetPrestigeDefinition(int32 PrestigeLevel) const;

	/** Get all Tier 1 (starter) classes */
	UFUNCTION(BlueprintCallable, Category = "Classes")
	TArray<EHarmoniaCharacterClass> GetStarterClasses() const;

	/** Get all advancement options for a class */
	UFUNCTION(BlueprintCallable, Category = "Classes")
	TArray<EHarmoniaCharacterClass> GetAdvancementOptions(EHarmoniaCharacterClass BaseClass) const;

	/** Check if a class is an advanced/promoted class */
	UFUNCTION(BlueprintPure, Category = "Classes")
	bool IsAdvancedClass(EHarmoniaCharacterClass ClassType) const;

	/** Get class tier (1 = base, 2 = advanced) */
	UFUNCTION(BlueprintPure, Category = "Classes")
	int32 GetClassTier(EHarmoniaCharacterClass ClassType) const;
};
