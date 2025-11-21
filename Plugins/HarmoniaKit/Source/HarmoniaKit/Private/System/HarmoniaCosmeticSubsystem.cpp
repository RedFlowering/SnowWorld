// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaCosmeticSubsystem.h"
#include "HarmoniaKit.h"

void UHarmoniaCosmeticSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaCosmeticSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaCosmeticSubsystem::UnlockCosmetic(const FHarmoniaID& CosmeticId)
{
	FCosmeticState& State = CosmeticStates.FindOrAdd(CosmeticId);
	if (!State.bUnlocked)
	{
		State.CosmeticId = CosmeticId;
		State.bUnlocked = true;
		OnCosmeticUnlocked.Broadcast(CosmeticId);
	}
}

void UHarmoniaCosmeticSubsystem::EquipCosmetic(const FHarmoniaID& CosmeticId, ECosmeticSlotType Slot)
{
	if (IsCosmeticUnlocked(CosmeticId) || !CosmeticId.IsValid())
	{
		EquippedCosmetics.Add(Slot, CosmeticId);
		OnCosmeticEquipped.Broadcast(CosmeticId, Slot);
	}
}

FHarmoniaID UHarmoniaCosmeticSubsystem::GetEquippedCosmetic(ECosmeticSlotType Slot) const
{
	if (const FHarmoniaID* Id = EquippedCosmetics.Find(Slot))
	{
		return *Id;
	}
	return FHarmoniaID();
}

bool UHarmoniaCosmeticSubsystem::IsCosmeticUnlocked(const FHarmoniaID& CosmeticId) const
{
	if (const FCosmeticState* State = CosmeticStates.Find(CosmeticId))
	{
		return State->bUnlocked;
	}
	return false;
}
