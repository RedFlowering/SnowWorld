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

	// 매니?�???�스?�스 ?�록
	if (UHarmoniaDungeonManager* Manager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaDungeonManager>(this))
	{
		if (DungeonData)
		{
			// ?�스?�스 ?�보??매니?�?�서 관�?
		}
	}
}

void AHarmoniaDungeonInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 몬스???�거
	ClearAllMonsters();

	// ?�?�머 ?�리
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

	// ?�레?�어???�전 컴포?�트???�작 ?�림
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

	// �??�이�??�작
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

	// ?�레?�어???�전 컴포?�트???�료 ?�림
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

	// 몬스???�리
	ClearAllMonsters();

	// ?�?�머 ?�리
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

		// ?�레?�어�??�장 지?�으�??�동
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(EntranceTransform.GetLocation(), EntranceTransform.Rotator());
		}

		// ?�레?�어???�전 컴포?�트???�장 ?�림
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

		// ?�레?�어�?출구�??�동
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(ExitTransform.GetLocation(), ExitTransform.Rotator());
		}

		// ?�레?�어???�전 컴포?�트???�장 ?�림
		if (AActor* PlayerPawn = Player->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->ExitDungeon(false);
			}
		}

		// 모든 ?�레?�어가 ?��?�??�전 종료
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

	// ?�당 ?�이브의 몬스???�폰
	ProcessMonsterSpawns(WaveNumber);

	// ?�이�??�료 체크 ?�?�머 ?�작
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

		// 몬스???�망 ??콜백 ?�결 (구현 ?�요)
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

	// ?�전 �?몬스???�리
	ClearAllMonsters();

	// ??�??�성 로직 (?�로?��????�성?� 별도 구현 ?�요)
	// ?�재??기존 ?�폰 ?�인?��? ?�사?�하???�이??증�?

	// ?�레?�어???�전 컴포?�트??�?변�??�림
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

	// ?�음 ?�이�??�작
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

	// 모든 몬스?��? 죽었?��? ?�인
	if (GetAliveMonsterCount() == 0)
	{
		// ?�음 ?�이브�? ?�는지 ?�인
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
			// ?�음 ?�이브로
			AdvanceToNextWave();
		}
		else
		{
			// 무한 ?�전?�면 ?�음 층으�?
			if (DungeonData && DungeonData->DungeonType == EDungeonType::Infinite)
			{
				GenerateNextFloor();
			}
			else
			{
				// ?�전 ?�리??
				CheckCompletionConditions();
			}
		}
	}
}

void AHarmoniaDungeonInstance::CheckCompletionConditions()
{
	// 모든 ?�이브�? ?�리?�했?�면 ?�공
	if (GetAliveMonsterCount() == 0)
	{
		EndDungeon(true);
	}
}
