// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaUpdateRateSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

DECLARE_CYCLE_STAT(TEXT("UpdateRate Tick"), STAT_UpdateRateTick, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("UpdateRate CalculateTiers"), STAT_UpdateRateCalculateTiers, STATGROUP_Game);

void UHarmoniaUpdateRateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default combat tag
	DefaultConfig.InCombatTag = FGameplayTag::RequestGameplayTag(FName("State.Combat"));

	// Register for world tick
	if (UWorld* World = GetWorld())
	{
		TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UHarmoniaUpdateRateSubsystem::Tick),
			0.0f // Tick every frame
		);
	}
}

void UHarmoniaUpdateRateSubsystem::Deinitialize()
{
	// Unregister tick
	if (TickDelegateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
		TickDelegateHandle.Reset();
	}

	TrackedActors.Empty();
	ForcedTiers.Empty();

	Super::Deinitialize();
}

bool UHarmoniaUpdateRateSubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateRateTick);

	// Reset frame statistics
	CurrentStats.UpdatesThisFrame = 0;
	CurrentStats.UpdatesSkippedThisFrame = 0;

	// Update player location cache
	CachedPlayerLocation = GetPlayerLocation();

	// Accumulate time for tier updates
	TierUpdateAccumulator += DeltaTime;

	// Periodically recalculate tiers (not every frame)
	if (TierUpdateAccumulator >= TierUpdateInterval)
	{
		TierUpdateAccumulator = 0.0f;
		UpdateActorTiers();
		CleanupInvalidActors();
	}

	// Update statistics
	if (bCollectStatistics)
	{
		UpdateStatistics();
	}

	return true;  // Continue ticking
}

// ============================================================================
// Actor Registration
// ============================================================================

void UHarmoniaUpdateRateSubsystem::RegisterActor(AActor* Actor, const FHarmoniaUpdateRateConfig& Config)
{
	if (!Actor)
	{
		return;
	}

	FHarmoniaTrackedActorData Data;
	Data.Actor = Actor;
	Data.Config = Config;
	Data.CurrentTier = EHarmoniaUpdateTier::Medium;
	Data.LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Data.DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), CachedPlayerLocation);

	TrackedActors.Add(Actor, Data);
}

void UHarmoniaUpdateRateSubsystem::RegisterActorWithDefaults(AActor* Actor)
{
	RegisterActor(Actor, DefaultConfig);
}

void UHarmoniaUpdateRateSubsystem::UnregisterActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	TrackedActors.Remove(Actor);
	ForcedTiers.Remove(Actor);
}

bool UHarmoniaUpdateRateSubsystem::IsActorRegistered(AActor* Actor) const
{
	return Actor && TrackedActors.Contains(Actor);
}

// ============================================================================
// Update Queries
// ============================================================================

bool UHarmoniaUpdateRateSubsystem::ShouldActorUpdate(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	const FHarmoniaTrackedActorData* Data = TrackedActors.Find(Actor);
	if (!Data)
	{
		return true; // Non-tracked actors always update
	}

	// Dormant actors never update
	if (Data->CurrentTier == EHarmoniaUpdateTier::Dormant)
	{
		return false;
	}

	// Critical actors always update
	if (Data->CurrentTier == EHarmoniaUpdateTier::Critical)
	{
		return true;
	}

	// Check time since last update
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	float TimeSinceUpdate = CurrentTime - Data->LastUpdateTime;
	float RequiredInterval = GetIntervalForTier(Data->CurrentTier, Data->Config) * GlobalUpdateMultiplier;

	return TimeSinceUpdate >= RequiredInterval;
}

EHarmoniaUpdateTier UHarmoniaUpdateRateSubsystem::GetActorUpdateTier(AActor* Actor) const
{
	if (!Actor)
	{
		return EHarmoniaUpdateTier::Dormant;
	}

	// Check forced tier first
	if (const EHarmoniaUpdateTier* ForcedTier = ForcedTiers.Find(Actor))
	{
		return *ForcedTier;
	}

	const FHarmoniaTrackedActorData* Data = TrackedActors.Find(Actor);
	return Data ? Data->CurrentTier : EHarmoniaUpdateTier::Medium;
}

