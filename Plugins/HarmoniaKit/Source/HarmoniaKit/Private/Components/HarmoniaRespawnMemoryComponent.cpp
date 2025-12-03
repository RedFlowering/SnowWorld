// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaRespawnMemoryComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"

UHarmoniaRespawnMemoryComponent::UHarmoniaRespawnMemoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bEnableMemory = true;
	MaxMemories = 5;
	bAutoApplyCounterStrategy = true;
}

void UHarmoniaRespawnMemoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());

	if (bAutoApplyCounterStrategy && DeathMemories.Num() > 0)
	{
		FHarmoniaCounterStrategy Strategy = AnalyzeAndGenerateStrategy();
		ApplyCounterStrategy(Strategy);
	}
}

void UHarmoniaRespawnMemoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UHarmoniaRespawnMemoryComponent::RecordDeath(FVector Location, EHarmoniaDeathCause Cause, AActor* Killer, float HealthBeforeDeath)
{
	if (!bEnableMemory)
	{
		return;
	}

	FHarmoniaDeathMemory Memory;
	Memory.DeathLocation = Location;
	Memory.Cause = Cause;
	Memory.Killer = Killer;
	Memory.DeathTime = GetWorld()->GetTimeSeconds();
	Memory.HealthBeforeDeath = HealthBeforeDeath;

	DeathMemories.Add(Memory);

	// Keep only recent memories
	if (DeathMemories.Num() > MaxMemories)
	{
		DeathMemories.RemoveAt(0);
	}

	OnDeathRecorded.Broadcast(Location, Cause);

	UE_LOG(LogTemp, Log, TEXT("Death recorded: Cause=%d, Location=%s"), (int32)Cause, *Location.ToString());
}

FHarmoniaCounterStrategy UHarmoniaRespawnMemoryComponent::AnalyzeAndGenerateStrategy()
{
	FHarmoniaCounterStrategy Strategy;

	if (DeathMemories.Num() == 0)
	{
		return Strategy;
	}

	// Count death causes
	TMap<EHarmoniaDeathCause, int32> CauseCounts;
	for (const FHarmoniaDeathMemory& Memory : DeathMemories)
	{
		int32& Count = CauseCounts.FindOrAdd(Memory.Cause);
		Count++;
	}

	// Find most common cause
	EHarmoniaDeathCause MostCommonCause = EHarmoniaDeathCause::Unknown;
	int32 MaxCount = 0;
	for (const auto& Pair : CauseCounts)
	{
		if (Pair.Value > MaxCount)
		{
			MaxCount = Pair.Value;
			MostCommonCause = Pair.Key;
		}
	}

	// Generate counter-strategy based on common cause
	switch (MostCommonCause)
	{
	case EHarmoniaDeathCause::RangedAttack:
		Strategy.bPrioritizeRanged = false; // Close the distance
		Strategy.AggressionAdjustment = 0.3f; // More aggressive
		break;

	case EHarmoniaDeathCause::MeleeAttack:
		Strategy.bPrioritizeRanged = true; // Keep distance
		Strategy.bPlayDefensive = true;
		Strategy.AggressionAdjustment = -0.2f;
		break;

	case EHarmoniaDeathCause::Magic:
		Strategy.bPlayDefensive = true;
		Strategy.AggressionAdjustment = 0.1f; // Slightly more aggressive, interrupt casters
		break;

	case EHarmoniaDeathCause::Environmental:
		// Add death locations to avoid list
		for (const FHarmoniaDeathMemory& Memory : DeathMemories)
		{
			if (Memory.Cause == EHarmoniaDeathCause::Environmental)
			{
				Strategy.LocationsToAvoid.Add(Memory.DeathLocation);
			}
		}
		break;

	default:
		break;
	}

	return Strategy;
}

void UHarmoniaRespawnMemoryComponent::ApplyCounterStrategy(const FHarmoniaCounterStrategy& Strategy)
{
	CurrentStrategy = Strategy;
	bStrategyApplied = true;

	OnCounterStrategyApplied.Broadcast(Strategy);

	UE_LOG(LogTemp, Log, TEXT("Counter-strategy applied: Ranged=%d, Defensive=%d, Aggression=%.2f"),
		Strategy.bPrioritizeRanged, Strategy.bPlayDefensive, Strategy.AggressionAdjustment);
}

EHarmoniaDeathCause UHarmoniaRespawnMemoryComponent::GetMostCommonDeathCause() const
{
	TMap<EHarmoniaDeathCause, int32> CauseCounts;

	for (const FHarmoniaDeathMemory& Memory : DeathMemories)
	{
		int32& Count = CauseCounts.FindOrAdd(Memory.Cause);
		Count++;
	}

	EHarmoniaDeathCause MostCommonCause = EHarmoniaDeathCause::Unknown;
	int32 MaxCount = 0;

	for (const auto& Pair : CauseCounts)
	{
		if (Pair.Value > MaxCount)
		{
			MaxCount = Pair.Value;
			MostCommonCause = Pair.Key;
		}
	}

	return MostCommonCause;
}

bool UHarmoniaRespawnMemoryComponent::ShouldAvoidLocation(const FVector& Location, float Radius) const
{
	for (const FVector& AvoidLocation : CurrentStrategy.LocationsToAvoid)
	{
		if (FVector::Dist(Location, AvoidLocation) < Radius)
		{
			return true;
		}
	}

	return false;
}
