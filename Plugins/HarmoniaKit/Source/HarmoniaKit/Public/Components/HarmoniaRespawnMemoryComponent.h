// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaRespawnMemoryComponent.generated.h"

/**
 * Death cause tracking
 */
UENUM(BlueprintType)
enum class EHarmoniaDeathCause : uint8
{
	MeleeAttack		UMETA(DisplayName = "Melee Attack"),
	RangedAttack	UMETA(DisplayName = "Ranged Attack"),
	Magic			UMETA(DisplayName = "Magic/Ability"),
	Environmental	UMETA(DisplayName = "Environmental Hazard"),
	Unknown			UMETA(DisplayName = "Unknown")
};

/**
 * Death memory record
 */
USTRUCT(BlueprintType)
struct FHarmoniaDeathMemory
{
	GENERATED_BODY()

	/** Death location */
	UPROPERTY(BlueprintReadOnly)
	FVector DeathLocation;

	/** Cause of death */
	UPROPERTY(BlueprintReadOnly)
	EHarmoniaDeathCause Cause;

	/** Killer actor */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Killer;

	/** Abilities used by player before death */
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> PlayerAbilitiesUsed;

	/** Time of death */
	UPROPERTY(BlueprintReadOnly)
	float DeathTime;

	/** Health at time of death attempt */
	UPROPERTY(BlueprintReadOnly)
	float HealthBeforeDeath;
};

/**
 * Counter-strategy for next respawn
 */
USTRUCT(BlueprintType)
struct FHarmoniaCounterStrategy
{
	GENERATED_BODY()

	/** Should prioritize ranged combat */
	UPROPERTY(BlueprintReadWrite)
	bool bPrioritizeRanged = false;

	/** Should play more defensively */
	UPROPERTY(BlueprintReadWrite)
	bool bPlayDefensive = false;

	/** Should avoid certain locations */
	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> LocationsToAvoid;

	/** Should use specific counter-abilities */
	UPROPERTY(BlueprintReadWrite)
	TArray<FName> CounterAbilities;

	/** Aggression level adjustment (-1.0 to 1.0) */
	UPROPERTY(BlueprintReadWrite)
	float AggressionAdjustment = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathRecorded, FVector, Location, EHarmoniaDeathCause, Cause);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCounterStrategyApplied, const FHarmoniaCounterStrategy&, Strategy);

/**
 * Component that remembers how monster died and applies counter-strategies on respawn
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaRespawnMemoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRespawnMemoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Enable respawn memory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Memory")
	bool bEnableMemory = true;

	/** Maximum death memories to keep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Memory")
	int32 MaxMemories = 5;

	/** Apply counter-strategy on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Memory")
	bool bAutoApplyCounterStrategy = true;

	/** Death memories */
	UPROPERTY(BlueprintReadOnly, Category = "Respawn Memory")
	TArray<FHarmoniaDeathMemory> DeathMemories;

	/** Current counter-strategy */
	UPROPERTY(BlueprintReadOnly, Category = "Respawn Memory")
	FHarmoniaCounterStrategy CurrentStrategy;

	UPROPERTY(BlueprintAssignable, Category = "Respawn Memory")
	FOnDeathRecorded OnDeathRecorded;

	UPROPERTY(BlueprintAssignable, Category = "Respawn Memory")
	FOnCounterStrategyApplied OnCounterStrategyApplied;

	/** Record death */
	UFUNCTION(BlueprintCallable, Category = "Respawn Memory")
	void RecordDeath(FVector Location, EHarmoniaDeathCause Cause, AActor* Killer, float HealthBeforeDeath);

	/** Analyze deaths and generate counter-strategy */
	UFUNCTION(BlueprintCallable, Category = "Respawn Memory")
	FHarmoniaCounterStrategy AnalyzeAndGenerateStrategy();

	/** Apply counter-strategy to monster */
	UFUNCTION(BlueprintCallable, Category = "Respawn Memory")
	void ApplyCounterStrategy(const FHarmoniaCounterStrategy& Strategy);

	/** Get most common death cause */
	UFUNCTION(BlueprintCallable, Category = "Respawn Memory")
	EHarmoniaDeathCause GetMostCommonDeathCause() const;

	/** Should avoid location */
	UFUNCTION(BlueprintCallable, Category = "Respawn Memory")
	bool ShouldAvoidLocation(const FVector& Location, float Radius = 500.0f) const;

protected:
	UPROPERTY()
	class AHarmoniaMonsterBase* OwnerMonster;

	bool bStrategyApplied = false;
};
