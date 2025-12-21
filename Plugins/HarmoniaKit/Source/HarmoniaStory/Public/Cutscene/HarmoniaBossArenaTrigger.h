// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Cutscene/HarmoniaCutsceneTrigger.h"
#include "System/HarmoniaSaveGame.h"
#include "HarmoniaBossArenaTrigger.generated.h"

class AHarmoniaBossMonster;
class AHarmoniaMonsterSpawner;
class AHarmoniaMonsterBase;
class USceneComponent;

/** 보스 아레나 상태 */
UENUM(BlueprintType)
enum class EBossArenaState : uint8
{
	/** 대기 중 */
	Inactive,
	
	/** 인트로 재생 중 */
	PlayingIntro,
	
	/** 전투 중 */
	InCombat,
	
	/** 클리어됨 */
	Cleared
};

/** 보스 아레나 이벤트 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossArenaActivated, AHarmoniaBossMonster*, Boss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossArenaCombatStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossArenaCleared);

/**
 * @class AHarmoniaBossArenaTrigger
 * @brief 보스 아레나 트리거 (HarmoniaCutsceneTrigger 확장)
 * 
 * 기존 CutsceneTrigger의 시퀀스 재생 기능을 활용하면서,
 * 보스 스폰, 인트로 스킵 여부 판단, 세이브 연동 기능을 추가합니다.
 * 
 * 사용법:
 * 1. 레벨에 배치
 * 2. BossID 설정 (세이브 시스템 키)
 * 3. BossSpawner 또는 BossClass 설정
 * 4. CutsceneSequence 설정 (부모 클래스에서 상속)
 * 5. 시퀀서에서 Boss, Player 바인딩 태그 설정
 */
UCLASS(Blueprintable, meta=(DisplayName="Boss Arena Trigger"))
class HARMONIASTORY_API AHarmoniaBossArenaTrigger : public AHarmoniaCutsceneTrigger
{
	GENERATED_BODY()

public:
	AHarmoniaBossArenaTrigger();

protected:
	virtual void BeginPlay() override;

public:
	// ===== Components =====

	/** 보스 스폰 위치 (스포너 없이 직접 스폰할 경우) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Arena")
	TObjectPtr<USceneComponent> BossSpawnPoint;

	// ===== Boss Configuration =====

	/** 보스 고유 식별자 (세이브 시스템 키) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Arena")
	FName BossID;

	/** 보스 스포너 (레벨에 배치된 스포너 참조) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Arena")
	TObjectPtr<AHarmoniaMonsterSpawner> BossSpawner;

	/** 보스 클래스 (스포너 없이 직접 스폰할 경우) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Arena", 
		meta=(EditCondition="BossSpawner==nullptr"))
	TSubclassOf<AHarmoniaBossMonster> BossClass;

	// ===== Sequence Binding =====

	/** 시퀀스에서 보스에 사용할 바인딩 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Arena|Sequence")
	FName BossBindingTag = TEXT("Boss");

	/** 시퀀스에서 플레이어에 사용할 바인딩 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Arena|Sequence")
	FName PlayerBindingTag = TEXT("Player");

	// ===== Respawn Settings =====

	/** 보스 리스폰 정책 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Arena|Respawn")
	EBossRespawnPolicy RespawnPolicy = EBossRespawnPolicy::Never;

	/** AfterTime 정책일 때 리스폰 대기 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Arena|Respawn", 
		meta=(EditCondition="RespawnPolicy==EBossRespawnPolicy::AfterTime", ClampMin="0"))
	float RespawnTimeSeconds = 3600.0f;

	// ===== Events =====

	/** 아레나 활성화 시 (보스 스폰됨) */
	UPROPERTY(BlueprintAssignable, Category = "Boss Arena|Events")
	FOnBossArenaActivated OnArenaActivated;

	/** 전투 시작 시 */
	UPROPERTY(BlueprintAssignable, Category = "Boss Arena|Events")
	FOnBossArenaCombatStarted OnCombatStarted;

	/** 보스 클리어 시 */
	UPROPERTY(BlueprintAssignable, Category = "Boss Arena|Events")
	FOnBossArenaCleared OnArenaCleared;

	// ===== Public Functions =====

	/** 현재 아레나 상태 */
	UFUNCTION(BlueprintPure, Category = "Boss Arena")
	EBossArenaState GetArenaState() const { return ArenaState; }

	/** 스폰된 보스 */
	UFUNCTION(BlueprintPure, Category = "Boss Arena")
	AHarmoniaBossMonster* GetSpawnedBoss() const { return SpawnedBoss; }

	/** 보스 처치 알림 (보스에서 호출 또는 자동 연결) */
	UFUNCTION(BlueprintCallable, Category = "Boss Arena")
	void NotifyBossDefeated();

protected:
	// ===== Override from HarmoniaCutsceneTrigger =====

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
		const FHitResult& SweepResult) override;

	// ===== Internal Functions =====

	/** 아레나 활성화 (조건 확인 후 처리) */
	void ActivateArena(APawn* PlayerPawn);

	/** 보스 스폰 */
	AHarmoniaBossMonster* SpawnBoss(bool bStartHidden = false);

	/** 인트로 시퀀스 재생 */
	void PlayBossIntroSequence(APawn* PlayerPawn);

	/** 시퀀스 종료 콜백 */
	UFUNCTION()
	void OnBossIntroFinished(ULevelSequence* Sequence);

	/** 전투 시작 */
	void StartBossCombat();

	/** 보스 사망 콜백 (OnMonsterDeath 델리게이트용) */
	UFUNCTION()
	void OnBossDeathInternal(AHarmoniaMonsterBase* Monster, AActor* Killer);

private:
	/** 현재 아레나 상태 */
	UPROPERTY(Transient)
	EBossArenaState ArenaState = EBossArenaState::Inactive;

	/** 스폰된 보스 */
	UPROPERTY(Transient)
	TObjectPtr<AHarmoniaBossMonster> SpawnedBoss;

	/** 트리거한 플레이어 Pawn */
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> TriggeringPlayerPawn;
};
