// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "Definitions/HarmoniaCheckpointSystemDefinitions.h"
#include "Definitions/HarmoniaDifficultySystemDefinitions.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
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

/** ?�레?�어 ?�성 ?�???�이??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedPlayerAttributes
{
	GENERATED_BODY()

	/** 체력 */
	UPROPERTY(SaveGame)
	float Health = 100.0f;

	/** 최�? 체력 */
	UPROPERTY(SaveGame)
	float MaxHealth = 100.0f;

	/** ?�태미나 */
	UPROPERTY(SaveGame)
	float Stamina = 100.0f;

	/** 최�? ?�태미나 */
	UPROPERTY(SaveGame)
	float MaxStamina = 100.0f;

	/** 공격??*/
	UPROPERTY(SaveGame)
	float AttackPower = 10.0f;

	/** 방어??*/
	UPROPERTY(SaveGame)
	float Defense = 5.0f;

	/** ?�리?�컬 ?�률 */
	UPROPERTY(SaveGame)
	float CriticalChance = 0.1f;

	/** ?�리?�컬 배수 */
	UPROPERTY(SaveGame)
	float CriticalDamage = 1.5f;

	/** ?�동 ?�도 */
	UPROPERTY(SaveGame)
	float MovementSpeed = 600.0f;

	/** 공격 ?�도 */
	UPROPERTY(SaveGame)
	float AttackSpeed = 1.0f;
};

/** 빌딩 배치 ?�이??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedBuildingInstance
{
	GENERATED_BODY()

	/** 빌딩 ?�트 ID */
	UPROPERTY(SaveGame)
	FName PartID;

	/** ?�치 */
	UPROPERTY(SaveGame)
	FVector Location = FVector::ZeroVector;

	/** ?�전 */
	UPROPERTY(SaveGame)
	FRotator Rotation = FRotator::ZeroRotator;

	/** ?��???*/
	UPROPERTY(SaveGame)
	FVector Scale = FVector::OneVector;
};

/** ?�레?�어�??�이�??�이??*/
USTRUCT(BlueprintType)
struct FHarmoniaPlayerSaveData
{
	GENERATED_BODY()

	/** ?��? ID (UniqueNetId�?문자?�로 ?�?? */
	UPROPERTY(SaveGame)
	FString SteamID;

	/** ?�레?�어 ?�름 */
	UPROPERTY(SaveGame)
	FString PlayerName;

	/** ?�레?�어 ?�치 */
	UPROPERTY(SaveGame)
	FVector PlayerLocation = FVector::ZeroVector;

	/** ?�레?�어 ?�전 */
	UPROPERTY(SaveGame)
	FRotator PlayerRotation = FRotator::ZeroRotator;

	/** ?�레?�어 ?�성 */
	UPROPERTY(SaveGame)
	FHarmoniaSavedPlayerAttributes Attributes;

	/** ?�벤?�리 ?�이??목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedInventoryItem> InventoryItems;

	/** ?�비 ?�이??*/
	UPROPERTY(SaveGame)
	FEquipmentSaveData EquipmentData;

	/** ?�탯 ?�그 (체력 버프, ?�도 증�? ?? */
	UPROPERTY(SaveGame)
	FGameplayTagContainer StatTags;

	/** 마�?�??�???�간 */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTime;

	/** 마�?막으�?공명??체크?�인??ID */
	UPROPERTY(SaveGame)
	FName LastCheckpointID;

	/** ?�험??지??목록 */
	UPROPERTY(SaveGame)
	TArray<FExploredRegion> ExploredRegions;

	/** 발견???�치 목록 */
	UPROPERTY(SaveGame)
	TArray<FMapLocationData> DiscoveredLocations;

	/** 캐릭??진행 ?�이??(경험�? ?�벨, ?�킬, ?�래???? */
	UPROPERTY(SaveGame)
	FHarmoniaProgressionSaveData ProgressionData;
};

