// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaPartyScalingComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Scaling factor configuration for party size
 */
USTRUCT(BlueprintType)
struct FHarmoniaPartyScalingConfig
{
	GENERATED_BODY()

	/** Number of party members for this scaling tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	int32 PartySize = 1;

	/** Health multiplier for this party size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float HealthMultiplier = 1.0f;

	/** Damage multiplier for this party size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float DamageMultiplier = 1.0f;

	/** Experience reward multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float ExperienceMultiplier = 1.0f;

	/** Loot quantity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float LootMultiplier = 1.0f;

	FHarmoniaPartyScalingConfig()
		: PartySize(1)
		, HealthMultiplier(1.0f)
		, DamageMultiplier(1.0f)
		, ExperienceMultiplier(1.0f)
		, LootMultiplier(1.0f)
	{
	}
};

/**
 * Delegate for party size changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartySizeChanged, int32, OldSize, int32, NewSize);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScalingApplied, int32, PartySize, float, HealthMultiplier, float, DamageMultiplier);

/**
 * Component that detects nearby party members and scales monster difficulty accordingly
 * Attached to monster actors to make them scale based on party size instead of level
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaPartyScalingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaPartyScalingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== Configuration ====================

	/** Enable/disable party scaling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Scaling")
	bool bEnablePartyScaling = true;

	/** Detection radius to find party members */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Scaling", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
	float DetectionRadius = 3000.0f;

	/** How often to update party member count (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Scaling", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float UpdateInterval = 1.0f;

	/** Minimum distance between players to be considered separate party members */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Scaling", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float PlayerSeparationDistance = 500.0f;

	/** Scaling configurations per party size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Scaling")
	TArray<FHarmoniaPartyScalingConfig> ScalingConfigs;

	/** Show debug info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

	// ==================== Events ====================

	/** Called when party size changes */
	UPROPERTY(BlueprintAssignable, Category = "Party Scaling")
	FOnPartySizeChanged OnPartySizeChanged;

	/** Called when scaling is applied */
	UPROPERTY(BlueprintAssignable, Category = "Party Scaling")
	FOnScalingApplied OnScalingApplied;

	// ==================== Public API ====================

	/** Get current detected party size */
	UFUNCTION(BlueprintCallable, Category = "Party Scaling")
	int32 GetCurrentPartySize() const { return CurrentPartySize; }

	/** Get currently applied scaling configuration */
	UFUNCTION(BlueprintCallable, Category = "Party Scaling")
	FHarmoniaPartyScalingConfig GetCurrentScaling() const;

	/** Force update party member detection */
	UFUNCTION(BlueprintCallable, Category = "Party Scaling")
	void ForceUpdatePartySize();

	/** Manually set party size (for testing or forced scaling) */
	UFUNCTION(BlueprintCallable, Category = "Party Scaling")
	void SetPartySize(int32 NewSize);

	/** Get all detected party members */
	UFUNCTION(BlueprintCallable, Category = "Party Scaling")
	TArray<AActor*> GetDetectedPartyMembers() const { return DetectedPartyMembers; }

protected:
	// ==================== Internal Functions ====================

	/** Detect nearby party members */
	void DetectPartyMembers();

	/** Apply scaling based on party size */
	void ApplyScaling(int32 PartySize);

	/** Get scaling config for specific party size */
	FHarmoniaPartyScalingConfig GetScalingForPartySize(int32 PartySize) const;

	/** Initialize default scaling configs if empty */
	void InitializeDefaultScalingConfigs();

	/** Draw debug information */
	void DrawDebugInfo() const;

	// ==================== State ====================

	/** Current detected party size */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Party Scaling", meta = (AllowPrivateAccess = "true"))
	int32 CurrentPartySize = 1;

	/** Previously detected party size */
	int32 PreviousPartySize = 1;

	/** Currently detected party members */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Party Scaling", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> DetectedPartyMembers;

	/** Timer for update interval */
	float UpdateTimer = 0.0f;

	/** Cached owner monster */
	UPROPERTY()
	AHarmoniaMonsterBase* OwnerMonster = nullptr;

	/** Has scaling been applied at least once */
	bool bScalingApplied = false;
};
