// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaCheckpointSystemDefinitions.h"
#include "HarmoniaCheckpointSubsystem.generated.h"

class AHarmoniaCrystalResonator;
class AHarmoniaMonsterSpawner;
class APlayerController;

/**
 * Checkpoint Events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointActivated, FName, CheckpointID, const FHarmoniaCheckpointData&, CheckpointData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResonanceStarted, FName, CheckpointID, APlayerController*, Player, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResonanceCompleted, FName, CheckpointID, const FHarmoniaResonanceResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResonanceCancelled, FName, CheckpointID, FText, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointTeleport, FName, FromCheckpointID, FName, ToCheckpointID, const FHarmoniaTeleportResult&, Result);

/**
 * UHarmoniaCheckpointSubsystem
 *
 * 크리스탈 공명형 체크포인트 시스템 관리
 *
 * 주요 기능:
 * - 체크포인트 등록 및 활성화
 * - 공명(휴식) 시스템 - 회복 + 적 리스폰 + 자동 저장
 * - 체크포인트간 텔레포트 (공명 네트워크)
 * - 체크포인트 강화 시스템
 * - 죽었을때 마지막 체크포인트에서 리스폰
 * - 공명 주파수별 고유한 효과
 *
 * 확장할 요소:
 * - 각 크리스탈마다 고유한 "공명 주파수" (색상/음향)
 * - 체크포인트 강화로 회복량 증가
 * - 가까운 체크포인트들끼리 공명 효과
 * - 공명 네트워크를 통한 텔레포트
 */
