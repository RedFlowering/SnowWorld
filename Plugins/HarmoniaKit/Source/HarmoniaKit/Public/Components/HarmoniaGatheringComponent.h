// Copyright Epic Games, Inc. All Rights Reserved.

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
 * ì±„ì§‘ ?œìŠ¤??ì»´í¬?ŒíŠ¸
 * ê´‘ë¬¼, ?½ì´ˆ, ë²Œëª© ???¤ì–‘???ì› ì±„ì§‘ ì²˜ë¦¬
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
	// ì±„ì§‘ ê¸°ë³¸ ê¸°ëŠ¥
	// ====================================

	/** ì±„ì§‘ ?œì‘ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	bool StartGathering(FName ResourceID, AActor* TargetNode = nullptr);

	/** ì±„ì§‘ ì·¨ì†Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void CancelGathering();

	/** ì±„ì§‘ ì¤‘ì¸ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool IsGathering() const { return bIsGathering; }

	/** ?„ì¬ ì±„ì§‘ ì§„í–‰??(0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetGatheringProgress() const;

	/** ?ì›??ì±„ì§‘?????ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool CanGatherResource(FName ResourceID) const;

	// ====================================
	// ?„êµ¬ ê´€ë¦?
	// ====================================

	/** ?„êµ¬ ?¥ì°© */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void EquipTool(const FGatheringToolData& Tool);

	/** ?„êµ¬ ?´ì œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void UnequipTool();

	/** ?„ì¬ ?¥ì°©???„êµ¬ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	FGatheringToolData GetEquippedTool() const { return EquippedTool; }

	/** ?„êµ¬ê°€ ?¥ì°©?˜ì–´ ?ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool HasToolEquipped() const;

	/** ?„êµ¬ ?´êµ¬??ê°ì†Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void ReduceToolDurability(int32 Amount);

	// ====================================
	// ?ˆë²¨ ë°?ê²½í—˜ì¹??œìŠ¤??(per resource type)
	// ====================================

	/** ì±„ì§‘ ê²½í—˜ì¹??ë“ (per resource type) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddGatheringExperience(int32 Amount, EGatheringResourceType ResourceType);

	/** ?„ì¬ ì±„ì§‘ ?ˆë²¨ (per resource type) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	int32 GetGatheringLevel(EGatheringResourceType ResourceType) const;

	/** ?„ì¬ ê²½í—˜ì¹?(per resource type) */
	int32 GetCurrentExperience(EGatheringResourceType ResourceType) const;

	/** ?¤ìŒ ?ˆë²¨ê¹Œì? ?„ìš”??ê²½í—˜ì¹?(per resource type) */
	int32 GetExperienceForNextLevel(EGatheringResourceType ResourceType) const;

	// ====================================
	// ?¹ì„± ?œìŠ¤??
	// ====================================

	/** ?¹ì„± ì¶”ê? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddTrait(const FGatheringTrait& Trait);

	/** ?¹ì„± ?œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void RemoveTrait(FName TraitName);

	/** ëª¨ë“  ?¹ì„± ê°€?¸ì˜¤ê¸?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	TArray<FGatheringTrait> GetAllTraits() const { return ActiveTraits; }

	/** ì´?ì±„ì§‘ ?ë„ ë³´ë„ˆ??ê³„ì‚° */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalGatheringSpeedBonus() const;

	/** ì´??ë“??ë³´ë„ˆ??ê³„ì‚° */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalYieldBonus() const;

	/** ?¬ë¦¬?°ì»¬ ?•ë¥  ê³„ì‚° */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetCriticalChance() const;

	// ====================================
	// ?´ë²¤??
	// ====================================

	/** ì±„ì§‘ ?œì‘ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringStarted OnGatheringStarted;

	/** ì±„ì§‘ ì·¨ì†Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCancelled OnGatheringCancelled;

	/** ì±„ì§‘ ?„ë£Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCompleted OnGatheringCompleted;

	/** ?ˆë²¨???´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringLevelUp OnGatheringLevelUp;

	/** ?„êµ¬ ?´êµ¬??ë³€ê²??´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnToolDurabilityChanged OnToolDurabilityChanged;

	// ====================================
	// ?¤ì •
	// ====================================

	/** ?ì› ?°ì´?°ë² ?´ìŠ¤ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	TMap<FName, FGatheringResourceData> ResourceDatabase;

	// Note: ExperienceMultiplier, BaseExperiencePerLevel?€ base class???•ì˜??

	/** ê¸°ë³¸ ?¬ë¦¬?°ì»¬ ?•ë¥  (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float BaseCriticalChance = 5.0f;

	/** ?¬ë¦¬?°ì»¬ ???ë“??ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float CriticalYieldMultiplier = 2.0f;

private:
	/** ì±„ì§‘ ì¤??Œë˜ê·?*/
	UPROPERTY()
	bool bIsGathering = false;

	/** ?„ì¬ ì±„ì§‘ ì¤‘ì¸ ?ì› */
	UPROPERTY()
	FName CurrentResourceID;

	/** ì±„ì§‘ ?œì‘ ?œê°„ */
	UPROPERTY()
	float GatheringStartTime = 0.0f;

	/** ì±„ì§‘ ?„ë£Œê¹Œì? ?„ìš”???œê°„ */
	UPROPERTY()
	float RequiredGatheringTime = 0.0f;

	/** ?¥ì°©???„êµ¬ */
	UPROPERTY()
	FGatheringToolData EquippedTool;

	/** ?ì› ?€?…ë³„ ?ˆë²¨ */
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringLevels;

	/** ?ì› ?€?…ë³„ ê²½í—˜ì¹?*/
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringExperience;

	/** ?œì„±?”ëœ ?¹ì„± ëª©ë¡ */
	UPROPERTY()
	TArray<FGatheringTrait> ActiveTraits;

	/** ?€ê²??¸ë“œ */
	UPROPERTY()
	TObjectPtr<AActor> TargetGatheringNode;

	/** ì±„ì§‘ ?„ë£Œ ì²˜ë¦¬ */
	void CompleteGathering();

	/** ì±„ì§‘ ê²°ê³¼ ê³„ì‚° */
	FGatheringResult CalculateGatheringResult(const FGatheringResourceData& ResourceData);

	/** ?ˆë²¨ ì²´í¬ ë°?ì²˜ë¦¬ (per resource type) */
	void CheckAndProcessLevelUp(EGatheringResourceType ResourceType);

	/** ì±„ì§‘ ?œê°„ ê³„ì‚° (ë³´ë„ˆ???ìš©) */
	float CalculateGatheringTime(float BaseTime) const;
};