/** 리소???�드 ?�태 ?�???�이??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedResourceNodeState
{
	GENERATED_BODY()

	/** 리소???�드 ID (?�치 기반 ?�시 ?�는 고유 ID) */
	UPROPERTY(SaveGame)
	int32 NodeID = 0;

	/** 고갈 ?�태 (0.0 = ?�전 고갈, 1.0 = ?�전 충전) */
	UPROPERTY(SaveGame)
	float DepletionAmount = 1.0f;

	/** 리스???��?중인지 */
	UPROPERTY(SaveGame)
	bool bIsRespawning = false;

	/** 리스?�까지 ?��? ?�간 (�? */
	UPROPERTY(SaveGame)
	float RemainingRespawnTime = 0.0f;
};

/** POI 진행 ?�태 ?�???�이??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedPOIState
{
	GENERATED_BODY()

	/** POI ID */
	UPROPERTY(SaveGame)
	int32 POIID = 0;

	/** ?�리???��? */
	UPROPERTY(SaveGame)
	bool bIsCompleted = false;

	/** 보물 ?�득 ?��? */
	UPROPERTY(SaveGame)
	bool bTreasureCollected = false;

	/** 방문 ?�수 */
	UPROPERTY(SaveGame)
	int32 VisitCount = 0;

	/** 마�?�?방문 ?�간 */
	UPROPERTY(SaveGame)
	FDateTime LastVisitTime;
};

/** ?�드 ?�이�??�이??*/
USTRUCT(BlueprintType)
struct FHarmoniaWorldSaveData
{
	GENERATED_BODY()

	// ===== 빌딩 ?�스??=====

	/** 배치??빌딩 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedBuildingInstance> PlacedBuildings;

	// ===== ?�드 ?�성 ?�보 =====

	/** ?�동 ?�성 ?��? (true = ?�드 기반, false = ?�동 ?�성) */
	UPROPERTY(SaveGame)
	bool bIsAutomaticallyGenerated = true;

	/** ?�드 ?�드 (?�동 ?�성 ???�수, ?�동 ?�성 ??무시) */
	UPROPERTY(SaveGame)
	int32 WorldSeed = 0;

	/** ?�드 ?�기 X */
	UPROPERTY(SaveGame)
	int32 WorldSizeX = 512;

	/** ?�드 ?�기 Y */
	UPROPERTY(SaveGame)
	int32 WorldSizeY = 512;

	// ===== 계절 ?�스??=====

	/** ?�재 계절 */
	UPROPERTY(SaveGame)
	uint8 CurrentSeason = 0; // ESeasonType (Spring=0, Summer=1, Fall=2, Winter=3)

	/** 계절 진행??(0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float SeasonProgress = 0.0f;

	/** �?경과 ?�수 */
	UPROPERTY(SaveGame)
	int32 TotalDaysElapsed = 0;

	// ===== ?�씨 ?�스??=====

	/** ?�재 ?�씨 ?�??*/
	UPROPERTY(SaveGame)
	uint8 CurrentWeatherType = 0; // EWeatherType

	/** ?�전 ?�씨 ?�??(?�환?? */
	UPROPERTY(SaveGame)
	uint8 PreviousWeatherType = 0; // EWeatherType

	/** ?�씨 강도 (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherIntensity = 1.0f;

	/** ?�씨 ?�환 진행??(0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherTransitionProgress = 1.0f;

	/** ?�재 ?�씨 지???�간 (�? */
	UPROPERTY(SaveGame)
	float CurrentWeatherDuration = 3600.0f;

	/** ?�재 ?�씨 경과 ?�간 (�? */
	UPROPERTY(SaveGame)
	float CurrentWeatherElapsedTime = 0.0f;

	// ===== ?�간 ?�스??=====

	/** ?�재 게임 ?�간 (0.0 ~ 24.0) */
	UPROPERTY(SaveGame)
	float CurrentGameHour = 8.0f;

	/** ?�재 게임 ??(1부???�작) */
	UPROPERTY(SaveGame)
	int32 CurrentGameDay = 1;

	/** ?�간 ?�도 배율 (1.0 = ?�상 ?�도) */
	UPROPERTY(SaveGame)
	float TimeSpeedMultiplier = 1.0f;

