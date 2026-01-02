// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_BossAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

UHarmoniaGameplayAbility_BossAttack::UHarmoniaGameplayAbility_BossAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MontagePlayRate = 1.0f;
	DamageMultiplier = 1.0f;
	bEndAbilityOnMontageComplete = true;
}

void UHarmoniaGameplayAbility_BossAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossAttack: No AttackMontage set for %s"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// NOTE: Rotation is handled by Motion Warping AnimNotifyState in the montage
	// - For players: LockOnComponent sets the warp target
	// - For AI: BT sets TargetActor in Blackboard, which is used by MotionWarpingComponent

	// Play montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		MontagePlayRate,
		MontageStartSection
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_BossAttack::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_BossAttack::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_BossAttack::OnMontageInterrupted);
		MontageTask->OnBlendOut.AddDynamic(this, &UHarmoniaGameplayAbility_BossAttack::OnMontageBlendOut);
		MontageTask->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BossAttack: Failed to create montage task for %s"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UHarmoniaGameplayAbility_BossAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_BossAttack::OnMontageCompleted()
{
	if (bEndAbilityOnMontageComplete)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UHarmoniaGameplayAbility_BossAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_BossAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_BossAttack::OnMontageBlendOut()
{
	// Optional: 블렌드 아웃 시 처리
	// 일반적으로 OnMontageCompleted에서 처리하므로 여기서는 아무것도 하지 않음
}
