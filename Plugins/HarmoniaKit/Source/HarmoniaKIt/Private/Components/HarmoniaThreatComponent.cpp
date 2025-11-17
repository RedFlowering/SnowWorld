// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaThreatComponent.h"
#include "TimerManager.h"

UHarmoniaThreatComponent::UHarmoniaThreatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second
}

void UHarmoniaThreatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update threat decay
	UpdateThreatDecay(DeltaTime);

	// Cleanup invalid entries
	CleanupThreatTable();

	// Update highest threat
	UpdateHighestThreatActor();
}

// ============================================================================
// Threat Management
// ============================================================================

void UHarmoniaThreatComponent::AddThreat(AActor* ThreatActor, float ThreatAmount, bool bMultiplyByModifier)
{
	if (!ThreatActor || ThreatAmount <= 0.0f)
	{
		return;
	}

	FHarmoniaThreatEntry* Entry = FindOrAddThreatEntry(ThreatActor);
	if (!Entry)
	{
		return;
	}

	float OldThreat = Entry->ThreatValue;

	// Apply threat multiplier if needed
	// This could be extended with actor-specific multipliers (tank, DPS, healer roles)
	float FinalThreat = ThreatAmount;

	Entry->ThreatValue = FMath::Clamp(Entry->ThreatValue + FinalThreat, 0.0f, MaximumThreat);
	Entry->LastThreatTime = GetWorld()->GetTimeSeconds();

	// Broadcast threat change
	OnThreatChanged.Broadcast(ThreatActor, OldThreat, Entry->ThreatValue);
}

void UHarmoniaThreatComponent::RemoveThreat(AActor* ThreatActor, float ThreatAmount)
{
	if (!ThreatActor || ThreatAmount <= 0.0f)
	{
		return;
	}

	for (int32 i = 0; i < ThreatTable.Num(); i++)
	{
		if (ThreatTable[i].ThreatActor == ThreatActor)
		{
			float OldThreat = ThreatTable[i].ThreatValue;
			ThreatTable[i].ThreatValue = FMath::Max(0.0f, ThreatTable[i].ThreatValue - ThreatAmount);

			OnThreatChanged.Broadcast(ThreatActor, OldThreat, ThreatTable[i].ThreatValue);

			if (ThreatTable[i].ThreatValue < MinimumThreat)
			{
				ThreatTable.RemoveAt(i);
			}
			break;
		}
	}
}

void UHarmoniaThreatComponent::SetThreat(AActor* ThreatActor, float ThreatValue)
{
	if (!ThreatActor)
	{
		return;
	}

	FHarmoniaThreatEntry* Entry = FindOrAddThreatEntry(ThreatActor);
	if (!Entry)
	{
		return;
	}

	float OldThreat = Entry->ThreatValue;
	Entry->ThreatValue = FMath::Clamp(ThreatValue, 0.0f, MaximumThreat);
	Entry->LastThreatTime = GetWorld()->GetTimeSeconds();

	OnThreatChanged.Broadcast(ThreatActor, OldThreat, Entry->ThreatValue);
}

void UHarmoniaThreatComponent::ClearThreat(AActor* ThreatActor)
{
	if (!ThreatActor)
	{
		return;
	}

	for (int32 i = 0; i < ThreatTable.Num(); i++)
	{
		if (ThreatTable[i].ThreatActor == ThreatActor)
		{
			float OldThreat = ThreatTable[i].ThreatValue;
			ThreatTable.RemoveAt(i);
			OnThreatChanged.Broadcast(ThreatActor, OldThreat, 0.0f);
			break;
		}
	}
}

void UHarmoniaThreatComponent::ClearAllThreat()
{
	for (const FHarmoniaThreatEntry& Entry : ThreatTable)
	{
		OnThreatChanged.Broadcast(Entry.ThreatActor, Entry.ThreatValue, 0.0f);
	}

	ThreatTable.Empty();
	CurrentHighestThreatActor = nullptr;
}

