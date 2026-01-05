// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaMonsterTrigger.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaBossMonster.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

AHarmoniaMonsterTrigger::AHarmoniaMonsterTrigger()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Trigger volume
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
	TriggerVolume->SetGenerateOverlapEvents(true);

	// Spawn point
	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(RootComponent);
	SpawnPoint->SetRelativeLocation(FVector(300.0f, 0.0f, 0.0f));
}

void AHarmoniaMonsterTrigger::BeginPlay()
{
	Super::BeginPlay();

	// 트리거 오버랩 바인딩
	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaMonsterTrigger::OnOverlapBegin);
	}

	// 세이브 상태 로드
	LoadSaveState();

	// 스폰 모드에 따른 초기화
	switch (SpawnMode)
	{
	case EHarmoniaMonsterSpawnMode::OnBeginPlay:
		if (InitialSpawnDelay > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				SpawnDelayTimerHandle,
				this,
				&AHarmoniaMonsterTrigger::OnSpawnDelayComplete,
				InitialSpawnDelay,
				false
			);
		}
		else
		{
			ActivateTrigger(nullptr);
		}
		break;

	case EHarmoniaMonsterSpawnMode::Wave:
		// 웨이브 모드는 트리거 또는 수동 활성화 대기
		break;

	case EHarmoniaMonsterSpawnMode::OnTrigger:
		// 트리거 대기
		break;

	case EHarmoniaMonsterSpawnMode::Manual:
	case EHarmoniaMonsterSpawnMode::Respawn:
		// 수동 호출 대기
		break;
	}
}

void AHarmoniaMonsterTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 디버그 시각화
	if (bShowDebugVolume)
	{
		// 트리거 볼륨
		if (TriggerVolume && SpawnMode == EHarmoniaMonsterSpawnMode::OnTrigger)
		{
			DrawDebugBox(
				GetWorld(),
				TriggerVolume->GetComponentLocation(),
				TriggerVolume->GetScaledBoxExtent(),
				TriggerVolume->GetComponentQuat(),
				FColor::Green,
				false,
				-1.0f,
				0,
				2.0f
			);
		}

		// 스폰 반경
		if (SpawnRadius > 0.0f && SpawnPoint)
		{
			DrawDebugSphere(
				GetWorld(),
				SpawnPoint->GetComponentLocation(),
				SpawnRadius,
				16,
				FColor::Yellow,
				false,
				-1.0f,
				0,
				2.0f
			);
		}
	}

	// 유효하지 않은 레퍼런스 정리
	CleanupInvalidMonsterReferences();
}

void AHarmoniaMonsterTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

// ============================================================================
// Public Functions
// ============================================================================

AHarmoniaMonsterBase* AHarmoniaMonsterTrigger::SpawnMonster()
{
	if (!MonsterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterTrigger [%s]: MonsterClass is not set"), *GetName());
		return nullptr;
	}

	if (!HasAuthority())
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 스폰 위치 계산
	FVector SpawnLocation = GetRandomSpawnLocation();
	FRotator SpawnRotation = SpawnPoint ? SpawnPoint->GetComponentRotation() : GetActorRotation();

	// 스폰 파라미터
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 몬스터 스폰
	AHarmoniaMonsterBase* Monster = World->SpawnActor<AHarmoniaMonsterBase>(
		MonsterClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!Monster)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterTrigger [%s]: Failed to spawn monster"), *GetName());
		return nullptr;
	}

	// AI가 스폰된 몬스터를 자동으로 Possess하도록 설정
	Monster->AutoPossessAI = EAutoPossessAI::Spawned;
	
	// SpawnDefaultController를 호출하여 기본 AI Controller 스폰 및 Possess
	Monster->SpawnDefaultController();

	// 사망 델리게이트 바인딩
	Monster->OnMonsterDeath.AddDynamic(this, &AHarmoniaMonsterTrigger::OnMonsterDeathInternal);

	// 활성 몬스터 목록에 추가
	ActiveMonsters.Add(Monster);

	// 이벤트 브로드캐스트
	OnMonsterSpawned.Broadcast(Monster);

	bHasSpawned = true;

	UE_LOG(LogTemp, Log, TEXT("MonsterTrigger [%s]: Spawned monster at %s"), 
		*GetName(), *SpawnLocation.ToString());

	return Monster;
}

