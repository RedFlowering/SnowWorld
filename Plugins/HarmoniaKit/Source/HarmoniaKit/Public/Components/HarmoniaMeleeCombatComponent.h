// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseCombatComponent.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "HarmoniaMeleeCombatComponent.generated.h"

class UHarmoniaAttributeSet;
class UAbilitySystemComponent;
class UHarmoniaEquipmentComponent;
class UHarmoniaSenseComponent;
class UDataTable;
class USensorBase;
struct FSensedStimulus;

/** Delegate broadcast when attack is blocked. Damage is the incoming damage before reduction. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlockedAttackSignature, AActor*, Attacker, float, Damage);

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// Weapon Management
	// ============================================================================

	/** Get the owner's AbilitySystemComponent (handles Lyra-style PlayerState ASC) */
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/** Get the owner's HarmoniaAttributeSet */
	UHarmoniaAttributeSet* GetAttributeSet() const;

	/** Get current equipped weapon type tag from EquipmentComponent */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	FGameplayTag GetCurrentWeaponTypeTag() const;

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

	/** Is currently in attack window? (hit detection is active) */
	UFUNCTION(BlueprintPure, Category = "Melee Combat")
	bool IsInAttackWindow() const { return bInAttackWindow; }

	/** Set attack window state (called by AnimNotify) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat")
	void SetInAttackWindow(bool bInWindow);


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

	/** Queue next combo attack (calls Server RPC if on client) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Combo")
	void QueueNextCombo();

	/** Server RPC to queue combo (called by client) */
	UFUNCTION(Server, Reliable)
	void ServerQueueNextCombo();

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

	// ============================================================================
	// Defense
	// ============================================================================

	/** Defense angle in degrees (full arc, e.g., 120 = 60 degrees each side of forward) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Combat|Defense")
	float DefenseAngle = 120.0f;

	/** Can currently block? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	bool CanBlock() const;

	/** Can currently parry? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	bool CanParry() const;

	/** Can currently dodge? */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	bool CanDodge() const;

	/**
	 * Check if attacker is within defense angle (front arc)
	 * @param AttackerLocation World location of the attacker
	 * @return true if attacker is in front and can be blocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	bool IsDefenseAngleValid(const FVector& AttackerLocation) const;

	/** Process blocked attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Defense")
	void OnAttackBlocked(AActor* Attacker, float Damage);

	/** Delegate broadcast when attack is blocked (for ability to handle stamina cost) */
	UPROPERTY(BlueprintAssignable, Category = "Melee Combat|Defense")
	FOnBlockedAttackSignature OnBlockedAttack;

	/** Delegate broadcast when attack is blocked by defense angle check (from SenseSystem) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBlockedByDefenseSignature, AActor*, Attacker, float, BlockedDamage, bool, bWasInAngle);

	UPROPERTY(BlueprintAssignable, Category = "Melee Combat|Defense")
	FOnBlockedByDefenseSignature OnBlockedByDefense;

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

	/** Start riposte window */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	void StartRiposteWindow(AActor* Target, float Duration);

	/** End riposte window */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	void EndRiposteWindow();

	/** Get current parried target */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	AActor* GetParriedTarget() const;

	/** Clear parried target */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	void ClearParriedTarget();

	/** Get riposte damage multiplier */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Riposte")
	float GetRiposteDamageMultiplier() const;

	// ============================================================================
	// Backstab System
	// ============================================================================

	/** Check if attack is considered backstab */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Backstab")
	bool IsBackstabAttack(AActor* Target, FVector AttackOrigin) const;

	/** Get backstab damage multiplier */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Backstab")
	float GetBackstabDamageMultiplier() const;

	// ============================================================================
	// Damage Processing
	// ============================================================================

	/** Apply damage to target actor */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Damage")
	void ApplyDamageToTarget(AActor* TargetActor, const FVector& HitLocation = FVector::ZeroVector);

	// ============================================================================
	// Hit Reaction (Network)
	// ============================================================================

	/**
	 * Client RPC to receive hit reaction from server
	 * Handles camera shake (from combo sequence table) and GameplayEvent on the owning client
	 * @param Damage The damage amount received
	 * @param HitLocation The location where the hit occurred
	 * @param CameraShakePath Soft path to camera shake class (serialized as string for network safety)
	 * @param CameraShakeScale Scale for the camera shake
	 */
	UFUNCTION(Client, Reliable)
	void ClientReceiveHitReaction(float Damage, const FVector& HitLocation, const FSoftClassPath& CameraShakePath, float CameraShakeScale);

