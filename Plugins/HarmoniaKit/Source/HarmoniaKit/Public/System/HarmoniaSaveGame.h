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

/** ?¸ë²¤? ë¦¬ ?„ì´???€???°ì´??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedInventoryItem
{
	GENERATED_BODY()

	/** ?„ì´???•ì˜ ê²½ë¡œ */
	UPROPERTY(SaveGame)
	FSoftObjectPath ItemDefinitionPath;

	/** ?¤íƒ ê°œìˆ˜ */
	UPROPERTY(SaveGame)
	int32 StackCount = 1;

	FHarmoniaSavedInventoryItem()
		: StackCount(1)
	{
	}
};

/** ?Œë ˆ?´ì–´ ?ì„± ?€???°ì´??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedPlayerAttributes
{
	GENERATED_BODY()

	/** ì²´ë ¥ */
	UPROPERTY(SaveGame)
	float Health = 100.0f;

	/** ìµœë? ì²´ë ¥ */
	UPROPERTY(SaveGame)
	float MaxHealth = 100.0f;

	/** ?¤íƒœë¯¸ë‚˜ */
	UPROPERTY(SaveGame)
	float Stamina = 100.0f;

	/** ìµœë? ?¤íƒœë¯¸ë‚˜ */
	UPROPERTY(SaveGame)
	float MaxStamina = 100.0f;

	/** ê³µê²©??*/
	UPROPERTY(SaveGame)
	float AttackPower = 10.0f;

	/** ë°©ì–´??*/
	UPROPERTY(SaveGame)
	float Defense = 5.0f;

	/** ?¬ë¦¬?°ì»¬ ?•ë¥  */
	UPROPERTY(SaveGame)
	float CriticalChance = 0.1f;

	/** ?¬ë¦¬?°ì»¬ ë°°ìˆ˜ */
	UPROPERTY(SaveGame)
	float CriticalDamage = 1.5f;

	/** ?´ë™ ?ë„ */
	UPROPERTY(SaveGame)
	float MovementSpeed = 600.0f;

	/** ê³µê²© ?ë„ */
	UPROPERTY(SaveGame)
	float AttackSpeed = 1.0f;
};

/** ë¹Œë”© ë°°ì¹˜ ?°ì´??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedBuildingInstance
{
	GENERATED_BODY()

	/** ë¹Œë”© ?ŒíŠ¸ ID */
	UPROPERTY(SaveGame)
	FName PartID;

	/** ?„ì¹˜ */
	UPROPERTY(SaveGame)
	FVector Location = FVector::ZeroVector;

	/** ?Œì „ */
	UPROPERTY(SaveGame)
	FRotator Rotation = FRotator::ZeroRotator;

	/** ?¤ì???*/
	UPROPERTY(SaveGame)
	FVector Scale = FVector::OneVector;
};

/** ?Œë ˆ?´ì–´ë³??¸ì´ë¸??°ì´??*/
USTRUCT(BlueprintType)
struct FHarmoniaPlayerSaveData
{
	GENERATED_BODY()

	/** ?¤í? ID (UniqueNetIdë¥?ë¬¸ì?´ë¡œ ?€?? */
	UPROPERTY(SaveGame)
	FString SteamID;

	/** ?Œë ˆ?´ì–´ ?´ë¦„ */
	UPROPERTY(SaveGame)
	FString PlayerName;

	/** ?Œë ˆ?´ì–´ ?„ì¹˜ */
	UPROPERTY(SaveGame)
	FVector PlayerLocation = FVector::ZeroVector;

	/** ?Œë ˆ?´ì–´ ?Œì „ */
	UPROPERTY(SaveGame)
	FRotator PlayerRotation = FRotator::ZeroRotator;

	/** ?Œë ˆ?´ì–´ ?ì„± */
	UPROPERTY(SaveGame)
	FHarmoniaSavedPlayerAttributes Attributes;

	/** ?¸ë²¤? ë¦¬ ?„ì´??ëª©ë¡ */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedInventoryItem> InventoryItems;

	/** ?¥ë¹„ ?°ì´??*/
	UPROPERTY(SaveGame)
	FEquipmentSaveData EquipmentData;

