// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaMonsterSpawner.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AHarmoniaMonsterSpawner::AHarmoniaMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AHarmoniaMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bActive)
	{
		InitializeSpawner();
	}
}

void AHarmoniaMonsterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Draw debug radius
	if (bShowDebugRadius && SpawnRadius > 0.0f)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), SpawnRadius, 32, FColor::Yellow, false, -1.0f, 0, 2.0f);
	}

	// Cleanup invalid references
	CleanupMonsterReferences();
}

void AHarmoniaMonsterSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

// ============================================================================
// Spawning Functions
// ============================================================================

AHarmoniaMonsterBase* AHarmoniaMonsterSpawner::SpawnMonster()
{
	if (!CanSpawn())
	{
		return nullptr;
	}

	// Select random monster config
	FHarmoniaMonsterSpawnConfig Config = SelectRandomMonsterConfig();
	if (!Config.MonsterData && !Config.MonsterClass)
	{
		return nullptr;
	}

	return SpawnMonsterFromConfig(Config);
}

AHarmoniaMonsterBase* AHarmoniaMonsterSpawner::SpawnMonsterFromConfig(const FHarmoniaMonsterSpawnConfig& Config)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	if (!Config.MonsterData && !Config.MonsterClass)
	{
		return nullptr;
	}

	// Get spawn location
	FVector SpawnLocation = GetRandomSpawnLocation();
	FRotator SpawnRotation = GetActorRotation();

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Determine class to spawn
	TSubclassOf<AHarmoniaMonsterBase> ClassToSpawn = Config.MonsterClass;
	if (!ClassToSpawn)
	{
		ClassToSpawn = AHarmoniaMonsterBase::StaticClass();
	}

	// Spawn monster
	AHarmoniaMonsterBase* Monster = GetWorld()->SpawnActor<AHarmoniaMonsterBase>(
		ClassToSpawn,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!Monster)
	{
		return nullptr;
	}

	// Initialize monster
	if (Config.MonsterData)
	{
		int32 Level = FMath::RandRange(Config.MinLevel, Config.MaxLevel);
		Monster->InitializeMonster(Config.MonsterData, Level);
	}

	// Bind to death event
	Monster->OnMonsterDeath.AddDynamic(this, &AHarmoniaMonsterSpawner::OnSpawnedMonsterDeath);

	// Add to spawned list
	SpawnedMonsters.Add(Monster);

	// Broadcast spawn event
	OnMonsterSpawned.Broadcast(Monster);

	return Monster;
}

void AHarmoniaMonsterSpawner::DespawnMonster(AHarmoniaMonsterBase* Monster)
{
	if (!Monster)
	{
		return;
	}

	SpawnedMonsters.Remove(Monster);
	OnMonsterDespawned.Broadcast(Monster);

	Monster->Destroy();
}

void AHarmoniaMonsterSpawner::DespawnAllMonsters()
{
	TArray<AHarmoniaMonsterBase*> MonstersToRemove = SpawnedMonsters;

	for (AHarmoniaMonsterBase* Monster : MonstersToRemove)
	{
		DespawnMonster(Monster);
	}
}

void AHarmoniaMonsterSpawner::ActivateSpawner()
{
	if (bActive)
	{
		return;
	}

	bActive = true;
	InitializeSpawner();
}

void AHarmoniaMonsterSpawner::DeactivateSpawner()
{
	bActive = false;

	// Clear timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(InitialSpawnTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaMonsterSpawner::InitializeSpawner()
{
	if (MonsterConfigs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterSpawner has no monster configurations!"));
		return;
	}

	// Handle different spawn modes
	switch (SpawnMode)
	{
	case EHarmoniaSpawnMode::OnBeginPlay:
		if (InitialSpawnDelay > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				InitialSpawnTimerHandle,
				FTimerDelegate::CreateLambda([this]() { SpawnMonster(); }),
				InitialSpawnDelay,
				false
			);
		}
		else
		{
			SpawnMonster();
		}
		break;

	case EHarmoniaSpawnMode::Respawn:
		// Initial spawn
		if (InitialSpawnDelay > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				InitialSpawnTimerHandle,
				FTimerDelegate::CreateLambda([this]() { SpawnMonster(); }),
				InitialSpawnDelay,
				false
			);
		}
		else
		{
			SpawnMonster();
		}
		break;

	case EHarmoniaSpawnMode::Continuous:
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerHandle,
			FTimerDelegate::CreateUObject(this, &AHarmoniaMonsterSpawner::HandleWaveSpawn),
			InitialSpawnDelay > 0.0f ? InitialSpawnDelay : WaveInterval,
			true
		);
		break;

	case EHarmoniaSpawnMode::Manual:
		// Do nothing, wait for manual spawn calls
		break;
	}
}

