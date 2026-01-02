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
 * 보스 전용 도약 공격 어빌리티
 * - Motion Warping을 사용한 타겟 추적 도약
 * - CustomMovementMode(Leaping)를 사용한 포물선 이동
 * 
 * 동작 순서:
 * 1. LeapMontage 재생 시작 (도약 시작 애니메이션)
 * 2. StartLeaping() 호출로 캐릭터 이동 시작
 * 3. 공중 체류 시간 = 몽타주 길이로 결정
 * 4. 착지 시 LandingAttackMontage 재생 (옵션)
 * 5. 착지 위치는 모션워핑으로 타겟 방향 조정
 * 
 * NOTE: 도약 중 이동은 몽타주의 루트모션이 아닌 CustomMovement로 처리됨
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
	// Montage Callbacks
	//~=============================================================================

	UFUNCTION()
	void OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	//~=============================================================================
	// Helper Functions
	//~=============================================================================

	/** Get target location for leap (from AIController blackboard) */
	FVector GetLeapTargetLocation() const;

	/** Get movement component */
	UHarmoniaCharacterMovementComponent* GetHarmoniaMovementComponent() const;

	/** Get motion warping component */
	UMotionWarpingComponent* GetMotionWarpingComponent() const;

public:
	//~=============================================================================
	// Leap Settings (블루프린트에서 편집 가능)
	//~=============================================================================

	/** 도약 각도 (도, 0=수평, 90=수직) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap", meta = (ClampMin = "15.0", ClampMax = "75.0"))
	float LeapAngle = 45.0f;

	/** 최소 도약 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap")
	float MinLeapDistance = 200.0f;

	/** 최대 도약 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap")
	float MaxLeapDistance = 1500.0f;

	/** 타겟 앞 착지 오프셋 (타겟 위치에서 이 거리만큼 앞에 착지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap")
	float TargetLocationOffset = 150.0f;

	/** 착지 데미지 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Leap")
	float LandingDamageMultiplier = 1.5f;

	//~=============================================================================
	// Animation (블루프린트에서 편집 가능)
	//~=============================================================================

	/** 도약 몽타주 (공중 체류 시간 = 이 몽타주 길이) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> LeapMontage = nullptr;

	/** 착지 공격 몽타주 (옵션, 착지 후 추가 공격) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> LandingAttackMontage = nullptr;

	// NOTE: Rotation is handled by Motion Warping AnimNotifyState in the montages
	// - For players: LockOnComponent sets the warp target
	// - For AI: BT sets TargetActor, MotionWarpingComponent handles rotation

protected:
	/** Cached target location for this leap */
	FVector CachedTargetLocation;
};
