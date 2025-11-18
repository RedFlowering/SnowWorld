// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaMeleeCombatComponent.generated.h"

class UHarmoniaAttributeSet;
class UAbilitySystemComponent;
class UHarmoniaEquipmentComponent;
class UHarmoniaSenseAttackComponent;
class UDataTable;

/**
 * Melee Combat Component
 * Manages melee combat for Soul-like gameplay including:
 * - Weapon-based combat
 * - Combo system
 * - Blocking/Parrying
 * - Dodging
 * - Stamina management
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMeleeCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaMeleeCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Weapon Management
	// ============================================================================

	/** Get current equipped weapon type */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	EHarmoniaMeleeWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }

	/** Set current weapon type (called by equipment system) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	void SetCurrentWeaponType(EHarmoniaMeleeWeaponType NewWeaponType);

	/** Get weapon data for current weapon */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	bool GetCurrentWeaponData(FHarmoniaMeleeWeaponData& OutWeaponData) const;

	/** Get weapon data for specific weapon type */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	bool GetWeaponDataForType(EHarmoniaMeleeWeaponType WeaponType, FHarmoniaMeleeWeaponData& OutWeaponData) const;

	// ============================================================================
	// Combat State
	// ============================================================================

	/** Is currently attacking? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	bool IsAttacking() const { return bIsAttacking; }

	/** Is currently blocking? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	bool IsBlocking() const { return DefenseState == EHarmoniaDefenseState::Blocking; }

	/** Is currently parrying? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	bool IsParrying() const { return DefenseState == EHarmoniaDefenseState::Parrying; }

	/** Is currently dodging? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	bool IsDodging() const { return DefenseState == EHarmoniaDefenseState::Dodging; }

	/** Get current defense state */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	EHarmoniaDefenseState GetDefenseState() const { return DefenseState; }

	/** Set defense state (called by abilities) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	void SetDefenseState(EHarmoniaDefenseState NewState);

	/** Is currently in i-frames? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	bool IsInvulnerable() const { return bInIFrames; }

	/** Set invulnerability state */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	void SetInvulnerable(bool bInvulnerable, float Duration = 0.0f);

	// ============================================================================
	// Combo System
	// ============================================================================

	/** Get current combo index */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	int32 GetCurrentComboIndex() const { return CurrentComboIndex; }

	/** Get maximum combo count for current weapon */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	int32 GetMaxComboCount() const;

	/** Advance to next combo step */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void AdvanceCombo();

	/** Reset combo to beginning */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void ResetCombo();

	/** Is combo window active? (can queue next attack) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	bool IsInComboWindow() const;

	/** Queue next combo attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void QueueNextCombo();

	/** Is next combo queued? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	bool IsNextComboQueued() const { return bNextComboQueued; }

	/** Clear queued combo */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void ClearQueuedCombo() { bNextComboQueued = false; }

	/** Get combo sequence for current weapon */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	bool GetComboSequence(bool bHeavyCombo, FHarmoniaComboAttackSequence& OutSequence) const;

	// ============================================================================
	// Attack Execution
	// ============================================================================

	/** Request light attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Attack")
	bool RequestLightAttack();

	/** Request heavy attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Attack")
	bool RequestHeavyAttack();

	/** Start attack (called by ability) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Attack")
	void StartAttack(bool bHeavyAttack);

	/** End attack (called by ability) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Attack")
	void EndAttack();

	/** Can currently perform attack? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Attack")
	bool CanAttack() const;

	/** Get stamina cost for light attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Attack")
	float GetLightAttackStaminaCost() const;

	/** Get stamina cost for heavy attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Attack")
	float GetHeavyAttackStaminaCost() const;

	// ============================================================================
	// Defense
	// ============================================================================

	/** Can currently block? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	bool CanBlock() const;

	/** Can currently parry? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	bool CanParry() const;

	/** Can currently dodge? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	bool CanDodge() const;

	/** Get block damage reduction */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	float GetBlockDamageReduction() const;

	/** Get block stamina cost */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	float GetBlockStaminaCost() const;

	/** Process blocked attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	void OnAttackBlocked(AActor* Attacker, float Damage);

	/** Process successful parry */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	void OnParrySuccess(AActor* Attacker);

	// ============================================================================
	// Riposte System
	// ============================================================================

	/** Can currently riposte? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	bool CanRiposte() const;

	/** Get riposte window duration */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	float GetRiposteWindowDuration() const;

	/** Get riposte damage multiplier */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	float GetRiposteDamageMultiplier() const;

	/** Start riposte window */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	void StartRiposteWindow(AActor* ParriedTarget, float Duration);

	/** End riposte window */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	void EndRiposteWindow();

	/** Get parried target */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	AActor* GetParriedTarget() const;

	/** Clear parried target */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	void ClearParriedTarget();

	// ============================================================================
	// Backstab System
	// ============================================================================

	/** Check if attack is a backstab */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Backstab")
	bool IsBackstabAttack(AActor* Target, FVector AttackOrigin) const;

	/** Get backstab damage multiplier */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Backstab")
	float GetBackstabDamageMultiplier() const;

	// ============================================================================
	// Stamina Management
	// ============================================================================

	/** Check if has enough stamina */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Stamina")
	bool HasEnoughStamina(float StaminaCost) const;

	/** Consume stamina */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Stamina")
	bool ConsumeStamina(float StaminaCost);

	/** Get current stamina */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Stamina")
	float GetCurrentStamina() const;

	/** Get maximum stamina */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Stamina")
	float GetMaxStamina() const;

	// ============================================================================
	// Component References
	// ============================================================================

	/** Get or find attack component */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	UHarmoniaSenseAttackComponent* GetAttackComponent() const;

	/** Get or find ability system component */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/** Get or find attribute set */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	UHarmoniaAttributeSet* GetAttributeSet() const;

	/** Get or find equipment component */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	UHarmoniaEquipmentComponent* GetEquipmentComponent() const;

