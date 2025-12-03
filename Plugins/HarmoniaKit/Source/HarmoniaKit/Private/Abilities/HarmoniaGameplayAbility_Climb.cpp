// Copyright RedFlowering. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_Climb.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

UHarmoniaGameplayAbility_Climb::UHarmoniaGameplayAbility_Climb(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// ============================================================================
	// Tag Configuration: Do NOT hardcode tags in constructor!
	// Configure these in Blueprint or derived class CDO:
	// ============================================================================
	// ActivationOwnedTags:
	//   - State.Climbing
	// BlockAbilitiesWithTag:
	//   - State.Combat.Attacking
	//   - State.Mounted
	//   - State.Swimming
	// ============================================================================
}

void UHarmoniaGameplayAbility_Climb::ActivateAbility(
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

	// Set movement mode to flying (for climbing)
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->SetMovementMode(MOVE_Flying);
		MovementComp->MaxFlySpeed = ClimbingSpeed;
	}

	// Send climbing started event
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_Climbing_Started, &EventData);
	}
}

void UHarmoniaGameplayAbility_Climb::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		// Restore normal movement
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (MovementComp)
		{
			MovementComp->SetMovementMode(MOVE_Walking);
		}
	}

	// Send climbing stopped event
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_Climbing_Stopped, &EventData);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHarmoniaGameplayAbility_Climb::CanActivateAbility(
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

	// Check for climbing restrictions
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::Movement_Restricted_NoClimb))
		{
			return false;
		}
	}

	// Check if there's a climbable surface nearby
	FVector SurfaceNormal;
	if (!FindClimbableSurface(ActorInfo->AvatarActor.Get(), SurfaceNormal))
	{
		return false;
	}

	return true;
}

void UHarmoniaGameplayAbility_Climb::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActivationInfo.GetActivationPredictionKey().IsValidForMorePrediction())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

bool UHarmoniaGameplayAbility_Climb::FindClimbableSurface(const AActor* Avatar, FVector& OutSurfaceNormal) const
{
	if (!Avatar || !Avatar->GetWorld())
	{
		return false;
	}

	FVector Start = Avatar->GetActorLocation();
	FVector Forward = Avatar->GetActorForwardVector();
	FVector End = Start + (Forward * DetectionRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);

	if (Avatar->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		// Check if surface is climbable (has appropriate tag or angle)
		if (HitResult.Component.IsValid())
		{
			OutSurfaceNormal = HitResult.Normal;

			// Check surface angle
			float Angle = FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector));
			Angle = FMath::RadiansToDegrees(Angle);

			// Surface should be roughly vertical (80-100 degrees from up)
			if (Angle >= 80.0f - MaxClimbingAngle && Angle <= 100.0f + MaxClimbingAngle)
			{
				return true;
			}
		}
	}

	return false;
}

void UHarmoniaGameplayAbility_Climb::ApplyClimbingMovement(float DeltaTime)
{
	// This would be called from Tick if needed
	// For now, climbing movement is handled by the character movement component
}
