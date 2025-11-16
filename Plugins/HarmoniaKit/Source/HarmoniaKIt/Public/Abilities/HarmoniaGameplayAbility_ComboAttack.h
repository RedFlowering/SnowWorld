// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_ComboAttack.generated.h"

class UAnimMontage;

/**
 * FComboAttackData
 *
 * Defines data for a single attack in a combo sequence
 */
USTRUCT(BlueprintType)
struct FComboAttackData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name of this combo attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FText DisplayName;

	// Animation montage to play for this combo attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> AttackMontage;

	// Damage multiplier for this combo attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float DamageMultiplier = 1.0f;

	// Attack range multiplier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float RangeMultiplier = 1.0f;

	// GameplayTag that identifies this combo step
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FGameplayTag ComboTag;

	// Duration of the combo window (time allowed to input next attack)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboWindowDuration = 1.0f;

	// Can this combo step be canceled?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bCanBeCanceled = true;
};

/**
 * UHarmoniaGameplayAbility_ComboAttack
 *
 * Gameplay Ability that handles combo attack sequences
 * - Tracks current combo step
 * - Manages combo windows
 * - Resets combo on timeout
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_ComboAttack : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_ComboAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	// Called to perform the actual attack
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	virtual void PerformComboAttack();

	// Called to advance to the next combo step
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	virtual void AdvanceCombo();

	// Called to reset the combo sequence
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	virtual void ResetCombo();

	// Called when montage completes
	UFUNCTION()
	virtual void OnMontageCompleted();

	// Called when montage is cancelled
	UFUNCTION()
	virtual void OnMontageCancelled();

	// Called when montage is interrupted
	UFUNCTION()
	virtual void OnMontageInterrupted();

	// Returns the current combo data
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ability")
	FComboAttackData GetCurrentComboData() const;

	// Returns the next combo data (or empty if at the end)
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ability")
	FComboAttackData GetNextComboData() const;

	// Returns whether there is valid current combo data
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ability")
	bool HasCurrentComboData() const;

	// Returns whether there is valid next combo data
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ability")
	bool HasNextComboData() const;

protected:
	// DataTable containing combo attack data
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UDataTable> ComboDataTable;

	// Ordered list of row names from the DataTable that defines the combo sequence
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FName> ComboSequence;

	// Current combo index (0-based)
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	int32 CurrentComboIndex = 0;

	// Whether the next combo input has been queued
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	bool bNextComboQueued = false;

	// Timer handle for combo window timeout
	FTimerHandle ComboWindowTimerHandle;

	// Timer handle for combo reset
	FTimerHandle ComboResetTimerHandle;

	// Tags to apply while attacking
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGameplayTagContainer AttackingTags;

	// Tags to block while attacking
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGameplayTagContainer BlockedTags;

	// ============================================================================
	// Attack Component Integration
	// ============================================================================

	/**
	 * Name of the attack component to trigger (leave empty to find first)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FName AttackComponentName = NAME_None;

	/**
	 * Whether to trigger attack component on each combo step
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	bool bTriggerAttackComponent = true;
};