void AHarmoniaMonsterTrigger::SpawnAllMonsters()
{
	int32 CountToSpawn = (SpawnMode == EHarmoniaMonsterSpawnMode::Wave) ? MonstersPerWave : SpawnCount;

	for (int32 i = 0; i < CountToSpawn; ++i)
	{
		SpawnMonster();
	}
}

void AHarmoniaMonsterTrigger::DespawnMonster(AHarmoniaMonsterBase* Monster)
{
	if (!Monster)
	{
		return;
	}

	ActiveMonsters.Remove(Monster);
	Monster->Destroy();
}

void AHarmoniaMonsterTrigger::DespawnAllMonsters()
{
	TArray<TObjectPtr<AHarmoniaMonsterBase>> MonstersToRemove = ActiveMonsters;
	for (TObjectPtr<AHarmoniaMonsterBase>& Monster : MonstersToRemove)
	{
		if (Monster)
		{
			Monster->Destroy();
		}
	}
	ActiveMonsters.Empty();
}

void AHarmoniaMonsterTrigger::ActivateTrigger(APawn* TriggeringPlayer)
{
	if (bTriggerActivated && SpawnMode == EHarmoniaMonsterSpawnMode::OnTrigger)
	{
		// OnTrigger 모드는 한 번만 활성화
		return;
	}

	// 이미 클리어된 상태면 세이브 확인
	if (TriggerState == EHarmoniaMonsterTriggerState::Cleared)
	{
		// TODO: 리스폰 정책 확인 (AfterTime 등)
		return;
	}

	TriggeringPlayerPawn = TriggeringPlayer;
	bTriggerActivated = true;

	// 인트로 시퀀스 확인
	if (IntroSequence && (!bIntroWatched || !bSkipIntroOnRevisit))
	{
		// 먼저 몬스터 스폰 (숨김 상태로)
		if (bHideMonsterDuringIntro)
		{
			SpawnAllMonsters();
			for (TObjectPtr<AHarmoniaMonsterBase>& Monster : ActiveMonsters)
			{
				if (Monster)
				{
					Monster->SetActorHiddenInGame(true);
					Monster->SetActorEnableCollision(false);
				}
			}
		}

		PlayIntroSequence();
	}
	else
	{
		// 인트로 없이 바로 스폰
		StartCombat();
	}
}

void AHarmoniaMonsterTrigger::DeactivateTrigger()
{
	bTriggerActivated = false;
	TriggerState = EHarmoniaMonsterTriggerState::Inactive;

	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(WaveTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}
}

TArray<AHarmoniaMonsterBase*> AHarmoniaMonsterTrigger::GetActiveMonsters() const
{
	TArray<AHarmoniaMonsterBase*> Result;
	for (const TObjectPtr<AHarmoniaMonsterBase>& Monster : ActiveMonsters)
	{
		if (Monster)
		{
			Result.Add(Monster);
		}
	}
	return Result;
}

int32 AHarmoniaMonsterTrigger::GetActiveMonsterCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<AHarmoniaMonsterBase>& Monster : ActiveMonsters)
	{
		if (Monster && !Monster->IsPendingKillPending())
		{
			Count++;
		}
	}
	return Count;
}

// ============================================================================
// Internal Functions
// ============================================================================

void AHarmoniaMonsterTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (SpawnMode != EHarmoniaMonsterSpawnMode::OnTrigger && SpawnMode != EHarmoniaMonsterSpawnMode::Wave)
	{
		return;
	}

	if (bTriggerActivated)
	{
		return;
	}

	// 플레이어인지 확인
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!PlayerPawn)
	{
		if (ACharacter* Character = Cast<ACharacter>(OtherActor))
		{
			PlayerPawn = Character;
		}
	}

	if (!PlayerPawn || !PlayerPawn->IsPlayerControlled())
	{
		return;
	}

	ActivateTrigger(PlayerPawn);
}

