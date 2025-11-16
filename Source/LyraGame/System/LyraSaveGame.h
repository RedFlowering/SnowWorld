// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "LyraSaveGame.generated.h"

class ULyraInventoryItemInstance;
class ULyraInventoryItemDefinition;

/** 인벤토리 아이템 저장 데이터 */
USTRUCT(BlueprintType)
struct FLyraSavedInventoryItem
{
	GENERATED_BODY()

	/** 아이템 정의 경로 */
	UPROPERTY(SaveGame)
	FSoftObjectPath ItemDefinitionPath;

	/** 스택 개수 */
	UPROPERTY(SaveGame)
	int32 StackCount = 1;

	FLyraSavedInventoryItem()
		: StackCount(1)
	{
	}
};

/** 플레이어 속성 저장 데이터 */
USTRUCT(BlueprintType)
struct FLyraSavedPlayerAttributes
{
	GENERATED_BODY()

	/** 체력 */
	UPROPERTY(SaveGame)
	float Health = 100.0f;

	/** 최대 체력 */
	UPROPERTY(SaveGame)
	float MaxHealth = 100.0f;

	/** 스태미나 (HarmoniaAttributeSet 사용 시) */
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
struct FLyraSavedBuildingInstance
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
struct FLyraPlayerSaveData
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
	FLyraSavedPlayerAttributes Attributes;

	/** 인벤토리 아이템 목록 */
	UPROPERTY(SaveGame)
	TArray<FLyraSavedInventoryItem> InventoryItems;

	/** 스탯 태그 (체력 버프, 속도 증가 등) */
	UPROPERTY(SaveGame)
	FGameplayTagContainer StatTags;

	/** 마지막 저장 시간 */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTime;
};

/** 월드 세이브 데이터 */
USTRUCT(BlueprintType)
struct FLyraWorldSaveData
{
	GENERATED_BODY()

	/** 배치된 빌딩 목록 */
	UPROPERTY(SaveGame)
	TArray<FLyraSavedBuildingInstance> PlacedBuildings;

	/** 월드 시간 (게임 내 시간) */
	UPROPERTY(SaveGame)
	float WorldTime = 0.0f;

	/** 날씨 상태 */
	UPROPERTY(SaveGame)
	FString WeatherState;
};

/**
 * 게임 세이브 데이터
 * 로컬 PC 및 스팀 클라우드에 저장됩니다.
 */
UCLASS()
class LYRAGAME_API ULyraSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	ULyraSaveGame();

	/** 세이브 슬롯 이름 */
	UPROPERTY(SaveGame)
	FString SaveSlotName;

	/** 세이브 파일 버전 (호환성 체크용) */
	UPROPERTY(SaveGame)
	int32 SaveVersion = 1;

	/** 마지막 저장 시간 */
	UPROPERTY(SaveGame)
	FDateTime LastSaveTimestamp;

	/** 서버 소유주의 스팀 ID */
	UPROPERTY(SaveGame)
	FString ServerOwnerSteamID;

	/** 플레이어별 세이브 데이터 (스팀 ID를 키로 사용) */
	UPROPERTY(SaveGame)
	TMap<FString, FLyraPlayerSaveData> PlayerDataMap;

	/** 월드 공유 데이터 (빌딩 등) */
	UPROPERTY(SaveGame)
	FLyraWorldSaveData WorldData;

	/** 플레이 시간 (초 단위) */
	UPROPERTY(SaveGame)
	float TotalPlayTime = 0.0f;

	// Helper functions

	/** 플레이어 데이터 추가/업데이트 */
	void SetPlayerData(const FString& InSteamID, const FLyraPlayerSaveData& InPlayerData);

	/** 플레이어 데이터 가져오기 */
	bool GetPlayerData(const FString& InSteamID, FLyraPlayerSaveData& OutPlayerData) const;

	/** 플레이어 데이터 존재 여부 확인 */
	bool HasPlayerData(const FString& InSteamID) const;

	/** 빌딩 추가 */
	void AddBuilding(const FLyraSavedBuildingInstance& Building);

	/** 모든 빌딩 제거 */
	void ClearBuildings();
};
