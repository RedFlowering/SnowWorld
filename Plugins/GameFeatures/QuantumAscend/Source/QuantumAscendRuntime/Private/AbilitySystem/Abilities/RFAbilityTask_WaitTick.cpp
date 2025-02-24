// Copyright 2025 RedFlowering.

#include "AbilitySystem/Abilities/RFAbilityTask_WaitTick.h"

URFAbilityTask_WaitTick* URFAbilityTask_WaitTick::WaitTick(UGameplayAbility* OwningAbility)
{
	URFAbilityTask_WaitTick* TickTask = NewAbilityTask<URFAbilityTask_WaitTick>(OwningAbility);
	return TickTask;
}

void URFAbilityTask_WaitTick::Activate()
{
	// Set to receive a tick every frame
	bTickingTask = true;
}

void URFAbilityTask_WaitTick::TickTask(float DeltaTime)
{
	// If the Ability is valid, it calls the OnTick delegate.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}

void URFAbilityTask_WaitTick::OnDestroy(bool AbilityEnded)
{
	// Stop Tick when AbilityTask is destroyed
	bTickingTask = false;
	Super::OnDestroy(AbilityEnded);
}