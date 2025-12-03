// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaProgressionLibrary.generated.h"

class UHarmoniaProgressionComponent;
class AActor;
class APlayerState;

/**
 * Blueprint function library for progression system
 * Provides convenient access to progression features from Blueprints
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaProgressionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//~ Component Access

	/** Get progression component from an actor */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaProgressionComponent* GetProgressionComponent(AActor* Actor);

	/** Get progression component from player state */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression")
	static UHarmoniaProgressionComponent* GetProgressionComponentFromPlayerState(APlayerState* PlayerState);

	/** Get progression component from local player */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaProgressionComponent* GetLocalPlayerProgressionComponent(UObject* WorldContextObject);

	//~ Experience & Leveling Helpers

	/** Calculate experience needed for a level range */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Experience")
	static int32 CalculateTotalExperienceForLevelRange(int32 StartLevel, int32 EndLevel);

	/** Get level from total experience */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Experience")
	static int32 GetLevelFromTotalExperience(int32 TotalExperience);

	/** Format experience as text (e.g., "1,234 / 5,000") */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Experience")
	static FText FormatExperienceText(int32 CurrentExp, int32 RequiredExp);

	/** Get experience reward for enemy level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Experience")
	static int32 CalculateEnemyExperienceReward(int32 EnemyLevel, int32 PlayerLevel, float BaseReward = 100.0f);

	//~ Class System Helpers

	/** Get class name as text */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Class")
	static FText GetClassName(EHarmoniaCharacterClass ClassType);

	/** Get class description as text */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Class")
	static FText GetClassDescription(EHarmoniaCharacterClass ClassType);

	/** Get class icon color */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Class")
	static FLinearColor GetClassColor(EHarmoniaCharacterClass ClassType);

	/** Check if class is a melee class */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Class")
	static bool IsMeleeClass(EHarmoniaCharacterClass ClassType);

	/** Check if class is a ranged class */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Class")
	static bool IsRangedClass(EHarmoniaCharacterClass ClassType);

	/** Check if class is a magic class */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Class")
	static bool IsMagicClass(EHarmoniaCharacterClass ClassType);

	//~ Awakening System Helpers

	/** Get awakening tier name */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Awakening")
	static FText GetAwakeningTierName(EHarmoniaAwakeningTier Tier);

	/** Get awakening tier color */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Awakening")
	static FLinearColor GetAwakeningTierColor(EHarmoniaAwakeningTier Tier);

	/** Calculate stat multiplier for awakening tier */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Awakening")
	static float GetAwakeningStatMultiplier(EHarmoniaAwakeningTier Tier);

	//~ Skill Tree Helpers

	/** Get skill tree category name */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|SkillTree")
	static FText GetSkillTreeCategoryName(EHarmoniaSkillTreeCategory Category);

	/** Get skill tree category icon color */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|SkillTree")
	static FLinearColor GetSkillTreeCategoryColor(EHarmoniaSkillTreeCategory Category);

	//~ Prestige Helpers

	/** Format prestige level as text (e.g., "NG+3") */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Prestige")
	static FText FormatPrestigeLevelText(int32 PrestigeLevel);

	/** Get prestige rank name (e.g., "Master", "Grandmaster") */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Prestige")
	static FText GetPrestigeRankName(int32 PrestigeLevel);

	/** Get prestige color */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Prestige")
	static FLinearColor GetPrestigeColor(int32 PrestigeLevel);

	//~ Stat Calculation Helpers

	/** Calculate primary stat contribution to combat stats */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Stats")
	static float CalculateStatContribution(FGameplayTag StatTag, int32 StatValue);

	/** Get recommended stats for a class */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Stats")
	static TMap<FGameplayTag, int32> GetRecommendedStatsForClass(EHarmoniaCharacterClass ClassType, int32 Level);

	//~ Progression Validation

	/** Validate skill tree prerequisites */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Validation")
	static bool ValidateSkillNodePrerequisites(const FHarmoniaSkillNode& Node, const TArray<FSkillNodeInvestment>& UnlockedNodes);

	/** Check if player meets class requirements */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|Validation")
	static bool MeetsClassRequirements(int32 PlayerLevel, EHarmoniaCharacterClass CurrentClass, EHarmoniaCharacterClass TargetClass);

	//~ UI Formatting Helpers

	/** Format large numbers with K/M suffixes (e.g., 1500 -> "1.5K") */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|UI")
	static FText FormatLargeNumber(int64 Number);

	/** Get progress bar color based on percentage */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|UI")
	static FLinearColor GetProgressBarColor(float Percentage);

	/** Create stat comparison text (e.g., "+5 Strength") */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Progression|UI")
	static FText CreateStatChangeText(FGameplayTag StatTag, int32 Change);
};