	/** ?�재 ?�간?� */
	UPROPERTY(SaveGame)
	uint8 CurrentTimeOfDay = 2; // ETimeOfDay (Night=0, Dawn=1, Morning=2, Noon=3, Afternoon=4, Dusk=5)

	/** ?�시 ?��? ?��? */
	UPROPERTY(SaveGame)
	bool bIsTimePaused = false;

	// ===== ?�동 ?�성 ?�드 ?�이??(bIsAutomaticallyGenerated == false ???�만 ?�용) =====

	/** ?�동 배치???�드 ?�브?�트 (?�무, 바위 ?? */
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectLocations;

	/** ?�동 배치???�브?�트 ?�??*/
	UPROPERTY(SaveGame)
	TArray<uint8> ManualObjectTypes; // EWorldObjectType

	/** ?�동 배치???�브?�트 ?�전 */
	UPROPERTY(SaveGame)
	TArray<FRotator> ManualObjectRotations;

	/** ?�동 배치???�브?�트 ?��???*/
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectScales;

	// ===== ?�적 ?�태 (?�동/?�동 모두 ?�?? =====

	/** 리소???�드 ?�태 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedResourceNodeState> ResourceNodeStates;

	/** POI 진행 ?�태 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedPOIState> POIStates;

	/** 변경된 바이???�이??(?�본�??�른 경우�??�?? */
	UPROPERTY(SaveGame)
	TArray<int32> ModifiedBiomeIndices;

	/** 변경된 바이???�??*/
	UPROPERTY(SaveGame)
	TArray<uint8> ModifiedBiomeTypes; // EBiomeType

	// ===== ?�거???�환??(Deprecated) =====

	/** [DEPRECATED] ?�드 ?�간 (CurrentGameHour�??�체됨) */
	UPROPERTY(SaveGame)
	float WorldTime = 0.0f;

	/** [DEPRECATED] ?�씨 ?�태 (CurrentWeatherType?�로 ?�체됨) */
	UPROPERTY(SaveGame)
	FString WeatherState;

	// ===== 체크?�인???�스??=====

	/** 체크?�인???�이??목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaCheckpointData> CheckpointStates;
};

/**
 * 게임 ?�이�??�이??
 * 로컬 PC �??��? ?�라?�드???�?�됩?�다.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UHarmoniaSaveGame();

	/** ?�이�??�롯 ?�름 */
	UPROPERTY(SaveGame)
	FString SaveSlotName;

	/** ?�이�??�일 버전 (?�환??체크?? */
	UPROPERTY(SaveGame)
	int32 SaveVersion = 2;

	/** 마�?�??�???�간 */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTimestamp;

	/** ?�버 ?�유주의 ?��? ID */
	UPROPERTY(SaveGame)
	FString ServerOwnerSteamID;

	/** ?�레?�어�??�이�??�이??(?��? ID�??�로 ?�용) */
	UPROPERTY(SaveGame)
	TMap<FString, FHarmoniaPlayerSaveData> PlayerDataMap;

	/** ?�드 공유 ?�이??(빌딩 ?? */
	UPROPERTY(SaveGame)
	FHarmoniaWorldSaveData WorldData;

	/** ?�레???�간 (�??�위) */
	UPROPERTY(SaveGame)
	float TotalPlayTime = 0.0f;

	/** ?�이???�정 */
	UPROPERTY(SaveGame)
	FHarmoniaDifficultyConfiguration DifficultyConfiguration;

	// Helper functions

	/** ?�레?�어 ?�이??추�?/?�데?�트 */
	void SetPlayerData(const FString& InSteamID, const FHarmoniaPlayerSaveData& InPlayerData);

	/** ?�레?�어 ?�이??가?�오�?*/
	bool GetPlayerData(const FString& InSteamID, FHarmoniaPlayerSaveData& OutPlayerData) const;

	/** ?�레?�어 ?�이??존재 ?��? ?�인 */
	bool HasPlayerData(const FString& InSteamID) const;

	/** 빌딩 추�? */
	void AddBuilding(const FHarmoniaSavedBuildingInstance& Building);

	/** 모든 빌딩 ?�거 */
	void ClearBuildings();
};
