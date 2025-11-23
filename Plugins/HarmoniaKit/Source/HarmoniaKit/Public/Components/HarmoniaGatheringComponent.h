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
 * 채집 시스템 컴포넌트
 * 광물, 약초, 벌목 등 다양한 자원 채집 처리
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

	/** 채집 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	bool StartGathering(FName ResourceID, AActor* TargetNode = nullptr);

	/** 채집 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void CancelGathering();

	/** 채집 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool IsGathering() const { return bIsGathering; }

	/** 현재 채집 진행도 (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetGatheringProgress() const;

	/** 자원을 채집할 수 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool CanGatherResource(FName ResourceID) const;

	// ====================================
	// 도구 관리
	// ====================================

	/** 도구 장착 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void EquipTool(const FGatheringToolData& Tool);

	/** 도구 해제 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void UnequipTool();

	/** 현재 장착된 도구 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	FGatheringToolData GetEquippedTool() const { return EquippedTool; }

	/** 도구가 장착되어 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	bool HasToolEquipped() const;

	/** 도구 내구도 감소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void ReduceToolDurability(int32 Amount);

	// ====================================
	// 레벨 및 경험치 시스템 (per resource type)
	// ====================================

	/** 채집 경험치 획득 (per resource type) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddGatheringExperience(int32 Amount, EGatheringResourceType ResourceType);

	/** 현재 채집 레벨 (per resource type) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	int32 GetGatheringLevel(EGatheringResourceType ResourceType) const;

	/** 현재 경험치 (per resource type) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	int32 GetCurrentExperience(EGatheringResourceType ResourceType) const;

	/** 다음 레벨까지 필요한 경험치 (per resource type) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	int32 GetExperienceForNextLevel(EGatheringResourceType ResourceType) const;

	// ====================================
	// 특성 시스템
	// ====================================

	/** 특성 추가 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void AddTrait(const FGatheringTrait& Trait);

	/** 특성 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Gathering")
	void RemoveTrait(FName TraitName);

	/** 모든 특성 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	TArray<FGatheringTrait> GetAllTraits() const { return ActiveTraits; }

	/** 총 채집 속도 보너스 계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalGatheringSpeedBonus() const;

	/** 총 획득량 보너스 계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetTotalYieldBonus() const;

	/** 크리티컬 확률 계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Gathering")
	float GetCriticalChance() const;

	// ====================================
	// 이벤트
	// ====================================

	/** 채집 시작 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringStarted OnGatheringStarted;

	/** 채집 취소 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCancelled OnGatheringCancelled;

	/** 채집 완료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringCompleted OnGatheringCompleted;

	/** 레벨업 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnGatheringLevelUp OnGatheringLevelUp;

	/** 도구 내구도 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Gathering")
	FOnToolDurabilityChanged OnToolDurabilityChanged;

	// ====================================
	// 설정
	// ====================================

	/** 자원 데이터베이스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	TMap<FName, FGatheringResourceData> ResourceDatabase;

	// Note: ExperienceMultiplier, BaseExperiencePerLevel은 base class에 정의됨

	/** 기본 크리티컬 확률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float BaseCriticalChance = 5.0f;

	/** 크리티컬 시 획득량 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Settings")
	float CriticalYieldMultiplier = 2.0f;

private:
	/** 채집 중 플래그 */
	UPROPERTY()
	bool bIsGathering = false;

	/** 현재 채집 중인 자원 */
	UPROPERTY()
	FName CurrentResourceID;

	/** 채집 시작 시간 */
	UPROPERTY()
	float GatheringStartTime = 0.0f;

	/** 채집 완료까지 필요한 시간 */
	UPROPERTY()
	float RequiredGatheringTime = 0.0f;

	/** 장착된 도구 */
	UPROPERTY()
	FGatheringToolData EquippedTool;

	/** 자원 타입별 레벨 */
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringLevels;

	/** 자원 타입별 경험치 */
	UPROPERTY()
	TMap<EGatheringResourceType, int32> GatheringExperience;

	/** 활성화된 특성 목록 */
	UPROPERTY()
	TArray<FGatheringTrait> ActiveTraits;

	/** 타겟 노드 */
	UPROPERTY()
	TObjectPtr<AActor> TargetGatheringNode;

	/** 채집 완료 처리 */
	void CompleteGathering();

	/** 채집 결과 계산 */
	FGatheringResult CalculateGatheringResult(const FGatheringResourceData& ResourceData);

	/** 레벨 체크 및 처리 (per resource type) */
	void CheckAndProcessLevelUp(EGatheringResourceType ResourceType);

	/** 채집 시간 계산 (보너스 적용) */
	float CalculateGatheringTime(float BaseTime) const;
};
