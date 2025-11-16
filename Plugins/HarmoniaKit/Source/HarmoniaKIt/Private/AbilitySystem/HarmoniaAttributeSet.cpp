// Copyright 2025 Snow Game Studio.

#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UHarmoniaAttributeSet::UHarmoniaAttributeSet()
{
	// Set default values
	Health = 100.0f;
	MaxHealth = 100.0f;
	Stamina = 100.0f;
	MaxStamina = 100.0f;
	AttackPower = 10.0f;
	Defense = 5.0f;
	CriticalChance = 0.1f;
	CriticalDamage = 2.0f;
	MovementSpeed = 1.0f;
	AttackSpeed = 1.0f;
}

void UHarmoniaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, CriticalChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
}

bool UHarmoniaAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// Store pre-change values for damage/healing calculation
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();
	StaminaBeforeAttributeChange = GetStamina();
	MaxStaminaBeforeAttributeChange = GetMaxStamina();

	return true;
}

void UHarmoniaAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	// ============================================================================
	// Handle Healing
	// ============================================================================
	if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		const float LocalHealing = GetHealing();
		SetHealing(0.0f); // Clear meta attribute

		if (LocalHealing > 0.0f)
		{
			const float OldHealth = GetHealth();
			const float NewHealth = FMath::Clamp(OldHealth + LocalHealing, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			// Broadcast healing event
			OnHealingReceived.Broadcast(Instigator, Causer, &Data.EffectSpec, LocalHealing, OldHealth, NewHealth);
		}
	}

	// ============================================================================
	// Handle Damage
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		float LocalDamage = GetDamage();
		SetDamage(0.0f); // Clear meta attribute

		if (LocalDamage > 0.0f)
		{
			// Apply defense reduction
			const float DefenseReduction = GetDefense();
			LocalDamage = FMath::Max(LocalDamage - DefenseReduction, 0.0f);

			const float OldHealth = GetHealth();
			const float NewHealth = FMath::Clamp(OldHealth - LocalDamage, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			// Broadcast damage event
			OnDamageReceived.Broadcast(Instigator, Causer, &Data.EffectSpec, LocalDamage, OldHealth, NewHealth);

			// Trigger hit reaction event if damage was dealt
			if (LocalDamage > 0.0f && Instigator)
			{
				FGameplayEventData HitEventData;
				HitEventData.Instigator = Instigator;
				HitEventData.Target = GetOwningActor();
				HitEventData.EventMagnitude = LocalDamage;

				UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
				if (ASC)
				{
					ASC->HandleGameplayEvent(
						FGameplayTag::RequestGameplayTag(FName("GameplayEvent.HitReaction")),
						&HitEventData
					);
				}
			}

			// Check if health reached zero
			if (NewHealth <= 0.0f && OldHealth > 0.0f)
			{
				bOutOfHealth = true;
				OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, LocalDamage, OldHealth, NewHealth);
			}
		}
	}

	// ============================================================================
	// Handle Health Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float NewHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
		SetHealth(NewHealth);

		if (NewHealth <= 0.0f && HealthBeforeAttributeChange > 0.0f)
		{
			bOutOfHealth = true;
			OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, HealthBeforeAttributeChange, NewHealth);
		}

		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, HealthBeforeAttributeChange, NewHealth);
	}

	// ============================================================================
	// Handle Stamina Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		const float NewStamina = FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina());
		SetStamina(NewStamina);

		if (NewStamina <= 0.0f && StaminaBeforeAttributeChange > 0.0f)
		{
			bOutOfStamina = true;
			OnOutOfStamina.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, StaminaBeforeAttributeChange, NewStamina);
		}
		else if (NewStamina > 0.0f)
		{
			bOutOfStamina = false;
		}

		OnStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, StaminaBeforeAttributeChange, NewStamina);
	}

	// ============================================================================
	// Handle Max Health Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		const float NewMaxHealth = FMath::Max(GetMaxHealth(), 1.0f);
		SetMaxHealth(NewMaxHealth);

		// Adjust current health if it exceeds new max
		if (GetHealth() > NewMaxHealth)
		{
			SetHealth(NewMaxHealth);
		}

		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, MaxHealthBeforeAttributeChange, NewMaxHealth);
	}

	// ============================================================================
	// Handle Max Stamina Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		const float NewMaxStamina = FMath::Max(GetMaxStamina(), 1.0f);
		SetMaxStamina(NewMaxStamina);

		// Adjust current stamina if it exceeds new max
		if (GetStamina() > NewMaxStamina)
		{
			SetStamina(NewMaxStamina);
		}

		OnMaxStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, MaxStaminaBeforeAttributeChange, NewMaxStamina);
	}
}

void UHarmoniaAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHarmoniaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHarmoniaAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// Clamp health to max health
	if (Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	// Clamp stamina to max stamina
	else if (Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	// Ensure max health is always at least 1
	else if (Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(GetMaxHealth(), 1.0f));
	}
	// Ensure max stamina is always at least 1
	else if (Attribute == GetMaxStaminaAttribute())
	{
		SetMaxStamina(FMath::Max(GetMaxStamina(), 1.0f));
	}
}

void UHarmoniaAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetCriticalChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	}
	else if (Attribute == GetCriticalDamageAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetAttackPowerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}
	else if (Attribute == GetAttackSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}
}

UAbilitySystemComponent* UHarmoniaAttributeSet::GetAbilitySystemComponent() const
{
	return GetOwningAbilitySystemComponent();
}

// ============================================================================
// Replication Callbacks
// ============================================================================

void UHarmoniaAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Health, OldValue);
}

void UHarmoniaAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, MaxHealth, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Stamina, OldValue);
}

void UHarmoniaAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, MaxStamina, OldValue);
}

void UHarmoniaAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, AttackPower, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Defense, OldValue);
}

void UHarmoniaAttributeSet::OnRep_CriticalChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, CriticalChance, OldValue);
}

void UHarmoniaAttributeSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, CriticalDamage, OldValue);
}

void UHarmoniaAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, MovementSpeed, OldValue);
}

void UHarmoniaAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, AttackSpeed, OldValue);
}