void AHarmoniaMonsterTrigger::OnMonsterDeathInternal(AHarmoniaMonsterBase* Monster, AActor* Killer)
{
	if (!Monster)
	{
		return;
	}

	// 목록에서 제거
	ActiveMonsters.Remove(Monster);

	// 이벤트 브로드캐스트
	OnMonsterDeath.Broadcast(Monster, Killer);

	UE_LOG(LogTemp, Log, TEXT("MonsterTrigger [%s]: Monster died, %d remaining"), 
		*GetName(), GetActiveMonsterCount());

	// 남은 몬스터가 없으면
	if (GetActiveMonsterCount() == 0)
	{
		// 웨이브 모드에서 현재 웨이브 완료
		if (SpawnMode == EHarmoniaMonsterSpawnMode::Wave && TriggerState == EHarmoniaMonsterTriggerState::WaveInProgress)
		{
			OnWaveCompleted.Broadcast(CurrentWave);

			// 모든 웨이브 완료 여부는 타이머가 처리
		}
		// 리스폰 모드
		else if (SpawnMode == EHarmoniaMonsterSpawnMode::Respawn && RespawnPolicy != EHarmoniaRespawnPolicy::Never)
		{
			HandleRespawn();
		}
		// 그 외 모드
		else
		{
			HandleCleared();
		}
	}
}

void AHarmoniaMonsterTrigger::PlayIntroSequence()
{
	TriggerState = EHarmoniaMonsterTriggerState::PlayingIntro;
	OnIntroStarted.Broadcast();

	UWorld* World = GetWorld();
	if (!World || !IntroSequence)
	{
		OnIntroSequenceComplete();
		return;
	}

	// 시퀀스 플레이어 생성
	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	ALevelSequenceActor* LSActor = nullptr;
	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		World,
		IntroSequence,
		PlaybackSettings,
		LSActor
	);
	SequenceActor = LSActor;

	if (!SequencePlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterTrigger [%s]: Failed to create sequence player"), *GetName());
		OnIntroSequenceComplete();
		return;
	}

	// 액터 바인딩
	if (ActiveMonsters.Num() > 0 && ActiveMonsters[0])
	{
		BindActorsToSequence(ActiveMonsters[0], TriggeringPlayerPawn.Get());
	}

	// 시퀀스 종료 콜백
	SequencePlayer->OnFinished.AddDynamic(this, &AHarmoniaMonsterTrigger::OnIntroSequenceComplete);

	// 시퀀스 재생
	SequencePlayer->Play();

	UE_LOG(LogTemp, Log, TEXT("MonsterTrigger [%s]: Playing intro sequence"), *GetName());
}

void AHarmoniaMonsterTrigger::OnIntroSequenceComplete()
{
	// 시퀀스 정리
	if (SequencePlayer)
	{
		SequencePlayer->OnFinished.RemoveDynamic(this, &AHarmoniaMonsterTrigger::OnIntroSequenceComplete);
		SequencePlayer = nullptr;
	}

	if (SequenceActor)
	{
		SequenceActor->Destroy();
		SequenceActor = nullptr;
	}

	// 인트로 시청 완료
	bIntroWatched = true;
	SaveState();

	OnIntroFinished.Broadcast();

	// 몬스터 표시
	for (TObjectPtr<AHarmoniaMonsterBase>& Monster : ActiveMonsters)
	{
		if (Monster)
		{
			Monster->SetActorHiddenInGame(false);
			Monster->SetActorEnableCollision(true);
		}
	}

	// 전투 시작
	StartCombat();
}

