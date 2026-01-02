// Copyright 2024 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "ActiveGameplayEffectHandle.h"
#include "HarmoniaMonsterPatternComponent.generated.h"

class AHarmoniaMonsterBase;
class ULyraGameplayAbility;
class UBehaviorTree;
class UGameplayEffect;
class UGameplayAbility;

/**
 * Attack pattern execution mode
 */
UENUM(BlueprintType)
enum class EMonsterPatternExecutionMode : uint8
{
	/** Execute pattern once */
	Single,
	/** Execute pattern multiple times in sequence */
	Sequence,
	/** Execute pattern randomly from a set */
	Random,
	/** Execute all abilities in pattern simultaneously */
	Simultaneous
};

/**
 * FMonsterAttackPattern
 *
 * Attack pattern configuration. Each pattern can contain multiple abilities
 * with various execution modes and conditions.
 */
USTRUCT(BlueprintType)
struct FMonsterAttackPattern
{
	GENERATED_BODY()

	// ===== Basic Info =====

	/** Pattern name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Info")
	FName PatternName;

	/** Description for designers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Info")
	FText PatternDescription;

	// ===== Ability Execution =====

	/** Abilities to execute in this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Execution")
	TArray<TSubclassOf<ULyraGameplayAbility>> PatternAbilities;

	/** Execution mode for this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Execution")
	EMonsterPatternExecutionMode ExecutionMode = EMonsterPatternExecutionMode::Sequence;

	/** Delay between abilities in sequence mode (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Execution", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float AbilityDelay = 0.5f;

	/** Number of times to repeat the pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Execution", meta = (ClampMin = "1", ClampMax = "10"))
	int32 RepeatCount = 1;

	/** Delay between pattern repetitions (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Execution", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float RepeatDelay = 1.0f;

	// ===== Conditions =====

	/** Gameplay tags required to execute this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Conditions")
	FGameplayTagContainer RequiredTags;

	/** Gameplay tags that block this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Conditions")
	FGameplayTagContainer BlockedByTags;

	/** Cooldown before pattern can be used again (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Conditions", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float Cooldown = 5.0f;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Conditions", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RandomWeight = 1.0f;

	/** Whether pattern can be interrupted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Conditions")
	bool bCanBeInterrupted = false;

	// ===== Pattern Start Effects =====

	/** Montage to play when pattern starts (transition, intro) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TObjectPtr<UAnimMontage> StartMontage = nullptr;

	/** Abilities to grant when pattern starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

	/** Abilities to remove when pattern starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToRemove;

	/** Effects to apply when pattern starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TArray<TSubclassOf<UGameplayEffect>> PatternEffects;

	/** Tags to apply when pattern starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	FGameplayTagContainer TagsToApply;

	/** Tags to remove when pattern starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	FGameplayTagContainer TagsToRemove;
};

/**
 * FMonsterPhasePatterns
 *
 * Phase configuration containing HP threshold, enter effects, and attack patterns.
 */
USTRUCT(BlueprintType)
struct FMonsterPhasePatterns
{
	GENERATED_BODY()

	// ===== Phase Info =====

	/** Phase name (for debugging/UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Info")
	FName PhaseName;

	/** HP threshold: phase activates when HP <= this percent (1.0 = 100%, 0.7 = 70%, 0.3 = 30%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Info", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthThreshold = 1.0f;

	// ===== Phase Enter Effects (executes once on phase entry) =====

	/** Montage to play when entering this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Enter")
	TObjectPtr<UAnimMontage> PhaseEnterMontage = nullptr;

	/** Abilities to grant on phase entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Enter")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

	/** Abilities to remove on phase entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Enter")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToRemove;

	/** Effects to apply on phase entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Enter")
	TArray<TSubclassOf<UGameplayEffect>> PhaseEffects;

	/** Tags to apply on phase entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Enter")
	FGameplayTagContainer TagsToApply;

	/** Tags to remove on phase entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Enter")
	FGameplayTagContainer TagsToRemove;

	// ===== Attack Patterns =====

	/** Attack patterns available in this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase|Patterns")
	TArray<FMonsterAttackPattern> AttackPatterns;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatternExecutionStart, FName, PatternName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatternExecutionEnd, FName, PatternName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhaseChanged, int32, OldPhase, int32, NewPhase);

/**
 * UHarmoniaMonsterPatternComponent
 *
 * Component that manages monster attack patterns and pattern-based combat.
 * Used by both regular monsters and bosses.
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMonsterPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaMonsterPatternComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//~=============================================================================
	// Pattern Execution
	//~=============================================================================

	/** Execute a specific attack pattern by name */
	UFUNCTION(BlueprintCallable, Category = "Monster|Pattern")
	bool ExecutePattern(FName PatternName);

	/** Execute a random attack pattern from available patterns */
	UFUNCTION(BlueprintCallable, Category = "Monster|Pattern")
	bool ExecuteRandomPattern();

	/** Stop current pattern execution */
	UFUNCTION(BlueprintCallable, Category = "Monster|Pattern")
	void StopCurrentPattern();

	/** Check if pattern is available for execution */
	UFUNCTION(BlueprintPure, Category = "Monster|Pattern")
	bool IsPatternAvailable(FName PatternName) const;

	/** Get all available patterns for current phase */
	UFUNCTION(BlueprintPure, Category = "Monster|Pattern")
	TArray<FName> GetAvailablePatterns() const;

	//~=============================================================================
	// Phase Management
	//~=============================================================================

