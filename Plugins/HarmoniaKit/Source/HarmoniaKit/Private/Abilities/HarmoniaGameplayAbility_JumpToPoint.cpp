// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_JumpToPoint.h"
#include "AI/HarmoniaJumpPoint.h"
#include "Core/HarmoniaCharacterMovementComponent.h"
#include "SenseReceiverComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_JumpToPoint::UHarmoniaGameplayAbility_JumpToPoint()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHarmoniaGameplayAbility_JumpToPoint::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Find best jump point
	TargetJumpPoint = FindBestJumpPoint();
	if (!TargetJumpPoint)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Claim the point
	AActor* Avatar = ActorInfo->AvatarActor.Get();
	if (!TargetJumpPoint->ClaimPoint(Avatar))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Start jump
	StartJump();
}

void UHarmoniaGameplayAbility_JumpToPoint::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Release jump point
	if (TargetJumpPoint)
	{
		TargetJumpPoint->ReleasePoint(ActorInfo->AvatarActor.Get());
		TargetJumpPoint = nullptr;
	}

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(JumpTimerHandle);
	}

	// Remove invulnerable tag if set
	if (bInvulnerableDuringJump)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"));
			ASC->RemoveLooseGameplayTag(InvulnerableTag);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AHarmoniaJumpPoint* UHarmoniaGameplayAbility_JumpToPoint::FindBestJumpPoint_Implementation()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return nullptr;
	}

	// Try SenseReceiverComponent (from SenseSystem plugin)
	if (USenseReceiverComponent* SenseComp = Character->FindComponentByClass<USenseReceiverComponent>())
	{
		AHarmoniaJumpPoint* BestPoint = nullptr;
		float BestDistSq = FLT_MAX;
		
		// Get tracked target actors from SenseReceiverComponent
		TArray<AActor*> SensedActors = SenseComp->GetTrackTargetActors();
		
		for (AActor* Actor : SensedActors)
		{
			AHarmoniaJumpPoint* Point = Cast<AHarmoniaJumpPoint>(Actor);
			if (!Point || !Point->IsValidForActor(Character))
			{
				continue;
			}
			
			float DistSq = FVector::DistSquared(Character->GetActorLocation(), Point->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPoint = Point;
			}
		}
		
		if (BestPoint)
		{
			return BestPoint;
		}
	}

	// Fallback: iterate world
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AHarmoniaJumpPoint* BestPoint = nullptr;
	float BestDistSq = SearchRadius * SearchRadius;

	for (TActorIterator<AHarmoniaJumpPoint> It(World); It; ++It)
	{
		AHarmoniaJumpPoint* Point = *It;
		if (!Point || !Point->IsValidForActor(Character))
		{
			continue;
		}

		float DistSq = FVector::DistSquared(Character->GetActorLocation(), Point->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestPoint = Point;
		}
	}

	return BestPoint;
}

void UHarmoniaGameplayAbility_JumpToPoint::StartJump_Implementation()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !TargetJumpPoint)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Set invulnerable during jump
	if (bInvulnerableDuringJump)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"));
			ASC->AddLooseGameplayTag(InvulnerableTag);
		}
	}

	// Play jump montage
	if (JumpMontage)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(JumpMontage);
		}
	}

	// Calculate duration
	float JumpDuration = JumpMontage ? JumpMontage->GetPlayLength() : 1.0f;

	// Start leaping using movement component
	if (UHarmoniaCharacterMovementComponent* MoveComp = Cast<UHarmoniaCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		FVector TargetLocation = TargetJumpPoint->GetLandingLocation();
		MoveComp->StartLeaping(TargetLocation, JumpAngle, JumpDuration);
	}

	// Set timer for jump completion
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			JumpTimerHandle,
			this,
			&UHarmoniaGameplayAbility_JumpToPoint::OnJumpMontageComplete,
			JumpDuration,
			false
		);
	}
}

void UHarmoniaGameplayAbility_JumpToPoint::OnJumpMontageComplete()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Stop leaping
	if (UHarmoniaCharacterMovementComponent* MoveComp = Cast<UHarmoniaCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		MoveComp->StopLeaping();
	}

	// Play landing montage
	if (LandingMontage)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(LandingMontage);
			
			float LandingDuration = LandingMontage->GetPlayLength();
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					JumpTimerHandle,
					this,
					&UHarmoniaGameplayAbility_JumpToPoint::OnLandingComplete,
					LandingDuration,
					false
				);
			}
			return;
		}
	}

	OnLandingComplete();
}

void UHarmoniaGameplayAbility_JumpToPoint::OnLandingComplete()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	// Play post-landing montage if set
	if (PostLandingMontage && Character)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(PostLandingMontage);
			
			float Duration = PostLandingMontage->GetPlayLength();
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					JumpTimerHandle,
					[this]()
					{
						EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
					},
					Duration,
					false
				);
			}
			return;
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
