// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HarmoniaMonsterTrigger.generated.h"

class UBoxComponent;
class USceneComponent;
class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class AHarmoniaMonsterBase;
class AHarmoniaBossMonster;

/**
 * 스폰 모드
 */
UENUM(BlueprintType)
enum class EHarmoniaMonsterSpawnMode : uint8
{
	/** 직접 SpawnMonster() 호출 시에만 스폰 */
	Manual UMETA(DisplayName = "Manual"),

	/** 플레이어가 트리거 볼륨 진입 시 스폰 */
	OnTrigger UMETA(DisplayName = "On Trigger"),

	/** 게임 시작 시 즉시 스폰 */
	OnBeginPlay UMETA(DisplayName = "On Begin Play"),

	/** 몬스터 사망 시 리스폰 */
	Respawn UMETA(DisplayName = "Respawn"),

	/** 웨이브 주기로 스폰 (사망 여부와 무관하게 진행) */
	Wave UMETA(DisplayName = "Wave")
};

/**
 * 리스폰 정책 (Respawn 모드용)
 */
UENUM(BlueprintType)
enum class EHarmoniaRespawnPolicy : uint8
{
	/** 리스폰 안함 */
	Never UMETA(DisplayName = "Never"),

	/** 모든 몬스터 사망 시 즉시 리스폰 */
	OnDeath UMETA(DisplayName = "On Death"),

	/** 모든 몬스터 사망 후 대기 시간 뒤 리스폰 */
	OnDeathDelay UMETA(DisplayName = "On Death (Delay)"),

	/** 시간 경과 후 리스폰 (세이브/로드 시 - 월드 보스 등) */
	AfterTime UMETA(DisplayName = "After Time")
};

/**
 * 트리거 상태
 */
UENUM(BlueprintType)
enum class EHarmoniaMonsterTriggerState : uint8
{
	/** 비활성 - 대기 중 */
	Inactive UMETA(DisplayName = "Inactive"),

	/** 인트로 시퀀스 재생 중 */
	PlayingIntro UMETA(DisplayName = "Playing Intro"),

	/** 활성 - 몬스터가 스폰됨 */
	Active UMETA(DisplayName = "Active"),

	/** 클리어됨 - 모든 몬스터 처치 */
	Cleared UMETA(DisplayName = "Cleared"),

	/** 웨이브 진행 중 */
	WaveInProgress UMETA(DisplayName = "Wave In Progress")
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterTriggeredSpawned, AHarmoniaMonsterBase*, Monster);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterTriggeredDeath, AHarmoniaMonsterBase*, Monster, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllMonstersTriggeredDefeated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTriggerIntroSequenceStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTriggerIntroSequenceFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerWaveStarted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerWaveCompleted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTriggerAllWavesCompleted);

/**
 * AHarmoniaMonsterTrigger
 *
 * 통합 몬스터 스포너 및 트리거 시스템
 *
 * 기능:
 * - 트리거 볼륨 내장 (플레이어 진입 시 스폰)
 * - 다양한 스폰 모드 (수동, 트리거, 게임 시작, 리스폰, 웨이브)
 * - 인트로 시퀀스 재생 지원 (보통 보스용)
 * - 세이브 시스템 연동
 * - 보스 몬스터 자동 인식 (AHarmoniaBossMonster 캐스팅)
 *
 * 사용법:
 * 1. 레벨에 배치
 * 2. MonsterClass 설정
 * 3. SpawnMode 선택
 * 4. (선택) IntroSequence 설정
 * 5. (선택) TriggerID 설정 (세이브 연동)
 */
