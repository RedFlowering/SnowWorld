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

	// 매니저에 인스턴스 등록
	if (UHarmoniaDungeonManager* Manager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaDungeonManager>(this))
	{
		if (DungeonData)
		{
			// 인스턴스 정보는 매니저에서 관리
		}
	}
}

void AHarmoniaDungeonInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 몬스터 제거
	ClearAllMonsters();

	// 타이머 정리
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

	// 플레이어의 던전 컴포넌트에 시작 알림
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

	// 첫 웨이브 시작
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

	// 플레이어의 던전 컴포넌트에 완료 알림
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

	// 몬스터 정리
	ClearAllMonsters();

	// 타이머 정리
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

		// 플레이어를 입장 지점으로 이동
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(EntranceTransform.GetLocation(), EntranceTransform.Rotator());
		}

		// 플레이어의 던전 컴포넌트에 입장 알림
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

		// 플레이어를 출구로 이동
		if (APawn* PlayerPawn = Player->GetPawn())
		{
			PlayerPawn->SetActorLocationAndRotation(ExitTransform.GetLocation(), ExitTransform.Rotator());
		}

		// 플레이어의 던전 컴포넌트에 퇴장 알림
		if (AActor* PlayerPawn = Player->GetPawn())
		{
			if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
			{
				DungeonComp->ExitDungeon(false);
			}
		}

		// 모든 플레이어가 나가면 던전 종료
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

	// 해당 웨이브의 몬스터 스폰
	ProcessMonsterSpawns(WaveNumber);

	// 웨이브 완료 체크 타이머 시작
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

		// 몬스터 사망 시 콜백 연결 (구현 필요)
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

	// 이전 층 몬스터 정리
	ClearAllMonsters();

	// 새 층 생성 로직 (프로시저럴 생성은 별도 구현 필요)
	// 현재는 기존 스폰 포인트를 재사용하되 난이도 증가

	// 플레이어의 던전 컴포넌트에 층 변경 알림
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

	// 다음 웨이브 시작
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

	// 모든 몬스터가 죽었는지 확인
	if (GetAliveMonsterCount() == 0)
	{
		// 다음 웨이브가 있는지 확인
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
			// 다음 웨이브로
			AdvanceToNextWave();
		}
		else
		{
			// 무한 던전이면 다음 층으로
			if (DungeonData && DungeonData->DungeonType == EDungeonType::Infinite)
			{
				GenerateNextFloor();
			}
			else
			{
				// 던전 클리어
				CheckCompletionConditions();
			}
		}
	}
}

void AHarmoniaDungeonInstance::CheckCompletionConditions()
{
	// 모든 웨이브를 클리어했으면 성공
	if (GetAliveMonsterCount() == 0)
	{
		EndDungeon(true);
	}
}
