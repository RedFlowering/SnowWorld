// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LyraSaveGame.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "LyraSaveGameSubsystem.generated.h"

class APlayerController;
class ALyraPlayerState;
class ULyraInventoryManagerComponent;

/** 세이브/로드 결과 */
UENUM(BlueprintType)
enum class ELyraSaveGameResult : uint8
{
	Success,
	Failed,
	InProgress,
	NotSupported
};

/** 세이브/로드 완료 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveGameComplete, ELyraSaveGameResult, Result, const FString&, SaveSlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadGameComplete, ELyraSaveGameResult, Result, ULyraSaveGame*, SaveGameObject);

/**
 * 게임 세이브/로드 시스템
 *
 * 기능:
 * - 로컬 PC에 저장 (서버 소유주)
 * - 스팀 클라우드 세이브 지원
 * - 멀티플레이어: 각 플레이어의 스팀 ID별 데이터 저장
 *
 * 사용 방법:
 * 1. SaveGame() - 현재 게임 상태 저장
 * 2. LoadGame() - 게임 상태 로드
 * 3. AutoSave 활성화 시 자동 저장
 */
UCLASS()
class LYRAGAME_API ULyraSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ULyraSaveGameSubsystem();

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
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame")
	bool SaveGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 게임 로드
	 * @param SaveSlotName 로드할 슬롯 이름
	 * @param bUseSteamCloud 스팀 클라우드에서 로드 여부
	 * @return 로드 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame")
	bool LoadGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 세이브 파일 삭제
	 * @param SaveSlotName 삭제할 슬롯 이름
	 * @param bDeleteFromSteamCloud 스팀 클라우드에서도 삭제 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame")
	bool DeleteSaveGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bDeleteFromSteamCloud = true);

	/**
	 * 세이브 파일 존재 여부 확인
	 * @param SaveSlotName 확인할 슬롯 이름
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame")
	bool DoesSaveGameExist(const FString& SaveSlotName = TEXT("DefaultSave")) const;

	/**
	 * 현재 로드된 세이브 게임 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Lyra|SaveGame")
	ULyraSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }

	/**
	 * 자동 저장 활성화/비활성화
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame")
	void SetAutoSaveEnabled(bool bEnabled);

	/**
	 * 자동 저장 간격 설정 (초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame")
	void SetAutoSaveInterval(float IntervalInSeconds);

	/** 세이브 완료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Lyra|SaveGame")
	FOnSaveGameComplete OnSaveGameComplete;

	/** 로드 완료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Lyra|SaveGame")
	FOnLoadGameComplete OnLoadGameComplete;

protected:
	/** 플레이어 데이터를 SaveGame에 저장 */
	void SavePlayerData(APlayerController* PlayerController, ULyraSaveGame* SaveGameObject);

	/** SaveGame에서 플레이어 데이터 로드 */
	void LoadPlayerData(APlayerController* PlayerController, const ULyraSaveGame* SaveGameObject);

	/** 월드 데이터 저장 (빌딩 등) */
	void SaveWorldData(ULyraSaveGame* SaveGameObject);

	/** 월드 데이터 로드 */
	void LoadWorldData(const ULyraSaveGame* SaveGameObject);

	/** 플레이어 속성 저장 */
	void SavePlayerAttributes(ALyraPlayerState* PlayerState, FLyraSavedPlayerAttributes& OutAttributes);

	/** 플레이어 속성 로드 */
	void LoadPlayerAttributes(ALyraPlayerState* PlayerState, const FLyraSavedPlayerAttributes& Attributes);

	/** 인벤토리 저장 */
	void SaveInventory(ULyraInventoryManagerComponent* InventoryComponent, TArray<FLyraSavedInventoryItem>& OutItems);

	/** 인벤토리 로드 */
	void LoadInventory(ULyraInventoryManagerComponent* InventoryComponent, const TArray<FLyraSavedInventoryItem>& Items);

	/** 스팀 ID 가져오기 */
	FString GetSteamIDForPlayer(APlayerController* PlayerController) const;

	/** 로컬 플레이어가 서버 소유주인지 확인 */
	bool IsServerOwner(APlayerController* PlayerController) const;

	/** 스팀 클라우드에 저장 */
	bool SaveToSteamCloud(const FString& SaveSlotName, const TArray<uint8>& SaveData);

	/** 스팀 클라우드에서 로드 */
	bool LoadFromSteamCloud(const FString& SaveSlotName, TArray<uint8>& OutSaveData);

	/** 자동 저장 타이머 */
	void OnAutoSaveTimer();

private:
	/** 현재 로드된 세이브 게임 */
	UPROPERTY(Transient)
	TObjectPtr<ULyraSaveGame> CurrentSaveGame;

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

	/** 세이브 파일 사용자 인덱스 */
	static const int32 SaveGameUserIndex;
};
