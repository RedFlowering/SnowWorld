// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#include "Actors/HarmoniaDungeonInstance.h"
#include "Core/HarmoniaCoreBFL.h"
#include "Components/HarmoniaDungeonComponent.h"
#include "Managers/HarmoniaDungeonManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AHarmoniaDungeonInstance::AHarmoniaDungeonInstance()
{
	PrimaryActorTick.bCanEverTick = true;

	bIsActive = false;
	CurrentWave = 0;
	CurrentFloor = 1;
	InstanceID = FGuid::NewGuid();
}

void AHarmoniaDungeonInstance::BeginPlay()
{
	Super::BeginPlay();

	// Register instance to manager
	if (UHarmoniaDungeonManager* Manager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaDungeonManager>(this))
	{
		if (DungeonData)
		{
			// Instance info is managed by manager
		}
	}
}

void AHarmoniaDungeonInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove all monsters
	ClearAllMonsters();

	// Clean up timer
	GetWorld()->GetTimerManager().ClearTimer(WaveCheckTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AHarmoniaDungeonInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHarmoniaDungeonInstance::StartDungeon()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	CurrentWave = 0;
	OnDungeonInstanceStarted.Broadcast(this);

	UE_LOG(LogTemp, Log, TEXT("Dungeon Instance Started: %s"), *InstanceID.ToString());

	// Notify dungeon component of players about start
	for (APlayerController* PC : ActivePlayers)
	{
		if (AActor* PlayerPawn = PC->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->StartDungeon();
			}
		}
	}

	// Start timer
	StartWave(1);
}

void AHarmoniaDungeonInstance::EndDungeon(bool bSuccess)
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	OnDungeonInstanceCompleted.Broadcast(this, bSuccess);

	UE_LOG(LogTemp, Log, TEXT("Dungeon Instance Ended: %s (Success: %s)"), 
		*InstanceID.ToString(), bSuccess ? TEXT("True") : TEXT("False"));

	// Notify dungeon component of players about completion
	for (APlayerController* PC : ActivePlayers)
	{
		if (AActor* PlayerPawn = PC->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->CompleteDungeon(bSuccess);
			}
		}
	}

	// Clean up monsters
	ClearAllMonsters();

	// Clean up timer
	GetWorld()->GetTimerManager().ClearTimer(WaveCheckTimerHandle);
}

void AHarmoniaDungeonInstance::OnPlayerEnter(APlayerController* Player)
{
	if (!Player)
	{
		return;
	}

	if (!ActivePlayers.Contains(Player))
	{
		ActivePlayers.Add(Player);
		OnPlayerEnteredDungeon.Broadcast(Player);

		UE_LOG(LogTemp, Log, TEXT("Player entered dungeon: %s"), *Player->GetName());

		// Move player to storage area
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(EntranceTransform.GetLocation(), EntranceTransform.Rotator());
		}

		// Notify dungeon component of players about storage
		if (AActor* PlayerPawn = Player->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->EnterDungeon(DungeonData, Difficulty);
			}
		}
	}
}

void AHarmoniaDungeonInstance::OnPlayerExit(APlayerController* Player)
{
	if (!Player)
	{
		return;
	}

	if (ActivePlayers.Remove(Player) > 0)
	{
		OnPlayerExitedDungeon.Broadcast(Player);

		UE_LOG(LogTemp, Log, TEXT("Player exited dungeon: %s"), *Player->GetName());

		// Move player to exit
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(ExitTransform.GetLocation(), ExitTransform.Rotator());
		}

		// Notify dungeon component of players about storage
		if (AActor* PlayerPawn = Player->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->ExitDungeon(false);
			}
		}

		// End dungeon when all players have left
		if (ActivePlayers.Num() == 0 && bIsActive)
		{
			EndDungeon(false);
		}
	}
}

void AHarmoniaDungeonInstance::StartWave(int32 WaveNumber)
{
	CurrentWave = WaveNumber;

	UE_LOG(LogTemp, Log, TEXT("Starting Wave %d"), WaveNumber);

	// Spawn monsters for this wave
	ProcessMonsterSpawns(WaveNumber);

	// Start wave completion check timer
	GetWorld()->GetTimerManager().SetTimer(WaveCheckTimerHandle, this, &AHarmoniaDungeonInstance::CheckWaveCompletion, 1.0f, true);
}