UCLASS(Blueprintable, meta = (DisplayName = "Monster Trigger"))
class HARMONIAKIT_API AHarmoniaMonsterTrigger : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaMonsterTrigger();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ============================================================================
	// Components
	// ============================================================================

	/** 트리거 볼륨 (OnTrigger 모드에서 사용) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster Trigger")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** 스폰 위치 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster Trigger")
	TObjectPtr<USceneComponent> SpawnPoint;

	// ============================================================================
	// Monster Configuration
	// ============================================================================

	/** 스폰할 몬스터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Config")
	TSubclassOf<AHarmoniaMonsterBase> MonsterClass;

	/** 스폰할 몬스터 수 (웨이브 모드에서는 MonstersPerWave 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Config", 
		meta = (ClampMin = "1", EditCondition = "SpawnMode != EHarmoniaMonsterSpawnMode::Wave"))
	int32 SpawnCount = 1;

	/** 스폰 반경 (0 = 정확한 위치) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Config", 
		meta = (ClampMin = "0"))
	float SpawnRadius = 0.0f;

	// ============================================================================
	// Spawn Mode
	// ============================================================================

	/** 스폰 모드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Spawn Mode")
	EHarmoniaMonsterSpawnMode SpawnMode = EHarmoniaMonsterSpawnMode::OnTrigger;

	/** 첫 스폰 전 대기 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Spawn Mode", 
		meta = (ClampMin = "0"))
	float InitialSpawnDelay = 0.0f;

	// ============================================================================
	// Respawn Settings (Respawn Mode)
	// ============================================================================

	/** 리스폰 정책 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Respawn",
		meta = (EditCondition = "SpawnMode == EHarmoniaMonsterSpawnMode::Respawn"))
	EHarmoniaRespawnPolicy RespawnPolicy = EHarmoniaRespawnPolicy::Never;

	/** 리스폰 대기 시간 (OnDeathDelay, AfterTime 용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Respawn",
		meta = (EditCondition = "SpawnMode == EHarmoniaMonsterSpawnMode::Respawn && (RespawnPolicy == EHarmoniaRespawnPolicy::OnDeathDelay || RespawnPolicy == EHarmoniaRespawnPolicy::AfterTime)", ClampMin = "0"))
	float RespawnDelaySeconds = 30.0f;

	// ============================================================================
	// Wave Settings (Wave Mode)
	// ============================================================================

	/** 웨이브당 몬스터 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Wave",
		meta = (EditCondition = "SpawnMode == EHarmoniaMonsterSpawnMode::Wave", ClampMin = "1"))
	int32 MonstersPerWave = 3;

	/** 총 웨이브 수 (0 = 무한) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Wave",
		meta = (EditCondition = "SpawnMode == EHarmoniaMonsterSpawnMode::Wave", ClampMin = "0"))
	int32 WaveCount = 5;

	/** 웨이브 간격 (초) - 사망 여부와 무관하게 진행 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Wave",
		meta = (EditCondition = "SpawnMode == EHarmoniaMonsterSpawnMode::Wave", ClampMin = "1"))
	float WaveInterval = 30.0f;

	// ============================================================================
	// Intro Sequence (Optional)
	// ============================================================================

	/** 스폰 시 재생할 레벨 시퀀스 (없으면 재생 안함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Intro")
	TObjectPtr<ULevelSequence> IntroSequence;

	/** 인트로 중 몬스터 숨김 (시퀀서에서 등장 연출 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Intro",
		meta = (EditCondition = "IntroSequence != nullptr"))
	bool bHideMonsterDuringIntro = true;

	/** 재방문 시 인트로 스킵 (세이브 연동 필요) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Intro",
		meta = (EditCondition = "IntroSequence != nullptr"))
	bool bSkipIntroOnRevisit = true;

	// ============================================================================
	// Save System
	// ============================================================================

	/** 트리거 고유 ID (세이브 키) - 비어있으면 저장 안함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Save")
	FName TriggerID;

	// ============================================================================
	// Debug
	// ============================================================================

	/** 트리거 볼륨/스폰 범위 시각화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Trigger|Debug")
	bool bShowDebugVolume = true;

	// ============================================================================
	// Events
	// ============================================================================

	/** 몬스터 스폰 시 */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnMonsterTriggeredSpawned OnMonsterSpawned;

	/** 몬스터 사망 시 */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnMonsterTriggeredDeath OnMonsterDeath;

	/** 모든 몬스터 처치 시 */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnAllMonstersTriggeredDefeated OnAllMonstersDefeated;

	/** 인트로 시퀀스 시작 시 */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnTriggerIntroSequenceStarted OnIntroStarted;

	/** 인트로 시퀀스 종료 시 */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnTriggerIntroSequenceFinished OnIntroFinished;

	/** 웨이브 시작 시 */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnTriggerWaveStarted OnWaveStarted;

	/** 웨이브 완료 시 (해당 웨이브 몬스터 전멸) */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnTriggerWaveCompleted OnWaveCompleted;

	/** 모든 웨이브 완료 시 */
	UPROPERTY(BlueprintAssignable, Category = "Monster Trigger|Events")
	FOnTriggerAllWavesCompleted OnAllWavesCompleted;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/** 몬스터 하나 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Monster Trigger")
	AHarmoniaMonsterBase* SpawnMonster();

	/** SpawnCount 만큼 모두 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Monster Trigger")
	void SpawnAllMonsters();

	/** 특정 몬스터 디스폰 */
	UFUNCTION(BlueprintCallable, Category = "Monster Trigger")
	void DespawnMonster(AHarmoniaMonsterBase* Monster);

	/** 모든 몬스터 디스폰 */
	UFUNCTION(BlueprintCallable, Category = "Monster Trigger")
	void DespawnAllMonsters();

	/** 트리거 수동 활성화 */
	UFUNCTION(BlueprintCallable, Category = "Monster Trigger")
	void ActivateTrigger(APawn* TriggeringPlayer = nullptr);

	/** 트리거 비활성화 */
	UFUNCTION(BlueprintCallable, Category = "Monster Trigger")
	void DeactivateTrigger();

	/** 현재 활성 몬스터 목록 */
	UFUNCTION(BlueprintPure, Category = "Monster Trigger")
	TArray<AHarmoniaMonsterBase*> GetActiveMonsters() const;

	/** 활성 몬스터 수 */
	UFUNCTION(BlueprintPure, Category = "Monster Trigger")
	int32 GetActiveMonsterCount() const;

	/** 현재 트리거 상태 */
	UFUNCTION(BlueprintPure, Category = "Monster Trigger")
	EHarmoniaMonsterTriggerState GetTriggerState() const { return TriggerState; }

	/** 현재 웨이브 번호 */
	UFUNCTION(BlueprintPure, Category = "Monster Trigger")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** 트리거 활성 여부 */
	UFUNCTION(BlueprintPure, Category = "Monster Trigger")
	bool IsActive() const { return TriggerState == EHarmoniaMonsterTriggerState::Active || TriggerState == EHarmoniaMonsterTriggerState::WaveInProgress; }

