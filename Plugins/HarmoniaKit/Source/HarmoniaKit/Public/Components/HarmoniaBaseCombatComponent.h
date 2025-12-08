// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "HarmoniaBaseCombatComponent.generated.h"

class UAbilitySystemComponent;
class UHarmoniaAttributeSet;
class UHarmoniaSenseAttackComponent;
class UHarmoniaEquipmentComponent;
class UDataTable;
class UGameplayEffect;

/**
 * Base Combat Component
 * 
 * Abstract base class for combat components (Melee and Ranged).
 * Provides common functionality like:
 * - Component caching and references
 * - Stamina management
 * - Ability System integration
 * - Common combat state
 */
UCLASS(Abstract, ClassGroup = (HarmoniaKit))
class HARMONIAKIT_API UHarmoniaBaseCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBaseCombatComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================================
	// Component References
	// ============================================================================

	/** Get the Sense Attack Component (cacheed) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	UHarmoniaSenseAttackComponent* GetAttackComponent() const;

	/** Get the Ability System Component (cached) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/** Get the Attribute Set (cached) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	UHarmoniaAttributeSet* GetAttributeSet() const;

	/** Get the Equipment Component (cached) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	UHarmoniaEquipmentComponent* GetEquipmentComponent() const;

	// ============================================================================
	// Stamina Management
	// ============================================================================

	/** Check if we have enough stamina for an action */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Stamina")
	bool HasEnoughStamina(float StaminaCost) const;

	/** Consume stamina, returns false if not enough stamina */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Stamina")
	bool ConsumeStamina(float StaminaCost);

	/** Get current stamina */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Stamina")
	float GetCurrentStamina() const;

	/** Get maximum stamina */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Stamina")
	float GetMaxStamina() const;

	/** Get stamina percentage (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Stamina")
	float GetStaminaPercentage() const;

	/** Get stamina recovery delay (seconds before recovery starts after consumption) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Stamina")
	float GetStaminaRecoveryDelay() const;

public:
	// Mana Management
	// ============================================================================

	/** Check if we have enough mana for an action */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Mana")
	bool HasEnoughMana(float ManaCost) const;

	/** Consume mana, returns false if not enough mana */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Mana")
	bool ConsumeMana(float ManaCost);

	/** Get current mana */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Mana")
	float GetCurrentMana() const;

	/** Get maximum mana */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Mana")
	float GetMaxMana() const;

	// ============================================================================
	// Data Table Helpers
	// ============================================================================

protected:
	/** Convert enum to row name for data table lookup */
	template<typename TEnum>
	static FName EnumToRowName(TEnum EnumValue);

	/** Get row from data table by enum value */
	template<typename TEnum, typename TRow>
	static bool GetDataTableRowByEnum(const UDataTable* DataTable, TEnum EnumValue, TRow& OutRow);

	// ============================================================================
	// Properties
	// ============================================================================

protected:
	// Cached component references
	UPROPERTY(Transient)
	mutable TObjectPtr<UHarmoniaSenseAttackComponent> CachedAttackComponent;

	UPROPERTY(Transient)
	mutable TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY(Transient)
	mutable TObjectPtr<UHarmoniaAttributeSet> CachedAttributeSet;

	UPROPERTY(Transient)
	mutable TObjectPtr<UHarmoniaEquipmentComponent> CachedEquipmentComponent;
};

// ============================================================================
// Template Implementations
// ============================================================================

template<typename TEnum>
FName UHarmoniaBaseCombatComponent::EnumToRowName(TEnum EnumValue)
{
	FString EnumString = UEnum::GetValueAsString(EnumValue);
	// Remove the "EnumType::" prefix
	int32 Index = EnumString.Find(TEXT("::"));
	if (Index != INDEX_NONE)
	{
		EnumString = EnumString.RightChop(Index + 2);
	}
	return FName(*EnumString);
}

template<typename TEnum, typename TRow>
bool UHarmoniaBaseCombatComponent::GetDataTableRowByEnum(const UDataTable* DataTable, TEnum EnumValue, TRow& OutRow)
{
	if (!DataTable)
	{
		return false;
	}

	FName RowName = EnumToRowName(EnumValue);
	TRow* RowPtr = DataTable->FindRow<TRow>(RowName, TEXT("GetDataTableRowByEnum"));
	if (RowPtr)
	{
		OutRow = *RowPtr;
		return true;
	}

	return false;
}
