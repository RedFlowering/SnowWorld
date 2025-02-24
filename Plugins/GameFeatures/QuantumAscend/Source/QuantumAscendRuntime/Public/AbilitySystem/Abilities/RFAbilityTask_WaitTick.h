// Copyright 2025 RedFlowering.

#pragma once 

#include "Abilities/Tasks/AbilityTask.h"
#include "RFAbilityTask_WaitTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTickDelegate, float, DeltaTime);

UCLASS()
class QUANTUMASCENDRUNTIME_API URFAbilityTask_WaitTick : public UAbilityTask
{
	GENERATED_BODY()

public:
	// �� ƽ���� ȣ��Ǵ� ��������Ʈ
	UPROPERTY(BlueprintAssignable)
	FWaitTickDelegate OnTick;

	// AbilityTask ���� �Լ� (Ability���� ȣ��)
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HideFromLevelInfos, DefaultToSelf = "OwningAbility"))
	static URFAbilityTask_WaitTick* WaitTick(UGameplayAbility* OwningAbility);

	// AbilityTask Ȱ��ȭ �� ȣ��
	virtual void Activate() override;

	// �� ������ ȣ��Ǵ� �Լ� (Ability�� Ȱ��ȭ�� ����)
	virtual void TickTask(float DeltaTime) override;

protected:
	// Ability ���� �� ȣ�� (Tick ���� ��)
	virtual void OnDestroy(bool AbilityEnded) override;
};

