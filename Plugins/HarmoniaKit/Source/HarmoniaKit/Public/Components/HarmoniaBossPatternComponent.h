// Copyright 2024 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "HarmoniaBossPatternComponent.generated.h"

class AHarmoniaBossMonster;
class ULyraGameplayAbility;

/**
 * Attack pattern execution mode
 */
UENUM(BlueprintType)
enum class EBossPatternExecutionMode : uint8
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
 * Boss attack pattern configuration
 */
USTRUCT(BlueprintType)
struct FBossAttackPattern
{
	GENERATED_BODY()

	/** Pattern name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	FName PatternName;

	/** Description for designers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	FText PatternDescription;

	/** Abilities to execute in this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	TArray<TSubclassOf<ULyraGameplayAbility>> PatternAbilities;

	/** Execution mode for this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	EBossPatternExecutionMode ExecutionMode = EBossPatternExecutionMode::Sequence;

	/** Delay between abilities in sequence mode (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float AbilityDelay = 0.5f;

	/** Number of times to repeat the pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern", meta = (ClampMin = "1", ClampMax = "10"))
	int32 RepeatCount = 1;

	/** Delay between pattern repetitions (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float RepeatDelay = 1.0f;

	/** Phases in which this pattern is available (empty = all phases) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	TArray<int32> ValidPhases;

	/** Gameplay tags required to execute this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	FGameplayTagContainer RequiredTags;

	/** Gameplay tags that block this pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	FGameplayTagContainer BlockedByTags;

	/** Cooldown before pattern can be used again (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float Cooldown = 5.0f;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RandomWeight = 1.0f;

	/** Whether pattern can be interrupted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	bool bCanBeInterrupted = false;

	/** Animation montage to play during pattern */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	TObjectPtr<UAnimMontage> PatternMontage = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatternExecutionStart, FName, PatternName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatternExecutionEnd, FName, PatternName);

/**
 * UHarmoniaBossPatternComponent
 *
 * Component that manages boss attack patterns and pattern-based combat.
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaBossPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBossPatternComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//~=============================================================================
	// Pattern Execution
	//~=============================================================================

	/** Execute a specific attack pattern by name */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	bool ExecutePattern(FName PatternName);

	/** Execute a random attack pattern from available patterns */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	bool ExecuteRandomPattern();

	/** Stop current pattern execution */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void StopCurrentPattern();

	/** Check if pattern is available for execution */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	bool IsPatternAvailable(FName PatternName) const;

	/** Get all available patterns for current phase */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	TArray<FName> GetAvailablePatterns() const;

	//~=============================================================================
	// Pattern State
	//~=============================================================================

	/** Check if currently executing a pattern */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	bool IsExecutingPattern() const { return bIsExecutingPattern; }

	/** Get current executing pattern name */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	FName GetCurrentPatternName() const { return CurrentPatternName; }

	/** Get pattern configuration by name */
	const FBossAttackPattern* GetPatternConfig(FName PatternName) const;

	/** Check if pattern is on cooldown */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	bool IsPatternOnCooldown(FName PatternName) const;

	/** Get remaining cooldown time for pattern */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	float GetPatternCooldownRemaining(FName PatternName) const;

protected:
	//~=============================================================================
	// Pattern Execution Implementation
	//~=============================================================================

	/** Start executing a pattern */
	void StartPatternExecution(const FBossAttackPattern& Pattern);

	/** Execute next ability in sequence */
	void ExecuteNextAbility();

	/** Execute ability at index */
	void ExecuteAbilityAtIndex(int32 AbilityIndex);

	/** Complete pattern execution */
	void CompletePatternExecution();

	/** Check if pattern can be executed */
	bool CanExecutePattern(const FBossAttackPattern& Pattern) const;

	/** Select random pattern based on weights */
	FName SelectRandomPattern() const;

	/** Update cooldowns */
	void UpdateCooldowns(float DeltaTime);

	//~=============================================================================
	// Timer Callbacks
	//~=============================================================================

	UFUNCTION()
	void OnAbilityDelayComplete();

	UFUNCTION()
	void OnRepeatDelayComplete();

public:
	/** Broadcast when pattern execution starts */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FOnPatternExecutionStart OnPatternExecutionStart;

	/** Broadcast when pattern execution ends */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FOnPatternExecutionEnd OnPatternExecutionEnd;

protected:
	/** Available attack patterns */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern")
	TMap<FName, FBossAttackPattern> AttackPatterns;

	/** Currently executing pattern name */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Pattern")
	FName CurrentPatternName = NAME_None;

	/** Whether currently executing a pattern */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Pattern")
	bool bIsExecutingPattern = false;

	/** Current ability index in sequence */
	UPROPERTY()
	int32 CurrentAbilityIndex = 0;

	/** Current repeat count */
	UPROPERTY()
	int32 CurrentRepeatCount = 0;

	/** Active pattern configuration */
	UPROPERTY()
	FBossAttackPattern ActivePattern;

	/** Cached reference to boss owner */
	UPROPERTY()
	TObjectPtr<AHarmoniaBossMonster> BossOwner;

	/** Timer handle for ability delays */
	FTimerHandle AbilityDelayTimerHandle;

	/** Timer handle for repeat delays */
	FTimerHandle RepeatDelayTimerHandle;

	/** Pattern cooldown timers (pattern name -> remaining time) */
	UPROPERTY()
	TMap<FName, float> PatternCooldowns;

	/** Temporarily granted ability handles (need to be cleaned up after pattern completes) */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> TemporarilyGrantedAbilities;
};
