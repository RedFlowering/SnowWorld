// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_RangedAttack.h"
#include "Components/HarmoniaRangedCombatComponent.h"
#include "Actors/HarmoniaProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

UHarmoniaGameplayAbility_RangedAttack::UHarmoniaGameplayAbility_RangedAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Set default tags
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Ranged")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Melee")));

	// Block while dead, stunned, etc.
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Stunned")));
}

bool UHarmoniaGameplayAbility_RangedAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if we have ranged combat component
	UHarmoniaRangedCombatComponent* RangedCombat = GetRangedCombatComponent(ActorInfo);
	if (!RangedCombat)
	{
		return false;
	}

	// Check if we can fire
	if (!RangedCombat->CanFire())
	{
		return false;
	}

	// Check stamina
	if (StaminaCost > 0.0f)
	{
		const UHarmoniaAttributeSet* AttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UHarmoniaAttributeSet>();
		if (AttributeSet && AttributeSet->GetStamina() < StaminaCost)
		{
			return false;
		}
	}

	// Check mana
	if (ManaCost > 0.0f)
	{
		const UHarmoniaAttributeSet* AttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UHarmoniaAttributeSet>();
		if (AttributeSet && AttributeSet->GetMana() < ManaCost)
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_RangedAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply activation tags
	if (AttackingTags.Num() > 0)
	{
		ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, UGameplayEffect::StaticClass(), GetAbilityLevel()), FGameplayEffectSpecHandle());
	}

	UHarmoniaRangedCombatComponent* RangedCombat = GetRangedCombatComponent(ActorInfo);
	if (!RangedCombat)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get weapon data to determine behavior
	FHarmoniaRangedWeaponData WeaponData;
	if (RangedCombat->GetCurrentWeaponData(WeaponData))
	{
		// For bows, start charging
		if (WeaponData.WeaponType == EHarmoniaRangedWeaponType::Bow ||
			WeaponData.WeaponType == EHarmoniaRangedWeaponType::Longbow ||
			WeaponData.WeaponType == EHarmoniaRangedWeaponType::Shortbow ||
			WeaponData.WeaponType == EHarmoniaRangedWeaponType::CompositeBow)
		{
			StartCharging();

			// Play draw animation
			if (DrawMontage && ActorInfo->GetAnimInstance())
			{
				ActorInfo->GetAnimInstance()->Montage_Play(DrawMontage);
			}

			return; // Wait for input release
		}
	}

	// For other weapons (crossbow, guns, etc.), fire immediately
	FireProjectile();

	// Play fire animation
	if (FireMontage && ActorInfo->GetAnimInstance())
	{
		ActorInfo->GetAnimInstance()->Montage_Play(FireMontage);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_RangedAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	bIsCharging = false;
	ChargeStartTime = 0.0f;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_RangedAttack::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bIsCharging)
	{
		ReleaseAndFire();
	}
}

void UHarmoniaGameplayAbility_RangedAttack::StartCharging()
{
	bIsCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	UHarmoniaRangedCombatComponent* RangedCombat = GetRangedCombatComponent(GetCurrentActorInfo());
	if (RangedCombat)
	{
		RangedCombat->RequestFire();
	}
}

void UHarmoniaGameplayAbility_RangedAttack::ReleaseAndFire()
{
	if (!bIsCharging)
	{
		return;
	}

	UHarmoniaRangedCombatComponent* RangedCombat = GetRangedCombatComponent(GetCurrentActorInfo());
	if (RangedCombat)
	{
		RangedCombat->RequestRelease();
	}

	// Play fire animation
	if (FireMontage && GetCurrentActorInfo()->GetAnimInstance())
	{
		GetCurrentActorInfo()->GetAnimInstance()->Montage_Play(FireMontage);
	}

	bIsCharging = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_RangedAttack::FireProjectile()
{
	UHarmoniaRangedCombatComponent* RangedCombat = GetRangedCombatComponent(GetCurrentActorInfo());
	if (!RangedCombat)
	{
		return;
	}

	// Request fire from ranged combat component
	RangedCombat->RequestFire();

	// Play camera shake
	if (FireCameraShakeClass)
	{
		ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (Character)
		{
			APlayerController* PC = Cast<APlayerController>(Character->GetController());
			if (PC)
			{
				PC->ClientStartCameraShake(FireCameraShakeClass, CameraShakeScale);
			}
		}
	}
}

UHarmoniaRangedCombatComponent* UHarmoniaGameplayAbility_RangedAttack::GetRangedCombatComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return nullptr;
	}

	return ActorInfo->AvatarActor->FindComponentByClass<UHarmoniaRangedCombatComponent>();
}