FVector AHarmoniaMonsterSpawner::GetRandomSpawnLocation() const
{
	FVector BaseLocation = GetActorLocation();

	if (SpawnRadius <= 0.0f)
	{
		return BaseLocation;
	}

	// Random offset within radius
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.0f, SpawnRadius);
	RandomOffset.Z = 0.0f; // Keep on same height

	return BaseLocation + RandomOffset;
}

FHarmoniaMonsterSpawnConfig AHarmoniaMonsterSpawner::SelectRandomMonsterConfig() const
{
	if (MonsterConfigs.Num() == 0)
	{
		return FHarmoniaMonsterSpawnConfig();
	}

	// Calculate total weight
	int32 TotalWeight = 0;
	TArray<FHarmoniaMonsterSpawnConfig> ValidConfigs;

	for (const FHarmoniaMonsterSpawnConfig& Config : MonsterConfigs)
	{
		// Check if we can spawn more of this type
		int32 CurrentCount = 0;
		for (AHarmoniaMonsterBase* Monster : SpawnedMonsters)
		{
			if (Monster && IHarmoniaMonsterInterface::Execute_GetMonsterData(Monster) == Config.MonsterData)
			{
				CurrentCount++;
			}
		}

		if (CurrentCount < Config.MaxInstances)
		{
			ValidConfigs.Add(Config);
			TotalWeight += Config.SpawnWeight;
		}
	}

	if (ValidConfigs.Num() == 0 || TotalWeight == 0)
	{
		return MonsterConfigs[0]; // Fallback to first
	}

	// Weighted random selection
	int32 RandomValue = FMath::RandRange(0, TotalWeight - 1);
	int32 CurrentWeight = 0;

	for (const FHarmoniaMonsterSpawnConfig& Config : ValidConfigs)
	{
		CurrentWeight += Config.SpawnWeight;
		if (RandomValue < CurrentWeight)
		{
			return Config;
		}
	}

	return ValidConfigs[0];
}

bool AHarmoniaMonsterSpawner::CanSpawn() const
{
	if (!bActive)
	{
		return false;
	}

	if (SpawnedMonsters.Num() >= MaxTotalMonsters)
	{
		return false;
	}

	return true;
}

void AHarmoniaMonsterSpawner::OnSpawnedMonsterDeath(AHarmoniaMonsterBase* Monster, AActor* Killer)
{
	if (!Monster)
	{
		return;
	}

	// Remove from spawned list
	SpawnedMonsters.Remove(Monster);

	// Check if all monsters defeated
	if (SpawnedMonsters.Num() == 0)
	{
		OnAllMonstersDefeated.Broadcast();
	}

	// Handle respawn
	if (SpawnMode == EHarmoniaSpawnMode::Respawn && bActive)
	{
		HandleRespawn();
	}
}

void AHarmoniaMonsterSpawner::HandleRespawn()
{
	if (!CanSpawn())
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		FTimerDelegate::CreateLambda([this]() { SpawnMonster(); }),
		RespawnDelay,
		false
	);
}

void AHarmoniaMonsterSpawner::HandleWaveSpawn()
{
	CurrentWave++;

	int32 ToSpawn = FMath::Min(MonstersPerWave, MaxTotalMonsters - SpawnedMonsters.Num());

	for (int32 i = 0; i < ToSpawn; i++)
	{
		SpawnMonster();
	}
}

void AHarmoniaMonsterSpawner::CleanupMonsterReferences()
{
	SpawnedMonsters.RemoveAll([](const TObjectPtr<AHarmoniaMonsterBase>& Monster)
	{
		return !Monster || Monster->IsPendingKillPending();
	});
}
