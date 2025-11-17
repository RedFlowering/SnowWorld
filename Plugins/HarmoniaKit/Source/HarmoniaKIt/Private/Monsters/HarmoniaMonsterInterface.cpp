// Copyright 2025 Snow Game Studio.

#include "Monsters/HarmoniaMonsterInterface.h"

// Default implementations return nullptr/false/default values
// Implementers should override these functions

UHarmoniaMonsterData* IHarmoniaMonsterInterface::GetMonsterData_Implementation() const
{
	return nullptr;
}

int32 IHarmoniaMonsterInterface::GetMonsterLevel_Implementation() const
{
	return 1;
}

FText IHarmoniaMonsterInterface::GetMonsterName_Implementation() const
{
	return FText::FromString(TEXT("Unknown Monster"));
}

EHarmoniaMonsterState IHarmoniaMonsterInterface::GetMonsterState_Implementation() const
{
	return EHarmoniaMonsterState::Idle;
}

void IHarmoniaMonsterInterface::SetMonsterState_Implementation(EHarmoniaMonsterState NewState)
{
	// Override in implementation
}

bool IHarmoniaMonsterInterface::IsInCombat_Implementation() const
{
	return GetMonsterState_Implementation() == EHarmoniaMonsterState::Combat;
}

bool IHarmoniaMonsterInterface::IsDead_Implementation() const
{
	return GetMonsterState_Implementation() == EHarmoniaMonsterState::Dead;
}

void IHarmoniaMonsterInterface::OnDamageTaken_Implementation(float DamageAmount, AActor* DamageInstigator)
{
	// Override in implementation
}

void IHarmoniaMonsterInterface::OnDeath_Implementation(AActor* Killer)
{
	// Override in implementation
}

AActor* IHarmoniaMonsterInterface::GetCurrentTarget_Implementation() const
{
	return nullptr;
}

void IHarmoniaMonsterInterface::SetCurrentTarget_Implementation(AActor* NewTarget)
{
	// Override in implementation
}

TArray<FHarmoniaLootTableRow> IHarmoniaMonsterInterface::GenerateLoot_Implementation(AActor* Killer)
{
	return TArray<FHarmoniaLootTableRow>();
}

void IHarmoniaMonsterInterface::SpawnLoot_Implementation(const TArray<FHarmoniaLootTableRow>& LootItems, const FVector& SpawnLocation)
{
	// Override in implementation
}

EHarmoniaMonsterAggroType IHarmoniaMonsterInterface::GetAggroType_Implementation() const
{
	return EHarmoniaMonsterAggroType::Neutral;
}

float IHarmoniaMonsterInterface::GetAggroRange_Implementation() const
{
	return 1000.0f;
}

void IHarmoniaMonsterInterface::OnTargetDetected_Implementation(AActor* DetectedActor)
{
	// Override in implementation
}

void IHarmoniaMonsterInterface::OnTargetLost_Implementation()
{
	// Override in implementation
}
