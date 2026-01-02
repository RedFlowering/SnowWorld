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
 * FBossAttackPattern
 *
 * 보스 공격 패턴 구성. 단순화된 구조로 패턴 시작 시 몽타주와 효과를 적용합니다.
 * 
 * 사용 예:
 * - 일반 공격: PatternAbilities에 공격 어빌리티 설정
 * - 페이즈 진입 공격: StartMontage에 전환 몽타주 설정 + AbilitiesToGrant
 */
USTRUCT(BlueprintType)
struct FMonsterAttackPattern
{
	GENERATED_BODY()

	// ===== 기본 정보 =====

	/** Pattern name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Info")
	FName PatternName;

	/** Description for designers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Info")
	FText PatternDescription;

	// ===== 어빌리티 실행 =====

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

	// ===== 조건 =====

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

	// ===== 패턴 시작 시 효과 =====

	/** 패턴 시작 시 재생할 몽타주 (트랜지션, 시작 연출 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TObjectPtr<UAnimMontage> StartMontage = nullptr;

	/** 패턴 시작 시 부여할 어빌리티 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

	/** 패턴 시작 시 제거할 어빌리티 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToRemove;

	/** 패턴 시작 시 적용할 GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	TArray<TSubclassOf<UGameplayEffect>> PatternEffects;

	/** 패턴 시작 시 적용할 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	FGameplayTagContainer TagsToApply;

	/** 패턴 시작 시 제거할 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Start")
	FGameplayTagContainer TagsToRemove;
};

/**
 * FBossPhasePatterns
 *
 * 특정 페이즈에서 사용 가능한 패턴들의 그룹
 */
USTRUCT(BlueprintType)
struct FMonsterPhasePatterns
{
	GENERATED_BODY()

	/** 이 페이즈에서 사용 가능한 공격 패턴들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	TArray<FMonsterAttackPattern> AttackPatterns;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatternExecutionStart, FName, PatternName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatternExecutionEnd, FName, PatternName);

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
	const FMonsterAttackPattern* GetPatternConfig(FName PatternName) const;

	/** Check if pattern is on cooldown */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	bool IsPatternOnCooldown(FName PatternName) const;

	/** Get remaining cooldown time for pattern */
	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
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
	// Pattern Effects (효과 적용)
	//~=============================================================================

	/** Apply pattern effects (abilities, GE, tags) */
	void ApplyPatternEffects(const FMonsterAttackPattern& Pattern);

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
	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FOnPatternExecutionStart OnPatternExecutionStart;

	/** Broadcast when pattern execution ends */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FOnPatternExecutionEnd OnPatternExecutionEnd;

protected:
	/** 페이즈별 공격 패턴 (페이즈 인덱스 → 해당 페이즈의 패턴들) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern")
	TMap<int32, FMonsterPhasePatterns> PhasePatterns;

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
