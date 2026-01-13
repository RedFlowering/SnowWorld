// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_BossAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "MotionWarpingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "HarmoniaLogCategories.h"

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
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("BossAttack: No AttackMontage set for %s"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Reset hit tracking for this new attack
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (UHarmoniaMeleeCombatComponent* MeleeCombat = AvatarActor->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
		{
			MeleeCombat->ResetHitTracking();
		}
	}

	// Setup Motion Warping target before playing montage
	SetupMotionWarpingTarget();

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
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("BossAttack: Failed to create montage task for %s"), *GetName());
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

void UHarmoniaGameplayAbility_BossAttack::SetupMotionWarpingTarget()
{
	if (!bEnableMotionWarping)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("BossAttack: Motion Warping disabled for %s"), *GetName());
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("BossAttack: No Avatar Actor for %s"), *GetName());
		return;
	}

	// Get Motion Warping Component from the boss
	UMotionWarpingComponent* WarpComp = AvatarActor->FindComponentByClass<UMotionWarpingComponent>();
	if (!WarpComp)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("BossAttack: No MotionWarpingComponent on %s"), *AvatarActor->GetName());
		return;
	}

	// Get target from AI Blackboard
	AActor* TargetActor = nullptr;
	if (APawn* Pawn = Cast<APawn>(AvatarActor))
	{
		if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
			{
				// Note: AI Controller sets "Target", not "TargetActor"
				TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
			}
		}
	}

	if (!TargetActor)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("BossAttack: No TargetActor in Blackboard for %s"), *GetName());
		return;
	}

	// Use component tracking for automatic target following during animation
	// The animation's Motion Warping settings (Warp To Feet Location offset) handle the safe distance
	if (USceneComponent* TargetRoot = TargetActor->GetRootComponent())
	{
		WarpComp->AddOrUpdateWarpTargetFromComponent(
			WarpTargetName,
			TargetRoot,
			NAME_None,              // No specific bone
			true,                   // Follow component
			FVector::ZeroVector,    // Location offset
			FRotator::ZeroRotator   // Rotation offset
		);
		
		UE_LOG(LogHarmoniaCombat, Log, TEXT("BossAttack: Motion Warping target set to %s"), *TargetActor->GetName());
	}
}
