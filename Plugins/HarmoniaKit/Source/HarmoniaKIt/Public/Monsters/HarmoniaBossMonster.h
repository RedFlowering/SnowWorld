// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpec.h"
#include "HarmoniaBossMonster.generated.h"

/**
 * Boss Phase Change Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBossPhaseChangedDelegate, int32, OldPhase, int32, NewPhase, const FHarmoniaBossPhase&, PhaseData);

/**
 * AHarmoniaBossMonster
 *
 * Extended monster class with boss-specific features:
 * - Phase system (HP threshold-based)
 * - Phase-specific attacks and behaviors
 * - Phase transition animations
 * - Boss UI integration (health bar, name display)
 * - Enhanced loot tables
 *
 * Supports all creature types: humanoid, dragon, mechanical, etc.
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaBossMonster : public AHarmoniaMonsterBase
{
	GENERATED_BODY()

public:
	AHarmoniaBossMonster(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~AHarmoniaMonsterBase interface
	virtual void InitializeMonster(UHarmoniaMonsterData* InMonsterData, int32 InLevel) override;
	virtual void OnDeath_Implementation(AActor* Killer) override;
	//~End of AHarmoniaMonsterBase interface

	// ============================================================================
	// Phase System
	// ============================================================================

	/**
	 * Get current phase index
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Boss|Phase")
	int32 GetCurrentPhase() const { return CurrentPhaseIndex; }

	/**
	 * Get current phase data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Boss|Phase")
	FHarmoniaBossPhase GetCurrentPhaseData() const;

	/**
	 * Get total number of phases
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Boss|Phase")
	int32 GetTotalPhases() const;

	/**
	 * Check if boss is in phase transition
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Boss|Phase")
	bool IsInPhaseTransition() const { return bInPhaseTransition; }

	/**
	 * Manually trigger phase change (for testing or scripted events)
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase", meta = (BlueprintProtected))
	void TriggerPhaseChange(int32 NewPhaseIndex);

	// ============================================================================
	// Boss Configuration
	// ============================================================================

	/**
	 * Whether to show boss health bar
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	bool bShowHealthBar = true;

	/**
	 * Boss title/subtitle (e.g., "The Devourer", "Guardian of the Abyss")
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	FText BossTitle;

	/**
	 * Boss music to play during encounter
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Audio")
	TObjectPtr<USoundBase> BossMusic = nullptr;

	/**
	 * Whether boss music is currently playing
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Audio")
	bool bMusicPlaying = false;

	// ============================================================================
	// Delegates
	// ============================================================================

	/**
	 * Called when boss changes phase
	 */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossPhaseChangedDelegate OnBossPhaseChanged;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * Start boss encounter (play music, show UI, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	virtual void StartBossEncounter();

	/**
	 * End boss encounter (stop music, hide UI, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	virtual void EndBossEncounter();

	/**
	 * Get available attacks for current phase
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	TArray<FHarmoniaMonsterAttackPattern> GetCurrentPhaseAttacks() const;

protected:
	// ============================================================================
	// Phase State
	// ============================================================================

	/**
	 * Current phase index (-1 = not initialized, 0 = first phase)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Phase", ReplicatedUsing = OnRep_CurrentPhaseIndex)
	int32 CurrentPhaseIndex = -1;

	/**
	 * Whether boss is currently transitioning between phases
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Phase", Replicated)
	bool bInPhaseTransition = false;

	/**
	 * Whether boss encounter has started
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Boss", Replicated)
	bool bEncounterStarted = false;

	// ============================================================================
	// Internal Functions
	// ============================================================================

	/**
	 * Check if boss should change phase based on health
	 */
	virtual void CheckPhaseTransition();

	/**
	 * Change to a new phase
	 */
	virtual void ChangeToPhase(int32 NewPhaseIndex);

	/**
	 * Apply phase effects (gameplay effects, stat changes, etc.)
	 */
	virtual void ApplyPhaseEffects(const FHarmoniaBossPhase& PhaseData);

	/**
	 * Remove previous phase effects
	 */
	virtual void RemovePreviousPhaseEffects();

	/**
	 * Play phase transition animation
	 */
	virtual void PlayPhaseTransitionAnimation(const FHarmoniaBossPhase& PhaseData);

	/**
	 * Called when phase transition animation completes
	 */
	virtual void OnPhaseTransitionComplete();

	/**
	 * Update behavior tree for current phase
	 */
	virtual void UpdatePhaseBehaviorTree();

	/**
	 * Play boss music
	 */
	virtual void PlayBossMusic();

	/**
	 * Stop boss music
	 */
	virtual void StopBossMusic();

	// ============================================================================
	// Attribute Callbacks (Override from base)
	// ============================================================================

	virtual void OnHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue) override;

	// ============================================================================
	// Replication
	// ============================================================================

	UFUNCTION()
	virtual void OnRep_CurrentPhaseIndex(int32 OldPhaseIndex);

	// ============================================================================
	// Internal State
	// ============================================================================

	/**
	 * Active gameplay effect handles from current phase
	 */
	UPROPERTY(Transient)
	TArray<FActiveGameplayEffectHandle> ActivePhaseEffects;

	/**
	 * Granted ability handles from current phase
	 */
	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	/**
	 * Audio component for boss music
	 */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BossMusicComponent = nullptr;

	/**
	 * Timer handle for phase transition
	 */
	FTimerHandle PhaseTransitionTimerHandle;
};