protected:
	// ============================================================================
	// Data Tables
	// ============================================================================

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
	// Damage Configuration
	// ============================================================================

	/** Damage gameplay effect to apply on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** Additional effects to apply on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Damage")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalHitEffects;

	/** Gameplay cue to trigger on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Damage")
	FGameplayTag HitGameplayCueTag;

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

	/** Tag applied while invulnerable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag InvulnerableTag;

	/** Tag that blocks attacks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat|Tags")
	FGameplayTag AttackBlockedTag;

	// ============================================================================
	// Combat State
	// ============================================================================

	/** Current defense state */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	EHarmoniaDefenseState DefenseState = EHarmoniaDefenseState::None;

	/** Is currently attacking? */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Melee Combat|State")
	bool bIsAttacking = false;

	/** Is in attack window? (hit detection active) */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	bool bInAttackWindow = false;

	/** Current attack type */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Melee Combat|State")
	EHarmoniaAttackType CurrentAttackType = EHarmoniaAttackType::Light;

	/** Current combo index */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Melee Combat|State")
	int32 CurrentComboIndex = 0;

	/** Is next combo queued? */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Melee Combat|State")
	bool bNextComboQueued = false;

	/** Is in i-frames? */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	bool bInIFrames = false;

	/** Parried target (for riposte) */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|State")
	TWeakObjectPtr<AActor> ParriedTarget;

	/** Actors hit during current attack window (prevents duplicate hits) */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> HitActorsThisAttack;

	// ============================================================================
	// Charge Attack State (updated by GA for debug display)
	// ============================================================================

	/** Current charge time (set by GA) */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|Charge")
	float CurrentChargeTime = 0.0f;

	/** Current charge level (set by GA) */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|Charge")
	int32 CurrentChargeLevel = 0;

	/** Max charge time (set by GA) */
	UPROPERTY(BlueprintReadOnly, Category = "Melee Combat|Charge")
	float MaxChargeTime = 0.0f;

public:
	/** Update charge state (called by MeleeAttack ability) */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Charge")
	void UpdateChargeState(float InChargeTime, int32 InChargeLevel, float InMaxChargeTime)
	{
		CurrentChargeTime = InChargeTime;
		CurrentChargeLevel = InChargeLevel;
		MaxChargeTime = InMaxChargeTime;
	}

	/** Clear charge state */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Charge")
	void ClearChargeState()
	{
		CurrentChargeTime = 0.0f;
		CurrentChargeLevel = 0;
		MaxChargeTime = 0.0f;
	}

	/** Get current charge time */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Charge")
	float GetCurrentChargeTime() const { return CurrentChargeTime; }

	/** Get current charge level */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Charge")
	int32 GetCurrentChargeLevel() const { return CurrentChargeLevel; }

	/** Get max charge time */
	UFUNCTION(BlueprintCallable, Category = "Melee Combat|Charge")
	float GetMaxChargeTime() const { return MaxChargeTime; }

protected:

	// ============================================================================
	// Timers
	// ============================================================================

	/** Combo window timer */
	FTimerHandle ComboWindowTimerHandle;

	/** I-frame timer */
	FTimerHandle IFrameTimerHandle;

	/** Riposte window timer */
	FTimerHandle RiposteWindowTimerHandle;

	// ============================================================================
	// Event Handlers
	// ============================================================================

	/** Handle equipment changes to update cached combos */
	UFUNCTION()
	void OnEquipmentChanged(EEquipmentSlot Slot, const FHarmoniaID& OldId, const FHarmoniaID& NewId);

private:
	/** Clear i-frames */
	void ClearInvulnerability();

	/** Combo window expired */
	void OnComboWindowExpired();

	/** Riposte window expired */
	void OnRiposteWindowExpired();

	/** Handle SenseSystem hit detection */
	UFUNCTION()
	void OnSenseHit(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus>& SensedStimuli);

	/** Cached SenseComponent reference */
	TObjectPtr<UHarmoniaSenseComponent> CachedSenseComponent;
};
