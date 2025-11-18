// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaRecoveryItemDefinitions.h"
#include "HarmoniaRechargeableItemComponent.generated.h"

class APlayerController;
class UHarmoniaCheckpointSubsystem;

/**
 * Rechargeable Item Events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemUsed, EHarmoniaRecoveryItemType, ItemType, int32, RemainingCharges, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemRecharged, EHarmoniaRecoveryItemType, ItemType, int32, CurrentCharges, int32, MaxCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChargesChanged, EHarmoniaRecoveryItemType, ItemType, int32, NewCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpgraded, EHarmoniaRecoveryItemType, ItemType, int32, NewMaxCharges);

/**
 * UHarmoniaRechargeableItemComponent
 *
 * 충전 가능한 회복 아이템을 관리하는 컴포넌트
 * 모든 회복 아이템 타입에 재사용 가능한 범용 컴포넌트
 *
 * 주요 기능:
 * - 아이템 충전 횟수 관리
 * - 체크포인트에서 자동 충전
 * - 아이템 사용 검증
 * - 아이템 업그레이드 (최대 충전 횟수 증가)
 * - 네트워크 리플리케이션
 * - 저장/로드 시스템 연동
 *
 * 재사용 가능성:
 * - 회복 아이템 외에도 충전식 아이템에 사용 가능
 * - 던전 입장권, 부활 토큰 등 확장 가능
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaRechargeableItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRechargeableItemComponent();

	// UActorComponent interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End of UActorComponent interface

	// ============================================================================
	// Item Registration
	// ============================================================================

	/**
	 * 회복 아이템 등록 (초기화)
	 * @param ItemType 아이템 타입
	 * @param Config 아이템 설정
	 * @return 등록 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterRecoveryItem(EHarmoniaRecoveryItemType ItemType, const FHarmoniaRecoveryItemConfig& Config);

	/**
	 * 공명 파편 등록 (주파수별)
	 * @param Frequency 공명 주파수
	 * @param VariantConfig 파편 설정
	 * @return 등록 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterResonanceShard(EHarmoniaResonanceFrequency Frequency, const FHarmoniaResonanceShardVariant& VariantConfig);

	/**
	 * 아이템 등록 해제
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void UnregisterRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	// ============================================================================
	// Item Usage
	// ============================================================================

	/**
	 * 회복 아이템 사용 (충전 횟수 1 소모)
	 * @param ItemType 사용할 아이템 타입
	 * @return 사용 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	/**
	 * 공명 파편 사용 (주파수별)
	 * @param Frequency 사용할 파편의 공명 주파수
	 * @return 사용 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseResonanceShard(EHarmoniaResonanceFrequency Frequency);

	/**
	 * 아이템 사용 가능 여부 확인
	 * @param ItemType 확인할 아이템 타입
	 * @param OutReason 사용 불가능한 이유 (실패 시)
	 * @return 사용 가능 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool CanUseRecoveryItem(EHarmoniaRecoveryItemType ItemType, FText& OutReason) const;

	/**
	 * 남은 충전 횟수 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetRemainingCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * 최대 충전 횟수 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetMaxCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * 아이템 설정 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetRecoveryItemConfig(EHarmoniaRecoveryItemType ItemType, FHarmoniaRecoveryItemConfig& OutConfig) const;

	/**
	 * 공명 파편 설정 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetResonanceShardVariant(EHarmoniaResonanceFrequency Frequency, FHarmoniaResonanceShardVariant& OutVariant) const;

	// ============================================================================
	// Item Recharge
	// ============================================================================

	/**
	 * 아이템 충전
	 * @param ItemType 충전할 아이템 타입
	 * @param Amount 충전할 횟수 (-1이면 최대치까지 충전)
	 * @return 충전된 횟수
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeItem(EHarmoniaRecoveryItemType ItemType, int32 Amount = -1);

	/**
	 * 모든 아이템 충전 (체크포인트에서 공명 시)
	 * @return 충전된 총 횟수
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeAllItems();

	/**
	 * 충전 가능한 아이템만 충전
	 * @return 충전된 총 횟수
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeRechargeableItems();

	// ============================================================================
	// Item Upgrade
	// ============================================================================

	/**
	 * 아이템 업그레이드 (최대 충전 횟수 증가)
	 * @param ItemType 업그레이드할 아이템 타입
	 * @param AdditionalMaxCharges 증가시킬 최대 충전 횟수
	 * @return 업그레이드 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UpgradeItemMaxCharges(EHarmoniaRecoveryItemType ItemType, int32 AdditionalMaxCharges);

	// ============================================================================
	// Checkpoint Integration
	// ============================================================================

	/**
	 * 체크포인트 공명 이벤트 핸들러
	 */
	UFUNCTION()
	void OnCheckpointResonanceCompleted(FName CheckpointID, const FHarmoniaResonanceResult& Result);

	// ============================================================================
	// Data Persistence
	// ============================================================================

	/**
	 * 아이템 상태 저장 데이터 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	TArray<FHarmoniaRecoveryItemState> GetItemStatesForSave() const;

	/**
	 * 아이템 상태 로드
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void LoadItemStates(const TArray<FHarmoniaRecoveryItemState>& LoadedStates);

	// ============================================================================
	// Events
	// ============================================================================

	/** 아이템 사용 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUsed OnItemUsed;

	/** 아이템 충전 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemRecharged OnItemRecharged;

	/** 충전 횟수 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnChargesChanged OnChargesChanged;

	/** 아이템 업그레이드 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUpgraded OnItemUpgraded;

protected:
	/**
	 * 아이템 등록 여부 확인
	 */
	bool IsItemRegistered(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * 충전 횟수 변경 (내부용, 리플리케이션 포함)
	 */
	void SetCharges(EHarmoniaRecoveryItemType ItemType, int32 NewCharges);

	/**
	 * 체크포인트 서브시스템 초기화
	 */
	void InitializeCheckpointIntegration();

private:
	/** 등록된 회복 아이템 설정 */
	UPROPERTY(Transient)
	TMap<EHarmoniaRecoveryItemType, FHarmoniaRecoveryItemConfig> RegisteredItems;

	/** 등록된 공명 파편 변형 (주파수별) */
	UPROPERTY(Transient)
	TMap<EHarmoniaResonanceFrequency, FHarmoniaResonanceShardVariant> ResonanceShardVariants;

	/** 아이템별 현재 상태 (네트워크 리플리케이션) */
	UPROPERTY(ReplicatedUsing=OnRep_ItemStates)
	TArray<FHarmoniaRecoveryItemState> ItemStates;

	UFUNCTION()
	void OnRep_ItemStates();

	/** 체크포인트 서브시스템 캐시 */
	UPROPERTY(Transient)
	TObjectPtr<UHarmoniaCheckpointSubsystem> CheckpointSubsystem;
};
