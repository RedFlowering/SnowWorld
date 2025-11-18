// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "HarmoniaAdvancedAIComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Emotion State Changed Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionStateChangedDelegate, EHarmoniaMonsterEmotion, OldEmotion, EHarmoniaMonsterEmotion, NewEmotion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboExecutedDelegate, FName, AttackID, int32, ComboCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTacticalPositionFoundDelegate, EHarmoniaTacticalPosition, PositionType);

/**
 * UHarmoniaAdvancedAIComponent
 *
 * Component that adds advanced AI behaviors to monsters
 * - Emotion system (rage, fear, exhaustion)
 * - Combo attack chains
 * - Tactical positioning (cover, high ground)
 * - Context-aware attack selection
 *
 * Attach to monster actors to enable advanced AI features
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaAdvancedAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaAdvancedAIComponent();

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Enable emotion system
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI|Emotion")
	bool bEnableEmotions = true;

	/**
	 * Enable combo system
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI|Combat")
	bool bEnableCombos = true;

	/**
	 * Enable tactical positioning
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI|Tactical")
	bool bEnableTacticalPositioning = true;

	/**
	 * Health threshold to trigger rage (0.0 - 1.0)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI|Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RageHealthThreshold = 0.25f;

	/**
	 * Enemy power threshold to trigger fear (relative to own power)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI|Emotion", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float FearPowerThreshold = 2.0f;

	/**
	 * Combat duration to trigger exhaustion (in seconds)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI|Emotion", meta = (ClampMin = "10.0"))
	float ExhaustionThreshold = 60.0f;

	/**
	 * Range to search for tactical positions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI|Tactical", meta = (ClampMin = "500.0"))
	float TacticalSearchRadius = 2000.0f;

	// ============================================================================
	// State
	// ============================================================================

	/**
	 * Current emotion state
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Advanced AI|State", Replicated)
	FHarmoniaEmotionStateData EmotionState;

	/**
	 * Current combo state
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Advanced AI|State", Replicated)
	FHarmoniaComboState ComboState;

	/**
	 * Current tactical state
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Advanced AI|State", Replicated)
	FHarmoniaTacticalStateData TacticalState;

	// ============================================================================
	// Delegates
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Advanced AI|Events")
	FOnEmotionStateChangedDelegate OnEmotionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Advanced AI|Events")
	FOnComboExecutedDelegate OnComboExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Advanced AI|Events")
	FOnTacticalPositionFoundDelegate OnTacticalPositionFound;

	// ============================================================================
	// Emotion System
	// ============================================================================

	/**
	 * Trigger an emotion state
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Emotion")
	void TriggerEmotion(EHarmoniaMonsterEmotion Emotion, float Intensity = 1.0f, float Duration = 10.0f);

	/**
	 * Get current emotion
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Emotion")
	EHarmoniaMonsterEmotion GetCurrentEmotion() const { return EmotionState.CurrentEmotion; }

	/**
	 * Check if in specific emotion
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Emotion")
	bool IsInEmotion(EHarmoniaMonsterEmotion Emotion) const;

	/**
	 * Get attack modifier from emotion
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Emotion")
	float GetEmotionAttackModifier() const { return EmotionState.AttackModifier; }

	/**
	 * Get defense modifier from emotion
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Emotion")
	float GetEmotionDefenseModifier() const { return EmotionState.DefenseModifier; }

	// ============================================================================
	// Combo System
	// ============================================================================

	/**
	 * Start a combo chain with an attack
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Combat")
	void StartCombo(const FHarmoniaMonsterAttackPattern& AttackPattern);

	/**
	 * Try to continue combo with next attack
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Combat")
	bool TryContinueCombo(FName& OutNextAttackID);

	/**
	 * Reset combo state
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Combat")
	void ResetCombo();

	/**
	 * Check if combo is active
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Combat")
	bool IsComboActive() const { return ComboState.IsActive(); }

	// ============================================================================
	// Tactical Positioning
	// ============================================================================

	/**
	 * Find optimal tactical position
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Tactical")
	bool FindTacticalPosition(AActor* Target, EHarmoniaTacticalPosition PreferredType = EHarmoniaTacticalPosition::None);

	/**
	 * Get tactical position location
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Tactical")
	FVector GetTacticalPositionLocation() const { return TacticalState.TargetLocation; }

	/**
	 * Check if in optimal tactical position
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Tactical")
	bool IsInOptimalPosition() const { return TacticalState.bInOptimalPosition; }

	/**
	 * Find cover points near location
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Tactical")
	TArray<FVector> FindCoverPoints(const FVector& FromLocation, AActor* ThreatActor, float SearchRadius = 1000.0f);

	/**
	 * Check if location has high ground advantage
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Tactical")
	bool HasHighGroundAdvantage(const FVector& MyLocation, const FVector& TargetLocation, float MinHeightAdvantage = 100.0f) const;

	// ============================================================================
	// Enhanced Attack Selection
	// ============================================================================

	/**
	 * Select attack with context awareness (health, position, emotion)
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Combat")
	FHarmoniaMonsterAttackPattern SelectContextualAttack(AActor* Target, const TArray<FHarmoniaMonsterAttackPattern>& AvailableAttacks);

	/**
	 * Calculate attack priority based on context
	 */
	UFUNCTION(BlueprintCallable, Category = "Advanced AI|Combat")
	float CalculateAttackPriority(const FHarmoniaMonsterAttackPattern& AttackPattern, AActor* Target) const;

protected:
	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Update emotion system
	 */
	void UpdateEmotionSystem(float DeltaTime);

	/**
	 * Update combo system
	 */
	void UpdateComboSystem(float DeltaTime);

	/**
	 * Evaluate emotion triggers
	 */
	void EvaluateEmotionTriggers();

	/**
	 * Apply emotion modifiers to stats
	 */
	void ApplyEmotionModifiers(EHarmoniaMonsterEmotion Emotion, float Intensity);

	/**
	 * Get target health percentage
	 */
	float GetTargetHealthPercent(AActor* Target) const;

	/**
	 * Get relative power compared to target
	 */
	float GetRelativePower(AActor* Target) const;

	// ============================================================================
	// Internal State
	// ============================================================================

	/**
	 * Time in combat (for exhaustion tracking)
	 */
	float CombatTime = 0.0f;

	/**
	 * Cached owner monster
	 */
	UPROPERTY(Transient)
	TObjectPtr<AHarmoniaMonsterBase> OwnerMonster = nullptr;
};