protected:
	// ============================================================================
	// Data Tables
	// ============================================================================

	/** Weapon data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Data")
	TObjectPtr<UDataTable> WeaponDataTable;

	/** Combo sequences data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Data")
	TObjectPtr<UDataTable> ComboSequencesDataTable;

	// ============================================================================
	// Default Configurations
	// ============================================================================

	/** Default defense configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Defense")
	FHarmoniaDefenseConfig DefaultDefenseConfig;

	/** Default dodge configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Dodge")
	FHarmoniaDodgeConfig DefaultDodgeConfig;

	/** Default riposte configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Riposte")
	FHarmoniaRiposteConfig DefaultRiposteConfig;

	/** Default backstab configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Backstab")
	FHarmoniaBackstabConfig DefaultBackstabConfig;

	// ============================================================================
	// Gameplay Tags
	// ============================================================================

	/** Tag applied while attacking */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag AttackingTag;

	/** Tag applied while blocking */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag BlockingTag;

	/** Tag applied while parrying */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag ParryingTag;

	/** Tag applied while dodging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag DodgingTag;

	/** Tag applied during i-frames */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag InvulnerableTag;

	// ============================================================================
	// State
	// ============================================================================

	/** Current equipped weapon type */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	EHarmoniaMeleeWeaponType CurrentWeaponType = EHarmoniaMeleeWeaponType::Fist;

	/** Current defense state */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	EHarmoniaDefenseState DefenseState = EHarmoniaDefenseState::None;

	/** Is currently attacking? */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	bool bIsAttacking = false;

	/** Is current attack heavy? */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	bool bIsHeavyAttack = false;

	/** Current combo index */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	int32 CurrentComboIndex = 0;

	/** Is next combo queued? */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	bool bNextComboQueued = false;

	/** Is in i-frames? */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	bool bInIFrames = false;

	/** Parried target (for riposte) */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	TWeakObjectPtr<AActor> ParriedTarget;

	// ============================================================================
	// Cached References
	// ============================================================================

	UPROPERTY()
	mutable TObjectPtr<UHarmoniaSenseAttackComponent> CachedAttackComponent;

	UPROPERTY()
	mutable TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY()
	mutable TObjectPtr<UHarmoniaAttributeSet> CachedAttributeSet;

	UPROPERTY()
	mutable TObjectPtr<UHarmoniaEquipmentComponent> CachedEquipmentComponent;

	// ============================================================================
	// Timers
	// ============================================================================

	/** Combo window timer */
	FTimerHandle ComboWindowTimerHandle;

	/** I-frame timer */
	FTimerHandle IFrameTimerHandle;

	/** Riposte window timer */
	FTimerHandle RiposteWindowTimerHandle;

private:
	/** Clear i-frames */
	void ClearInvulnerability();

	/** Combo window expired */
	void OnComboWindowExpired();

	/** Riposte window expired */
	void OnRiposteWindowExpired();
};
