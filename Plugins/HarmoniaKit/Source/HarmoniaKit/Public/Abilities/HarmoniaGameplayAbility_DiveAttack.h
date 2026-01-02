// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_DiveAttack.generated.h"

// 착지 이벤트 델리게이트 (AnimNotify에서 호출)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiveLandingDelegate, FVector, LandingLocation);

/**
 * UHarmoniaGameplayAbility_DiveAttack
 *
 * 높은 곳에서 타겟을 향해 급강하 공격하는 어빌리티입니다.
 * 데미지는 AnimNotify에서 처리합니다 (MeleeAttack과 동일).
 *
 * 사용법:
 * - 캐릭터가 높은 곳에 있을 때 활성화
 * - 착지 몽타주에서 AnimNotify 호출 → OnDiveLanding 브로드캐스트
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_DiveAttack : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_DiveAttack();

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	// ==========================================================================
	// Delegates (AnimNotify에서 사용)
	// ==========================================================================

	/** 착지 시 브로드캐스트 (데미지 처리용) */
	UPROPERTY(BlueprintAssignable, Category = "Dive|Events")
	FOnDiveLandingDelegate OnDiveLanding;

protected:
	// ==========================================================================
	// Configuration
	// ==========================================================================

	/** 급강하 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dive|Animation")
	TObjectPtr<UAnimMontage> DiveMontage = nullptr;

	/** 착지 공격 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dive|Animation")
	TObjectPtr<UAnimMontage> LandingAttackMontage = nullptr;

	/** 급강하 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dive|Movement")
	float DiveSpeed = 2000.0f;

	/** 급강하 가능 최소 높이 차이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dive|Movement")
	float MinHeightDifference = 200.0f;

	/** 급강하 중 무적 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dive|Movement")
	bool bInvulnerableDuringDive = false;

	/** 착지 시 화면 흔들림 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dive|Effects")
	TSubclassOf<class UCameraShakeBase> LandingCameraShake;

	/** 착지 카메라 셰이크 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dive|Effects")
	float CameraShakeRadius = 800.0f;

	// ==========================================================================
	// Functions
	// ==========================================================================

	/** 급강하 시작 */
	UFUNCTION(BlueprintNativeEvent, Category = "Dive")
	void StartDive();

	/** 착지 이벤트 브로드캐스트 (AnimNotify에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Dive")
	void BroadcastLanding();

	/** 급강하 완료 콜백 */
	UFUNCTION()
	void OnDiveMontageComplete();

	/** 착지 공격 완료 콜백 */
	UFUNCTION()
	void OnLandingAttackComplete();

private:
	FVector DiveStartLocation;
	FVector DiveTargetLocation;
	FTimerHandle DiveTimerHandle;
};
