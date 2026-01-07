// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HarmoniaGameplayAbility_Boss.h"
#include "HarmoniaGameplayAbility_BossAttack.generated.h"

class UAnimMontage;

/**
 * UHarmoniaGameplayAbility_BossAttack
 *
 * 보스 공격 어빌리티 베이스 클래스
 * - UHarmoniaGameplayAbility_Boss의 기능 상속
 * - 공격 몽타주 재생 기능 추가
 * 
 * 사용법:
 * - 이 클래스를 상속받아 BP에서 AttackMontage 설정
 * - 또는 C++에서 직접 상속하여 커스텀 공격 로직 구현
 */
UCLASS(BlueprintType, Blueprintable)
class HARMONIAKIT_API UHarmoniaGameplayAbility_BossAttack : public UHarmoniaGameplayAbility_Boss
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_BossAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	//~=============================================================================
	// Montage Callbacks
	//~=============================================================================

	/** 몽타주 완료 콜백 */
	UFUNCTION()
	virtual void OnMontageCompleted();

	/** 몽타주 취소 콜백 */
	UFUNCTION()
	virtual void OnMontageCancelled();

	/** 몽타주 중단 콜백 */
	UFUNCTION()
	virtual void OnMontageInterrupted();

	/** 몽타주 블렌드 아웃 콜백 */
	UFUNCTION()
	virtual void OnMontageBlendOut();

public:
	//~=============================================================================
	// Attack Configuration (블루프린트에서 편집 가능)
	//~=============================================================================

	/** 공격 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	/** 몽타주 재생 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

	/** 몽타주 시작 섹션 (비어있으면 처음부터) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	FName MontageStartSection = NAME_None;

	/** 데미지 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	float DamageMultiplier = 1.0f;

	/** 몽타주 완료 시 어빌리티 종료 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	bool bEndAbilityOnMontageComplete = true;

	/** Motion Warping target name to use in AnimNotifyState_MotionWarping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack|Motion Warping")
	FName WarpTargetName = FName("AttackTarget");

	/** Whether to enable motion warping towards target during attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack|Motion Warping")
	bool bEnableMotionWarping = true;

protected:
	/** Setup motion warping target from AI Blackboard TargetActor */
	virtual void SetupMotionWarpingTarget();
};

