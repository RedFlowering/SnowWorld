// Copyright 2025 Snow Game Studio.

#pragma once 

#include "Abilities/Tasks/AbilityTask.h"
#include "QAAbilityTask_WaitTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTickDelegate, float, DeltaTime);

UCLASS()
class QUANTUMASCENDRUNTIME_API UQAAbilityTask_WaitTick : public UAbilityTask
{
	GENERATED_BODY()

public:
	// 매 틱마다 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FWaitTickDelegate OnTick;

	// AbilityTask 생성 함수 (Ability에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HideFromLevelInfos, DefaultToSelf = "OwningAbility"))
	static UQAAbilityTask_WaitTick* WaitTick(UGameplayAbility* OwningAbility);

	// AbilityTask 활성화 시 호출
	virtual void Activate() override;

	// 매 프레임 호출되는 함수 (Ability가 활성화된 동안)
	virtual void TickTask(float DeltaTime) override;

protected:
	// Ability 종료 시 호출 (Tick 중지 등)
	virtual void OnDestroy(bool AbilityEnded) override;
};

