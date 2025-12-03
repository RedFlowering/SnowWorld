// Copyright 2025 Snow Game Studio.

#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UHarmoniaAttributeSet::UHarmoniaAttributeSet()
{
	// Set default values for core attributes
	Health = 100.0f;
	MaxHealth = 100.0f;
	Stamina = 100.0f;
	MaxStamina = 100.0f;
	StaminaRegenRate = 10.0f; // 10 stamina per second
	Mana = 50.0f;
	MaxMana = 50.0f;
	ManaRegenRate = 5.0f; // 5 mana per second

	// Primary stats (Soul-like RPG system) - start at 10 each
	Vitality = 10.0f;
	Endurance = 10.0f;
	Strength = 10.0f;
	Dexterity = 10.0f;
	Intelligence = 10.0f;
	Faith = 10.0f;
	Luck = 10.0f;

	// Combat stats
	AttackPower = 10.0f;
	Defense = 5.0f;
	CriticalChance = 0.1f;  // 10% base crit chance
	CriticalDamage = 2.0f;  // 2x damage on crit
	Poise = 50.0f;
	MaxPoise = 50.0f;

	// Movement & equipment
	MovementSpeed = 1.0f;
	AttackSpeed = 1.0f;
	EquipLoad = 0.0f;
	MaxEquipLoad = 100.0f;
}

void UHarmoniaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Core attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, ManaRegenRate, COND_None, REPNOTIFY_Always);

	// Primary stats
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Vitality, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Endurance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Faith, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Luck, COND_None, REPNOTIFY_Always);

	// Combat stats
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, CriticalChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Poise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);

	// Movement & equipment
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, EquipLoad, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxEquipLoad, COND_None, REPNOTIFY_Always);
}

bool UHarmoniaAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// Store pre-change values for damage/healing/stamina/mana calculation
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();
	StaminaBeforeAttributeChange = GetStamina();
	MaxStaminaBeforeAttributeChange = GetMaxStamina();
	ManaBeforeAttributeChange = GetMana();
	MaxManaBeforeAttributeChange = GetMaxMana();
	PoiseBeforeAttributeChange = GetPoise();
	MaxPoiseBeforeAttributeChange = GetMaxPoise();

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

	// ============================================================================
	// Handle Mana Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		const float NewMana = FMath::Clamp(GetMana(), 0.0f, GetMaxMana());
		SetMana(NewMana);

		if (NewMana <= 0.0f && ManaBeforeAttributeChange > 0.0f)
		{
			bOutOfMana = true;
			OnOutOfMana.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, ManaBeforeAttributeChange, NewMana);
		}
		else if (NewMana > 0.0f)
		{
			bOutOfMana = false;
		}

		OnManaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, ManaBeforeAttributeChange, NewMana);
	}

	// ============================================================================
	// Handle Max Mana Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetMaxManaAttribute())
	{
		const float NewMaxMana = FMath::Max(GetMaxMana(), 1.0f);
		SetMaxMana(NewMaxMana);

		// Adjust current mana if it exceeds new max
		if (GetMana() > NewMaxMana)
		{
			SetMana(NewMaxMana);
		}

		OnMaxManaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, MaxManaBeforeAttributeChange, NewMaxMana);
	}

	// ============================================================================
	// Handle Poise Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
	{
		const float NewPoise = FMath::Clamp(GetPoise(), 0.0f, GetMaxPoise());
		SetPoise(NewPoise);

		if (NewPoise <= 0.0f && PoiseBeforeAttributeChange > 0.0f)
		{
			bPoiseBroken = true;
			OnPoiseBroken.Broadcast(Instigator, Causer, &Data.EffectSpec, 0.0f, PoiseBeforeAttributeChange, NewPoise);
		}
		else if (NewPoise > 0.0f)
		{
			bPoiseBroken = false;
		}
	}

	// ============================================================================
	// Handle Max Poise Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetMaxPoiseAttribute())
	{
		const float NewMaxPoise = FMath::Max(GetMaxPoise(), 1.0f);
		SetMaxPoise(NewMaxPoise);

		// Adjust current poise if it exceeds new max
		if (GetPoise() > NewMaxPoise)
		{
			SetPoise(NewMaxPoise);
		}
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
	// Clamp mana to max mana
	else if (Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
	// Clamp poise to max poise
	else if (Attribute == GetPoiseAttribute())
	{
		SetPoise(FMath::Clamp(GetPoise(), 0.0f, GetMaxPoise()));
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
	// Ensure max mana is always at least 1
	else if (Attribute == GetMaxManaAttribute())
	{
		SetMaxMana(FMath::Max(GetMaxMana(), 1.0f));
	}
	// Ensure max poise is always at least 1
	else if (Attribute == GetMaxPoiseAttribute())
	{
		SetMaxPoise(FMath::Max(GetMaxPoise(), 1.0f));
	}
}

void UHarmoniaAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// Core attributes
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
	else if (Attribute == GetStaminaRegenRateAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetManaRegenRateAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}

	// Primary stats - minimum 1, maximum 99 (typical soul-like cap)
	else if (Attribute == GetVitalityAttribute() || Attribute == GetEnduranceAttribute() ||
	         Attribute == GetStrengthAttribute() || Attribute == GetDexterityAttribute() ||
	         Attribute == GetIntelligenceAttribute() || Attribute == GetFaithAttribute() ||
	         Attribute == GetLuckAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.0f, 99.0f);
	}

	// Combat stats
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
	else if (Attribute == GetPoiseAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoise());
	}
	else if (Attribute == GetMaxPoiseAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}

	// Movement & equipment
	else if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}
	else if (Attribute == GetAttackSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}
	else if (Attribute == GetEquipLoadAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEquipLoad());
	}
	else if (Attribute == GetMaxEquipLoadAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
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

void UHarmoniaAttributeSet::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, StaminaRegenRate, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Mana, OldValue);
}

void UHarmoniaAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, MaxMana, OldValue);
}

void UHarmoniaAttributeSet::OnRep_ManaRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, ManaRegenRate, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Vitality(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Vitality, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Endurance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Endurance, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Strength, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Dexterity, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Intelligence, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Faith(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Faith, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Luck(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Luck, OldValue);
}

void UHarmoniaAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, Poise, OldValue);
}

void UHarmoniaAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, MaxPoise, OldValue);
}

void UHarmoniaAttributeSet::OnRep_EquipLoad(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, EquipLoad, OldValue);
}

void UHarmoniaAttributeSet::OnRep_MaxEquipLoad(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, MaxEquipLoad, OldValue);
}