void UHarmoniaThreatComponent::Taunt(AActor* TauntingActor, float Duration)
{
	if (!TauntingActor)
	{
		return;
	}

	// Set as taunt actor
	TauntActor = TauntingActor;

	// Add high threat
	FHarmoniaThreatEntry* Entry = FindOrAddThreatEntry(TauntingActor);
	if (Entry)
	{
		// Set to maximum threat
		float OldThreat = Entry->ThreatValue;
		Entry->ThreatValue = MaximumThreat;
		Entry->LastThreatTime = GetWorld()->GetTimeSeconds();

		OnThreatChanged.Broadcast(TauntingActor, OldThreat, Entry->ThreatValue);
	}

	// Clear existing taunt timer
	if (GetWorld()->GetTimerManager().IsTimerActive(TauntTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(TauntTimerHandle);
	}

	// Set new taunt timer
	GetWorld()->GetTimerManager().SetTimer(
		TauntTimerHandle,
		this,
		&UHarmoniaThreatComponent::OnTauntExpired,
		Duration,
		false
	);
}

// ============================================================================
// Threat Queries
// ============================================================================

float UHarmoniaThreatComponent::GetThreat(AActor* ThreatActor) const
{
	if (!ThreatActor)
	{
		return 0.0f;
	}

	for (const FHarmoniaThreatEntry& Entry : ThreatTable)
	{
		if (Entry.ThreatActor == ThreatActor)
		{
			return Entry.ThreatValue;
		}
	}

	return 0.0f;
}

AActor* UHarmoniaThreatComponent::GetHighestThreatActor() const
{
	// If taunted, return taunt actor
	if (TauntActor && !TauntActor->IsPendingKillPending())
	{
		return TauntActor;
	}

	return CurrentHighestThreatActor;
}

TArray<FHarmoniaThreatEntry> UHarmoniaThreatComponent::GetThreatTable(bool bSorted) const
{
	TArray<FHarmoniaThreatEntry> Result = ThreatTable;

	if (bSorted)
	{
		Result.Sort([](const FHarmoniaThreatEntry& A, const FHarmoniaThreatEntry& B)
		{
			return A.ThreatValue > B.ThreatValue;
		});
	}

	return Result;
}

// ============================================================================
// Protected Functions
// ============================================================================

void UHarmoniaThreatComponent::UpdateThreatDecay(float DeltaTime)
{
	if (ThreatDecayRate <= 0.0f)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector OwnerLocation = Owner->GetActorLocation();

	for (FHarmoniaThreatEntry& Entry : ThreatTable)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		// Calculate decay rate
		float DecayMultiplier = 1.0f;

		// Increase decay if out of range
		if (ThreatDecayDistance > 0.0f)
		{
			float Distance = FVector::Dist(OwnerLocation, Entry.ThreatActor->GetActorLocation());
			if (Distance > ThreatDecayDistance)
			{
				DecayMultiplier = OutOfRangeDecayMultiplier;
			}
		}

		// Apply decay
		float DecayAmount = Entry.ThreatValue * ThreatDecayRate * DecayMultiplier * DeltaTime;
		float OldThreat = Entry.ThreatValue;
		Entry.ThreatValue = FMath::Max(0.0f, Entry.ThreatValue - DecayAmount);

		if (!FMath::IsNearlyEqual(OldThreat, Entry.ThreatValue))
		{
			OnThreatChanged.Broadcast(Entry.ThreatActor, OldThreat, Entry.ThreatValue);
		}
	}
}

void UHarmoniaThreatComponent::CleanupThreatTable()
{
	for (int32 i = ThreatTable.Num() - 1; i >= 0; i--)
	{
		const FHarmoniaThreatEntry& Entry = ThreatTable[i];

		// Remove if invalid or threat too low
		if (!Entry.IsValid() || Entry.ThreatValue < MinimumThreat)
		{
			OnThreatChanged.Broadcast(Entry.ThreatActor, Entry.ThreatValue, 0.0f);
			ThreatTable.RemoveAt(i);
		}
	}
}

void UHarmoniaThreatComponent::UpdateHighestThreatActor()
{
	AActor* OldHighest = CurrentHighestThreatActor;
	CurrentHighestThreatActor = nullptr;
	float HighestThreat = 0.0f;

	for (const FHarmoniaThreatEntry& Entry : ThreatTable)
	{
		if (Entry.IsValid() && Entry.ThreatValue > HighestThreat)
		{
			HighestThreat = Entry.ThreatValue;
			CurrentHighestThreatActor = Entry.ThreatActor;
		}
	}

	// Broadcast if changed
	if (OldHighest != CurrentHighestThreatActor)
	{
		OnHighestThreatChanged.Broadcast(OldHighest, CurrentHighestThreatActor);
	}
}

FHarmoniaThreatEntry* UHarmoniaThreatComponent::FindOrAddThreatEntry(AActor* ThreatActor)
{
	if (!ThreatActor)
	{
		return nullptr;
	}

	// Find existing entry
	for (FHarmoniaThreatEntry& Entry : ThreatTable)
	{
		if (Entry.ThreatActor == ThreatActor)
		{
			return &Entry;
		}
	}

	// Create new entry
	FHarmoniaThreatEntry NewEntry;
	NewEntry.ThreatActor = ThreatActor;
	NewEntry.ThreatValue = 0.0f;
	NewEntry.LastThreatTime = GetWorld()->GetTimeSeconds();

	ThreatTable.Add(NewEntry);
	return &ThreatTable.Last();
}

void UHarmoniaThreatComponent::OnTauntExpired()
{
	TauntActor = nullptr;
}