float UHarmoniaUpdateRateSubsystem::GetActorUpdateInterval(AActor* Actor) const
{
	if (!Actor)
	{
		return 0.0f;
	}

	const FHarmoniaTrackedActorData* Data = TrackedActors.Find(Actor);
	if (!Data)
	{
		return 0.0f;
	}

	return GetIntervalForTier(Data->CurrentTier, Data->Config);
}

float UHarmoniaUpdateRateSubsystem::GetTimeSinceLastUpdate(AActor* Actor) const
{
	if (!Actor)
	{
		return 0.0f;
	}

	const FHarmoniaTrackedActorData* Data = TrackedActors.Find(Actor);
	if (!Data)
	{
		return 0.0f;
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return CurrentTime - Data->LastUpdateTime;
}

// ============================================================================
// Manual Control
// ============================================================================

void UHarmoniaUpdateRateSubsystem::ForceActorTier(AActor* Actor, EHarmoniaUpdateTier Tier)
{
	if (!Actor)
	{
		return;
	}

	ForcedTiers.Add(Actor, Tier);

	// Also update tracked data if registered
	if (FHarmoniaTrackedActorData* Data = TrackedActors.Find(Actor))
	{
		EHarmoniaUpdateTier OldTier = Data->CurrentTier;
		Data->CurrentTier = Tier;

		if (OldTier != Tier)
		{
			OnTierChanged.Broadcast(Actor, OldTier, Tier);
		}
	}
}

void UHarmoniaUpdateRateSubsystem::ClearForcedTier(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	ForcedTiers.Remove(Actor);
}

void UHarmoniaUpdateRateSubsystem::SetActorInCombat(AActor* Actor, bool bInCombat)
{
	if (!Actor)
	{
		return;
	}

	if (FHarmoniaTrackedActorData* Data = TrackedActors.Find(Actor))
	{
		Data->bIsInCombat = bInCombat;
	}
}

void UHarmoniaUpdateRateSubsystem::MarkActorUpdated(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	if (FHarmoniaTrackedActorData* Data = TrackedActors.Find(Actor))
	{
		Data->LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		CurrentStats.UpdatesThisFrame++;
	}
}

// ============================================================================
// Configuration
// ============================================================================

void UHarmoniaUpdateRateSubsystem::SetDefaultConfig(const FHarmoniaUpdateRateConfig& Config)
{
	DefaultConfig = Config;
}

void UHarmoniaUpdateRateSubsystem::SetGlobalUpdateMultiplier(float Multiplier)
{
	GlobalUpdateMultiplier = FMath::Clamp(Multiplier, 0.1f, 10.0f);
}

// ============================================================================
// Internal
// ============================================================================

void UHarmoniaUpdateRateSubsystem::UpdateActorTiers()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateRateCalculateTiers);

	for (auto& Pair : TrackedActors)
	{
		if (!Pair.Key.IsValid())
		{
			continue;
		}

		FHarmoniaTrackedActorData& Data = Pair.Value;
		AActor* Actor = Data.Actor.Get();

		if (!Actor)
		{
			continue;
		}

		// Skip if tier is forced
		if (ForcedTiers.Contains(Actor))
		{
			continue;
		}

		// Update distance
		Data.DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), CachedPlayerLocation);

		// Update visibility
		if (Data.Config.bConsiderVisibility)
		{
			Data.bIsVisible = IsActorVisible(Actor);
		}

		// Update combat state from GAS if configured
		if (Data.Config.bConsiderCombatState && Data.Config.InCombatTag.IsValid())
		{
			if (UAbilitySystemComponent* ASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
			{
				Data.bIsInCombat = ASC->HasMatchingGameplayTag(Data.Config.InCombatTag);
			}
		}

		// Calculate new tier
		EHarmoniaUpdateTier NewTier = CalculateTier(Data);

		// Broadcast if changed
		if (NewTier != Data.CurrentTier)
		{
			EHarmoniaUpdateTier OldTier = Data.CurrentTier;
			Data.CurrentTier = NewTier;
			OnTierChanged.Broadcast(Actor, OldTier, NewTier);
		}
	}
}

