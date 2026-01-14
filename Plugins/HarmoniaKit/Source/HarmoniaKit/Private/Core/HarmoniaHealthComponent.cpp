// Copyright 2025 Snow Game Studio.

#include "Core/HarmoniaHealthComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "LyraLogChannels.h"
#include "GameplayEffectExtension.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaHealthComponent)

UHarmoniaHealthComponent::UHarmoniaHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HarmoniaAttributeSet = nullptr;
}

void UHarmoniaHealthComponent::InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (!InASC)
	{
		UE_LOG(LogLyra, Error, TEXT("HarmoniaHealthComponent: Cannot initialize for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	// Try to find HarmoniaAttributeSet first
	HarmoniaAttributeSet = InASC->GetSet<UHarmoniaAttributeSet>();
	
	if (HarmoniaAttributeSet)
	{
		// We have HarmoniaAttributeSet - use it for health logic
		UE_LOG(LogLyra, Log, TEXT("HarmoniaHealthComponent: Initialized with HarmoniaAttributeSet for owner [%s]."), *GetNameSafe(Owner));
		
		// Register to listen for Health attribute changes from HarmoniaAttributeSet
		HarmoniaAttributeSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHarmoniaHealthChanged);
		HarmoniaAttributeSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleHarmoniaMaxHealthChanged);
		HarmoniaAttributeSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleHarmoniaOutOfHealth);
		
		// Initialize health to max
		InASC->SetNumericAttributeBase(UHarmoniaAttributeSet::GetHealthAttribute(), HarmoniaAttributeSet->GetMaxHealth());
	}
	else
	{
		// Fallback to parent LyraHealthComponent behavior (uses LyraHealthSet)
		UE_LOG(LogLyra, Warning, TEXT("HarmoniaHealthComponent: HarmoniaAttributeSet not found for owner [%s]. Falling back to LyraHealthSet."), *GetNameSafe(Owner));
		Super::InitializeWithAbilitySystem(InASC);
	}
}

float UHarmoniaHealthComponent::GetHarmoniaHealth() const
{
	return (HarmoniaAttributeSet ? HarmoniaAttributeSet->GetHealth() : Super::GetHealth());
}

float UHarmoniaHealthComponent::GetHarmoniaMaxHealth() const
{
	return (HarmoniaAttributeSet ? HarmoniaAttributeSet->GetMaxHealth() : Super::GetMaxHealth());
}

float UHarmoniaHealthComponent::GetHarmoniaHealthNormalized() const
{
	if (HarmoniaAttributeSet)
	{
		const float Health = HarmoniaAttributeSet->GetHealth();
		const float MaxHealth = HarmoniaAttributeSet->GetMaxHealth();
		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}
	return Super::GetHealthNormalized();
}

float UHarmoniaHealthComponent::GetStamina() const
{
	return (HarmoniaAttributeSet ? HarmoniaAttributeSet->GetStamina() : 0.0f);
}

float UHarmoniaHealthComponent::GetMaxStamina() const
{
	return (HarmoniaAttributeSet ? HarmoniaAttributeSet->GetMaxStamina() : 0.0f);
}

float UHarmoniaHealthComponent::GetStaminaNormalized() const
{
	if (HarmoniaAttributeSet)
	{
		const float Stamina = HarmoniaAttributeSet->GetStamina();
		const float MaxStamina = HarmoniaAttributeSet->GetMaxStamina();
		return ((MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f);
	}
	return 0.0f;
}

void UHarmoniaHealthComponent::HandleHarmoniaHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	// Broadcast through parent's delegate for compatibility
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UHarmoniaHealthComponent::HandleHarmoniaMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UHarmoniaHealthComponent::HandleHarmoniaOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	// Only trigger death if not already dead
	if (GetDeathState() != ELyraDeathState::NotDead)
	{
		return;
	}

	// Try to activate GA_Death ability via tag
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer DeathTags;
		DeathTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Death")));
		
		if (AbilitySystemComponent->TryActivateAbilitiesByTag(DeathTags))
		{
			// GA_Death was activated successfully
			return;
		}
	}

	// Fallback: If no GA_Death ability is available, use parent's death handling
	StartDeath();
}
