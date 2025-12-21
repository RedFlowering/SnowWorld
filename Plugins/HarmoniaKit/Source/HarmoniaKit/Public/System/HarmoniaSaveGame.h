// Copyright 2025 Snow Game Studio.

#pragma once

/**
 * @file HarmoniaSaveGame.h
 * @brief Save game system for persistent player and world data
 * @author Harmonia Team
 * 
 * Provides comprehensive save data structures for player attributes,
 * inventory, equipment, progression, and world state.
 */

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

/**
 * @struct FHarmoniaSavedInventoryItem
 * @brief Inventory item save data
 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedInventoryItem
{
	GENERATED_BODY()

	/** Item definition path */
	UPROPERTY(SaveGame)
	FSoftObjectPath ItemDefinitionPath;

	/** Stack count */
	UPROPERTY(SaveGame)
	int32 StackCount = 1;

	FHarmoniaSavedInventoryItem()
		: StackCount(1)
	{
	}
};

/**
 * @struct FHarmoniaSavedPlayerAttributes
 * @brief Player attribute save data
 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedPlayerAttributes
{
	GENERATED_BODY()

	/** Current health */
	UPROPERTY(SaveGame)
	float Health = 100.0f;

	/** Maximum health */
	UPROPERTY(SaveGame)
	float MaxHealth = 100.0f;

	/** Current stamina */
	UPROPERTY(SaveGame)
	float Stamina = 100.0f;

	/** Maximum stamina */
	UPROPERTY(SaveGame)
	float MaxStamina = 100.0f;

	/** Attack power */
	UPROPERTY(SaveGame)
	float AttackPower = 10.0f;

	/** Defense */
	UPROPERTY(SaveGame)
	float Defense = 5.0f;

	/** Critical hit chance */
	UPROPERTY(SaveGame)
	float CriticalChance = 0.1f;

	/** Critical damage multiplier */
	UPROPERTY(SaveGame)
	float CriticalDamage = 1.5f;

	/** Movement speed */
	UPROPERTY(SaveGame)
	float MovementSpeed = 600.0f;

	/** Attack speed */
	UPROPERTY(SaveGame)
	float AttackSpeed = 1.0f;
};

/**
 * @struct FHarmoniaSavedBuildingInstance
 * @brief Building placement save data
 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedBuildingInstance
{
	GENERATED_BODY()

	/** Building part ID */
	UPROPERTY(SaveGame)
	FName PartID;

	/** Location */
	UPROPERTY(SaveGame)
	FVector Location = FVector::ZeroVector;

	/** Rotation */
	UPROPERTY(SaveGame)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Scale */
	UPROPERTY(SaveGame)
	FVector Scale = FVector::OneVector;
};

/**
 * @struct FHarmoniaPlayerSaveData
 * @brief Player data save structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaPlayerSaveData
{
	GENERATED_BODY()

	/** Steam ID (UniqueNetId as string) */
	UPROPERTY(SaveGame)
	FString SteamID;

	/** Player name */
	UPROPERTY(SaveGame)
	FString PlayerName;

	/** Player location */
	UPROPERTY(SaveGame)
	FVector PlayerLocation = FVector::ZeroVector;

	/** Player rotation */
	UPROPERTY(SaveGame)
	FRotator PlayerRotation = FRotator::ZeroRotator;

	/** Player attributes */
	UPROPERTY(SaveGame)
	FHarmoniaSavedPlayerAttributes Attributes;

	/** Inventory item list */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedInventoryItem> InventoryItems;

	/** Equipment data */
	UPROPERTY(SaveGame)
	FEquipmentSaveData EquipmentData;

	/** Stat tags (health buff, speed boost, etc.) */
	UPROPERTY(SaveGame)
	FGameplayTagContainer StatTags;

	/** Last save time */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTime;

	/** Last resonated checkpoint ID */
	UPROPERTY(SaveGame)
	FName LastCheckpointID;

	/** Explored region list */
	UPROPERTY(SaveGame)
	TArray<FExploredRegion> ExploredRegions;

	/** Discovered location list */
	UPROPERTY(SaveGame)
	TArray<FMapLocationData> DiscoveredLocations;

	/** Character progression data (experience, level, skills, class, etc.) */
	UPROPERTY(SaveGame)
	FHarmoniaProgressionSaveData ProgressionData;
};

