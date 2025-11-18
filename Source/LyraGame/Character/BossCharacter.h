// Copyright 2024 Snow Game Studio.

#pragma once

#include "BaseCharacter.h"
#include "GameplayTagContainer.h"
#include "BossCharacter.generated.h"

class UBossPhaseComponent;
class UBossPatternComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossPhaseChanged, int32, OldPhase, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossEncounterStart, ABossCharacter*, Boss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossEncounterEnd, ABossCharacter*, Boss, bool, bDefeated);

/**
 * ABossCharacter
 *
 * Boss character with multi-phase combat system and pattern-based attacks.
 * Inherits from BaseCharacter to use existing combat systems (lock-on, GAS, etc.)
 */
UCLASS(Blueprintable)
class LYRAGAME_API ABossCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABossCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	//~=============================================================================
	// Boss Encounter Management
	//~=============================================================================

	/** Start the boss encounter */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	virtual void StartEncounter();

	/** End the boss encounter */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	virtual void EndEncounter(bool bDefeated);

	/** Check if boss encounter is active */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsEncounterActive() const { return bEncounterActive; }

	//~=============================================================================
	// Boss Phase System
	//~=============================================================================

	/** Get current boss phase (0-based index) */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	/** Get maximum number of phases */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	int32 GetMaxPhases() const { return PhaseHealthThresholds.Num() + 1; }

	/** Force transition to specific phase */
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void SetPhase(int32 NewPhase);

	/** Check if can transition to next phase */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	bool CanTransitionPhase() const;

	//~=============================================================================
	// Boss Pattern System
	//~=============================================================================

	/** Execute a specific attack pattern by name */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void ExecutePattern(FName PatternName);

	/** Get current active pattern name */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	FName GetCurrentPattern() const { return CurrentPatternName; }

	/** Check if boss is currently executing a pattern */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	bool IsExecutingPattern() const { return bIsExecutingPattern; }

	//~=============================================================================
	// Components
	//~=============================================================================

	UFUNCTION(BlueprintPure, Category = "Boss")
	UBossPhaseComponent* GetBossPhaseComponent() const { return BossPhaseComponent; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	UBossPatternComponent* GetBossPatternComponent() const { return BossPatternComponent; }

protected:
	//~=============================================================================
	// Phase System Events
	//~=============================================================================

	/** Called when entering a new phase */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|Phase")
	void OnPhaseEnter(int32 NewPhase);
	virtual void OnPhaseEnter_Implementation(int32 NewPhase);

	/** Called when exiting a phase */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|Phase")
	void OnPhaseExit(int32 OldPhase);
	virtual void OnPhaseExit_Implementation(int32 OldPhase);

	//~=============================================================================
	// Pattern System Events
	//~=============================================================================

	/** Called when pattern execution starts */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|Pattern")
	void OnPatternStart(FName PatternName);
	virtual void OnPatternStart_Implementation(FName PatternName);

	/** Called when pattern execution ends */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|Pattern")
	void OnPatternEnd(FName PatternName);
	virtual void OnPatternEnd_Implementation(FName PatternName);

	//~=============================================================================
	// Health Monitoring
	//~=============================================================================

	/** Monitor health changes for phase transitions */
	UFUNCTION()
	virtual void OnHealthChanged(ULyraHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator);

	/** Check health thresholds and trigger phase transitions */
	void CheckPhaseTransition(float CurrentHealth, float MaxHealth);

public:
	//~=============================================================================
	// Boss Delegates
	//~=============================================================================

	/** Broadcast when boss phase changes */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossPhaseChanged OnBossPhaseChanged;

	/** Broadcast when boss encounter starts */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossEncounterStart OnBossEncounterStart;

	/** Broadcast when boss encounter ends */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossEncounterEnd OnBossEncounterEnd;

protected:
	//~=============================================================================
	// Boss Components
	//~=============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBossPhaseComponent> BossPhaseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBossPatternComponent> BossPatternComponent;

	//~=============================================================================
	// Phase Configuration
	//~=============================================================================

	/**
	 * Health percentage thresholds for phase transitions (0.0 - 1.0)
	 * Example: {0.66, 0.33} creates 3 phases:
	 * - Phase 0: 100% - 66% health
	 * - Phase 1: 66% - 33% health
	 * - Phase 2: 33% - 0% health
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	TArray<float> PhaseHealthThresholds;

	/** Gameplay tags to apply when entering each phase */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TMap<int32, FGameplayTagContainer> PhaseGameplayTags;

	/** Whether phase transitions can be interrupted */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	bool bCanInterruptPhaseTransition = false;

	//~=============================================================================
	// Boss State
	//~=============================================================================

	/** Current phase index (0-based) */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|State", ReplicatedUsing = OnRep_CurrentPhase)
	int32 CurrentPhase = 0;

	/** Whether the boss encounter is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|State", Replicated)
	bool bEncounterActive = false;

	/** Whether boss is currently transitioning phases */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|State")
	bool bIsTransitioningPhase = false;

	/** Current active pattern name */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|State")
	FName CurrentPatternName = NAME_None;

	/** Whether boss is executing a pattern */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|State")
	bool bIsExecutingPattern = false;

	//~=============================================================================
	// Boss Behavior
	//~=============================================================================

	/** Boss display name for UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Display")
	FText BossName;

	/** Boss title/subtitle for UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Display")
	FText BossTitle;

	/** Whether to show boss health bar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Display")
	bool bShowBossHealthBar = true;

	/** Auto-start encounter on begin play */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Behavior")
	bool bAutoStartEncounter = false;

private:
	//~=============================================================================
	// Replication
	//~=============================================================================

	UFUNCTION()
	void OnRep_CurrentPhase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