UCLASS(Config=Game)
class HARMONIAKIT_API UHarmoniaCheckpointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaCheckpointSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End of USubsystem interface

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * 체크포인트 설정 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	const FHarmoniaCheckpointConfig& GetCheckpointConfig() const { return CheckpointConfig; }

	/**
	 * 체크포인트 설정 변경
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void SetCheckpointConfig(const FHarmoniaCheckpointConfig& NewConfig) { CheckpointConfig = NewConfig; }

	// ============================================================================
	// Checkpoint Registration
	// ============================================================================

	/**
	 * 체크포인트 등록
	 * @param Checkpoint 등록할 체크포인트 액터
	 * @return 등록 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool RegisterCheckpoint(AHarmoniaCrystalResonator* Checkpoint);

	/**
	 * 체크포인트 등록 해제
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void UnregisterCheckpoint(FName CheckpointID);

	/**
	 * 체크포인트 찾기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	AHarmoniaCrystalResonator* FindCheckpoint(FName CheckpointID) const;

	/**
	 * 모든 체크포인트 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	TArray<AHarmoniaCrystalResonator*> GetAllCheckpoints() const;

	/**
	 * 활성화된 체크포인트만 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	TArray<AHarmoniaCrystalResonator*> GetActivatedCheckpoints() const;

	// ============================================================================
	// Checkpoint Activation
	// ============================================================================

	/**
	 * 체크포인트 활성화
	 * @param CheckpointID 활성화할 체크포인트 ID
	 * @param Player 활성화하는 플레이어
	 * @return 활성화 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool ActivateCheckpoint(FName CheckpointID, APlayerController* Player);

	/**
	 * 체크포인트 활성화 여부 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	bool IsCheckpointActivated(FName CheckpointID) const;

	// ============================================================================
	// Resonance System (Rest/Recovery)
	// ============================================================================

	/**
	 * 체크포인트에서 공명 시작 (휴식/회복)
	 * @param CheckpointID 공명할 체크포인트 ID
	 * @param Player 공명하는 플레이어
	 * @return 공명 시작 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool StartResonance(FName CheckpointID, APlayerController* Player);

	/**
	 * 공명 취소
	 * @param Player 공명 중인 플레이어
	 * @param Reason 취소 이유
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void CancelResonance(APlayerController* Player, const FText& Reason);

	/**
	 * 플레이어가 현재 공명 중인지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	bool IsPlayerResonating(APlayerController* Player) const;

	/**
	 * 플레이어가 공명 중인 체크포인트 ID 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	FName GetResonatingCheckpointID(APlayerController* Player) const;

	// ============================================================================
	// Teleportation
	// ============================================================================

	/**
	 * 체크포인트로 텔레포트
	 * @param Player 텔레포트할 플레이어
	 * @param DestinationCheckpointID 목적지 체크포인트 ID
	 * @return 텔레포트 결과
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	FHarmoniaTeleportResult TeleportToCheckpoint(APlayerController* Player, FName DestinationCheckpointID);

	/**
	 * 텔레포트 가능 여부 확인
	 * @param Player 텔레포트할 플레이어
	 * @param DestinationCheckpointID 목적지 체크포인트 ID
	 * @param OutReason 불가능한 이유 (실패 시)
	 * @return 텔레포트 가능 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	bool CanTeleportToCheckpoint(APlayerController* Player, FName DestinationCheckpointID, FText& OutReason) const;

	/**
	 * 텔레포트 비용 계산
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	int32 CalculateTeleportCost(FName FromCheckpointID, FName ToCheckpointID) const;

	// ============================================================================
	// Checkpoint Upgrades
	// ============================================================================

	/**
	 * 체크포인트 강화
	 * @param CheckpointID 강화할 체크포인트 ID
	 * @param UpgradeType 강화 타입
	 * @return 강화 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool UpgradeCheckpoint(FName CheckpointID, EHarmoniaCheckpointUpgradeType UpgradeType);

	/**
	 * 체크포인트 강화 레벨 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	int32 GetCheckpointUpgradeLevel(FName CheckpointID, EHarmoniaCheckpointUpgradeType UpgradeType) const;

	// ============================================================================
	// Respawn System
	// ============================================================================

	/**
	 * 플레이어 마지막 공명 체크포인트 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void SetPlayerLastCheckpoint(APlayerController* Player, FName CheckpointID);

	/**
	 * 플레이어 마지막 공명 체크포인트 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	FName GetPlayerLastCheckpoint(APlayerController* Player) const;

	/**
	 * 플레이어를 마지막 체크포인트에서 리스폰
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool RespawnPlayerAtLastCheckpoint(APlayerController* Player);

	// ============================================================================
	// Monster Respawn
	// ============================================================================

	/**
	 * 몬스터 스포너 등록 (공명 시 리셋용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void RegisterMonsterSpawner(AHarmoniaMonsterSpawner* Spawner);

	/**
	 * 몬스터 스포너 등록 해제
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void UnregisterMonsterSpawner(AHarmoniaMonsterSpawner* Spawner);

	/**
	 * 모든 적 리스폰 (공명 시 호출)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	int32 RespawnAllEnemies();

	// ============================================================================
	// Data Persistence
	// ============================================================================

	/**
	 * 체크포인트 데이터 저장
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void SaveCheckpointData();

	/**
	 * 체크포인트 데이터 로드
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void LoadCheckpointData();

	/**
	 * 체크포인트 데이터 가져오기 (세이브 시스템 연동용)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	TArray<FHarmoniaCheckpointData> GetCheckpointDataForSave() const;

	/**
	 * 체크포인트 데이터 적용 (로드 시스템 연동용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void ApplyCheckpointDataFromLoad(const TArray<FHarmoniaCheckpointData>& LoadedData);

	// ============================================================================
	// Events
	// ============================================================================

	/** 체크포인트 활성화 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnCheckpointActivated OnCheckpointActivated;

	/** 공명 시작 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnResonanceStarted OnResonanceStarted;

	/** 공명 완료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnResonanceCompleted OnResonanceCompleted;

	/** 공명 취소 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnResonanceCancelled OnResonanceCancelled;

	/** 텔레포트 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnCheckpointTeleport OnCheckpointTeleport;

protected:
	/**
	 * 공명 완료 처리
	 */
	void CompleteResonance(APlayerController* Player, FName CheckpointID);

	/**
	 * 플레이어 회복
	 */
	void RestorePlayerHealth(APlayerController* Player, float RestorationRate);

	/**
	 * 게임 자동 저장
	 */
	void AutoSaveGame(APlayerController* Player);

	/**
	 * 공명 주파수에 따른 추가 효과
	 */
	void ApplyResonanceFrequencyEffects(APlayerController* Player, EHarmoniaResonanceFrequency Frequency);

private:
	/** 체크포인트 설정 */
	UPROPERTY(Config)
	FHarmoniaCheckpointConfig CheckpointConfig;

	/** 등록된 체크포인트들 (ID -> Actor) */
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<AHarmoniaCrystalResonator>> RegisteredCheckpoints;

	/** 체크포인트 데이터 (ID -> Data) */
	UPROPERTY(Transient)
	TMap<FName, FHarmoniaCheckpointData> CheckpointDataMap;

	/** 플레이어별 마지막 체크포인트 (PlayerController -> CheckpointID) */
	UPROPERTY(Transient)
	TMap<TObjectPtr<APlayerController>, FName> PlayerLastCheckpoints;

	/** 공명 중인 플레이어 (PlayerController -> CheckpointID) */
	UPROPERTY(Transient)
	TMap<TObjectPtr<APlayerController>, FName> ResonatingPlayers;

	/** 공명 타이머 핸들 (PlayerController -> TimerHandle) */
	TMap<APlayerController*, FTimerHandle> ResonanceTimerHandles;

	/** 등록된 몬스터 스포너들 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AHarmoniaMonsterSpawner>> RegisteredSpawners;
};