void AHarmoniaDungeonInstance::AdvanceToNextWave()
{
	OnWaveCompleted.Broadcast(CurrentWave);
	StartWave(CurrentWave + 1);
}

int32 AHarmoniaDungeonInstance::GetAliveMonsterCount() const
{
	int32 Count = 0;
	for (AActor* Monster : SpawnedMonsters)
	{
		if (IsValid(Monster))
		{
			Count++;
		}
	}
	return Count;
}

AActor* AHarmoniaDungeonInstance::SpawnMonster(const FDungeonSpawnPoint& SpawnPoint)
{
	if (!SpawnPoint.MonsterClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Monster = GetWorld()->SpawnActor<AActor>(
		SpawnPoint.MonsterClass,
		SpawnPoint.SpawnTransform.GetLocation(),
		SpawnPoint.SpawnTransform.Rotator(),
		SpawnParams
	);

	if (Monster)
	{
		RegisterMonster(Monster);
	}

	return Monster;
}

void AHarmoniaDungeonInstance::RegisterMonster(AActor* Monster)
{
	if (Monster && !SpawnedMonsters.Contains(Monster))
	{
		SpawnedMonsters.Add(Monster);

		// Connect callback for monster death (needs implementation)
	}
}

void AHarmoniaDungeonInstance::UnregisterMonster(AActor* Monster)
{
	SpawnedMonsters.Remove(Monster);
	CheckWaveCompletion();
}

void AHarmoniaDungeonInstance::ClearAllMonsters()
{
	for (AActor* Monster : SpawnedMonsters)
	{
		if (IsValid(Monster))
		{
			Monster->Destroy();
		}
	}

	SpawnedMonsters.Empty();
}

void AHarmoniaDungeonInstance::GenerateNextFloor()
{
	if (!DungeonData || DungeonData->DungeonType != EDungeonType::Infinite)
	{
		return;
	}

	CurrentFloor++;

	UE_LOG(LogTemp, Log, TEXT("Generating Floor %d"), CurrentFloor);

	// Clean up all monsters
	ClearAllMonsters();

	// Room generation logic (procedural generation needs separate implementation)
	// Currently reuse existing spawn points with increased difficulty

	// Notify player's dungeon component about floor change
	for (APlayerController* PC : ActivePlayers)
	{
		if (AActor* PlayerPawn = PC->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->AdvanceToNextFloor();
			}
		}
	}

	// Start next wave
	StartWave(1);
}

FInfiniteDungeonFloor AHarmoniaDungeonInstance::GetCurrentFloorInfo() const
{
	if (DungeonData && DungeonData->DungeonType == EDungeonType::Infinite)
	{
		if (DungeonData->InfiniteFloors.IsValidIndex(CurrentFloor - 1))
		{
			return DungeonData->InfiniteFloors[CurrentFloor - 1];
		}
	}

	return FInfiniteDungeonFloor();
}

void AHarmoniaDungeonInstance::ProcessMonsterSpawns(int32 WaveNumber)
{
	for (const FDungeonSpawnPoint& SpawnPoint : SpawnPoints)
	{
		if (SpawnPoint.WaveNumber == WaveNumber && SpawnPoint.bIsActive)
		{
			SpawnMonster(SpawnPoint);
		}
	}
}

void AHarmoniaDungeonInstance::CheckWaveCompletion()
{
	if (!bIsActive)
	{
		return;
	}

	// Check if all monsters are dead
	if (GetAliveMonsterCount() == 0)
	{
		// Check if there's a next wave
		bool bHasNextWave = false;
		for (const FDungeonSpawnPoint& SpawnPoint : SpawnPoints)
		{
			if (SpawnPoint.WaveNumber > CurrentWave && SpawnPoint.bIsActive)
			{
				bHasNextWave = true;
				break;
			}
		}

		if (bHasNextWave)
		{
			// Go to next wave
			AdvanceToNextWave();
		}
		else
		{
			// Go to next floor if infinite dungeon
			if (DungeonData && DungeonData->DungeonType == EDungeonType::Infinite)
			{
				GenerateNextFloor();
			}
			else
			{
				// Dungeon cleared
				CheckCompletionConditions();
			}
		}
	}
}

void AHarmoniaDungeonInstance::CheckCompletionConditions()
{
	// Success if all waves cleared
	if (GetAliveMonsterCount() == 0)
	{
		EndDungeon(true);
	}
}
