// Copyright 2025 Snow Game Studio.

#include "AbilitySystem/Abilities/QAAbilityTask_WaitTick.h"

UQAAbilityTask_WaitTick* UQAAbilityTask_WaitTick::WaitTick(UGameplayAbility* OwningAbility)
{
	UQAAbilityTask_WaitTick* TickTask = NewAbilityTask<UQAAbilityTask_WaitTick>(OwningAbility);
	return TickTask;
}

void UQAAbilityTask_WaitTick::Activate()
{
	// Set to receive a tick every frame
	bTickingTask = true;
}

void UQAAbilityTask_WaitTick::TickTask(float DeltaTime)
{
	// If the Ability is valid, it calls the OnTick delegate.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}

void UQAAbilityTask_WaitTick::OnDestroy(bool AbilityEnded)
{
	// Stop Tick when AbilityTask is destroyed
	bTickingTask = false;
	Super::OnDestroy(AbilityEnded);
}