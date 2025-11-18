// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "Definitions/HarmoniaCheckpointSystemDefinitions.h"
#include "HarmoniaSaveGame.generated.h"

class ULyraInventoryItemInstance;
class ULyraInventoryItemDefinition;

/** 인벤토리 아이템 저장 데이터 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedInventoryItem
{
	GENERATED_BODY()

	/** 아이템 정의 경로 */
	UPROPERTY(SaveGame)
	FSoftObjectPath ItemDefinitionPath;

	/** 스택 개수 */
	UPROPERTY(SaveGame)
	int32 StackCount = 1;

	FHarmoniaSavedInventoryItem()
		: StackCount(1)
	{
	}
};

/** 플레이어 속성 저장 데이터 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedPlayerAttributes
{
	GENERATED_BODY()

	/** 체력 */
	UPROPERTY(SaveGame)
	float Health = 100.0f;

	/** 최대 체력 */
	UPROPERTY(SaveGame)
	float MaxHealth = 100.0f;

	/** 스태미나 */
	UPROPERTY(SaveGame)
	float Stamina = 100.0f;

	/** 최대 스태미나 */
	UPROPERTY(SaveGame)
	float MaxStamina = 100.0f;

	/** 공격력 */
	UPROPERTY(SaveGame)
	float AttackPower = 10.0f;

	/** 방어력 */
	UPROPERTY(SaveGame)
	float Defense = 5.0f;

	/** 크리티컬 확률 */
	UPROPERTY(SaveGame)
	float CriticalChance = 0.1f;

	/** 크리티컬 배수 */
	UPROPERTY(SaveGame)
	float CriticalDamage = 1.5f;

	/** 이동 속도 */
	UPROPERTY(SaveGame)
	float MovementSpeed = 600.0f;

	/** 공격 속도 */
	UPROPERTY(SaveGame)
	float AttackSpeed = 1.0f;
};

/** 빌딩 배치 데이터 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedBuildingInstance
{
	GENERATED_BODY()

	/** 빌딩 파트 ID */
	UPROPERTY(SaveGame)
	FName PartID;

	/** 위치 */
	UPROPERTY(SaveGame)
	FVector Location = FVector::ZeroVector;

	/** 회전 */
	UPROPERTY(SaveGame)
	FRotator Rotation = FRotator::ZeroRotator;

	/** 스케일 */
	UPROPERTY(SaveGame)
	FVector Scale = FVector::OneVector;
};

/** 플레이어별 세이브 데이터 */
USTRUCT(BlueprintType)
struct FHarmoniaPlayerSaveData
{
	GENERATED_BODY()

	/** 스팀 ID (UniqueNetId를 문자열로 저장) */
	UPROPERTY(SaveGame)
	FString SteamID;

	/** 플레이어 이름 */
	UPROPERTY(SaveGame)
	FString PlayerName;

	/** 플레이어 위치 */
	UPROPERTY(SaveGame)
	FVector PlayerLocation = FVector::ZeroVector;

	/** 플레이어 회전 */
	UPROPERTY(SaveGame)
	FRotator PlayerRotation = FRotator::ZeroRotator;

	/** 플레이어 속성 */
	UPROPERTY(SaveGame)
	FHarmoniaSavedPlayerAttributes Attributes;

	/** 인벤토리 아이템 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedInventoryItem> InventoryItems;

	/** 장비 데이터 */
	UPROPERTY(SaveGame)
	FEquipmentSaveData EquipmentData;

	/** 스탯 태그 (체력 버프, 속도 증가 등) */
	UPROPERTY(SaveGame)
	FGameplayTagContainer StatTags;

	/** 마지막 저장 시간 */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTime;

	/** 마지막으로 공명한 체크포인트 ID */
	UPROPERTY(SaveGame)
	FName LastCheckpointID;
};

