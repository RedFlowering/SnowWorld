// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "HarmoniaAttributeSet.generated.h"

class UObject;
struct FGameplayEffectSpec;

// Note: ATTRIBUTE_ACCESSORS macro is already defined in LyraAttributeSet.h
// We use it here for Harmonia-specific attributes

/**
 * Delegate for attribute change events
 * @param EffectInstigator	The actor who initiated the effect
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The gameplay effect spec
 * @param EffectMagnitude	The magnitude before clamping
 * @param OldValue			Previous attribute value
 * @param NewValue			New attribute value
 */
DECLARE_MULTICAST_DELEGATE_SixParams(
	FHarmoniaAttributeEvent,
	AActor* /*EffectInstigator*/,
	AActor* /*EffectCauser*/,
	const FGameplayEffectSpec* /*EffectSpec*/,
	float /*EffectMagnitude*/,
	float /*OldValue*/,
	float /*NewValue*/);

/**
 * UHarmoniaAttributeSet
 *
 * Extended attribute set for Soul-like RPG gameplay.
 * Inherits from ULyraHealthSet to leverage Lyra's Health/MaxHealth/Healing/Damage functionality.
 * 
 * Adds Soul-like specific attributes:
 * - Stamina system (for dodging, attacking, sprinting)
 * - Mana system (for magic abilities)
 * - Primary stats (Vitality, Endurance, Strength, Dexterity, Intelligence, Faith, Luck)
 * - Combat stats (Poise, Critical chance/damage, Attack/Movement speed)
 * - Equipment load system
 *
 * Health and MaxHealth are inherited from ULyraHealthSet - do not redefine here.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaAttributeSet : public ULyraHealthSet
{
	GENERATED_BODY()

public:
	UHarmoniaAttributeSet();

	//~UObject interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface

	//~UAttributeSet interface
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	//~End of UAttributeSet interface

	// ============================================================================
	// Attribute Accessors
	// ============================================================================

	// Note: Health, MaxHealth, Healing, Damage are inherited from ULyraHealthSet
	// and accessed via ULyraHealthSet::GetHealthAttribute() etc.

	// Stamina Attributes
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Stamina);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MaxStamina);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, StaminaRegenRate);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, StaminaRecovery);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, StaminaRecoveryDelay);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Mana);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MaxMana);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, ManaRegenRate);

	// Primary Stats (Soul-like RPG system)
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Vitality);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Endurance);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Strength);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Dexterity);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Intelligence);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Faith);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Luck);

	// Combat Stats
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, AttackPower);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Defense);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, CriticalChance);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, CriticalDamage);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Poise);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MaxPoise);

	// Movement & Equipment
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MovementSpeed);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, AttackSpeed);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, EquipLoad);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MaxEquipLoad);

	// Note: Healing and Damage meta attributes are inherited from ULyraHealthSet

	// ============================================================================
	// Delegates
	// ============================================================================

	// Called when health changes
	mutable FHarmoniaAttributeEvent OnHealthChanged;

	// Called when max health changes
	mutable FHarmoniaAttributeEvent OnMaxHealthChanged;

	// Called when health reaches zero
	mutable FHarmoniaAttributeEvent OnOutOfHealth;

	// Called when stamina changes
	mutable FHarmoniaAttributeEvent OnStaminaChanged;

	// Called when max stamina changes
	mutable FHarmoniaAttributeEvent OnMaxStaminaChanged;

	// Called when stamina reaches zero
	mutable FHarmoniaAttributeEvent OnOutOfStamina;

	// Called when mana changes
	mutable FHarmoniaAttributeEvent OnManaChanged;

	// Called when max mana changes
	mutable FHarmoniaAttributeEvent OnMaxManaChanged;

	// Called when mana reaches zero
	mutable FHarmoniaAttributeEvent OnOutOfMana;

	// Called when poise is broken
	mutable FHarmoniaAttributeEvent OnPoiseBroken;

	// Called when damage is received
	mutable FHarmoniaAttributeEvent OnDamageReceived;

	// Called when healing is received
	mutable FHarmoniaAttributeEvent OnHealingReceived;

	// Called when stamina is recovered
	mutable FHarmoniaAttributeEvent OnStaminaRecovered;

protected:
	/**
	 * Replication callbacks
	 * Note: OnRep_Health and OnRep_MaxHealth are inherited from ULyraHealthSet
	 */
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StaminaRecoveryDelay(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ManaRegenRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Vitality(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Endurance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Dexterity(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Faith(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Luck(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Poise(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxPoise(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_EquipLoad(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxEquipLoad(const FGameplayAttributeData& OldValue);

	/**
	 * Helper function to clamp attribute values
	 */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	/**
	 * Helper function to get ability system component
	 */
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

private:
	// ============================================================================
	// Stamina Attributes
	// Note: Health/MaxHealth/Healing/Damage are inherited from ULyraHealthSet
	// ============================================================================

	/**
	 * Current stamina
	 * Used for attacks, dodges, sprinting, and other activities
	 * Clamped by MaxStamina
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Harmonia|Attributes", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Stamina;

	/**
	 * Maximum stamina
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxStamina;

	/**
	 * Stamina regeneration rate (per second)
	 * Modified by Endurance stat
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaRegenRate, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StaminaRegenRate;

	/**
	 * Meta Attribute: Incoming stamina recovery
	 * Similar to Lyra's Healing attribute - applied to Stamina and then reset to 0
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StaminaRecovery;

	/**
	 * Delay before stamina recovery starts after consumption (in seconds)
	 * When stamina is consumed, recovery is blocked for this duration
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaRecoveryDelay, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StaminaRecoveryDelay;

	/**
	 * Current mana
	 * Used for magic spells and abilities
	 * Clamped by MaxMana
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Harmonia|Attributes", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Mana;

	/**
	 * Maximum mana
	 * Modified by Intelligence stat
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxMana;

	/**
	 * Mana regeneration rate (per second)
	 * Modified by Intelligence stat
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegenRate, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData ManaRegenRate;

	// ============================================================================
	// Primary Stats (Soul-like RPG System)
	// ============================================================================

	/**
	 * Vitality
	 * Increases MaxHealth and physical defense
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vitality, Category = "Harmonia|Attributes|Primary", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Vitality;

	/**
	 * Endurance
	 * Increases MaxStamina, StaminaRegenRate, and EquipLoad
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Endurance, Category = "Harmonia|Attributes|Primary", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Endurance;

	/**
	 * Strength
	 * Increases physical damage and equipment requirements
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Harmonia|Attributes|Primary", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Strength;

	/**
	 * Dexterity
	 * Increases attack speed, critical chance, and fall damage reduction
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Dexterity, Category = "Harmonia|Attributes|Primary", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Dexterity;

	/**
	 * Intelligence
	 * Increases magic damage and magic defense
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Harmonia|Attributes|Primary", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Intelligence;

	/**
	 * Faith
	 * Increases miracles/incantations power and healing effectiveness
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Faith, Category = "Harmonia|Attributes|Primary", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Faith;

	/**
	 * Luck
	 * Increases item discovery and affects status buildup
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Luck, Category = "Harmonia|Attributes|Primary", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Luck;

	// ============================================================================
	// Combat Stats
	// ============================================================================

	/**
	 * Attack power
	 * Base damage multiplier for all attacks
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AttackPower;

	/**
	 * Defense
	 * Reduces incoming damage
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Defense;

	/**
	 * Critical hit chance (0-1)
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalChance, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CriticalChance;

	/**
	 * Critical damage multiplier
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalDamage, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CriticalDamage;

	/**
	 * Poise (stance/stagger resistance)
	 * When poise reaches zero, character is staggered
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Poise, Category = "Harmonia|Attributes", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Poise;

	/**
	 * Maximum poise
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoise, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPoise;

	// ============================================================================
	// Movement & Equipment
	// ============================================================================

	/**
	 * Movement speed multiplier
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MovementSpeed;

	/**
	 * Attack speed multiplier
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackSpeed, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AttackSpeed;

	/**
	 * Current equipment load
	 * Affects movement speed and stamina regeneration
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EquipLoad, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData EquipLoad;

	/**
	 * Maximum equipment load
	 * Modified by Endurance stat
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEquipLoad, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxEquipLoad;

	// Note: Healing and Damage meta attributes are inherited from ULyraHealthSet

	// ============================================================================
	// Internal State
	// ============================================================================

	// Whether health reached zero
	bool bOutOfHealth = false;

	// Whether stamina reached zero
	bool bOutOfStamina = false;

	// Whether mana reached zero
	bool bOutOfMana = false;

	// Whether poise is broken
	bool bPoiseBroken = false;

	// Store values before changes for delta calculations
	float MaxHealthBeforeAttributeChange = 0.0f;
	float HealthBeforeAttributeChange = 0.0f;
	float MaxStaminaBeforeAttributeChange = 0.0f;
	float StaminaBeforeAttributeChange = 0.0f;
	float MaxManaBeforeAttributeChange = 0.0f;
	float ManaBeforeAttributeChange = 0.0f;
	float MaxPoiseBeforeAttributeChange = 0.0f;
	float PoiseBeforeAttributeChange = 0.0f;
};
