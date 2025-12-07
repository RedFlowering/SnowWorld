// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseCombatComponent.h"
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
UCLASS(Blueprintable, ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMeleeCombatComponent : public UHarmoniaBaseCombatComponent
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

	/** Open combo window manually (called by AnimNotify) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void OpenComboWindow(float Duration);

	/** Queue next combo attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void QueueNextCombo();

	/** Is next combo queued? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	bool IsNextComboQueued() const { return bNextComboQueued; }

	/** Clear queued combo */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void ClearQueuedCombo() { bNextComboQueued = false; }

	/** Get combo sequence for current weapon (returns cached data) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	bool GetComboSequence(EHarmoniaAttackType AttackType, FHarmoniaComboAttackSequence& OutSequence) const;

	/** Refresh cached combo data (call on weapon change or owner setup) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void RefreshCachedCombos();

	/** Get current attack type */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	EHarmoniaAttackType GetCurrentAttackType() const { return CurrentAttackType; }

	/** Get current combo step's attack data (uses AttackDataOverride if enabled) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	bool GetCurrentComboAttackData(FHarmoniaAttackData& OutAttackData) const;

	/** Is current attack a heavy attack? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	bool IsCurrentAttackHeavy() const { return CurrentAttackType == EHarmoniaAttackType::Heavy; }

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
	void StartAttack(EHarmoniaAttackType InAttackType);

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
	// Owner Configuration
	// ============================================================================

	/** Owner type tag for combo lookup (e.g., Character.Race.Human) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Melee Combat|Owner")
	FGameplayTag OwnerTypeTag;

	// ============================================================================
	// Cached Combo Data
	// ============================================================================

	/** Cached combo sequences by attack type */
	UPROPERTY(Transient)
	TMap<EHarmoniaAttackType, FHarmoniaComboAttackSequence> CachedCombos;

	/** Is combo cache valid? */
	bool bComboCacheValid = false;

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

	/** Tag that blocks attack (if character has this tag, attack is blocked) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag AttackBlockedTag;

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

	/** Current attack type */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	EHarmoniaAttackType CurrentAttackType = EHarmoniaAttackType::Light;

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