/**
 * @struct FHarmoniaSavedResourceNodeState
 * @brief Resource node state save data
 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedResourceNodeState
{
	GENERATED_BODY()

	/** Resource node ID (location-based hash or unique ID) */
	UPROPERTY(SaveGame)
	int32 NodeID = 0;

	/** Depletion state (0.0 = fully depleted, 1.0 = fully charged) */
	UPROPERTY(SaveGame)
	float DepletionAmount = 1.0f;

	/** Whether respawn is in progress */
	UPROPERTY(SaveGame)
	bool bIsRespawning = false;

	/** Remaining time until respawn (seconds) */
	UPROPERTY(SaveGame)
	float RemainingRespawnTime = 0.0f;
};

/**
 * @enum EBossRespawnPolicy
 * @brief 보스 클리어 후 리스폰 정책
 */
UENUM(BlueprintType)
enum class EBossRespawnPolicy : uint8
{
	/** 클리어 후 리스폰 안함 */
	Never UMETA(DisplayName = "Never Respawn"),
	
	/** 일정 시간 후 리스폰 */
	AfterTime UMETA(DisplayName = "Respawn After Time"),
	
	/** 영역 재진입 시 리스폰 */
	OnAreaReload UMETA(DisplayName = "Respawn On Area Reload"),
	
	/** 항상 리스폰 (테스트용) */
	Always UMETA(DisplayName = "Always Respawn")
};

/**
 * @struct FHarmoniaSavedBossState
 * @brief 보스 진행 상태 세이브 데이터
 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedBossState
{
	GENERATED_BODY()

	/** 보스 고유 식별자 */
	UPROPERTY(SaveGame)
	FName BossID;

	/** 인트로 시퀀스 시청 완료 여부 */
	UPROPERTY(SaveGame)
	bool bHasViewedIntro = false;

	/** 보스 처치 완료 여부 */
	UPROPERTY(SaveGame)
	bool bHasBeenDefeated = false;

	/** 마지막 처치 시각 (리스폰 시간 계산용) */
	UPROPERTY(SaveGame)
	FDateTime DefeatTime;

	/** 총 처치 횟수 */
	UPROPERTY(SaveGame)
	int32 DefeatCount = 0;

	FHarmoniaSavedBossState()
		: bHasViewedIntro(false)
		, bHasBeenDefeated(false)
		, DefeatCount(0)
	{
	}
};

/**
 * @struct FHarmoniaSavedPOIState
 * @brief POI (Point of Interest) progress state save data
 */
USTRUCT(BlueprintType)
struct FHarmoniaSavedPOIState
{
	GENERATED_BODY()

	/** POI ID */
	UPROPERTY(SaveGame)
	int32 POIID = 0;

	/** Clear status */
	UPROPERTY(SaveGame)
	bool bIsCompleted = false;

	/** Treasure collected status */
	UPROPERTY(SaveGame)
	bool bTreasureCollected = false;

	/** Visit count */
	UPROPERTY(SaveGame)
	int32 VisitCount = 0;

	/** Last visit time */
	UPROPERTY(SaveGame)
	FDateTime LastVisitTime;
};

