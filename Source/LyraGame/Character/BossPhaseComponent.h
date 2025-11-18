// Copyright 2024 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "BossPhaseComponent.generated.h"

class ABossCharacter;
class ULyraGameplayAbility;

/**
 * Phase configuration for a boss
 */
USTRUCT(BlueprintType)
struct FBossPhaseConfig
{
	GENERATED_BODY()

	/** Phase index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	int32 PhaseIndex = 0;

	/** Abilities to grant during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	TArray<TSubclassOf<ULyraGameplayAbility>> PhaseAbilities;

	/** Abilities to remove when entering this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	TArray<TSubclassOf<ULyraGameplayAbility>> AbilitiesToRemove;

	/** Gameplay tags to apply during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	FGameplayTagContainer PhaseTags;

	/** Gameplay tags to remove when entering this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	FGameplayTagContainer TagsToRemove;

	/** Damage multiplier for this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DamageMultiplier = 1.0f;

	/** Defense multiplier for this phase (lower = more damage taken) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DefenseMultiplier = 1.0f;

	/** Movement speed multiplier for this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float SpeedMultiplier = 1.0f;

	/** Whether boss is invulnerable during phase transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	bool bInvulnerableDuringTransition = true;

	/** Duration of phase transition (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float TransitionDuration = 2.0f;

	/** Animation montage to play during phase transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	TObjectPtr<UAnimMontage> TransitionMontage = nullptr;

	/** Visual effect to spawn during phase transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	TObjectPtr<UParticleSystem> TransitionEffect = nullptr;

	/** Niagara effect to spawn during phase transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	TObjectPtr<class UNiagaraSystem> TransitionNiagaraEffect = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseTransitionComplete, int32, NewPhase);

/**
 * UBossPhaseComponent
 *
 * Component that manages boss phase transitions and phase-specific behavior.
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API UBossPhaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossPhaseComponent();

	virtual void BeginPlay() override;

	//~=============================================================================
	// Phase Management
	//~=============================================================================

	/** Trigger phase transition */
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void OnPhaseTransition(int32 NewPhase);

	/** Get configuration for specific phase */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	const FBossPhaseConfig* GetPhaseConfig(int32 PhaseIndex) const;

	/** Get current phase config */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	const FBossPhaseConfig* GetCurrentPhaseConfig() const;

	/** Check if currently transitioning phases */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	bool IsTransitioning() const { return bIsTransitioning; }

protected:
	//~=============================================================================
	// Phase Transition Implementation
	//~=============================================================================

	/** Start phase transition sequence */
	void StartPhaseTransition(int32 NewPhase);

	/** Complete phase transition */
	void CompletePhaseTransition();

	/** Apply phase configuration */
	void ApplyPhaseConfig(const FBossPhaseConfig& Config);

	/** Remove phase configuration */
	void RemovePhaseConfig(const FBossPhaseConfig& Config);

	/** Grant phase abilities */
	void GrantPhaseAbilities(const TArray<TSubclassOf<ULyraGameplayAbility>>& Abilities);

	/** Remove phase abilities */
	void RemovePhaseAbilities(const TArray<TSubclassOf<ULyraGameplayAbility>>& Abilities);

	/** Timer callback for transition completion */
	UFUNCTION()
	void OnTransitionTimerComplete();

public:
	/** Broadcast when phase transition completes */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Phase")
	FOnPhaseTransitionComplete OnPhaseTransitionComplete;

protected:
	/** Phase configurations (one per phase) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TMap<int32, FBossPhaseConfig> PhaseConfigs;

	/** Current active phase */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Phase")
	int32 CurrentPhaseIndex = 0;

	/** Whether currently transitioning */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Phase")
	bool bIsTransitioning = false;

	/** Cached reference to boss owner */
	UPROPERTY()
	TObjectPtr<ABossCharacter> BossOwner;

	/** Timer handle for phase transition */
	FTimerHandle TransitionTimerHandle;

	/** Granted ability handles for cleanup */
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
};
