// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseAIComponent.h"
#include "System/HarmoniaCombatPowerCalculator.h"
#include "HarmoniaEnvironmentAwarenessComponent.generated.h"

/**
 * Environmental feature types
 */
UENUM(BlueprintType)
enum class EHarmoniaEnvironmentFeature : uint8
{
	Water			UMETA(DisplayName = "Water"),
	Cliff			UMETA(DisplayName = "Cliff"),
	NarrowPassage	UMETA(DisplayName = "Narrow Passage"),
	Cover			UMETA(DisplayName = "Cover"),
	Explosive		UMETA(DisplayName = "Explosive Object"),
	Trap			UMETA(DisplayName = "Trap"),
	HighGround		UMETA(DisplayName = "High Ground"),
	Hazard			UMETA(DisplayName = "Environmental Hazard")
};

/**
 * Environmental tactical opportunity
 */
USTRUCT(BlueprintType)
struct FHarmoniaEnvironmentalOpportunity
{
	GENERATED_BODY()

	/** Feature type */
	UPROPERTY(BlueprintReadOnly)
	EHarmoniaEnvironmentFeature FeatureType = EHarmoniaEnvironmentFeature::Water;

	/** Location of feature */
	UPROPERTY(BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;

	/** Tactical value (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float TacticalValue = 0.5f;

	/** Distance to feature */
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.0f;

	/** Is feature favorable for monster's element */
	UPROPERTY(BlueprintReadOnly)
	bool bFavorableForElement = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentOpportunityFound, EHarmoniaEnvironmentFeature, Feature, FVector, Location);

/**
 * Component that makes monsters aware of and utilize environmental features
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaEnvironmentAwarenessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaEnvironmentAwarenessComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Enable environment awareness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	bool bEnableAwareness = true;

	/** Scan radius for environmental features */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float ScanRadius = 2000.0f;

	/** Scan interval (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float ScanInterval = 2.0f;

	/** Monster element type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	EHarmoniaElementType MonsterElement = EHarmoniaElementType::None;

	UPROPERTY(BlueprintAssignable, Category = "Environment")
	FOnEnvironmentOpportunityFound OnOpportunityFound;

	/** Find best environmental opportunity */
	UFUNCTION(BlueprintCallable, Category = "Environment")
	FHarmoniaEnvironmentalOpportunity FindBestOpportunity(AActor* Target) const;

	/** Try to lure target to environmental feature */
	UFUNCTION(BlueprintCallable, Category = "Environment")
	bool TryLureToFeature(AActor* Target, EHarmoniaEnvironmentFeature FeatureType);

	/** Check if location has favorable terrain */
	UFUNCTION(BlueprintCallable, Category = "Environment")
	bool IsFavorableTerrain(const FVector& Location) const;

	/** Get detected terrain type at location */
	UFUNCTION(BlueprintCallable, Category = "Environment")
	EHarmoniaTerrainType DetectTerrainType(const FVector& Location) const;

protected:
	/** Scan for environmental features */
	void ScanEnvironment();

	/** Evaluate tactical value of feature */
	float EvaluateTacticalValue(const FHarmoniaEnvironmentalOpportunity& Opportunity, AActor* Target) const;

	float ScanTimer = 0.0f;

	UPROPERTY()
	TArray<FHarmoniaEnvironmentalOpportunity> DetectedOpportunities;
};