/**
 * @struct FHarmoniaWorldSaveData
 * @brief World data save structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaWorldSaveData
{
	GENERATED_BODY()

	// ===== Building System =====

	/** Placed buildings list */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedBuildingInstance> PlacedBuildings;

	// ===== World Generation Info =====

	/** Auto-generation enabled (true = seed-based, false = manual generation) */
	UPROPERTY(SaveGame)
	bool bIsAutomaticallyGenerated = true;

	/** World seed (required for auto-generation, ignored for manual) */
	UPROPERTY(SaveGame)
	int32 WorldSeed = 0;

	/** World size X */
	UPROPERTY(SaveGame)
	int32 WorldSizeX = 512;

	/** World size Y */
	UPROPERTY(SaveGame)
	int32 WorldSizeY = 512;

	// ===== Season System =====

	/** Current season */
	UPROPERTY(SaveGame)
	uint8 CurrentSeason = 0; // ESeasonType (Spring=0, Summer=1, Fall=2, Winter=3)

	/** Season progress (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float SeasonProgress = 0.0f;

	/** Total days elapsed */
	UPROPERTY(SaveGame)
	int32 TotalDaysElapsed = 0;

	// ===== Weather System =====

	/** Current weather type */
	UPROPERTY(SaveGame)
	uint8 CurrentWeatherType = 0; // EWeatherType

	/** Previous weather type (for transitions) */
	UPROPERTY(SaveGame)
	uint8 PreviousWeatherType = 0; // EWeatherType

	/** Weather intensity (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherIntensity = 1.0f;

	/** Weather transition progress (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherTransitionProgress = 1.0f;

	/** Current weather duration (seconds) */
	UPROPERTY(SaveGame)
	float CurrentWeatherDuration = 3600.0f;

	/** Current weather elapsed time (seconds) */
	UPROPERTY(SaveGame)
	float CurrentWeatherElapsedTime = 0.0f;

	// ===== Time System =====

	/** Current game hour (0.0 ~ 24.0) */
	UPROPERTY(SaveGame)
	float CurrentGameHour = 8.0f;

	/** Current game day (starts from 1) */
	UPROPERTY(SaveGame)
	int32 CurrentGameDay = 1;

	/** Time speed multiplier (1.0 = normal speed) */
	UPROPERTY(SaveGame)
	float TimeSpeedMultiplier = 1.0f;

	/** Current time of day */
	UPROPERTY(SaveGame)
	uint8 CurrentTimeOfDay = 2; // ETimeOfDay (Night=0, Dawn=1, Morning=2, Noon=3, Afternoon=4, Dusk=5)

	/** Time pause status */
	UPROPERTY(SaveGame)
	bool bIsTimePaused = false;

	// ===== Manual Generation World Data (only used when bIsAutomaticallyGenerated == false) =====

	/** Manually placed world object locations (trees, rocks, etc.) */
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectLocations;

	/** Manually placed object types */
	UPROPERTY(SaveGame)
	TArray<uint8> ManualObjectTypes; // EWorldObjectType

	/** Manually placed object rotations */
	UPROPERTY(SaveGame)
	TArray<FRotator> ManualObjectRotations;

	/** Manually placed object scales */
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectScales;

	// ===== Dynamic State (used in both auto/manual modes) =====

	/** Resource node state list */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedResourceNodeState> ResourceNodeStates;

	/** POI progress state list */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedPOIState> POIStates;

	/** Modified biome indices (only saved when different from original) */
	UPROPERTY(SaveGame)
	TArray<int32> ModifiedBiomeIndices;

	/** Modified biome types */
	UPROPERTY(SaveGame)
	TArray<uint8> ModifiedBiomeTypes; // EBiomeType

	// ===== Deprecated (for backward compatibility) =====

	/** [DEPRECATED] World time (replaced by CurrentGameHour) */
	UPROPERTY(SaveGame)
	float WorldTime = 0.0f;

	/** [DEPRECATED] Weather state (replaced by CurrentWeatherType) */
	UPROPERTY(SaveGame)
	FString WeatherState;

	// ===== Checkpoint System =====

	/** Checkpoint data list */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaCheckpointData> CheckpointStates;

	// ===== Boss System =====

	/** 보스 진행 상태 목록 */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedBossState> BossStates;
};

/**
 * @class UHarmoniaSaveGame
 * @brief Game save data class
 * 
 * Saved to local PC or cloud storage.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UHarmoniaSaveGame();

	/** Save slot name */
	UPROPERTY(SaveGame)
	FString SaveSlotName;

	/** Save file version (for compatibility check) */
	UPROPERTY(SaveGame)
	int32 SaveVersion = 2;

	/** Last save timestamp */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTimestamp;

	/** Server owner Steam ID */
	UPROPERTY(SaveGame)
	FString ServerOwnerSteamID;

	/** Player save data (keyed by Steam ID) */
	UPROPERTY(SaveGame)
	TMap<FString, FHarmoniaPlayerSaveData> PlayerDataMap;

	/** World shared data (buildings, etc.) */
	UPROPERTY(SaveGame)
	FHarmoniaWorldSaveData WorldData;

	/** Total play time (in seconds) */
	UPROPERTY(SaveGame)
	float TotalPlayTime = 0.0f;

	/** Difficulty configuration */
	UPROPERTY(SaveGame)
	FHarmoniaDifficultyConfiguration DifficultyConfiguration;

	// Helper functions

	/**
	 * @brief Adds or updates player data
	 * @param InSteamID Player Steam ID
	 * @param InPlayerData Player data to save
	 */
	void SetPlayerData(const FString& InSteamID, const FHarmoniaPlayerSaveData& InPlayerData);

	/**
	 * @brief Gets player data
	 * @param InSteamID Player Steam ID
	 * @param OutPlayerData Output player data
	 * @return True if player data exists
	 */
	bool GetPlayerData(const FString& InSteamID, FHarmoniaPlayerSaveData& OutPlayerData) const;

	/**
	 * @brief Checks if player data exists
	 * @param InSteamID Player Steam ID
	 * @return True if player data exists
	 */
	bool HasPlayerData(const FString& InSteamID) const;

	/**
	 * @brief Adds a building
	 * @param Building Building instance to add
	 */
	void AddBuilding(const FHarmoniaSavedBuildingInstance& Building);

	/**
	 * @brief Clears all buildings
	 */
	void ClearBuildings();
};