	/** ?¤íƒ¯ ?œê·¸ (ì²´ë ¥ ë²„í”„, ?ë„ ì¦ê? ?? */
	UPROPERTY(SaveGame)
	FGameplayTagContainer StatTags;

	/** ë§ˆì?ë§??€???œê°„ */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTime;

	/** ë§ˆì?ë§‰ìœ¼ë¡?ê³µëª…??ì²´í¬?¬ì¸??ID */
	UPROPERTY(SaveGame)
	FName LastCheckpointID;

	/** ?í—˜??ì§€??ëª©ë¡ */
	UPROPERTY(SaveGame)
	TArray<FExploredRegion> ExploredRegions;

	/** ë°œê²¬???„ì¹˜ ëª©ë¡ */
	UPROPERTY(SaveGame)
	TArray<FMapLocationData> DiscoveredLocations;

	/** ìºë¦­??ì§„í–‰ ?°ì´??(ê²½í—˜ì¹? ?ˆë²¨, ?¤í‚¬, ?´ë˜???? */
	UPROPERTY(SaveGame)
	FHarmoniaProgressionSaveData ProgressionData;
};

/** ë¦¬ì†Œ???¸ë“œ ?íƒœ ?€???°ì´??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedResourceNodeState
{
	GENERATED_BODY()

	/** ë¦¬ì†Œ???¸ë“œ ID (?„ì¹˜ ê¸°ë°˜ ?´ì‹œ ?ëŠ” ê³ ìœ  ID) */
	UPROPERTY(SaveGame)
	int32 NodeID = 0;

	/** ê³ ê°ˆ ?íƒœ (0.0 = ?„ì „ ê³ ê°ˆ, 1.0 = ?„ì „ ì¶©ì „) */
	UPROPERTY(SaveGame)
	float DepletionAmount = 1.0f;

	/** ë¦¬ìŠ¤???€ê¸?ì¤‘ì¸ì§€ */
	UPROPERTY(SaveGame)
	bool bIsRespawning = false;

	/** ë¦¬ìŠ¤?°ê¹Œì§€ ?¨ì? ?œê°„ (ì´? */
	UPROPERTY(SaveGame)
	float RemainingRespawnTime = 0.0f;
};

/** POI ì§„í–‰ ?íƒœ ?€???°ì´??*/
USTRUCT(BlueprintType)
struct FHarmoniaSavedPOIState
{
	GENERATED_BODY()

	/** POI ID */
	UPROPERTY(SaveGame)
	int32 POIID = 0;

	/** ?´ë¦¬???¬ë? */
	UPROPERTY(SaveGame)
	bool bIsCompleted = false;

	/** ë³´ë¬¼ ?ë“ ?¬ë? */
	UPROPERTY(SaveGame)
	bool bTreasureCollected = false;

	/** ë°©ë¬¸ ?Ÿìˆ˜ */
	UPROPERTY(SaveGame)
	int32 VisitCount = 0;

	/** ë§ˆì?ë§?ë°©ë¬¸ ?œê°„ */
	UPROPERTY(SaveGame)
	FDateTime LastVisitTime;
};

/** ?”ë“œ ?¸ì´ë¸??°ì´??*/
USTRUCT(BlueprintType)
struct FHarmoniaWorldSaveData
{
	GENERATED_BODY()

	// ===== ë¹Œë”© ?œìŠ¤??=====

	/** ë°°ì¹˜??ë¹Œë”© ëª©ë¡ */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedBuildingInstance> PlacedBuildings;

	// ===== ?”ë“œ ?ì„± ?•ë³´ =====

	/** ?ë™ ?ì„± ?¬ë? (true = ?œë“œ ê¸°ë°˜, false = ?˜ë™ ?ì„±) */
	UPROPERTY(SaveGame)
	bool bIsAutomaticallyGenerated = true;

	/** ?”ë“œ ?œë“œ (?ë™ ?ì„± ???„ìˆ˜, ?˜ë™ ?ì„± ??ë¬´ì‹œ) */
	UPROPERTY(SaveGame)
	int32 WorldSeed = 0;

