// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaMonsterDebugComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Harmonia Monster Debug Component
 *
 * Provides visual debugging for monster AI behavior
 * Displays: State, Target, Threat Table, Sensed Targets, etc.
 *
 * Usage:
 * - Add to monster Blueprint
 * - Enable bShowDebugInfo in editor or runtime
 * - Toggle specific debug categories
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMonsterDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaMonsterDebugComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Debug Settings
	// ============================================================================

	/**
	 * Master toggle for all debug visualization
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo = false;

	/**
	 * Show current AI state and behavior
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Categories")
	bool bShowAIState = true;

	/**
	 * Show current target with line
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Categories")
	bool bShowTarget = true;

	/**
	 * Show threat table (top 5 threats)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Categories")
	bool bShowThreatTable = true;

	/**
	 * Show patrol path and home location
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Categories")
	bool bShowPatrolInfo = true;

	/**
	 * Show health bar above monster
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Categories")
	bool bShowHealthBar = true;

	/**
	 * Show faction information
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Categories")
	bool bShowFaction = true;

	/**
	 * Draw distance for debug information
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Settings")
	float DebugDrawDistance = 5000.0f;

	/**
	 * Text size multiplier
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Settings", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float TextScale = 1.0f;

protected:
	/**
	 * Draw AI state information
	 */
	void DrawAIState(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation);

	/**
	 * Draw current target
	 */
	void DrawTarget(AHarmoniaMonsterBase* Monster);

	/**
	 * Draw threat table
	 */
	void DrawThreatTable(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation);

	/**
	 * Draw patrol information
	 */
	void DrawPatrolInfo(AHarmoniaMonsterBase* Monster);

	/**
	 * Draw health bar
	 */
	void DrawHealthBar(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation);

	/**
	 * Draw faction information
	 */
	void DrawFaction(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation);

	/**
	 * Get monster reference (cached)
	 */
	AHarmoniaMonsterBase* GetMonster();

private:
	/** Cached monster reference */
	UPROPERTY(Transient)
	TObjectPtr<AHarmoniaMonsterBase> CachedMonster = nullptr;
};
