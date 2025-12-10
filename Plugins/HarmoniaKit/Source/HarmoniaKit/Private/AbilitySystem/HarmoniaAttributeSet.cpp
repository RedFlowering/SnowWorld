// Copyright 2025 Snow Game Studio.

#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UHarmoniaAttributeSet::UHarmoniaAttributeSet()
{
	// Note: Health and MaxHealth are initialized by parent class ULyraHealthSet
	
	// Set default values for stamina attributes
	Stamina = 100.0f;
	MaxStamina = 100.0f;
	StaminaRegenRate = 10.0f; // 10 stamina per second
	StaminaRecoveryDelay = 1.5f; // 1.5 seconds delay before recovery starts
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

	// Ultimate gauge
	UltimateGauge = 0.0f;
	MaxUltimateGauge = 100.0f;
}

void UHarmoniaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Note: Health and MaxHealth replication is handled by parent class ULyraHealthSet
	
	// Stamina attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, StaminaRecoveryDelay, COND_None, REPNOTIFY_Always);
	// Note: StaminaRecovery is a meta attribute, not replicated
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

	// Ultimate gauge
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, UltimateGauge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHarmoniaAttributeSet, MaxUltimateGauge, COND_None, REPNOTIFY_Always);
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
	if (Data.EvaluatedData.Attribute == ULyraHealthSet::GetHealingAttribute())
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
	else if (Data.EvaluatedData.Attribute == ULyraHealthSet::GetDamageAttribute())
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
	else if (Data.EvaluatedData.Attribute == ULyraHealthSet::GetHealthAttribute())
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
	// Handle Stamina Recovery (Meta Attribute - similar to Lyra's Healing)
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == GetStaminaRecoveryAttribute())
	{
		const float LocalRecovery = GetStaminaRecovery();
		SetStaminaRecovery(0.0f);

		if (LocalRecovery > 0.0f)
		{
			const float OldStamina = GetStamina();
			const float NewStamina = FMath::Clamp(OldStamina + LocalRecovery, 0.0f, GetMaxStamina());
			SetStamina(NewStamina);

			OnStaminaRecovered.Broadcast(Instigator, Causer, &Data.EffectSpec, LocalRecovery, OldStamina, NewStamina);

			if (NewStamina > 0.0f)
			{
				bOutOfStamina = false;
			}
		}
	}

	// ============================================================================
	// Handle Max Health Changes
	// ============================================================================
	else if (Data.EvaluatedData.Attribute == ULyraHealthSet::GetMaxHealthAttribute())
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

	// NOTE: Do NOT call SetXXX() here - it causes infinite recursion!
	// Clamping is already handled in PreAttributeChange via ClampAttribute()
	
	// Broadcast stamina change events
	if (Attribute == GetStaminaAttribute())
	{
		OnStaminaChanged.Broadcast(nullptr, nullptr, nullptr, NewValue - OldValue, OldValue, NewValue);
		
		// Apply recovery block effect when stamina decreases
		if (NewValue < OldValue && StaminaRecoveryBlockEffectClass)
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				ContextHandle.AddSourceObject(GetOwningActor());
				
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaRecoveryBlockEffectClass, 1.0f, ContextHandle);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
		
		// Check if stamina reached zero
		if (NewValue <= 0.0f && OldValue > 0.0f)
		{
			OnOutOfStamina.Broadcast(nullptr, nullptr, nullptr, 0.0f, OldValue, NewValue);
		}
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		OnMaxStaminaChanged.Broadcast(nullptr, nullptr, nullptr, NewValue - OldValue, OldValue, NewValue);
	}
	// Broadcast mana change events
	else if (Attribute == GetManaAttribute())
	{
		OnManaChanged.Broadcast(nullptr, nullptr, nullptr, NewValue - OldValue, OldValue, NewValue);
		
		// Check if mana reached zero
		if (NewValue <= 0.0f && OldValue > 0.0f)
		{
			OnOutOfMana.Broadcast(nullptr, nullptr, nullptr, 0.0f, OldValue, NewValue);
		}
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		OnMaxManaChanged.Broadcast(nullptr, nullptr, nullptr, NewValue - OldValue, OldValue, NewValue);
	}
	// Broadcast poise events
	else if (Attribute == GetPoiseAttribute())
	{
		// Check if poise is broken (reached zero)
		if (NewValue <= 0.0f && OldValue > 0.0f)
		{
			OnPoiseBroken.Broadcast(nullptr, nullptr, nullptr, 0.0f, OldValue, NewValue);
		}
	}
}

void UHarmoniaAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// Health attributes - use parent class (ULyraHealthSet) accessors
	if (Attribute == ULyraHealthSet::GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == ULyraHealthSet::GetMaxHealthAttribute())
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
	else if (Attribute == GetStaminaRecoveryDelayAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f); // Delay can be 0 (instant recovery)
	}
	else if (Attribute == GetStaminaRecoveryAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f); // Recovery can't be negative
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
	// Ultimate gauge
	else if (Attribute == GetUltimateGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxUltimateGauge());
	}
	else if (Attribute == GetMaxUltimateGaugeAttribute())
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
// Note: OnRep_Health and OnRep_MaxHealth are handled by parent class ULyraHealthSet
// ============================================================================

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

void UHarmoniaAttributeSet::OnRep_StaminaRecoveryDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, StaminaRecoveryDelay, OldValue);
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

void UHarmoniaAttributeSet::OnRep_UltimateGauge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, UltimateGauge, OldValue);
}

void UHarmoniaAttributeSet::OnRep_MaxUltimateGauge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHarmoniaAttributeSet, MaxUltimateGauge, OldValue);
}
