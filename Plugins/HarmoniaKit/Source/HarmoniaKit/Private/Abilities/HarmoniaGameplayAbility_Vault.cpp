// Copyright RedFlowering. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_Vault.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UHarmoniaGameplayAbility_Vault::UHarmoniaGameplayAbility_Vault(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationOwnedTags.AddTag(HarmoniaGameplayTags::State_Vaulting);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Combat_Attacking);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Mounted);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Climbing);
}

void UHarmoniaGameplayAbility_Vault::ActivateAbility(
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

	// Find vault location
	FVector VaultLocation;
	if (!FindVaultableObstacle(Character, VaultLocation))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Send vault event
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayEventData EventData;
		EventData.TargetLocation = VaultLocation;
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_Parkour_Vault, &EventData);
	}

	// Play vault animation
	if (VaultMontage)
	{
		if (Character->GetMesh())
		{
			UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(VaultMontage);
				AnimInstance->Montage_SetEndDelegate(
					FOnMontageEnded::CreateUObject(this, &UHarmoniaGameplayAbility_Vault::OnMontageCompleted),
					VaultMontage
				);
			}
		}
	}
	else
	{
		// No animation, complete immediately
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}

	// Launch character forward
	FVector LaunchVelocity = Character->GetActorForwardVector() * VaultDistance;
	LaunchVelocity.Z = 400.0f; // Add some upward velocity
	Character->LaunchCharacter(LaunchVelocity, false, false);
}

void UHarmoniaGameplayAbility_Vault::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHarmoniaGameplayAbility_Vault::CanActivateAbility(
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

	// Check if moving fast enough
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (MovementComp)
		{
			float Speed = MovementComp->Velocity.Size2D();
			if (Speed < 200.0f) // Minimum speed to vault
			{
				return false;
			}
		}
	}

	// Check if there's a vaultable obstacle
	FVector VaultLocation;
	if (!FindVaultableObstacle(ActorInfo->AvatarActor.Get(), VaultLocation))
	{
		return false;
	}

	return true;
}

void UHarmoniaGameplayAbility_Vault::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UHarmoniaGameplayAbility_Vault::FindVaultableObstacle(const AActor* Avatar, FVector& OutVaultLocation) const
{
	if (!Avatar || !Avatar->GetWorld())
	{
		return false;
	}

	FVector Start = Avatar->GetActorLocation();
	FVector Forward = Avatar->GetActorForwardVector();
	FVector End = Start + (Forward * DetectionRange);

	// Trace at character height to find obstacles
	Start.Z += 50.0f;
	End.Z += 50.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);

	if (Avatar->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		// Check if obstacle is the right height (not too tall, not too short)
		float ObstacleHeight = HitResult.Location.Z - Avatar->GetActorLocation().Z;

		if (ObstacleHeight > 50.0f && ObstacleHeight < MaxVaultHeight)
		{
			// Calculate vault target location (on top or beyond the obstacle)
			OutVaultLocation = HitResult.Location + (Forward * 100.0f);
			OutVaultLocation.Z += 100.0f;
			return true;
		}
	}

	return false;
}