	/** ?”ë“œ ?¬ê¸° X */
	UPROPERTY(SaveGame)
	int32 WorldSizeX = 512;

	/** ?”ë“œ ?¬ê¸° Y */
	UPROPERTY(SaveGame)
	int32 WorldSizeY = 512;

	// ===== ê³„ì ˆ ?œìŠ¤??=====

	/** ?„ì¬ ê³„ì ˆ */
	UPROPERTY(SaveGame)
	uint8 CurrentSeason = 0; // ESeasonType (Spring=0, Summer=1, Fall=2, Winter=3)

	/** ê³„ì ˆ ì§„í–‰??(0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float SeasonProgress = 0.0f;

	/** ì´?ê²½ê³¼ ?¼ìˆ˜ */
	UPROPERTY(SaveGame)
	int32 TotalDaysElapsed = 0;

	// ===== ? ì”¨ ?œìŠ¤??=====

	/** ?„ì¬ ? ì”¨ ?€??*/
	UPROPERTY(SaveGame)
	uint8 CurrentWeatherType = 0; // EWeatherType

	/** ?´ì „ ? ì”¨ ?€??(?„í™˜?? */
	UPROPERTY(SaveGame)
	uint8 PreviousWeatherType = 0; // EWeatherType

	/** ? ì”¨ ê°•ë„ (0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherIntensity = 1.0f;

	/** ? ì”¨ ?„í™˜ ì§„í–‰??(0.0 ~ 1.0) */
	UPROPERTY(SaveGame)
	float WeatherTransitionProgress = 1.0f;

	/** ?„ì¬ ? ì”¨ ì§€???œê°„ (ì´? */
	UPROPERTY(SaveGame)
	float CurrentWeatherDuration = 3600.0f;

	/** ?„ì¬ ? ì”¨ ê²½ê³¼ ?œê°„ (ì´? */
	UPROPERTY(SaveGame)
	float CurrentWeatherElapsedTime = 0.0f;

	// ===== ?œê°„ ?œìŠ¤??=====

	/** ?„ì¬ ê²Œì„ ?œê°„ (0.0 ~ 24.0) */
	UPROPERTY(SaveGame)
	float CurrentGameHour = 8.0f;

	/** ?„ì¬ ê²Œì„ ??(1ë¶€???œì‘) */
	UPROPERTY(SaveGame)
	int32 CurrentGameDay = 1;

	/** ?œê°„ ?ë„ ë°°ìœ¨ (1.0 = ?•ìƒ ?ë„) */
	UPROPERTY(SaveGame)
	float TimeSpeedMultiplier = 1.0f;

	/** ?„ì¬ ?œê°„?€ */
	UPROPERTY(SaveGame)
	uint8 CurrentTimeOfDay = 2; // ETimeOfDay (Night=0, Dawn=1, Morning=2, Noon=3, Afternoon=4, Dusk=5)

	/** ?¼ì‹œ ?•ì? ?¬ë? */
	UPROPERTY(SaveGame)
	bool bIsTimePaused = false;

	// ===== ?˜ë™ ?ì„± ?”ë“œ ?°ì´??(bIsAutomaticallyGenerated == false ???Œë§Œ ?¬ìš©) =====

	/** ?˜ë™ ë°°ì¹˜???”ë“œ ?¤ë¸Œ?íŠ¸ (?˜ë¬´, ë°”ìœ„ ?? */
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectLocations;

	/** ?˜ë™ ë°°ì¹˜???¤ë¸Œ?íŠ¸ ?€??*/
	UPROPERTY(SaveGame)
	TArray<uint8> ManualObjectTypes; // EWorldObjectType

	/** ?˜ë™ ë°°ì¹˜???¤ë¸Œ?íŠ¸ ?Œì „ */
	UPROPERTY(SaveGame)
	TArray<FRotator> ManualObjectRotations;

	/** ?˜ë™ ë°°ì¹˜???¤ë¸Œ?íŠ¸ ?¤ì???*/
	UPROPERTY(SaveGame)
	TArray<FVector> ManualObjectScales;

