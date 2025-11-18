// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaEnvironmentAwarenessComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UHarmoniaEnvironmentAwarenessComponent::UHarmoniaEnvironmentAwarenessComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;

	bEnableAwareness = true;
	ScanRadius = 2000.0f;
	ScanInterval = 2.0f;
	MonsterElement = EHarmoniaElementType::None;
}

void UHarmoniaEnvironmentAwarenessComponent::BeginPlay()
{
	Super::BeginPlay();

	ScanEnvironment();
}

void UHarmoniaEnvironmentAwarenessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableAwareness)
	{
		return;
	}

	ScanTimer += DeltaTime;
	if (ScanTimer >= ScanInterval)
	{
		ScanTimer = 0.0f;
		ScanEnvironment();
	}
}

void UHarmoniaEnvironmentAwarenessComponent::ScanEnvironment()
{
	DetectedOpportunities.Empty();

	// Simplified scan - in full implementation would use physics queries
	// to find actual environmental features
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Placeholder: detect terrain type at owner location
	FVector OwnerLocation = Owner->GetActorLocation();
	EHarmoniaTerrainType Terrain = DetectTerrainType(OwnerLocation);

	// Log detected terrain
	UE_LOG(LogTemp, Verbose, TEXT("Environment scan complete: Terrain=%s"),
		*UHarmoniaCombatPowerCalculator::TerrainToString(Terrain));
}

FHarmoniaEnvironmentalOpportunity UHarmoniaEnvironmentAwarenessComponent::FindBestOpportunity(AActor* Target) const
{
	FHarmoniaEnvironmentalOpportunity BestOpportunity;
	float BestValue = 0.0f;

	for (const FHarmoniaEnvironmentalOpportunity& Opp : DetectedOpportunities)
	{
		float Value = EvaluateTacticalValue(Opp, Target);
		if (Value > BestValue)
		{
			BestValue = Value;
			BestOpportunity = Opp;
		}
	}

	return BestOpportunity;
}

bool UHarmoniaEnvironmentAwarenessComponent::TryLureToFeature(AActor* Target, EHarmoniaEnvironmentFeature FeatureType)
{
	// Placeholder implementation
	return false;
}

bool UHarmoniaEnvironmentAwarenessComponent::IsFavorableTerrain(const FVector& Location) const
{
	EHarmoniaTerrainType Terrain = DetectTerrainType(Location);
	float Multiplier = UHarmoniaCombatPowerCalculator::GetTerrainMultiplier(MonsterElement, Terrain);
	return Multiplier > 1.0f;
}

EHarmoniaTerrainType UHarmoniaEnvironmentAwarenessComponent::DetectTerrainType(const FVector& Location) const
{
	// Simplified terrain detection based on Z height and position
	// In full implementation would use landscape queries

	// Check Z height
	if (Location.Z > 5000.0f)
	{
		return EHarmoniaTerrainType::HighGround;
	}
	else if (Location.Z < -1000.0f)
	{
		return EHarmoniaTerrainType::LowGround;
	}

	// Default to flat
	return EHarmoniaTerrainType::Flat;
}

float UHarmoniaEnvironmentAwarenessComponent::EvaluateTacticalValue(const FHarmoniaEnvironmentalOpportunity& Opportunity, AActor* Target) const
{
	float Value = Opportunity.TacticalValue;

	if (Opportunity.bFavorableForElement)
	{
		Value *= 1.5f;
	}

	// Closer is generally better
	float DistanceFactor = 1.0f - FMath::Clamp(Opportunity.Distance / ScanRadius, 0.0f, 1.0f);
	Value *= (0.5f + DistanceFactor * 0.5f);

	return Value;
}