	/** Update phase based on current health percentage */
	UFUNCTION(BlueprintCallable, Category = "Monster|Phase")
	void UpdatePhase(float HealthPercent);

	/** Get current phase index */
	UFUNCTION(BlueprintPure, Category = "Monster|Phase")
	int32 GetCurrentPhase() const { return CurrentPhaseIndex; }

	/** Get current phase data */
	UFUNCTION(BlueprintPure, Category = "Monster|Phase")
	const FMonsterPhasePatterns& GetCurrentPhaseData() const;

	//~=============================================================================
	// Pattern State
	//~=============================================================================

	/** Check if currently executing a pattern */
	UFUNCTION(BlueprintPure, Category = "Monster|Pattern")
	bool IsExecutingPattern() const { return bIsExecutingPattern; }

	/** Get current executing pattern name */
	UFUNCTION(BlueprintPure, Category = "Monster|Pattern")
	FName GetCurrentPatternName() const { return CurrentPatternName; }

	/** Get pattern configuration by name */
	const FMonsterAttackPattern* GetPatternConfig(FName PatternName) const;

	/** Check if pattern is on cooldown */
	UFUNCTION(BlueprintPure, Category = "Monster|Pattern")
	bool IsPatternOnCooldown(FName PatternName) const;

	/** Get remaining cooldown time for pattern */
	UFUNCTION(BlueprintPure, Category = "Monster|Pattern")
	float GetPatternCooldownRemaining(FName PatternName) const;

protected:
	/** Get patterns for current phase (C++ only) */
	const FMonsterPhasePatterns* GetCurrentPhasePatterns() const;

	//~=============================================================================
	// Pattern Execution Implementation
	//~=============================================================================

	/** Start executing a pattern */
	void StartPatternExecution(const FMonsterAttackPattern& Pattern);

	/** Execute abilities based on active pattern's execution mode */
	void ExecutePatternAbilities();

	/** Execute next ability in sequence */
	void ExecuteNextAbility();

	/** Execute ability at index */
	void ExecuteAbilityAtIndex(int32 AbilityIndex);

	/** Complete pattern execution */
	void CompletePatternExecution();

	/** Check if pattern can be executed */
	bool CanExecutePattern(const FMonsterAttackPattern& Pattern) const;

	/** Select random pattern based on weights */
	FName SelectRandomPattern() const;

	/** Update cooldowns */
	void UpdateCooldowns(float DeltaTime);

	//~=============================================================================
	// Effects Application
	//~=============================================================================

	/** Apply pattern effects (abilities, GE, tags) */
	void ApplyPatternEffects(const FMonsterAttackPattern& Pattern);

	/** Apply phase enter effects (montage, abilities, GE, tags) */
	void ApplyPhaseEnterEffects(const FMonsterPhasePatterns& Phase);

	/** Remove pattern effects */
	void RemovePatternEffects();

	/** Play start montage if set */
	void PlayStartMontage(const FMonsterAttackPattern& Pattern);

	//~=============================================================================
	// Timer Callbacks
	//~=============================================================================

	UFUNCTION()
	void OnAbilityDelayComplete();

	UFUNCTION()
	void OnRepeatDelayComplete();

	UFUNCTION()
	void OnStartMontageComplete();

public:
	/** Broadcast when pattern execution starts */
	UPROPERTY(BlueprintAssignable, Category = "Monster|Pattern")
	FOnPatternExecutionStart OnPatternExecutionStart;

	/** Broadcast when pattern execution ends */
	UPROPERTY(BlueprintAssignable, Category = "Monster|Pattern")
	FOnPatternExecutionEnd OnPatternExecutionEnd;

	/** Broadcast when phase changes */
	UPROPERTY(BlueprintAssignable, Category = "Monster|Phase")
	FOnPhaseChanged OnPhaseChanged;

protected:
	/** Phase configurations (use descending HealthThreshold order) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Phase")
	TArray<FMonsterPhasePatterns> Phases;

	/** Current phase index */
	UPROPERTY(BlueprintReadOnly, Category = "Monster|Phase")
	int32 CurrentPhaseIndex = 0;

	/** Currently executing pattern name */
	UPROPERTY(BlueprintReadOnly, Category = "Monster|Pattern")
	FName CurrentPatternName = NAME_None;

	/** Whether currently executing a pattern */
	UPROPERTY(BlueprintReadOnly, Category = "Monster|Pattern")
	bool bIsExecutingPattern = false;

	/** Current ability index in sequence */
	UPROPERTY()
	int32 CurrentAbilityIndex = 0;

	/** Current repeat count */
	UPROPERTY()
	int32 CurrentRepeatCount = 0;

	/** Active pattern configuration */
	UPROPERTY()
	FMonsterAttackPattern ActivePattern;

	/** Cached reference to monster owner */
	UPROPERTY()
	TObjectPtr<AHarmoniaMonsterBase> MonsterOwner;

	/** Timer handle for ability delays */
	FTimerHandle AbilityDelayTimerHandle;

	/** Timer handle for repeat delays */
	FTimerHandle RepeatDelayTimerHandle;

	/** Timer handle for start montage */
	FTimerHandle StartMontageTimerHandle;

	/** Pattern cooldown timers (pattern name -> remaining time) */
	UPROPERTY()
	TMap<FName, float> PatternCooldowns;

	/** Temporarily granted ability handles (need to be cleaned up after pattern completes) */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> TemporarilyGrantedAbilities;

	/** Active gameplay effect handles from patterns */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> ActivePatternEffects;
};
