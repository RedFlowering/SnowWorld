// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaNetworkOptimizationComponent.generated.h"

class APlayerController;

/**
 * Network optimization level
 */
UENUM(BlueprintType)
enum class EHarmoniaNetOptLevel : uint8
{
	Critical,   // Always replicate
	High,       // High frequency updates
	Medium,     // Medium frequency updates
	Low,        // Low frequency updates
	Minimal,    // Minimal updates
	Dormant     // Dormancy enabled
};

/**
 * Network optimization configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaNetOptConfig
{
	GENERATED_BODY()

	// ============================================================================
	// Distance Thresholds
	// ============================================================================

	/** Distance for Critical level (closest) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float CriticalDistance = 500.0f;

	/** Distance for High level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float HighDistance = 1500.0f;

	/** Distance for Medium level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float MediumDistance = 3000.0f;

	/** Distance for Low level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float LowDistance = 6000.0f;

	/** Distance for Minimal level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float MinimalDistance = 10000.0f;

	// ============================================================================
	// Net Update Frequency Settings
	// ============================================================================

	/** NetUpdateFrequency at Critical level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NetUpdate")
	float CriticalNetUpdateFrequency = 100.0f;

	/** NetUpdateFrequency at High level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NetUpdate")
	float HighNetUpdateFrequency = 60.0f;

	/** NetUpdateFrequency at Medium level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NetUpdate")
	float MediumNetUpdateFrequency = 30.0f;

	/** NetUpdateFrequency at Low level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NetUpdate")
	float LowNetUpdateFrequency = 10.0f;

	/** NetUpdateFrequency at Minimal level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NetUpdate")
	float MinimalNetUpdateFrequency = 5.0f;

	// ============================================================================
	// Dormancy Settings
	// ============================================================================

	/** Whether to enable dormancy at Dormant level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dormancy")
	bool bEnableDormancy = true;

	/** Dormancy mode to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dormancy")
	TEnumAsByte<ENetDormancy> DormancyMode = DORM_DormantPartial;

	/** Time to wait before going dormant after conditions are met */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dormancy")
	float DormancyDelay = 2.0f;

	// ============================================================================
	// Relevancy Settings
	// ============================================================================

	/** Whether to use custom net relevancy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relevancy")
	bool bUseCustomRelevancy = false;

	/** Custom net cull distance squared */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relevancy", meta = (EditCondition = "bUseCustomRelevancy"))
	float NetCullDistanceSquared = 225000000.0f; // 15000 units

	// ============================================================================
	// Update Settings
	// ============================================================================

	/** How often to recalculate optimization level (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update")
	float RecalculationInterval = 0.5f;

	/** Whether to use combat state awareness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update")
	bool bUseCombatBoost = true;

	/** Minimum level during combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update", meta = (EditCondition = "bUseCombatBoost"))
	EHarmoniaNetOptLevel CombatMinLevel = EHarmoniaNetOptLevel::High;
};

/**
 * Delegate for optimization level changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetOptLevelChanged, EHarmoniaNetOptLevel, OldLevel, EHarmoniaNetOptLevel, NewLevel);

/**
 * Harmonia Network Optimization Component
 *
 * Manages network replication frequency based on distance to players.
 * Reduces bandwidth usage by adjusting NetUpdateFrequency and
 * enabling dormancy for distant actors.
 *
 * Features:
 * - Distance-based NetUpdateFrequency adjustment
 * - Automatic dormancy management
 * - Combat state awareness
 * - Net relevancy optimization
 * - Smooth level transitions
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaNetworkOptimizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaNetworkOptimizationComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Network optimization configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Network")
	FHarmoniaNetOptConfig Config;

	// ============================================================================
	// State Queries
	// ============================================================================

	/** Get current optimization level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Network")
	EHarmoniaNetOptLevel GetCurrentLevel() const { return CurrentLevel; }

	/** Check if actor is dormant */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Network")
	bool IsDormant() const;

	/** Get distance to nearest player */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Network")
	float GetDistanceToNearestPlayer() const { return CachedDistanceToNearestPlayer; }

	/** Get current net update frequency */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Network")
	float GetCurrentNetUpdateFrequency() const;

	// ============================================================================
	// Manual Control
	// ============================================================================

	/** Force a specific optimization level */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Network")
	void ForceLevel(EHarmoniaNetOptLevel Level);

	/** Clear forced level, return to automatic */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Network")
	void ClearForcedLevel();

	/** Set combat state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Network")
	void SetInCombat(bool bInCombat);

	/** Force wake from dormancy */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Network")
	void ForceNetUpdate();

	/** Flush dormancy (replicate all properties) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Network")
	void FlushNetDormancy();

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Network")
	FOnNetOptLevelChanged OnLevelChanged;

protected:
	/** Recalculate optimization level */
	void RecalculateLevel();

	/** Apply current level settings */
	void ApplyLevelSettings();

	/** Calculate distance to nearest player */
	float CalculateDistanceToNearestPlayer() const;

	/** Convert distance to optimization level */
	EHarmoniaNetOptLevel DistanceToLevel(float Distance) const;

	/** Get net update frequency for level */
	float GetNetUpdateFrequencyForLevel(EHarmoniaNetOptLevel Level) const;

	/** Handle dormancy transition */
	void HandleDormancyTransition(bool bShouldBeDormant);

private:
	/** Current optimization level */
	EHarmoniaNetOptLevel CurrentLevel = EHarmoniaNetOptLevel::Critical;

	/** Whether level is forced */
	bool bLevelForced = false;

	/** Forced level value */
	EHarmoniaNetOptLevel ForcedLevel = EHarmoniaNetOptLevel::Critical;

	/** Whether in combat */
	bool bIsInCombat = false;

	/** Time since last recalculation */
	float TimeSinceRecalculation = 0.0f;

	/** Cached distance to nearest player */
	float CachedDistanceToNearestPlayer = 0.0f;

	/** Original net update frequency */
	float OriginalNetUpdateFrequency = 100.0f;

	/** Original net dormancy */
	ENetDormancy OriginalDormancy = DORM_DormantPartial;

	/** Time in dormant-eligible state */
	float DormantEligibleTime = 0.0f;

	/** Whether dormancy is currently active */
	bool bDormancyActive = false;
};
