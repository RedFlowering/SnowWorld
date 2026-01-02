// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_JumpToPoint.generated.h"

class AHarmoniaJumpPoint;

/**
 * UHarmoniaGameplayAbility_JumpToPoint
 *
 * 캐릭터가 JumpPoint (바위, 건물 등)로 점프하는 어빌리티입니다.
 * 플레이어와 AI (보스/몬스터) 모두 사용 가능합니다.
 *
 * 사용법:
 * - JumpPoint 액터가 레벨에 배치되어 있어야 함
 * - SenseComponent로 JumpPoint 탐지
 * - 가장 적합한 JumpPoint로 점프
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_JumpToPoint : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_JumpToPoint();

	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

protected:
	// ==========================================================================
	// Configuration
	// ==========================================================================

	/** 점프 몽타주 (도약 애니메이션) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Animation")
	TObjectPtr<UAnimMontage> JumpMontage = nullptr;

	/** 착지 몽타주 (선택) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Animation")
	TObjectPtr<UAnimMontage> LandingMontage = nullptr;

	/** 착지 후 추가 몽타주 (예: 포효, 옵션) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Animation")
	TObjectPtr<UAnimMontage> PostLandingMontage = nullptr;

	/** 도약 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Movement")
	float JumpAngle = 45.0f;

	/** 점프 중 무적 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Movement")
	bool bInvulnerableDuringJump = false;

	/** JumpPoint 탐색 반경 (Sense 미사용 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Detection")
	float SearchRadius = 2000.0f;

	// ==========================================================================
	// Runtime State
	// ==========================================================================

	/** 현재 점프 대상 포인트 */
	UPROPERTY(BlueprintReadOnly, Category = "Jump|State")
	TObjectPtr<AHarmoniaJumpPoint> TargetJumpPoint = nullptr;

protected:
	/** 적합한 JumpPoint 찾기 (Override 가능) */
	UFUNCTION(BlueprintNativeEvent, Category = "Jump")
	AHarmoniaJumpPoint* FindBestJumpPoint();

	/** 점프 시작 */
	UFUNCTION(BlueprintNativeEvent, Category = "Jump")
	void StartJump();

	/** 점프 완료 콜백 */
	UFUNCTION()
	void OnJumpMontageComplete();

	/** 착지 완료 콜백 */
	UFUNCTION()
	void OnLandingComplete();

private:
	FTimerHandle JumpTimerHandle;
};

