// Copyright 2025 Snow Game Studio.

#include "Cutscene/HarmoniaBossArenaTrigger.h"
#include "Cutscene/HarmoniaCutsceneManager.h"
#include "Actors/HarmoniaMonsterSpawner.h"
#include "Monsters/HarmoniaBossMonster.h"
#include "System/HarmoniaSaveGameSubsystem.h"
#include "Core/HarmoniaCoreBFL.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "HarmoniaStoryLog.h"

AHarmoniaBossArenaTrigger::AHarmoniaBossArenaTrigger()
{
	// 보스 스폰 위치 컴포넌트
	BossSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BossSpawnPoint"));
	BossSpawnPoint->SetupAttachment(RootComponent);
	BossSpawnPoint->SetRelativeLocation(FVector(500.0f, 0.0f, 0.0f));

	// 부모의 bTriggerOnce는 세션 내에서만 동작하므로, 
	// 세이브 기반 스킵을 위해 false로 설정하고 직접 관리
	bTriggerOnce = false;
}

void AHarmoniaBossArenaTrigger::BeginPlay()
{
	Super::BeginPlay();
}

void AHarmoniaBossArenaTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 이미 활성화되었거나 클리어된 상태면 무시
	if (ArenaState != EBossArenaState::Inactive)
	{
		return;
	}

	// 플레이어인지 확인
	if (!OtherActor)
	{
		return;
	}

	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!PlayerPawn)
	{
		// Character인 경우도 확인
		if (ACharacter* Character = Cast<ACharacter>(OtherActor))
		{
			PlayerPawn = Character;
		}
	}

	if (!PlayerPawn || !PlayerPawn->IsPlayerControlled())
	{
		return;
	}

	ActivateArena(PlayerPawn);
}

void AHarmoniaBossArenaTrigger::ActivateArena(APawn* PlayerPawn)
{
	// 세이브 시스템에서 보스 상태 확인
	UHarmoniaSaveGameSubsystem* SaveSubsystem = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaSaveGameSubsystem>(this);
	if (!SaveSubsystem)
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("BossArena [%s]: SaveSubsystem not found"), *BossID.ToString());
		return;
	}

	// 리스폰 가능 여부 확인
	if (!SaveSubsystem->ShouldBossRespawn(BossID, RespawnPolicy, RespawnTimeSeconds))
	{
		UE_LOG(LogHarmoniaStory, Log, TEXT("BossArena [%s]: Boss should not respawn, skipping"), *BossID.ToString());
		ArenaState = EBossArenaState::Cleared;
		return;
	}

	TriggeringPlayerPawn = PlayerPawn;
	FHarmoniaSavedBossState BossState = SaveSubsystem->GetBossState(BossID);

	// 인트로 시퀀스 시청 여부 확인
	if (!BossState.bHasViewedIntro && CutsceneSequence)
	{
		// 보스 스폰 (숨김 상태)
		SpawnBoss(true);
		
		// 인트로 시퀀스 재생
		PlayBossIntroSequence(PlayerPawn);
	}
	else
	{
		// 이미 인트로를 봤으면 즉시 전투 시작
		SpawnBoss(false);
		StartBossCombat();
	}
}

AHarmoniaBossMonster* AHarmoniaBossArenaTrigger::SpawnBoss(bool bStartHidden)
{
	AHarmoniaBossMonster* Boss = nullptr;

	// 스포너가 있으면 스포너 사용
	if (BossSpawner)
	{
		if (AHarmoniaMonsterBase* Monster = BossSpawner->SpawnMonster())
		{
			Boss = Cast<AHarmoniaBossMonster>(Monster);
		}
	}
	// 스포너 없으면 직접 스폰
	else if (BossClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector SpawnLocation = BossSpawnPoint->GetComponentLocation();
			FRotator SpawnRotation = BossSpawnPoint->GetComponentRotation();

			Boss = World->SpawnActor<AHarmoniaBossMonster>(
				BossClass, 
				SpawnLocation, 
				SpawnRotation, 
				SpawnParams
			);
		}
	}

	if (Boss)
	{
		SpawnedBoss = Boss;

		if (bStartHidden)
		{
			Boss->SetActorHiddenInGame(true);
			Boss->SetActorEnableCollision(false);
		}

		// 보스 사망 델리게이트 바인딩
		Boss->OnMonsterDeath.AddDynamic(this, &AHarmoniaBossArenaTrigger::OnBossDeathInternal);

		UE_LOG(LogHarmoniaStory, Log, TEXT("BossArena [%s]: Boss spawned at %s"), 
			*BossID.ToString(), *BossSpawnPoint->GetComponentLocation().ToString());
	}
	else
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("BossArena [%s]: Failed to spawn boss. Check BossSpawner or BossClass."), 
			*BossID.ToString());
	}

	return Boss;
}