void AHarmoniaMonsterTrigger::StartCombat()
{
	// 아직 스폰 안했으면 스폰
	if (ActiveMonsters.Num() == 0)
	{
		if (SpawnMode == EHarmoniaMonsterSpawnMode::Wave)
		{
			TriggerState = EHarmoniaMonsterTriggerState::WaveInProgress;
			CurrentWave = 0;
			StartNextWave();
		}
		else
		{
			TriggerState = EHarmoniaMonsterTriggerState::Active;
			SpawnAllMonsters();
		}
	}
	else
	{
		TriggerState = SpawnMode == EHarmoniaMonsterSpawnMode::Wave 
			? EHarmoniaMonsterTriggerState::WaveInProgress 
			: EHarmoniaMonsterTriggerState::Active;
	}

	// 보스 인카운터 시작
	for (TObjectPtr<AHarmoniaMonsterBase>& Monster : ActiveMonsters)
	{
		if (Monster)
		{
			TryStartBossEncounter(Monster);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MonsterTrigger [%s]: Combat started with %d monsters"), 
		*GetName(), GetActiveMonsterCount());
}

void AHarmoniaMonsterTrigger::HandleCleared()
{
	TriggerState = EHarmoniaMonsterTriggerState::Cleared;
	SaveState();

	OnAllMonstersDefeated.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("MonsterTrigger [%s]: Cleared"), *GetName());
}

void AHarmoniaMonsterTrigger::HandleRespawn()
{
	switch (RespawnPolicy)
	{
	case EHarmoniaRespawnPolicy::OnDeath:
		// 즉시 리스폰
		SpawnAllMonsters();
		for (TObjectPtr<AHarmoniaMonsterBase>& Monster : ActiveMonsters)
		{
			TryStartBossEncounter(Monster);
		}
		break;

	case EHarmoniaRespawnPolicy::OnDeathDelay:
		// 대기 후 리스폰
		GetWorld()->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&AHarmoniaMonsterTrigger::OnRespawnDelayComplete,
			RespawnDelaySeconds,
			false
		);
		break;

	case EHarmoniaRespawnPolicy::AfterTime:
		// 세이브 기반 - 시간 체크는 LoadSaveState에서
		SaveState();
		break;

	case EHarmoniaRespawnPolicy::Never:
	default:
		HandleCleared();
		break;
	}
}

void AHarmoniaMonsterTrigger::StartNextWave()
{
	CurrentWave++;

	// 모든 웨이브 완료 체크
	if (WaveCount > 0 && CurrentWave > WaveCount)
	{
		TriggerState = EHarmoniaMonsterTriggerState::Cleared;
		OnAllWavesCompleted.Broadcast();
		SaveState();
		UE_LOG(LogTemp, Log, TEXT("MonsterTrigger [%s]: All %d waves completed"), *GetName(), WaveCount);
		return;
	}

	OnWaveStarted.Broadcast(CurrentWave);

	// 웨이브 몬스터 스폰
	for (int32 i = 0; i < MonstersPerWave; ++i)
	{
		AHarmoniaMonsterBase* Monster = SpawnMonster();
		if (Monster)
		{
			TryStartBossEncounter(Monster);
		}
	}

	// 다음 웨이브 타이머 설정 (사망 여부와 무관하게 진행)
	if (WaveCount == 0 || CurrentWave < WaveCount)
	{
		GetWorld()->GetTimerManager().SetTimer(
			WaveTimerHandle,
			this,
			&AHarmoniaMonsterTrigger::OnWaveTimerFired,
			WaveInterval,
			false
		);
	}

	UE_LOG(LogTemp, Log, TEXT("MonsterTrigger [%s]: Wave %d started (%d monsters)"), 
		*GetName(), CurrentWave, MonstersPerWave);
}

void AHarmoniaMonsterTrigger::OnWaveTimerFired()
{
	StartNextWave();
}

void AHarmoniaMonsterTrigger::OnSpawnDelayComplete()
{
	ActivateTrigger(nullptr);
}

void AHarmoniaMonsterTrigger::OnRespawnDelayComplete()
{
	SpawnAllMonsters();
	for (TObjectPtr<AHarmoniaMonsterBase>& Monster : ActiveMonsters)
	{
		TryStartBossEncounter(Monster);
	}
}

