// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaAILODComponent.generated.h"

class APlayerController;

/**
 * AI LOD (Level of Detail) Level
 * Defines different update frequencies based on distance/visibility
 */
UENUM(BlueprintType)
enum class EHarmoniaAILODLevel : uint8
{
	VeryHigh UMETA(DisplayName = "Very High"),	// 0.1s - Close and visible
	High UMETA(DisplayName = "High"),			// 0.25s - Medium distance
	Medium UMETA(DisplayName = "Medium"),		// 0.5s - Far distance
	Low UMETA(DisplayName = "Low"),				// 1.0s - Very far
	VeryLow UMETA(DisplayName = "Very Low"),	// 2.0s - Off-screen or extremely far
	Disabled UMETA(DisplayName = "Disabled")	// No updates
};

/**
 * LOD State Changed Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAILODLevelChangedDelegate, EHarmoniaAILODLevel, OldLevel, EHarmoniaAILODLevel, NewLevel);

/**
 * UHarmoniaAILODComponent
 *
 * Manages AI update frequency based on distance and visibility
 * Optimizes performance by reducing update rate for distant/off-screen monsters
 *
 * Features:
 * - Distance-based LOD levels
 * - Screen visibility checks
 * - Player proximity tracking
 * - Automatic tick rate adjustment
 * - Combat override (always high LOD in combat)
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaAILODComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaAILODComponent();

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~End of UActorComponent interface

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Enable LOD system
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD")
	bool bEnableLOD = true;

	/**
	 * Distance thresholds for LOD levels (in cm)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Distance")
	float VeryHighDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Distance")
	float HighDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Distance")
	float MediumDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Distance")
	float LowDistance = 8000.0f;

	/**
	 * Update intervals for each LOD level (in seconds)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Update Rate")
	float VeryHighUpdateInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Update Rate")
	float HighUpdateInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Update Rate")
	float MediumUpdateInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Update Rate")
	float LowUpdateInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Update Rate")
	float VeryLowUpdateInterval = 2.0f;

	/**
	 * Whether to check screen visibility
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Visibility")
	bool bCheckScreenVisibility = true;

	/**
	 * Always use high LOD when in combat
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Combat")
	bool bForceHighLODInCombat = true;

	/**
	 * Distance to always use high LOD (regardless of other factors)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Combat")
	float CombatHighLODDistance = 2500.0f;

	// ============================================================================
	// State
	// ============================================================================

	/**
	 * Current LOD level
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AI LOD|State")
	EHarmoniaAILODLevel CurrentLODLevel = EHarmoniaAILODLevel::High;

	/**
	 * Whether currently visible on screen
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AI LOD|State")
	bool bIsVisibleOnScreen = true;

	/**
	 * Distance to nearest player
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AI LOD|State")
	float DistanceToNearestPlayer = 0.0f;

	// ============================================================================
	// Delegates
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "AI LOD|Events")
	FOnAILODLevelChangedDelegate OnLODLevelChanged;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * Get current LOD level
	 */
	UFUNCTION(BlueprintCallable, Category = "AI LOD")
	EHarmoniaAILODLevel GetCurrentLODLevel() const { return CurrentLODLevel; }

	/**
	 * Get update interval for current LOD level
	 */
	UFUNCTION(BlueprintCallable, Category = "AI LOD")
	float GetCurrentUpdateInterval() const;

	/**
	 * Check if should update this frame (based on LOD)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI LOD")
	bool ShouldUpdateThisFrame() const;

	/**
	 * Force specific LOD level (overrides automatic calculation)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI LOD")
	void ForceLODLevel(EHarmoniaAILODLevel Level);

	/**
	 * Clear forced LOD level (return to automatic)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI LOD")
	void ClearForcedLODLevel();

	/**
	 * Get nearest player controller
	 */
	UFUNCTION(BlueprintCallable, Category = "AI LOD")
	APlayerController* GetNearestPlayer() const;

protected:
	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Update LOD level based on distance and visibility
	 */
	void UpdateLODLevel();

	/**
	 * Calculate LOD level from distance
	 */
	EHarmoniaAILODLevel CalculateLODFromDistance(float Distance) const;

	/**
	 * Check if actor is visible on screen
	 */
	bool IsVisibleOnScreen(APlayerController* PlayerController) const;

	/**
	 * Find nearest player and distance
	 */
	void FindNearestPlayer();

	/**
	 * Check if owner is in combat
	 */
	bool IsInCombat() const;

	// ============================================================================
	// Internal State
	// ============================================================================

	/**
	 * Time since last update
	 */
	float TimeSinceLastUpdate = 0.0f;

	/**
	 * Forced LOD level (if set)
	 */
	EHarmoniaAILODLevel ForcedLODLevel = EHarmoniaAILODLevel::VeryHigh;

	/**
	 * Whether LOD level is forced
	 */
	bool bLODLevelForced = false;

	/**
	 * Cached nearest player
	 */
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> NearestPlayer = nullptr;
};
