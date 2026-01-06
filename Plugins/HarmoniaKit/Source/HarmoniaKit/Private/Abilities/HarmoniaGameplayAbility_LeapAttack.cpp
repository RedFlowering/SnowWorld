// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_LeapAttack.h"
#include "Core/HarmoniaCharacterMovementComponent.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "MotionWarpingComponent.h"

UHarmoniaGameplayAbility_LeapAttack::UHarmoniaGameplayAbility_LeapAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UHarmoniaGameplayAbility_LeapAttack::CanActivateAbility(
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

	// Need movement component
	if (!GetHarmoniaMovementComponent())
	{
		return false;
	}

	// Don't leap while already leaping
	UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent();
	if (MoveComp && MoveComp->IsLeaping())
	{
		return false;
	}

	return true;
}

void UHarmoniaGameplayAbility_LeapAttack::ActivateAbility(
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

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("LeapAttack: No HarmoniaMovementComponent"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get target location (from MotionWarpingComponent or event data)
	CachedTargetLocation = GetLeapTargetLocation();

	// Clamp distance
	FVector StartLocation = Character->GetActorLocation();
	FVector ToTarget = CachedTargetLocation - StartLocation;
	float Distance = ToTarget.Size2D();

	if (Distance < MinLeapDistance)
	{
		// Too close, use minimum distance in forward direction
		CachedTargetLocation = StartLocation + Character->GetActorForwardVector() * MinLeapDistance;
	}
	else if (Distance > MaxLeapDistance)
	{
		// Too far, clamp to max distance
		FVector Direction = ToTarget.GetSafeNormal2D();
		CachedTargetLocation = StartLocation + Direction * MaxLeapDistance;
		CachedTargetLocation.Z = StartLocation.Z; // Keep same height for landing
	}

	// Apply target offset (land in front of target, not on top)
	if (TargetLocationOffset > 0.0f)
	{
		FVector Direction = (CachedTargetLocation - StartLocation).GetSafeNormal2D();
		CachedTargetLocation -= Direction * TargetLocationOffset;
	}

	// Calculate leap duration from montage length
	float LeapDuration = 1.0f;
	if (LeapMontage)
	{
		LeapDuration = LeapMontage->GetPlayLength();
	}

	// Start leaping (CustomMovementMode)
	MoveComp->StartLeaping(CachedTargetLocation, LeapAngle, LeapDuration);

	// Play montage
	if (LeapMontage)
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UHarmoniaGameplayAbility_LeapAttack::OnMontageCompleted);

				FOnMontageBlendingOutStarted BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &UHarmoniaGameplayAbility_LeapAttack::OnMontageBlendingOut);

				AnimInstance->Montage_Play(LeapMontage);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, LeapMontage);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, LeapMontage);
			}
		}
	}
	else
	{
		// No montage - end ability after leap duration using timer
		if (UWorld* World = GetWorld())
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(
				TimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo]()
				{
					EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
				}),
				LeapDuration,
				false
			);
		}
	}
}

void UHarmoniaGameplayAbility_LeapAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Stop leaping if still in progress
	if (UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent())
	{
		if (MoveComp->IsLeaping())
		{
			MoveComp->StopLeaping();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_LeapAttack::OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bInterrupted);
}

void UHarmoniaGameplayAbility_LeapAttack::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// Stop leaping when blend out starts
	if (UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent())
	{
		MoveComp->StopLeaping();
	}
}

FVector UHarmoniaGameplayAbility_LeapAttack::GetLeapTargetLocation() const
{
	// Try to get target from MotionWarpingComponent first
	if (UMotionWarpingComponent* WarpComp = GetMotionWarpingComponent())
	{
		const FMotionWarpingTarget* WarpTarget = WarpComp->FindWarpTarget(FName("AttackTarget"));
		if (WarpTarget)
		{
			return WarpTarget->GetLocation();
		}
	}

	// Fallback: use forward direction
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		return Character->GetActorLocation() + Character->GetActorForwardVector() * MaxLeapDistance * 0.5f;
	}

	return FVector::ZeroVector;
}

UHarmoniaCharacterMovementComponent* UHarmoniaGameplayAbility_LeapAttack::GetHarmoniaMovementComponent() const
{
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		return Cast<UHarmoniaCharacterMovementComponent>(Character->GetCharacterMovement());
	}
	return nullptr;
}

UMotionWarpingComponent* UHarmoniaGameplayAbility_LeapAttack::GetMotionWarpingComponent() const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		return Avatar->FindComponentByClass<UMotionWarpingComponent>();
	}
	return nullptr;
}
