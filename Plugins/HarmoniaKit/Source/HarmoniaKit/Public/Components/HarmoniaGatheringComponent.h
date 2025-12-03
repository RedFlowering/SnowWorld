// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaGatheringComponent.h
 * @brief Gathering system component for resource collection (mining, herbalism, logging)
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseLifeContentComponent.h"
#include "Definitions/HarmoniaGatheringSystemDefinitions.h"
#include "HarmoniaGatheringComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGatheringStarted, FName, ResourceID, float, GatheringTime, EGatheringResourceType, ResourceType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGatheringCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGatheringCompleted, const FGatheringResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGatheringLevelUp, int32, NewLevel, int32, SkillPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnToolDurabilityChanged, FName, ToolID, int32, RemainingDurability);

/**
 * @class UHarmoniaGatheringComponent
 * @brief Gathering system component for various resource collection
 * 
 * Handles mining, herbalism, logging and other resource gathering activities.
 * Inherits leveling, experience, and activity management from UHarmoniaBaseLifeContentComponent
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaGatheringComponent : public UHarmoniaBaseLifeContentComponent
{
	GENERATED_BODY()

public:
	UHarmoniaGatheringComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnActivityComplete() override;
	virtual void OnLevelUpInternal(int32 NewLevel) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================================
	// Gathering Basic Functions
	// ====================================

	/** Start gathering resource */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	bool StartGathering(FName ResourceID, AActor* TargetNode = nullptr);

	/** Cancel current gathering */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void CancelGathering();

	/** Check if currently gathering */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool IsGathering() const { return bIsGathering; }

	/** Get current gathering progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetGatheringProgress() const;

	/** Check if resource can be gathered */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool CanGatherResource(FName ResourceID) const;

	// ====================================
	// Tool Management
	// ====================================

	/** Equip gathering tool */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void EquipTool(const FGatheringToolData& Tool);

	/** Unequip current tool */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void UnequipTool();

	/** Get currently equipped tool */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	FGatheringToolData GetEquippedTool() const { return EquippedTool; }

	/** Check if tool is equipped */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool HasToolEquipped() const;

	/** Reduce tool durability */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void ReduceToolDurability(int32 Amount);

	// ====================================
	// Level & Experience System (per resource type)
	// ====================================

	/** Add gathering experience (per resource type) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddGatheringExperience(int32 Amount, EGatheringResourceType ResourceType);

	/** Get current gathering level (per resource type) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	int32 GetGatheringLevel(EGatheringResourceType ResourceType) const;

	/** Get current experience (per resource type) */
	int32 GetCurrentExperience(EGatheringResourceType ResourceType) const;

	/** Get experience required for next level (per resource type) */
	int32 GetExperienceForNextLevel(EGatheringResourceType ResourceType) const;

	// ====================================
	// Trait System
	// ====================================

	/** Add gathering trait */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddTrait(const FGatheringTrait& Trait);

	/** Remove gathering trait */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void RemoveTrait(FName TraitName);

	/** Get all active traits */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	TArray<FGatheringTrait> GetAllTraits() const { return ActiveTraits; }

	/** Calculate total gathering speed bonus */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalGatheringSpeedBonus() const;

	/** Calculate total yield bonus */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalYieldBonus() const;

	/** Calculate critical chance */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetCriticalChance() const;

	// ====================================
	// Events
	// ====================================

	/** Event fired when gathering starts */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringStarted OnGatheringStarted;

	/** Event fired when gathering is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCancelled OnGatheringCancelled;

	/** Event fired when gathering completes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCompleted OnGatheringCompleted;

	/** Event fired on level up */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringLevelUp OnGatheringLevelUp;

	/** Event fired when tool durability changes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnToolDurabilityChanged OnToolDurabilityChanged;

	// ====================================
	// Settings
	// ====================================

	/** Resource database */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	TMap<FName, FGatheringResourceData> ResourceDatabase;

	// Note: ExperienceMultiplier, BaseExperiencePerLevel are defined in base class

	/** Base critical chance (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float BaseCriticalChance = 5.0f;

	/** Critical yield multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float CriticalYieldMultiplier = 2.0f;

private:
	/** Gathering status flag */
	UPROPERTY()
	bool bIsGathering = false;

	/** Currently gathering resource ID */
	UPROPERTY()
	FName CurrentResourceID;

	/** Gathering start time */
	UPROPERTY()
	float GatheringStartTime = 0.0f;

	/** Required time to complete gathering */
	UPROPERTY()
	float RequiredGatheringTime = 0.0f;

	/** Equipped tool data */
	UPROPERTY()
	FGatheringToolData EquippedTool;

	/** Gathering levels per resource type */
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringLevels;

	/** Experience per resource type */
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringExperience;

	/** List of active gathering traits */
	UPROPERTY()
	TArray<FGatheringTrait> ActiveTraits;

	/** Target gathering node */
	UPROPERTY()
	TObjectPtr<AActor> TargetGatheringNode;

	/** Process gathering completion */
	void CompleteGathering();

	/** Calculate gathering result */
	FGatheringResult CalculateGatheringResult(const FGatheringResourceData& ResourceData);

	/** Check and process level up (per resource type) */
	void CheckAndProcessLevelUp(EGatheringResourceType ResourceType);

	/** Calculate gathering time with bonuses */
	float CalculateGatheringTime(float BaseTime) const;
};