FVector AHarmoniaMonsterTrigger::GetRandomSpawnLocation() const
{
	FVector BaseLocation = SpawnPoint ? SpawnPoint->GetComponentLocation() : GetActorLocation();

	if (SpawnRadius <= 0.0f)
	{
		return BaseLocation;
	}

	// 랜덤 오프셋 (XY 평면에서)
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.0f, SpawnRadius);
	RandomOffset.Z = 0.0f;

	return BaseLocation + RandomOffset;
}

void AHarmoniaMonsterTrigger::LoadSaveState()
{
	if (TriggerID.IsNone())
	{
		return;
	}

	// TODO: HarmoniaSaveGameSubsystem에서 상태 로드
	// FHarmoniaTriggerSaveData SaveData = SaveSubsystem->GetTriggerState(TriggerID);
	// bHasSpawned = SaveData.bHasSpawned;
	// bIntroWatched = SaveData.bIntroWatched;
	// CurrentWave = SaveData.CurrentWave;
	// 
	// if (SaveData.LastState == EHarmoniaMonsterTriggerState::Cleared)
	// {
	//     if (RespawnPolicy == EHarmoniaRespawnPolicy::AfterTime)
	//     {
	//         // 시간 경과 확인
	//         if ((FDateTime::Now() - SaveData.LastDefeatTime).GetTotalSeconds() >= RespawnDelaySeconds)
	//         {
	//             // 리스폰 가능
	//         }
	//         else
	//         {
	//             TriggerState = EHarmoniaMonsterTriggerState::Cleared;
	//         }
	//     }
	//     else
	//     {
	//         TriggerState = EHarmoniaMonsterTriggerState::Cleared;
	//     }
	// }
}

void AHarmoniaMonsterTrigger::SaveState()
{
	if (TriggerID.IsNone())
	{
		return;
	}

	// TODO: HarmoniaSaveGameSubsystem에 상태 저장
	// FHarmoniaTriggerSaveData SaveData;
	// SaveData.TriggerID = TriggerID;
	// SaveData.bHasSpawned = bHasSpawned;
	// SaveData.bIntroWatched = bIntroWatched;
	// SaveData.CurrentWave = CurrentWave;
	// SaveData.LastState = TriggerState;
	// SaveData.LastDefeatTime = FDateTime::Now();
	// SaveSubsystem->SaveTriggerState(SaveData);
}

void AHarmoniaMonsterTrigger::BindActorsToSequence(AHarmoniaMonsterBase* Monster, APawn* Player)
{
	if (!SequencePlayer)
	{
		return;
	}

	// TODO: 시퀀서 바인딩 구현
	// ALevelSequenceActor* LSActor = SequencePlayer->GetLevelSequenceActor();
	// if (LSActor)
	// {
	//     FMovieSceneObjectBindingID MonsterBinding = LSActor->FindNamedBinding(TEXT("Monster"));
	//     if (MonsterBinding.IsValid() && Monster)
	//     {
	//         LSActor->AddBinding(MonsterBinding, Monster);
	//     }
	//     
	//     FMovieSceneObjectBindingID PlayerBinding = LSActor->FindNamedBinding(TEXT("Player"));
	//     if (PlayerBinding.IsValid() && Player)
	//     {
	//         LSActor->AddBinding(PlayerBinding, Player);
	//     }
	// }
}

void AHarmoniaMonsterTrigger::CleanupInvalidMonsterReferences()
{
	ActiveMonsters.RemoveAll([](const TObjectPtr<AHarmoniaMonsterBase>& Monster)
	{
		return !Monster || Monster->IsPendingKillPending();
	});
}

void AHarmoniaMonsterTrigger::TryStartBossEncounter(AHarmoniaMonsterBase* Monster)
{
	if (!Monster)
	{
		return;
	}

	// AHarmoniaBossMonster로 캐스팅하여 보스인지 확인
	if (AHarmoniaBossMonster* Boss = Cast<AHarmoniaBossMonster>(Monster))
	{
		Boss->StartBossEncounter();
	}
}