void AHarmoniaBossArenaTrigger::PlayBossIntroSequence(APawn* PlayerPawn)
{
	ArenaState = EBossArenaState::PlayingIntro;

	// 보스 표시
	if (SpawnedBoss)
	{
		SpawnedBoss->SetActorHiddenInGame(false);
		SpawnedBoss->SetActorEnableCollision(true);
	}

	// CutsceneManager를 통해 재생
	UHarmoniaCutsceneManager* CutsceneManager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaCutsceneManager>(this);
	if (!CutsceneManager)
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("BossArena [%s]: CutsceneManager not found"), *BossID.ToString());
		OnBossIntroFinished(nullptr);
		return;
	}

	// 시퀀스 종료 콜백 설정
	CutsceneManager->OnCutsceneEnded.AddDynamic(this, &AHarmoniaBossArenaTrigger::OnBossIntroFinished);

	// 시퀀스 재생
	CutsceneManager->PlayCutscene(CutsceneSequence, Settings);

	OnArenaActivated.Broadcast(SpawnedBoss);
	UE_LOG(LogHarmoniaStory, Log, TEXT("BossArena [%s]: Playing intro sequence"), *BossID.ToString());
}

void AHarmoniaBossArenaTrigger::OnBossIntroFinished(ULevelSequence* Sequence)
{
	// 다른 시퀀스의 종료 이벤트면 무시
	if (Sequence && Sequence != CutsceneSequence)
	{
		return;
	}

	// 콜백 해제
	UHarmoniaCutsceneManager* CutsceneManager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaCutsceneManager>(this);
	if (CutsceneManager)
	{
		CutsceneManager->OnCutsceneEnded.RemoveDynamic(this, &AHarmoniaBossArenaTrigger::OnBossIntroFinished);
	}

	// 인트로 시청 완료 표시
	UHarmoniaSaveGameSubsystem* SaveSubsystem = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaSaveGameSubsystem>(this);
	if (SaveSubsystem)
	{
		SaveSubsystem->MarkBossIntroViewed(BossID);
	}

	// 전투 시작
	StartBossCombat();
}

void AHarmoniaBossArenaTrigger::StartBossCombat()
{
	ArenaState = EBossArenaState::InCombat;

	// 보스의 StartBossEncounter 호출 (음악 재생, UI 표시 등)
	if (SpawnedBoss)
	{
		SpawnedBoss->StartBossEncounter();
	}

	OnCombatStarted.Broadcast();
	UE_LOG(LogHarmoniaStory, Log, TEXT("BossArena [%s]: Combat started"), *BossID.ToString());
}

void AHarmoniaBossArenaTrigger::OnBossDeathInternal(AHarmoniaMonsterBase* Monster, AActor* Killer)
{
	// 이 아레나의 보스인지 확인
	if (Monster == SpawnedBoss)
	{
		NotifyBossDefeated();
	}
}

void AHarmoniaBossArenaTrigger::NotifyBossDefeated()
{
	if (ArenaState != EBossArenaState::InCombat)
	{
		return;
	}

	ArenaState = EBossArenaState::Cleared;

	// 세이브 시스템에 클리어 기록
	UHarmoniaSaveGameSubsystem* SaveSubsystem = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaSaveGameSubsystem>(this);
	if (SaveSubsystem)
	{
		SaveSubsystem->MarkBossDefeated(BossID);
	}

	// 보스의 EndBossEncounter 호출
	if (SpawnedBoss)
	{
		SpawnedBoss->EndBossEncounter();
	}

	OnArenaCleared.Broadcast();
	UE_LOG(LogHarmoniaStory, Log, TEXT("BossArena [%s]: Boss defeated, arena cleared"), *BossID.ToString());
}