	// ===== ?™ì  ?íƒœ (?ë™/?˜ë™ ëª¨ë‘ ?€?? =====

	/** ë¦¬ì†Œ???¸ë“œ ?íƒœ ëª©ë¡ */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedResourceNodeState> ResourceNodeStates;

	/** POI ì§„í–‰ ?íƒœ ëª©ë¡ */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSavedPOIState> POIStates;

	/** ë³€ê²½ëœ ë°”ì´???°ì´??(?ë³¸ê³??¤ë¥¸ ê²½ìš°ë§??€?? */
	UPROPERTY(SaveGame)
	TArray<int32> ModifiedBiomeIndices;

	/** ë³€ê²½ëœ ë°”ì´???€??*/
	UPROPERTY(SaveGame)
	TArray<uint8> ModifiedBiomeTypes; // EBiomeType

	// ===== ?ˆê±°???¸í™˜??(Deprecated) =====

	/** [DEPRECATED] ?”ë“œ ?œê°„ (CurrentGameHourë¡??€ì²´ë¨) */
	UPROPERTY(SaveGame)
	float WorldTime = 0.0f;

	/** [DEPRECATED] ? ì”¨ ?íƒœ (CurrentWeatherType?¼ë¡œ ?€ì²´ë¨) */
	UPROPERTY(SaveGame)
	FString WeatherState;

	// ===== ì²´í¬?¬ì¸???œìŠ¤??=====

	/** ì²´í¬?¬ì¸???°ì´??ëª©ë¡ */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaCheckpointData> CheckpointStates;
};

/**
 * ê²Œì„ ?¸ì´ë¸??°ì´??
 * ë¡œì»¬ PC ë°??¤í? ?´ë¼?°ë“œ???€?¥ë©?ˆë‹¤.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UHarmoniaSaveGame();

	/** ?¸ì´ë¸??¬ë¡¯ ?´ë¦„ */
	UPROPERTY(SaveGame)
	FString SaveSlotName;

	/** ?¸ì´ë¸??Œì¼ ë²„ì „ (?¸í™˜??ì²´í¬?? */
	UPROPERTY(SaveGame)
	int32 SaveVersion = 2;

	/** ë§ˆì?ë§??€???œê°„ */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTimestamp;

	/** ?œë²„ ?Œìœ ì£¼ì˜ ?¤í? ID */
	UPROPERTY(SaveGame)
	FString ServerOwnerSteamID;

	/** ?Œë ˆ?´ì–´ë³??¸ì´ë¸??°ì´??(?¤í? IDë¥??¤ë¡œ ?¬ìš©) */
	UPROPERTY(SaveGame)
	TMap<FString, FHarmoniaPlayerSaveData> PlayerDataMap;

	/** ?”ë“œ ê³µìœ  ?°ì´??(ë¹Œë”© ?? */
	UPROPERTY(SaveGame)
	FHarmoniaWorldSaveData WorldData;

	/** ?Œë ˆ???œê°„ (ì´??¨ìœ„) */
	UPROPERTY(SaveGame)
	float TotalPlayTime = 0.0f;

	/** ?œì´???¤ì • */
	UPROPERTY(SaveGame)
	FHarmoniaDifficultyConfiguration DifficultyConfiguration;

	// Helper functions

	/** ?Œë ˆ?´ì–´ ?°ì´??ì¶”ê?/?…ë°?´íŠ¸ */
	void SetPlayerData(const FString& InSteamID, const FHarmoniaPlayerSaveData& InPlayerData);

	/** ?Œë ˆ?´ì–´ ?°ì´??ê°€?¸ì˜¤ê¸?*/
	bool GetPlayerData(const FString& InSteamID, FHarmoniaPlayerSaveData& OutPlayerData) const;

	/** ?Œë ˆ?´ì–´ ?°ì´??ì¡´ì¬ ?¬ë? ?•ì¸ */
	bool HasPlayerData(const FString& InSteamID) const;

	/** ë¹Œë”© ì¶”ê? */
	void AddBuilding(const FHarmoniaSavedBuildingInstance& Building);

	/** ëª¨ë“  ë¹Œë”© ?œê±° */
	void ClearBuildings();
};