protected:
	// ============================================================================
	// Internal Functions
	// ============================================================================

	/** 트리거 오버랩 */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	/** 몬스터 사망 콜백 */
	UFUNCTION()
	void OnMonsterDeathInternal(AHarmoniaMonsterBase* Monster, AActor* Killer);

	/** 인트로 시퀀스 재생 */
	void PlayIntroSequence();

	/** 인트로 종료 콜백 */
	UFUNCTION()
	void OnIntroSequenceComplete();

	/** 전투 시작 */
	void StartCombat();

	/** 클리어 처리 */
	void HandleCleared();

	/** 리스폰 처리 */
	void HandleRespawn();

	/** 웨이브 시작 */
	void StartNextWave();

	/** 웨이브 스폰 타이머 콜백 */
	UFUNCTION()
	void OnWaveTimerFired();

	/** 지연 스폰 타이머 콜백 */
	UFUNCTION()
	void OnSpawnDelayComplete();

	/** 리스폰 타이머 콜백 */
	UFUNCTION()
	void OnRespawnDelayComplete();

	/** 랜덤 스폰 위치 계산 */
	FVector GetRandomSpawnLocation() const;

	/** 세이브 상태 로드 */
	void LoadSaveState();

	/** 세이브 상태 저장 */
	void SaveState();

	/** 시퀀서에 액터 바인딩 */
	void BindActorsToSequence(AHarmoniaMonsterBase* Monster, APawn* Player);

	/** 유효하지 않은 몬스터 레퍼런스 정리 */
	void CleanupInvalidMonsterReferences();

	/** 보스 몬스터인지 확인하고 인카운터 시작 */
	void TryStartBossEncounter(AHarmoniaMonsterBase* Monster);

	// ============================================================================
	// Internal State
	// ============================================================================

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Monster Trigger|State")
	EHarmoniaMonsterTriggerState TriggerState = EHarmoniaMonsterTriggerState::Inactive;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AHarmoniaMonsterBase>> ActiveMonsters;

	UPROPERTY(Transient)
	int32 CurrentWave = 0;

	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> TriggeringPlayerPawn;

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> SequencePlayer;

	UPROPERTY(Transient)
	TObjectPtr<ALevelSequenceActor> SequenceActor;

	FTimerHandle SpawnDelayTimerHandle;
	FTimerHandle RespawnTimerHandle;
	FTimerHandle WaveTimerHandle;

	bool bIntroWatched = false;
	bool bHasSpawned = false;
	bool bTriggerActivated = false;
};
