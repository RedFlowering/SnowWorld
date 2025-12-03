// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacter.h"
#include "GameplayAbilitySpec.h"
#include "HarmoniaMeleeCharacterExample.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaEquipmentComponent;
class UHarmoniaSenseAttackComponent;
class UHarmoniaLockOnComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Example Melee Character
 * Demonstrates how to set up a character with melee combat system
 *
 * This is a reference implementation showing:
 * - Component setup
 * - Input handling
 * - Ability activation
 * - Weapon switching
 *
 * Usage:
 * - Create a Blueprint child of this class
 * - Assign input actions and mapping context
 * - Configure weapon and combo data tables
 * - Add to level and possess
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaMeleeCharacterExample : public ALyraCharacter
{
	GENERATED_BODY()

public:
	AHarmoniaMeleeCharacterExample(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ============================================================================
	// Components
	// ============================================================================

	/** Melee combat component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee Combat")
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent;

	/** Lock-on targeting component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHarmoniaLockOnComponent> LockOnComponent;

	/** Main hand attack component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee Combat")
	TObjectPtr<UHarmoniaSenseAttackComponent> MainHandAttackComponent;

	/** Off hand attack component (optional, for dual wielding) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee Combat")
	TObjectPtr<UHarmoniaSenseAttackComponent> OffHandAttackComponent;

	// ============================================================================
	// Input
	// ============================================================================

	/** Input Mapping Context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MeleeCombatMappingContext;

	/** Light Attack Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LightAttackAction;

	/** Heavy Attack Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;

	/** Block Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BlockAction;

	/** Parry Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ParryAction;

	/** Dodge Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	/** Lock On Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LockOnAction;

	// ============================================================================
	// Input Callbacks
	// ============================================================================

	/** Called when light attack is triggered */
	UFUNCTION()
	void OnLightAttack(const FInputActionValue& Value);

	/** Called when heavy attack is triggered */
	UFUNCTION()
	void OnHeavyAttack(const FInputActionValue& Value);

	/** Called when block is pressed */
	UFUNCTION()
	void OnBlockStarted(const FInputActionValue& Value);

	/** Called when block is released */
	UFUNCTION()
	void OnBlockCompleted(const FInputActionValue& Value);

	/** Called when parry is triggered */
	UFUNCTION()
	void OnParry(const FInputActionValue& Value);

	/** Called when dodge is triggered */
	UFUNCTION()
	void OnDodge(const FInputActionValue& Value);

	/** Called when lock on is triggered */
	UFUNCTION()
	void OnLockOnToggle(const FInputActionValue& Value);

	// ============================================================================
	// Ability Classes
	// ============================================================================

	/** Light Attack Ability Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<class UHarmoniaGameplayAbility_MeleeAttack> LightAttackAbilityClass;

	/** Heavy Attack Ability Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<class UHarmoniaGameplayAbility_MeleeAttack> HeavyAttackAbilityClass;

	/** Block Ability Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<class UHarmoniaGameplayAbility_Block> BlockAbilityClass;

	/** Parry Ability Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<class UHarmoniaGameplayAbility_Parry> ParryAbilityClass;

	/** Dodge Ability Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<class UHarmoniaGameplayAbility_Dodge> DodgeAbilityClass;

	// ============================================================================
	// Example Functions (Blueprint callable)
	// ============================================================================

	/** Switch to specific weapon type */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	void SwitchWeapon(EHarmoniaMeleeWeaponType NewWeaponType);

	/** Check if can perform action */
	UFUNCTION(BlueprintPure, Category = "Melee Combat")
	bool CanAttack() const;

	UFUNCTION(BlueprintPure, Category = "Melee Combat")
	bool CanBlock() const;

	UFUNCTION(BlueprintPure, Category = "Melee Combat")
	bool CanDodge() const;

	/** Get current combat stats */
	UFUNCTION(BlueprintPure, Category = "Melee Combat")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "Melee Combat")
	int32 GetCurrentComboIndex() const;

	UFUNCTION(BlueprintPure, Category = "Melee Combat")
	EHarmoniaMeleeWeaponType GetCurrentWeaponType() const;

private:
	/** Grant abilities to ASC */
	void GrantMeleeAbilities();

	/** Active block ability handle */
	FGameplayAbilitySpecHandle BlockAbilityHandle;
};