/** 리소스 노드 상태 저장 데이터 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedResourceNodeState
{
	GENERATED_BODY()

	/** 리소스 노드 ID (위치 기반 해시 또는 고유 ID) */
	UPROPERTY(SaveGame)
	int32 NodeID = 0;

	/** 고갈 상태 (0.0 = 완전 고갈, 1.0 = 완전 충전) */
	UPROPERTY(SaveGame)
	float DepletionAmount = 1.0f;

	/** 리스폰 대기 중인지 */
	UPROPERTY(SaveGame)
	bool bIsRespawning = false;

	/** 리스폰까지 남은 시간 (초) */
	UPROPERTY(SaveGame)
	float RemainingRespawnTime = 0.0f;
};

/** POI 진행 상태 저장 데이터 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedPOIState
{
	GENERATED_BODY()

	/** POI ID */
	UPROPERTY(SaveGame)
	int32 POIID = 0;

	/** 클리어 여부 */
	UPROPERTY(SaveGame)
	bool bIsCompleted = false;

	/** 보물 획득 여부 */
	UPROPERTY(SaveGame)
	bool bTreasureCollected = false;

	/** 방문 횟수 */
	UPROPERTY(SaveGame)
	int32 VisitCount = 0;

	/** 마지막 방문 시간 */
	UPROPERTY(SaveGame)
	FDateTime LastVisitTime;
};

/** 월드 세이브 데이터 */
USTRUCT(BlueprintType)
struct FHarmoniaWorldSaveData
{
	GENERATED_BODY()

	// ===== 빌딩 시스템 =====

	/** 배치된 빌딩 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedBuildingInstance> PlacedBuildings;

	// ===== 월드 생성 정보 =====

	/** 자동 생성 여부 (true = 시드 기반, false = 수동 생성) */
	UPROPERTY(SaveGame)
	bool bIsAutomaticallyGenerated = true;

	/** 월드 시드 (자동 생성 시 필수, 수동 생성 시 무시) */
	UPROPERTY(SaveGame)
	int32 WorldSeed = 0;

	/** 월드 크기 X */
	UPROPERTY(SaveGame)
	int32 WorldSizeX = 512;

	/** 월드 크기 Y */
	UPROPERTY(SaveGame)
	int32 WorldSizeY = 512;

	// ===== 계절 시스템 =====

	/** 현재 계절 */
	UPROPERTY(SaveGame)
	uint8 CurrentSeason = 0; // ESeasonType (Spring=0, Summer=1, Fall=2, Winter=3)

	/** 계절 진행도 (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float SeasonProgress = 0.0f;

	/** 총 경과 일수 */
	UPROPERTY(SaveGame)
	int32 TotalDaysElapsed = 0;

	// ===== 날씨 시스템 =====

	/** 현재 날씨 타입 */
	UPROPERTY(SaveGame)
	uint8 CurrentWeatherType = 0; // EWeatherType

	/** 이전 날씨 타입 (전환용) */
	UPROPERTY(SaveGame)
	uint8 PreviousWeatherType = 0; // EWeatherType

	/** 날씨 강도 (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherIntensity = 1.0f;

	/** 날씨 전환 진행도 (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherTransitionProgress = 1.0f;

	/** 현재 날씨 지속 시간 (초) */
	UPROPERTY(SaveGame)
	float CurrentWeatherDuration = 3600.0f;

	/** 현재 날씨 경과 시간 (초) */
	UPROPERTY(SaveGame)
	float CurrentWeatherElapsedTime = 0.0f;

	// ===== 시간 시스템 =====

	/** 현재 게임 시간 (0.0 ~ 24.0) */
	UPROPERTY(SaveGame)
	float CurrentGameHour = 8.0f;

	/** 현재 게임 일 (1부터 시작) */
	UPROPERTY(SaveGame)
	int32 CurrentGameDay = 1;

	/** 시간 속도 배율 (1.0 = 정상 속도) */
	UPROPERTY(SaveGame)
	float TimeSpeedMultiplier = 1.0f;

