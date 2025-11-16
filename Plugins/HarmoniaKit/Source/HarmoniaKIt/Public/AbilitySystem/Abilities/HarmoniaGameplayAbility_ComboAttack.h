// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "AbilitySystem/Definitions/HarmoniaGameplayTagsDefinitions.h"
#include "HarmoniaGameplayAbility_ComboAttack.generated.h"

class UAnimMontage;
struct FHarmoniaComboAttackData;

/**
 * Gameplay Ability for combo-based melee attacks
 * Supports sequential combo attacks with time windows for combo continuation
 *
 * Features:
 * - Sequential combo attacks triggered by repeated inputs
 * - Configurable combo window duration
 * - Animation montage support per combo step
 * - Damage scaling per combo step
 * - Automatic combo reset on timeout or ability end
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_ComboAttack : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_ComboAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ End UGameplayAbility Interface

	// Performs the current combo attack
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combo")
	virtual void PerformComboAttack();

	// Advances to the next combo in the sequence
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combo")
	virtual void AdvanceCombo();

	// Resets the combo back to the first attack
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combo")
	virtual void ResetCombo();

	// Called when the combo window expires
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combo")
	virtual void OnComboWindowExpired();

	// Called when an attack montage completes
	UFUNCTION()
	virtual void OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	virtual void OnMontageCancelled(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	virtual void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

protected:
	// Current combo index
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Combo")
	int32 CurrentComboIndex;

	// Maximum combo index available
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Combo")
	int32 MaxComboIndex;

	// Whether player has requested next combo during current attack
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Combo")
	bool bNextComboRequested;

	// Whether we're currently in a combo window
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Combo")
	bool bInComboWindow;

	// Whether an attack is currently being performed
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Combo")
	bool bIsAttacking;

	// Timer handle for combo window timeout
	FTimerHandle ComboWindowTimerHandle;

	// Current combo attack data
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Combo")
	FHarmoniaComboAttackData CurrentComboData;

	// All combo attack data loaded from DataTable
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Combo")
	TArray<FHarmoniaComboAttackData> AllComboData;

	// Tag applied during attack execution
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harmonia|Combo")
	FGameplayTag AttackingTag;

	// Tag applied during combo window
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harmonia|Combo")
	FGameplayTag ComboWindowTag;

	// Gameplay effect applied during attack (for damage)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harmonia|Combo")
	TSubclassOf<UGameplayEffect> AttackDamageEffect;

private:
	// Load combo data from DataTable
	void LoadComboData();

	// Get combo data for specific index
	bool GetComboDataForIndex(int32 ComboIndex, FHarmoniaComboAttackData& OutComboData) const;

	// Start the combo window timer
	void StartComboWindowTimer(float Duration);

	// Clear the combo window timer
	void ClearComboWindowTimer();

	// Play attack montage for current combo
	void PlayAttackMontage();

	// Apply damage gameplay effect
	void ApplyAttackDamage();
};
