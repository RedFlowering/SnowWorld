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
 * 채집 ?�스??컴포?�트
 * 광물, ?�초, 벌목 ???�양???�원 채집 처리
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
	// 채집 기본 기능
	// ====================================

	/** 채집 ?�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	bool StartGathering(FName ResourceID, AActor* TargetNode = nullptr);

	/** 채집 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void CancelGathering();

	/** 채집 중인지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool IsGathering() const { return bIsGathering; }

	/** ?�재 채집 진행??(0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetGatheringProgress() const;

	/** ?�원??채집?????�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool CanGatherResource(FName ResourceID) const;

	// ====================================
	// ?�구 관�?
	// ====================================

	/** ?�구 ?�착 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void EquipTool(const FGatheringToolData& Tool);

	/** ?�구 ?�제 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void UnequipTool();

	/** ?�재 ?�착???�구 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	FGatheringToolData GetEquippedTool() const { return EquippedTool; }

	/** ?�구가 ?�착?�어 ?�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool HasToolEquipped() const;

	/** ?�구 ?�구??감소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void ReduceToolDurability(int32 Amount);

	// ====================================
	// ?�벨 �?경험�??�스??(per resource type)
	// ====================================

	/** 채집 경험�??�득 (per resource type) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddGatheringExperience(int32 Amount, EGatheringResourceType ResourceType);

	/** ?�재 채집 ?�벨 (per resource type) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	int32 GetGatheringLevel(EGatheringResourceType ResourceType) const;

	/** ?�재 경험�?(per resource type) */
	int32 GetCurrentExperience(EGatheringResourceType ResourceType) const;

	/** ?�음 ?�벨까�? ?�요??경험�?(per resource type) */
	int32 GetExperienceForNextLevel(EGatheringResourceType ResourceType) const;

	// ====================================
	// ?�성 ?�스??
	// ====================================

	/** ?�성 추�? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddTrait(const FGatheringTrait& Trait);

	/** ?�성 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void RemoveTrait(FName TraitName);

	/** 모든 ?�성 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	TArray<FGatheringTrait> GetAllTraits() const { return ActiveTraits; }

	/** �?채집 ?�도 보너??계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalGatheringSpeedBonus() const;

	/** �??�득??보너??계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalYieldBonus() const;

	/** ?�리?�컬 ?�률 계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetCriticalChance() const;

	// ====================================
	// ?�벤??
	// ====================================

	/** 채집 ?�작 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringStarted OnGatheringStarted;

	/** 채집 취소 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCancelled OnGatheringCancelled;

	/** 채집 ?�료 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCompleted OnGatheringCompleted;

	/** ?�벨???�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringLevelUp OnGatheringLevelUp;

	/** ?�구 ?�구??변�??�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnToolDurabilityChanged OnToolDurabilityChanged;

	// ====================================
	// ?�정
	// ====================================

	/** ?�원 ?�이?�베?�스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	TMap<FName, FGatheringResourceData> ResourceDatabase;

	// Note: ExperienceMultiplier, BaseExperiencePerLevel?� base class???�의??

	/** 기본 ?�리?�컬 ?�률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float BaseCriticalChance = 5.0f;

	/** ?�리?�컬 ???�득??배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float CriticalYieldMultiplier = 2.0f;

private:
	/** 채집 �??�래�?*/
	UPROPERTY()
	bool bIsGathering = false;

	/** ?�재 채집 중인 ?�원 */
	UPROPERTY()
	FName CurrentResourceID;

	/** 채집 ?�작 ?�간 */
	UPROPERTY()
	float GatheringStartTime = 0.0f;

	/** 채집 ?�료까�? ?�요???�간 */
	UPROPERTY()
	float RequiredGatheringTime = 0.0f;

	/** ?�착???�구 */
	UPROPERTY()
	FGatheringToolData EquippedTool;

	/** ?�원 ?�?�별 ?�벨 */
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringLevels;

	/** ?�원 ?�?�별 경험�?*/
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringExperience;

	/** ?�성?�된 ?�성 목록 */
	UPROPERTY()
	TArray<FGatheringTrait> ActiveTraits;

	/** ?��??�드 */
	UPROPERTY()
	TObjectPtr<AActor> TargetGatheringNode;

	/** 채집 ?�료 처리 */
	void CompleteGathering();

	/** 채집 결과 계산 */
	FGatheringResult CalculateGatheringResult(const FGatheringResourceData& ResourceData);

	/** ?�벨 체크 �?처리 (per resource type) */
	void CheckAndProcessLevelUp(EGatheringResourceType ResourceType);

	/** 채집 ?�간 계산 (보너???�용) */
	float CalculateGatheringTime(float BaseTime) const;
};