EHarmoniaUpdateTier UHarmoniaUpdateRateSubsystem::CalculateTier(const FHarmoniaTrackedActorData& Data) const
{
	const FHarmoniaUpdateRateConfig& Config = Data.Config;
	float Distance = Data.DistanceToPlayer;

	// Base tier from distance
	EHarmoniaUpdateTier Tier;

	if (Distance <= Config.CriticalDistance)
	{
		Tier = EHarmoniaUpdateTier::Critical;
	}
	else if (Distance <= Config.HighDistance)
	{
		Tier = EHarmoniaUpdateTier::High;
	}
	else if (Distance <= Config.MediumDistance)
	{
		Tier = EHarmoniaUpdateTier::Medium;
	}
	else if (Distance <= Config.LowDistance)
	{
		Tier = EHarmoniaUpdateTier::Low;
	}
	else if (Distance <= Config.MinimalDistance)
	{
		Tier = EHarmoniaUpdateTier::Minimal;
	}
	else
	{
		Tier = EHarmoniaUpdateTier::Dormant;
	}

	// Promote if in combat
	if (Config.bConsiderCombatState && Data.bIsInCombat)
	{
		// In combat actors get at least High tier
		if (Tier > EHarmoniaUpdateTier::High)
		{
			Tier = EHarmoniaUpdateTier::High;
		}
	}

	// Demote if off-screen (unless Critical or in combat)
	if (Config.bDemoteOffScreen && !Data.bIsVisible && !Data.bIsInCombat)
	{
		if (Tier < EHarmoniaUpdateTier::Dormant && Tier != EHarmoniaUpdateTier::Critical)
		{
			Tier = static_cast<EHarmoniaUpdateTier>(static_cast<uint8>(Tier) + 1);
		}
	}

	return Tier;
}

float UHarmoniaUpdateRateSubsystem::GetIntervalForTier(EHarmoniaUpdateTier Tier, const FHarmoniaUpdateRateConfig& Config) const
{
	switch (Tier)
	{
	case EHarmoniaUpdateTier::Critical:
		return Config.CriticalInterval;
	case EHarmoniaUpdateTier::High:
		return Config.HighInterval;
	case EHarmoniaUpdateTier::Medium:
		return Config.MediumInterval;
	case EHarmoniaUpdateTier::Low:
		return Config.LowInterval;
	case EHarmoniaUpdateTier::Minimal:
		return Config.MinimalInterval;
	case EHarmoniaUpdateTier::Dormant:
	default:
		return MAX_FLT;
	}
}

FVector UHarmoniaUpdateRateSubsystem::GetPlayerLocation() const
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				return Pawn->GetActorLocation();
			}
		}
	}
	return FVector::ZeroVector;
}

bool UHarmoniaUpdateRateSubsystem::IsActorVisible(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// Simple visibility check - actor was recently rendered
	return Actor->WasRecentlyRendered(0.2f);
}

void UHarmoniaUpdateRateSubsystem::CleanupInvalidActors()
{
	TArray<TWeakObjectPtr<AActor>> ToRemove;

	for (auto& Pair : TrackedActors)
	{
		if (!Pair.Key.IsValid() || !Pair.Value.Actor.IsValid())
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const TWeakObjectPtr<AActor>& Actor : ToRemove)
	{
		TrackedActors.Remove(Actor);
		ForcedTiers.Remove(Actor);
	}
}

void UHarmoniaUpdateRateSubsystem::UpdateStatistics()
{
	CurrentStats.TotalTrackedActors = TrackedActors.Num();
	CurrentStats.CriticalCount = 0;
	CurrentStats.HighCount = 0;
	CurrentStats.MediumCount = 0;
	CurrentStats.LowCount = 0;
	CurrentStats.MinimalCount = 0;
	CurrentStats.DormantCount = 0;

	for (const auto& Pair : TrackedActors)
	{
		switch (Pair.Value.CurrentTier)
		{
		case EHarmoniaUpdateTier::Critical:
			CurrentStats.CriticalCount++;
			break;
		case EHarmoniaUpdateTier::High:
			CurrentStats.HighCount++;
			break;
		case EHarmoniaUpdateTier::Medium:
			CurrentStats.MediumCount++;
			break;
		case EHarmoniaUpdateTier::Low:
			CurrentStats.LowCount++;
			break;
		case EHarmoniaUpdateTier::Minimal:
			CurrentStats.MinimalCount++;
			break;
		case EHarmoniaUpdateTier::Dormant:
			CurrentStats.DormantCount++;
			break;
		}
	}
}
