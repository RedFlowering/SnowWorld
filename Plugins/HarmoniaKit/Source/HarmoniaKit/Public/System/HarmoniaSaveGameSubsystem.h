// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HarmoniaSaveGame.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "HarmoniaSaveGameSubsystem.generated.h"

class APlayerController;
class ALyraPlayerState;
class ULyraInventoryManagerComponent;

/** 세이브/로드 결과 */
UENUM(BlueprintType)
enum class EHarmoniaSaveGameResult : uint8
{
	Success,
	Failed,
	InProgress,
	NotSupported
};

/** 세이브/로드 완료 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaSaveGameComplete, EHarmoniaSaveGameResult, Result, const FString&, SaveSlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaLoadGameComplete, EHarmoniaSaveGameResult, Result, UHarmoniaSaveGame*, SaveGameObject);

/**
 * 게임 세이브/로드 서브시스템
 *
 * 기능:
 * - 로컬 PC에 저장 (서버와 공유 가능)
 * - 스팀 클라우드 저장 지원
 * - 멀티플레이어 시 각 플레이어별 고유 ID로 데이터 저장
 *
 * 사용 방법:
 * 1. SaveGame() - 현재 게임 상태 저장
 * 2. LoadGame() - 게임 상태 로드
 * 3. AutoSave 활성화 시 자동 저장
 */
UCLASS(config=Game)
class HARMONIAKIT_API UHarmoniaSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaSaveGameSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End of USubsystem interface

	/**
	 * 게임 저장
	 * @param SaveSlotName 저장 슬롯 이름
	 * @param bUseSteamCloud 스팀 클라우드 사용 여부
	 * @return 저장 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool SaveGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 게임 로드
	 * @param SaveSlotName 저장 슬롯 이름
	 * @param bUseSteamCloud 스팀 클라우드 사용 여부
	 * @return 로드 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool LoadGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 세이브 파일 삭제
	 * @param SaveSlotName 저장 슬롯 이름
	 * @param bDeleteFromSteamCloud 스팀 클라우드에서도 삭제할지 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool DeleteSaveGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bDeleteFromSteamCloud = true);

	/**
	 * 세이브 파일 존재 확인
	 * @param SaveSlotName 저장 슬롯 이름
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool DoesSaveGameExist(const FString& SaveSlotName = TEXT("DefaultSave")) const;

	/**
	 * 현재 로드된 SaveGame 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame")
	UHarmoniaSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }

	/**
	 * 자동 세이브 활성화/비활성화
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	void SetAutoSaveEnabled(bool bEnabled);

	/**
	 * 자동 세이브 간격 설정 (초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	void SetAutoSaveInterval(float IntervalInSeconds);

	/** 세이브 완료 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|SaveGame")
	FOnHarmoniaSaveGameComplete OnSaveGameComplete;

	/** 로드 완료 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|SaveGame")
	FOnHarmoniaLoadGameComplete OnLoadGameComplete;

	// ===== Boss State Management =====

	/**
	 * 보스 진행 상태 조회
	 * @param BossID 보스 고유 식별자
	 * @return 보스 진행 상태 (없으면 기본값)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame|Boss")
	FHarmoniaSavedBossState GetBossState(FName BossID) const;

	/**
	 * 보스 인트로 시퀀스 시청 완료 표시
	 * @param BossID 보스 고유 식별자
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame|Boss")
	void MarkBossIntroViewed(FName BossID);

	/**
	 * 보스 처치 완료 표시
	 * @param BossID 보스 고유 식별자
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame|Boss")
	void MarkBossDefeated(FName BossID);

	/**
	 * 보스 리스폰 여부 확인
	 * @param BossID 보스 고유 식별자
	 * @param RespawnPolicy 리스폰 정책
	 * @param RespawnTimeSeconds AfterTime 정책일 때 리스폰 대기 시간
	 * @return 리스폰 가능 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame|Boss")
	bool ShouldBossRespawn(FName BossID, EBossRespawnPolicy RespawnPolicy, float RespawnTimeSeconds) const;

protected:
	/** 플레이어 데이터를 SaveGame에 저장 */
	void SavePlayerData(APlayerController* PlayerController, UHarmoniaSaveGame* SaveGameObject);

	/** SaveGame에서 플레이어 데이터를 로드 */
	void LoadPlayerData(APlayerController* PlayerController, const UHarmoniaSaveGame* SaveGameObject);

	/** 월드 데이터를 저장 (빌딩 등) */
	void SaveWorldData(UHarmoniaSaveGame* SaveGameObject);

	/** 월드 데이터를 로드 */
	void LoadWorldData(const UHarmoniaSaveGame* SaveGameObject);

	/** 플레이어 속성 저장 */
	void SavePlayerAttributes(ALyraPlayerState* PlayerState, FHarmoniaSavedPlayerAttributes& OutAttributes);

	/** 플레이어 속성 로드 */
	void LoadPlayerAttributes(ALyraPlayerState* PlayerState, const FHarmoniaSavedPlayerAttributes& Attributes);

	/** 인벤토리 저장 */
	void SaveInventory(ULyraInventoryManagerComponent* InventoryComponent, TArray<FHarmoniaSavedInventoryItem>& OutItems);

	/** 인벤토리 로드 */
	void LoadInventory(ULyraInventoryManagerComponent* InventoryComponent, const TArray<FHarmoniaSavedInventoryItem>& Items);

	/** 스팀 ID 가져오기 */
	FString GetSteamIDForPlayer(APlayerController* PlayerController) const;

	/** 로컬 플레이어가 서버 소유주인지 확인 */
	bool IsServerOwner(APlayerController* PlayerController) const;

	/** 스팀 클라우드에 저장 */
	bool SaveToSteamCloud(const FString& SaveSlotName, const TArray<uint8>& SaveData);

	/** 스팀 클라우드에서 로드 */
	bool LoadFromSteamCloud(const FString& SaveSlotName, TArray<uint8>& OutSaveData);

	/** 자동 세이브 타이머 콜백 */
	void OnAutoSaveTimer();

	// [SECURITY] Save file encryption and integrity
	/** Encrypt save data using XOR cipher */
	void EncryptSaveData(const TArray<uint8>& InData, TArray<uint8>& OutEncryptedData) const;

	/** Decrypt save data using XOR cipher */
	void DecryptSaveData(const TArray<uint8>& InEncryptedData, TArray<uint8>& OutData) const;

	/** Calculate CRC32 checksum for save data integrity */
	uint32 CalculateChecksum(const TArray<uint8>& Data) const;

	/** Verify save data integrity using CRC32 checksum */
	bool VerifyChecksum(const TArray<uint8>& Data, uint32 ExpectedChecksum) const;

	/** 보스 상태 찾기 또는 생성 (내부용) */
	FHarmoniaSavedBossState* FindOrCreateBossState(FName BossID);

private:
	/** 현재 로드된 SaveGame */
	UPROPERTY(Transient)
	TObjectPtr<UHarmoniaSaveGame> CurrentSaveGame;

	/** 자동 저장 활성화 여부 */
	UPROPERTY(Config)
	bool bAutoSaveEnabled = true;

	/** 자동 저장 간격 (초) */
	UPROPERTY(Config)
	float AutoSaveIntervalSeconds = 300.0f; // 5분

	/** 자동 저장 타이머 핸들 */
	FTimerHandle AutoSaveTimerHandle;

	/** 마지막 저장 시간 */
	float LastSaveTime = 0.0f;

	/** 기본 저장 슬롯 이름 */
	static const FString DefaultSaveSlotName;

	/** SaveGame 사용자 인덱스 */
	static const int32 SaveGameUserIndex;
};
