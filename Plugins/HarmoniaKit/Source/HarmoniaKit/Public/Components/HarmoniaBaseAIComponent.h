// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaBaseAIComponent.generated.h"

class AAIController;
class AHarmoniaMonsterBase;
class UBehaviorTree;

/**
 * Base class for all AI-related components in HarmoniaKit
 * Provides common functionality for AI behavior components
 *
 * Common features:
 * - AI Controller reference caching
 * - Monster reference caching
 * - Enable/disable state management
 * - Replication setup helpers
 * - Debug visualization hooks
 */
UCLASS(Abstract, ClassGroup=(HarmoniaKit))
class HARMONIAKIT_API UHarmoniaBaseAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBaseAIComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================================
	// Enable/Disable
	// ====================================

	/** Enable or disable this AI component */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|AI")
	virtual void SetAIComponentEnabled(bool bEnabled);

	/** Check if this AI component is enabled */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AI")
	bool IsAIComponentEnabled() const { return bComponentEnabled; }

	// ====================================
	// AI Controller Access
	// ====================================

	/** Get cached AI controller (finds and caches on first call) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AI")
	AAIController* GetAIController();

	/** Get cached monster reference (finds and caches on first call) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AI")
	AHarmoniaMonsterBase* GetMonster();

	/** Get controlled pawn */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AI")
	APawn* GetControlledPawn();

	// ====================================
	// Component State Queries
	// ====================================

	/** Check if AI has valid target */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AI")
	bool HasValidTarget() const;

	/** Check if AI is in combat */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AI")
	virtual bool IsInCombat() const;

	/** Get current target actor */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AI")
	AActor* GetCurrentTarget() const;

	// ====================================
	// Debug Visualization
	// ====================================

	/** Whether to show debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Debug")
	bool bShowDebugInfo = false;

	/** Draw debug information (called from TickComponent if bShowDebugInfo is true) */
	virtual void DrawDebugInfo();

	// ====================================
	// Configuration
	// ====================================

	/** Whether this component is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	bool bComponentEnabled = true;

	/** Update rate for this component (0 = every frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float UpdateRate = 0.0f;

protected:
	// ====================================
	// Protected Helpers
	// ====================================

	/** Called when component is enabled */
	virtual void OnComponentEnabled();

	/** Called when component is disabled */
	virtual void OnComponentDisabled();

	/** Update function called at specified rate */
	virtual void UpdateAIComponent(float DeltaTime);

	/** Initialize component-specific AI data */
	virtual void InitializeAIComponent();

	/** Cleanup component-specific AI data */
	virtual void CleanupAIComponent();

	// ====================================
	// Cached References
	// ====================================

	/** Cached AI controller reference */
	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController = nullptr;

	/** Cached monster reference */
	UPROPERTY()
	TObjectPtr<AHarmoniaMonsterBase> CachedMonster = nullptr;

	/** Time accumulator for update rate */
	float UpdateTimeAccumulator = 0.0f;

	/** Whether component has been initialized */
	bool bInitialized = false;

private:
	/** Invalidate cached references (called when owner changes) */
	void InvalidateCachedReferences();
};
