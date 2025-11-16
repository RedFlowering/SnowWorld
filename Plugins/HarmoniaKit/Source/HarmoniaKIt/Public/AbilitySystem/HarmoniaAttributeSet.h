// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "HarmoniaAttributeSet.generated.h"

class UObject;
struct FGameplayEffectSpec;

/**
 * Attribute Accessor Macro
 * Creates getter/setter functions for attributes
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

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
 * Attribute set for character attributes (health, stamina, damage, etc.)
 * Used in both combat and non-combat situations (eating, resting, etc.)
 * Based on Lyra's attribute set architecture
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaAttributeSet : public UAttributeSet
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

	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Stamina);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MaxStamina);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, AttackPower);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Defense);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, CriticalChance);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, CriticalDamage);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, MovementSpeed);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, AttackSpeed);

	// Meta Attributes (temporary, not replicated)
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UHarmoniaAttributeSet, Damage);

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

	// Called when damage is received
	mutable FHarmoniaAttributeEvent OnDamageReceived;

	// Called when healing is received
	mutable FHarmoniaAttributeEvent OnHealingReceived;

protected:
	/**
	 * Replication callbacks
	 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);

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
	// Core Attributes
	// ============================================================================

	/**
	 * Current health
	 * Clamped by MaxHealth
	 * Hidden from modifiers - only executions can modify
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Harmonia|Attributes", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	/**
	 * Maximum health
	 * Can be modified by gameplay effects
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

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
	 * Movement speed multiplier
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MovementSpeed;

	/**
	 * Attack speed multiplier
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackSpeed, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AttackSpeed;

	// ============================================================================
	// Meta Attributes
	// These are not replicated and are used for temporary calculations
	// ============================================================================

	/**
	 * Incoming healing
	 * Mapped to +Health
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Healing;

	/**
	 * Incoming damage
	 * Mapped to -Health
	 * Hidden from modifiers - only executions can apply damage
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Attributes", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Damage;

	// ============================================================================
	// Internal State
	// ============================================================================

	// Whether health reached zero
	bool bOutOfHealth = false;

	// Whether stamina reached zero
	bool bOutOfStamina = false;

	// Store values before changes for delta calculations
	float MaxHealthBeforeAttributeChange = 0.0f;
	float HealthBeforeAttributeChange = 0.0f;
	float MaxStaminaBeforeAttributeChange = 0.0f;
	float StaminaBeforeAttributeChange = 0.0f;
};
