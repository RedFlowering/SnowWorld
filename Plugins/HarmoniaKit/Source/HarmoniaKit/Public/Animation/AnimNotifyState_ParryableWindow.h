// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "AnimNotifyState_ParryableWindow.generated.h"

class UHarmoniaMeleeCombatComponent;

/**
 * Parryable Window Notify State
 * 
 * 몬스터 공격 몽타주에 배치하여 패링 가능 구간을 정의합니다.
 * 이 NotifyState가 활성화된 동안 플레이어가 패링하면 성공으로 처리됩니다.
 * 
 * 사용법:
 * 1. 몬스터 공격 몽타주에 이 NotifyState 추가
 * 2. AttackWindow(히트 트레이스) 시작 직전 또는 약간 겹치게 배치
 * 3. ParryConfig 설정으로 패링 성공 효과 커스터마이즈
 * 
 * 타이밍:
 * - BranchingPoint를 사용하여 정확한 타이밍 보장
 * - 히트 시점에 몽타주 타임으로 한 번 더 검증 (프레임 밀림 대응)
 */
UCLASS(Blueprintable, meta = (DisplayName = "Parryable Window"))
class HARMONIAKIT_API UAnimNotifyState_ParryableWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_ParryableWindow();

	//~UAnimNotifyState interface
	virtual void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual FString GetNotifyName_Implementation() const override;
	
#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override { return true; }
#endif
	//~End interface

public:
	/** 패링 설정 (NotifyState별로 커스터마이즈 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parry")
	FHarmoniaParryConfig ParryConfig;
	
	/** 이 공격이 패링 불가능한 경우 (Grab, Sweep 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parry")
	bool bIsUnparryable = false;

private:
	/** 등록 시점의 몽타주 타임 저장 (히트 시점 검증용) */
	float RegisteredStartTime = 0.0f;
	
	/** 태그를 부여한 Owner 캐시 (NotifyEnd에서 정리용) */
	UPROPERTY()
	TWeakObjectPtr<AActor> CachedOwner;
};
