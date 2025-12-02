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

	// Îß§Îãà?Ä???∏Ïä§?¥Ïä§ ?±Î°ù
	if (UHarmoniaDungeonManager* Manager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaDungeonManager>(this))
	{
		if (DungeonData)
		{
			// ?∏Ïä§?¥Ïä§ ?ïÎ≥¥??Îß§Îãà?Ä?êÏÑú Í¥ÄÎ¶?
		}
	}
}

void AHarmoniaDungeonInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Î™®Îì† Î™¨Ïä§???úÍ±∞
	ClearAllMonsters();

	// ?Ä?¥Î®∏ ?ïÎ¶¨
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

	// ?åÎ†à?¥Ïñ¥???òÏ†Ñ Ïª¥Ìè¨?åÌä∏???úÏûë ?åÎ¶º
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

	// Ï≤??®Ïù¥Î∏??úÏûë
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

	// ?åÎ†à?¥Ïñ¥???òÏ†Ñ Ïª¥Ìè¨?åÌä∏???ÑÎ£å ?åÎ¶º
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

	// Î™¨Ïä§???ïÎ¶¨
	ClearAllMonsters();

	// ?Ä?¥Î®∏ ?ïÎ¶¨
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

		// ?åÎ†à?¥Ïñ¥Î•??ÖÏû• ÏßÄ?êÏúºÎ°??¥Îèô
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(EntranceTransform.GetLocation(), EntranceTransform.Rotator());
		}

		// ?åÎ†à?¥Ïñ¥???òÏ†Ñ Ïª¥Ìè¨?åÌä∏???ÖÏû• ?åÎ¶º
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

		// ?åÎ†à?¥Ïñ¥Î•?Ï∂úÍµ¨Î°??¥Îèô
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(ExitTransform.GetLocation(), ExitTransform.Rotator());
		}

		// ?åÎ†à?¥Ïñ¥???òÏ†Ñ Ïª¥Ìè¨?åÌä∏???¥Ïû• ?åÎ¶º
		if (AActor* PlayerPawn = Player->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->ExitDungeon(false);
			}
		}

		// Î™®Îì† ?åÎ†à?¥Ïñ¥Í∞Ä ?òÍ?Î©??òÏ†Ñ Ï¢ÖÎ£å
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

	// ?¥Îãπ ?®Ïù¥Î∏åÏùò Î™¨Ïä§???§Ìè∞
	ProcessMonsterSpawns(WaveNumber);

	// ?®Ïù¥Î∏??ÑÎ£å Ï≤¥ÌÅ¨ ?Ä?¥Î®∏ ?úÏûë
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

		// Î™¨Ïä§???¨Îßù ??ÏΩúÎ∞± ?∞Í≤∞ (Íµ¨ÌòÑ ?ÑÏöî)
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

	// ?¥Ï†Ñ Ï∏?Î™¨Ïä§???ïÎ¶¨
	ClearAllMonsters();

	// ??Ï∏??ùÏÑ± Î°úÏßÅ (?ÑÎ°ú?úÏ????ùÏÑ±?Ä Î≥ÑÎèÑ Íµ¨ÌòÑ ?ÑÏöî)
	// ?ÑÏû¨??Í∏∞Ï°¥ ?§Ìè∞ ?¨Ïù∏?∏Î? ?¨ÏÇ¨?©Ìïò???úÏù¥??Ï¶ùÍ?

	// ?åÎ†à?¥Ïñ¥???òÏ†Ñ Ïª¥Ìè¨?åÌä∏??Ï∏?Î≥ÄÍ≤??åÎ¶º
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

	// ?§Ïùå ?®Ïù¥Î∏??úÏûë
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

	// Î™®Îì† Î™¨Ïä§?∞Í? Ï£ΩÏóà?îÏ? ?ïÏù∏
	if (GetAliveMonsterCount() == 0)
	{
		// ?§Ïùå ?®Ïù¥Î∏åÍ? ?àÎäîÏßÄ ?ïÏù∏
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
			// ?§Ïùå ?®Ïù¥Î∏åÎ°ú
			AdvanceToNextWave();
		}
		else
		{
			// Î¨¥Ìïú ?òÏ†Ñ?¥Î©¥ ?§Ïùå Ï∏µÏúºÎ°?
			if (DungeonData && DungeonData->DungeonType == EDungeonType::Infinite)
			{
				GenerateNextFloor();
			}
			else
			{
				// ?òÏ†Ñ ?¥Î¶¨??
				CheckCompletionConditions();
			}
		}
	}
}

void AHarmoniaDungeonInstance::CheckCompletionConditions()
{
	// Î™®Îì† ?®Ïù¥Î∏åÎ? ?¥Î¶¨?¥Ìñà?ºÎ©¥ ?±Í≥µ
	if (GetAliveMonsterCount() == 0)
	{
		EndDungeon(true);
	}
}
