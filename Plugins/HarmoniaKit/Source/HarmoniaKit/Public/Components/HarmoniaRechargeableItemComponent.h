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
 * 충전 가?�한 ?�복 ?�이?�을 관리하??컴포?�트
 * 모든 ?�복 ?�이???�?�에 ?�사??가?�한 범용 컴포?�트
 *
 * 주요 기능:
 * - ?�이??충전 ?�수 관�?
 * - 체크?�인?�에???�동 충전
 * - ?�이???�용 검�?
 * - ?�이???�그?�이??(최�? 충전 ?�수 증�?)
 * - ?�트?�크 리플리�??�션
 * - ?�??로드 ?�스???�동
 *
 * ?�사??가?�성:
 * - ?�복 ?�이???�에??충전???�이?�에 ?�용 가??
 * - ?�전 ?�장�? 부???�큰 ???�장 가??
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
	 * ?�복 ?�이???�록 (초기??
	 * @param ItemType ?�이???�??
	 * @param Config ?�이???�정
	 * @return ?�록 ?�공 ?��?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterRecoveryItem(EHarmoniaRecoveryItemType ItemType, const FHarmoniaRecoveryItemConfig& Config);

	/**
	 * 공명 ?�편 ?�록 (주파?�별)
	 * @param Frequency 공명 주파??
	 * @param VariantConfig ?�편 ?�정
	 * @return ?�록 ?�공 ?��?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterResonanceShard(EHarmoniaResonanceFrequency Frequency, const FHarmoniaResonanceShardVariant& VariantConfig);

	/**
	 * ?�이???�록 ?�제
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void UnregisterRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	// ============================================================================
	// Item Usage
	// ============================================================================

	/**
	 * ?�복 ?�이???�용 (충전 ?�수 1 ?�모)
	 * @param ItemType ?�용???�이???�??
	 * @return ?�용 ?�공 ?��?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	/**
	 * 공명 ?�편 ?�용 (주파?�별)
	 * @param Frequency ?�용???�편??공명 주파??
	 * @return ?�용 ?�공 ?��?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseResonanceShard(EHarmoniaResonanceFrequency Frequency);

	/**
	 * ?�이???�용 가???��? ?�인
	 * @param ItemType ?�인???�이???�??
	 * @param OutReason ?�용 불�??�한 ?�유 (?�패 ??
	 * @return ?�용 가???��?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool CanUseRecoveryItem(EHarmoniaRecoveryItemType ItemType, FText& OutReason) const;

	/**
	 * ?��? 충전 ?�수 가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetRemainingCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * 최�? 충전 ?�수 가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetMaxCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * ?�이???�정 가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetRecoveryItemConfig(EHarmoniaRecoveryItemType ItemType, FHarmoniaRecoveryItemConfig& OutConfig) const;

	/**
	 * 공명 ?�편 ?�정 가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetResonanceShardVariant(EHarmoniaResonanceFrequency Frequency, FHarmoniaResonanceShardVariant& OutVariant) const;

	// ============================================================================
	// Item Recharge
	// ============================================================================

	/**
	 * ?�이??충전
	 * @param ItemType 충전???�이???�??
	 * @param Amount 충전???�수 (-1?�면 최�?치까지 충전)
	 * @return 충전???�수
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeItem(EHarmoniaRecoveryItemType ItemType, int32 Amount = -1);

	/**
	 * 모든 ?�이??충전 (체크?�인?�에??공명 ??
	 * @return 충전??�??�수
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeAllItems();

	/**
	 * 충전 가?�한 ?�이?�만 충전
	 * @return 충전??�??�수
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeRechargeableItems();

	// ============================================================================
	// Item Upgrade
	// ============================================================================

	/**
	 * ?�이???�그?�이??(최�? 충전 ?�수 증�?)
	 * @param ItemType ?�그?�이?�할 ?�이???�??
	 * @param AdditionalMaxCharges 증�??�킬 최�? 충전 ?�수
	 * @return ?�그?�이???�공 ?��?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UpgradeItemMaxCharges(EHarmoniaRecoveryItemType ItemType, int32 AdditionalMaxCharges);

	// ============================================================================
	// Checkpoint Integration
	// ============================================================================

	/**
	 * 체크?�인??공명 ?�벤???�들??
	 */
	UFUNCTION()
	void OnCheckpointResonanceCompleted(FName CheckpointID, const FHarmoniaResonanceResult& Result);

	// ============================================================================
	// Data Persistence
	// ============================================================================

	/**
	 * ?�이???�태 ?�???�이??가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	TArray<FHarmoniaRecoveryItemState> GetItemStatesForSave() const;

	/**
	 * ?�이???�태 로드
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void LoadItemStates(const TArray<FHarmoniaRecoveryItemState>& LoadedStates);

	// ============================================================================
	// Events
	// ============================================================================

	/** ?�이???�용 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUsed OnItemUsed;

	/** ?�이??충전 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemRecharged OnItemRecharged;

	/** 충전 ?�수 변�??�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnChargesChanged OnChargesChanged;

	/** ?�이???�그?�이???�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUpgraded OnItemUpgraded;

protected:
	/**
	 * ?�이???�록 ?��? ?�인
	 */
	bool IsItemRegistered(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * 충전 ?�수 변�?(?��??? 리플리�??�션 ?�함)
	 */
	void SetCharges(EHarmoniaRecoveryItemType ItemType, int32 NewCharges);

	/**
	 * 체크?�인???�브?�스??초기??
	 */
	void InitializeCheckpointIntegration();

private:
	/** ?�록???�복 ?�이???�정 */
	UPROPERTY(Transient)
	TMap<EHarmoniaRecoveryItemType, FHarmoniaRecoveryItemConfig> RegisteredItems;

	/** ?�록??공명 ?�편 변??(주파?�별) */
	UPROPERTY(Transient)
	TMap<EHarmoniaResonanceFrequency, FHarmoniaResonanceShardVariant> ResonanceShardVariants;

	/** ?�이?�별 ?�재 ?�태 (?�트?�크 리플리�??�션) */
	UPROPERTY(ReplicatedUsing=OnRep_ItemStates)
	TArray<FHarmoniaRecoveryItemState> ItemStates;

	UFUNCTION()
	void OnRep_ItemStates();

	/** 체크?�인???�브?�스??캐시 */
	UPROPERTY(Transient)
	TObjectPtr<UHarmoniaCheckpointSubsystem> CheckpointSubsystem;
};