	/** 현재 시간대 */
	UPROPERTY(SaveGame)
	uint8 CurrentTimeOfDay = 2; // ETimeOfDay (Night=0, Dawn=1, Morning=2, Noon=3, Afternoon=4, Dusk=5)

	/** 일시 정지 여부 */
	UPROPERTY(SaveGame)
	bool bIsTimePaused = false;

	// ===== 수동 생성 월드 데이터 (bIsAutomaticallyGenerated == false 일 때만 사용) =====

	/** 수동 배치된 월드 오브젝트 (나무, 바위 등) */
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectLocations;

	/** 수동 배치된 오브젝트 타입 */
	UPROPERTY(SaveGame)
	TArray<uint8> ManualObjectTypes; // EWorldObjectType

	/** 수동 배치된 오브젝트 회전 */
	UPROPERTY(SaveGame)
	TArray<FRotator> ManualObjectRotations;

	/** 수동 배치된 오브젝트 스케일 */
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectScales;

	// ===== 동적 상태 (자동/수동 모두 저장) =====

	/** 리소스 노드 상태 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedResourceNodeState> ResourceNodeStates;

	/** POI 진행 상태 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedPOIState> POIStates;

	/** 변경된 바이옴 데이터 (원본과 다른 경우만 저장) */
	UPROPERTY(SaveGame)
	TArray<int32> ModifiedBiomeIndices;

	/** 변경된 바이옴 타입 */
	UPROPERTY(SaveGame)
	TArray<uint8> ModifiedBiomeTypes; // EBiomeType

	// ===== 레거시 호환성 (Deprecated) =====

	/** [DEPRECATED] 월드 시간 (CurrentGameHour로 대체됨) */
	UPROPERTY(SaveGame)
	float WorldTime = 0.0f;

	/** [DEPRECATED] 날씨 상태 (CurrentWeatherType으로 대체됨) */
	UPROPERTY(SaveGame)
	FString WeatherState;

	// ===== 체크포인트 시스템 =====

	/** 체크포인트 데이터 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaCheckpointData> CheckpointStates;
};

/**
 * 게임 세이브 데이터
 * 로컬 PC 및 스팀 클라우드에 저장됩니다.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UHarmoniaSaveGame();

	/** 세이브 슬롯 이름 */
	UPROPERTY(SaveGame)
	FString SaveSlotName;

	/** 세이브 파일 버전 (호환성 체크용) */
	UPROPERTY(SaveGame)
	int32 SaveVersion = 2;

	/** 마지막 저장 시간 */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTimestamp;

	/** 서버 소유주의 스팀 ID */
	UPROPERTY(SaveGame)
	FString ServerOwnerSteamID;

	/** 플레이어별 세이브 데이터 (스팀 ID를 키로 사용) */
	UPROPERTY(SaveGame)
	TMap<FString, FHarmoniaPlayerSaveData> PlayerDataMap;

	/** 월드 공유 데이터 (빌딩 등) */
	UPROPERTY(SaveGame)
	FHarmoniaWorldSaveData WorldData;

	/** 플레이 시간 (초 단위) */
	UPROPERTY(SaveGame)
	float TotalPlayTime = 0.0f;

	// Helper functions

	/** 플레이어 데이터 추가/업데이트 */
	void SetPlayerData(const FString& InSteamID, const FHarmoniaPlayerSaveData& InPlayerData);

	/** 플레이어 데이터 가져오기 */
	bool GetPlayerData(const FString& InSteamID, FHarmoniaPlayerSaveData& OutPlayerData) const;

	/** 플레이어 데이터 존재 여부 확인 */
	bool HasPlayerData(const FString& InSteamID) const;

	/** 빌딩 추가 */
	void AddBuilding(const FHarmoniaSavedBuildingInstance& Building);

	/** 모든 빌딩 제거 */
	void ClearBuildings();
};
