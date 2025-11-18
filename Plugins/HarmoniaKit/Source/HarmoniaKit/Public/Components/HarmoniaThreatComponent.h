// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaThreatComponent.generated.h"

/**
 * Threat Entry
 * Stores threat/aggro information for a single actor
 */
USTRUCT(BlueprintType)
struct FHarmoniaThreatEntry
{
	GENERATED_BODY()

	// Actor causing threat
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	TObjectPtr<AActor> ThreatActor = nullptr;

	// Current threat value
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float ThreatValue = 0.0f;

	// Time when last threat was added
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float LastThreatTime = 0.0f;

	bool IsValid() const
	{
		return ThreatActor != nullptr && !ThreatActor->IsPendingKillPending();
	}
};

/**
 * Threat Changed Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnThreatChangedDelegate, AActor*, ThreatActor, float, OldThreat, float, NewThreat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHighestThreatChangedDelegate, AActor*, OldTarget, AActor*, NewTarget);

/**
 * UHarmoniaThreatComponent
 *
 * Manages threat/aggro table for monsters
 * Tracks which actors are threatening the monster and by how much
 *
 * Features:
 * - Threat generation from damage, healing, taunts
 * - Threat decay over time
 * - Threat multipliers for tanks/DPS/healers
 * - Smart target selection based on threat
 * - Tank taunt support
 *
 * Usage: Add to monster actors to enable threat system
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaThreatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaThreatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Threat decay rate per second (percentage)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat|Config")
	float ThreatDecayRate = 0.05f;

	/**
	 * Minimum threat value before removing from table
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat|Config")
	float MinimumThreat = 1.0f;

	/**
	 * Maximum threat value (clamped)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat|Config")
	float MaximumThreat = 10000.0f;

	/**
	 * Distance beyond which threat starts decaying faster
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat|Config")
	float ThreatDecayDistance = 3000.0f;

	/**
	 * Multiplier for threat decay when out of range
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat|Config")
	float OutOfRangeDecayMultiplier = 2.0f;

	// ============================================================================
	// Threat Management
	// ============================================================================

	/**
	 * Add threat to an actor
	 * @param ThreatActor Actor to add threat to
	 * @param ThreatAmount Amount of threat to add
	 * @param bMultiplyByModifier Whether to apply threat multipliers
	 */
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void AddThreat(AActor* ThreatActor, float ThreatAmount, bool bMultiplyByModifier = true);

	/**
	 * Remove threat from an actor
	 * @param ThreatActor Actor to remove threat from
	 * @param ThreatAmount Amount to remove (negative = add)
	 */
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void RemoveThreat(AActor* ThreatActor, float ThreatAmount);

	/**
	 * Set threat directly for an actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void SetThreat(AActor* ThreatActor, float ThreatValue);

	/**
	 * Clear threat for specific actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void ClearThreat(AActor* ThreatActor);

	/**
	 * Clear all threat
	 */
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void ClearAllThreat();

	/**
	 * Taunt - force highest threat for duration
	 * @param TauntActor Actor that taunted
	 * @param Duration How long the taunt lasts
	 */
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void Taunt(AActor* TauntActor, float Duration = 3.0f);

	// ============================================================================
	// Threat Queries
	// ============================================================================

	/**
	 * Get threat value for specific actor
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Threat")
	float GetThreat(AActor* ThreatActor) const;

	/**
	 * Get actor with highest threat
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Threat")
	AActor* GetHighestThreatActor() const;

	/**
	 * Get all threat entries sorted by threat value
	 */
	UFUNCTION(BlueprintCallable, Category = "Threat")
	TArray<FHarmoniaThreatEntry> GetThreatTable(bool bSorted = true) const;

	/**
	 * Get number of actors on threat table
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Threat")
	int32 GetThreatTableSize() const { return ThreatTable.Num(); }

	// ============================================================================
	// Delegates
	// ============================================================================

	/**
	 * Called when threat changes for an actor
	 */
	UPROPERTY(BlueprintAssignable, Category = "Threat|Events")
	FOnThreatChangedDelegate OnThreatChanged;

	/**
	 * Called when highest threat actor changes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Threat|Events")
	FOnHighestThreatChangedDelegate OnHighestThreatChanged;

protected:
	/**
	 * Threat table
	 */
	UPROPERTY(Transient)
	TArray<FHarmoniaThreatEntry> ThreatTable;

	/**
	 * Current highest threat actor (cached)
	 */
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentHighestThreatActor = nullptr;

	/**
	 * Actor currently taunting (has forced highest threat)
	 */
	UPROPERTY(Transient)
	TObjectPtr<AActor> TauntActor = nullptr;

	/**
	 * Timer handle for taunt duration
	 */
	FTimerHandle TauntTimerHandle;

	/**
	 * Update threat decay
	 */
	void UpdateThreatDecay(float DeltaTime);

	/**
	 * Remove invalid/expired entries
	 */
	void CleanupThreatTable();

	/**
	 * Update highest threat actor
	 */
	void UpdateHighestThreatActor();

	/**
	 * Find or create threat entry for actor
	 */
	FHarmoniaThreatEntry* FindOrAddThreatEntry(AActor* ThreatActor);

	/**
	 * Called when taunt expires
	 */
	void OnTauntExpired();
};
