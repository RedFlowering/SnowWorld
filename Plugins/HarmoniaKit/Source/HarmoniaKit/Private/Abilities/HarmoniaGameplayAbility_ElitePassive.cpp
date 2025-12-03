// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_ElitePassive.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UHarmoniaGameplayAbility_ElitePassive::UHarmoniaGameplayAbility_ElitePassive()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UHarmoniaGameplayAbility_ElitePassive::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// Auto-activate when avatar is set
	ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
}

void UHarmoniaGameplayAbility_ElitePassive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ApplyEliteBonuses();
	SpawnVisualEffects();

	// Keep ability active (passive)
	// Don't call EndAbility - this should stay active
}

void UHarmoniaGameplayAbility_ElitePassive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove elite effect
	if (EliteEffectHandle.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(EliteEffectHandle);
	}

	// Destroy visual effects
	if (SpawnedVisualEffect)
	{
		SpawnedVisualEffect->Destroy();
		SpawnedVisualEffect = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_ElitePassive::ApplyEliteBonuses()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// Apply elite status effect if specified
	if (EliteStatusEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EliteStatusEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			EliteEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Manually apply stat bonuses
	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (AttributeSet)
	{
		// Boost health
		float CurrentMaxHealth = AttributeSet->GetMaxHealth();
		float NewMaxHealth = CurrentMaxHealth * HealthMultiplier;
		ASC->SetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute(), NewMaxHealth);

		// Set current health to max (elite spawns at full health)
		ASC->SetNumericAttributeBase(AttributeSet->GetHealthAttribute(), NewMaxHealth);

		// Boost attack power
		float CurrentAttackPower = AttributeSet->GetAttackPower();
		float NewAttackPower = CurrentAttackPower * DamageMultiplier;
		ASC->SetNumericAttributeBase(AttributeSet->GetAttackPowerAttribute(), NewAttackPower);

		// Boost movement speed
		float CurrentMovementSpeed = AttributeSet->GetMovementSpeed();
		float NewMovementSpeed = CurrentMovementSpeed * MovementSpeedMultiplier;
		ASC->SetNumericAttributeBase(AttributeSet->GetMovementSpeedAttribute(), NewMovementSpeed);

		// Apply movement speed to character
		ACharacter* Character = Cast<ACharacter>(ASC->GetAvatarActor());
		if (Character && Character->GetCharacterMovement())
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = NewMovementSpeed;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Elite bonuses applied: Health x%.1f, Damage x%.1f, Speed x%.1f"),
		HealthMultiplier, DamageMultiplier, MovementSpeedMultiplier);
}

void UHarmoniaGameplayAbility_ElitePassive::SpawnVisualEffects()
{
	if (!EliteVisualEffect)
	{
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Cast<APawn>(Avatar);

	SpawnedVisualEffect = GetWorld()->SpawnActor<AActor>(EliteVisualEffect, Avatar->GetActorTransform(), SpawnParams);

	if (SpawnedVisualEffect)
	{
		// Attach to avatar
		SpawnedVisualEffect->AttachToActor(Avatar, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}
