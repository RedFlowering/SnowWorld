// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HarmoniaGameplayAbility_Boss.h"
#include "HarmoniaGameplayAbility_BossLeapAttack.generated.h"

class UHarmoniaCharacterMovementComponent;
class UMotionWarpingComponent;
class UAnimMontage;

/**
 * UHarmoniaGameplayAbility_BossLeapAttack
 *
 * 보스 전용 점프 대쉬 공격 어빌리티 (2단계 공격)
 * 
 * === 동작 패턴 ===
 * Phase 1: 점프 (위로 도약)
 *   - JumpMontage 재생
 *   - 설정된 JumpAngle로 상승
 *   - JumpHeight까지 상승 후 Phase 2로 전환
 * 
 * Phase 2: 대쉬 공격 (타겟에게 돌진)
 *   - DashAttackMontage 재생
 *   - Motion Warping으로 타겟 추적
 *   - 타겟에게 빠르게 접근하면서 공격
 * 
 * === Motion Warping ===
 * - Phase 2에서 'AttackTarget' 워프 타겟 사용
 * - AddOrUpdateWarpTargetFromComponent로 실시간 추적
 * 
 * @see AnimNotifyState_MotionWarping (Warp Target Name = "AttackTarget")
 */
UCLASS(BlueprintType, Blueprintable)
class HARMONIAKIT_API UHarmoniaGameplayAbility_BossLeapAttack : public UHarmoniaGameplayAbility_Boss
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_BossLeapAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	//~=============================================================================
	// Phase Transitions
	//~=============================================================================

	/** Start Phase 1: Jump up */
	void StartJumpPhase();

	/** Start Phase 2: Dash attack towards target */
	void StartDashAttackPhase();

	/** Setup Motion Warping target for dash attack phase */
	void SetupMotionWarpingTarget();

	//~=============================================================================
	// Montage Callbacks
	//~=============================================================================

	UFUNCTION()
	void OnJumpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnDashAttackMontageCompleted(UAnimMontage* Montage, bool bInterrupted);

	//~=============================================================================
	// Helper Functions
	//~=============================================================================

	/** Get target actor from AI Blackboard */
	AActor* GetTargetActor() const;

	/** Get movement component */
	UHarmoniaCharacterMovementComponent* GetHarmoniaMovementComponent() const;

	/** Get motion warping component */
	UMotionWarpingComponent* GetMotionWarpingComponent() const;

public:
	//~=============================================================================
	// Jump Phase Settings (Phase 1)
	//~=============================================================================

	/** 점프 각도 (도, 0=수평, 90=수직) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|Jump", meta = (ClampMin = "30.0", ClampMax = "90.0"))
	float JumpAngle = 70.0f;

	/** 점프 높이 (위로 얼마나 올라갈지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|Jump")
	float JumpHeight = 400.0f;

	/** 점프 시간 (Phase 1 지속 시간) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|Jump")
	float JumpDuration = 0.5f;

	/** 점프 몽타주 (Phase 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|Animation")
	TObjectPtr<UAnimMontage> JumpMontage = nullptr;

	//~=============================================================================
	// Dash Attack Phase Settings (Phase 2)
	//~=============================================================================

	/** 대쉬 속도 (타겟 방향 이동 속도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|DashAttack")
	float DashSpeed = 2000.0f;

	/** 최대 대쉬 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|DashAttack")
	float MaxDashDistance = 1500.0f;

	/** 타겟 앞에서 멈출 거리 (타겟 위에 착지하지 않도록) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|DashAttack")
	float TargetStopDistance = 100.0f;

	/** 대쉬 공격 몽타주 (Phase 2) - Motion Warping 포함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|Animation")
	TObjectPtr<UAnimMontage> DashAttackMontage = nullptr;

	//~=============================================================================
	// Motion Warping Settings
	//~=============================================================================

	/** Motion Warping target name (must match AnimNotifyState) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|Motion Warping")
	FName WarpTargetName = FName("AttackTarget");

	/** Whether to enable motion warping during dash attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap|Motion Warping")
	bool bEnableMotionWarping = true;

protected:
	/** Current attack phase */
	UPROPERTY(Transient)
	int32 CurrentPhase = 0;

	/** Cached target actor */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CachedTargetActor;

	/** Timer for phase transition */
	FTimerHandle PhaseTransitionTimerHandle;
};
