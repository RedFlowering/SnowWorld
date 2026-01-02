// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_DiveAttack.h"
#include "AI/HarmoniaJumpPoint.h"
#include "Core/HarmoniaCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_DiveAttack::UHarmoniaGameplayAbility_DiveAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UHarmoniaGameplayAbility_DiveAttack::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check height difference from ground
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		return false;
	}

	// Trace down to find ground
	FHitResult Hit;
	FVector Start = Character->GetActorLocation();
	FVector End = Start - FVector(0.0f, 0.0f, 10000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	if (Character->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		float HeightDiff = Start.Z - Hit.Location.Z;
		if (HeightDiff < MinHeightDifference)
		{
			return false; // Not high enough
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_DiveAttack::ActivateAbility(
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

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get target location from event data if available
	AActor* TargetActor = TriggerEventData ? const_cast<AActor*>(Cast<AActor>(TriggerEventData->Target.Get())) : nullptr;

	if (TargetActor)
	{
		DiveTargetLocation = TargetActor->GetActorLocation();
	}
	else
	{
		// Dive straight down if no target
		FHitResult Hit;
		FVector Start = Character->GetActorLocation();
		FVector End = Start - FVector(0.0f, 0.0f, 10000.0f);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Character);
		
		if (Character->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
		{
			DiveTargetLocation = Hit.Location;
		}
		else
		{
			DiveTargetLocation = Start - FVector(0.0f, 0.0f, 500.0f);
		}
	}

	DiveStartLocation = Character->GetActorLocation();
	
	StartDive();
}

void UHarmoniaGameplayAbility_DiveAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DiveTimerHandle);
	}

	// Remove invulnerable if set
	if (bInvulnerableDuringDive)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"));
			ASC->RemoveLooseGameplayTag(InvulnerableTag);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_DiveAttack::StartDive_Implementation()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Set invulnerable during dive
	if (bInvulnerableDuringDive)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"));
			ASC->AddLooseGameplayTag(InvulnerableTag);
		}
	}

	// Face target direction
	FVector DiveDirection = (DiveTargetLocation - DiveStartLocation).GetSafeNormal2D();
	if (!DiveDirection.IsNearlyZero())
	{
		Character->SetActorRotation(DiveDirection.Rotation());
	}

	// Play dive montage
	if (DiveMontage)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DiveMontage);
		}
	}

	// Calculate dive duration based on distance
	float DiveDistance = FVector::Dist(DiveStartLocation, DiveTargetLocation);
	float DiveDuration = DiveDistance / DiveSpeed;
	DiveDuration = FMath::Max(DiveDuration, 0.5f);

	// Use montage length if longer
	if (DiveMontage)
	{
		DiveDuration = FMath::Max(DiveDuration, DiveMontage->GetPlayLength());
	}

	// Use leaping for dive (negative angle for downward trajectory)
	if (UHarmoniaCharacterMovementComponent* MoveComp = Cast<UHarmoniaCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		MoveComp->StartLeaping(DiveTargetLocation, -30.0f, DiveDuration);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DiveTimerHandle,
			this,
			&UHarmoniaGameplayAbility_DiveAttack::OnDiveMontageComplete,
			DiveDuration,
			false
		);
	}
}

void UHarmoniaGameplayAbility_DiveAttack::OnDiveMontageComplete()
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

	// Remove invulnerable
	if (bInvulnerableDuringDive)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"));
			ASC->RemoveLooseGameplayTag(InvulnerableTag);
		}
	}

	// Camera shake
	if (LandingCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			Character->GetWorld(),
			LandingCameraShake,
			Character->GetActorLocation(),
			CameraShakeRadius,
			CameraShakeRadius * 2.0f
		);
	}

	// Play landing attack montage
	if (LandingAttackMontage)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(LandingAttackMontage);
			
			float AttackDuration = LandingAttackMontage->GetPlayLength();
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					DiveTimerHandle,
					this,
					&UHarmoniaGameplayAbility_DiveAttack::OnLandingAttackComplete,
					AttackDuration,
					false
				);
			}
			return;
		}
	}

	OnLandingAttackComplete();
}

void UHarmoniaGameplayAbility_DiveAttack::BroadcastLanding()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		// Broadcast landing event for AnimNotify damage handling
		OnDiveLanding.Broadcast(Character->GetActorLocation());
	}
}

void UHarmoniaGameplayAbility_DiveAttack::OnLandingAttackComplete()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
