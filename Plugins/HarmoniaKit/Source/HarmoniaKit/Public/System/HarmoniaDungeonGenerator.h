// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonGenerator.generated.h"

class UDungeonDataAsset;
class AHarmoniaDungeonInstance;

/**
 * ?�전 �??�??
 */
UENUM(BlueprintType)
enum class EDungeonRoomType : uint8
{
	Start UMETA(DisplayName = "Start Room"),
	Normal UMETA(DisplayName = "Normal Room"),
	Combat UMETA(DisplayName = "Combat Room"),
	Treasure UMETA(DisplayName = "Treasure Room"),
	Boss UMETA(DisplayName = "Boss Room"),
	Exit UMETA(DisplayName = "Exit Room"),
	Secret UMETA(DisplayName = "Secret Room")
};

/**
 * ?�전 �??�보
 */
USTRUCT(BlueprintType)
struct FDungeonRoom
{
	GENERATED_BODY()

	/** �??�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	EDungeonRoomType RoomType = EDungeonRoomType::Normal;

	/** �??�치 (그리??좌표) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridPosition;

	/** ?�제 ?�드 ?�치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector WorldPosition;

	/** �??�기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector RoomSize = FVector(2000.0f, 2000.0f, 400.0f);

	/** ?�결??방들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FIntPoint> ConnectedRooms;

	/** �?방향 (�? ?? ?? ?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> DoorDirections;

	/** ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Difficulty = 1;

	/** ?�폰??몬스????*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 MonsterCount = 0;
};

/**
 * ?�전 복도 ?�보
 */
USTRUCT(BlueprintType)
struct FDungeonCorridor
{
	GENERATED_BODY()

	/** ?�작 �?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint StartRoom;

	/** ??�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint EndRoom;

	/** 경로 ?�인??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> PathPoints;

	/** 복도 ?�비 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float Width = 400.0f;
};

/**
 * ?�전 ?�이?�웃 ?�이??
 */
USTRUCT(BlueprintType)
struct FDungeonLayout
{
	GENERATED_BODY()

	/** ?�전 ?�드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Seed = 0;

	/** �?목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonRoom> Rooms;

	/** 복도 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonCorridor> Corridors;

	/** ?�전 ?�체 ?�기 (그리?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridSize = FIntPoint(10, 10);

	/** ?�???�기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float TileSize = 2000.0f;
};

/**
 * ?�전 ?�성 ?�정
 */
USTRUCT(BlueprintType)
struct FDungeonGenerationSettings
{
	GENERATED_BODY()

	/** ?�전 ?�기 (그리?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint DungeonSize = FIntPoint(20, 20);

	/** 최소 �?개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MinRoomCount = 5;

	/** 최�? �?개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MaxRoomCount = 15;

	/** 최소 �??�기 (그리???�위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MinRoomSize = FIntPoint(3, 3);

	/** 최�? �??�기 (그리???�위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MaxRoomSize = FIntPoint(7, 7);

	/** 보스�??�률 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float BossRoomChance = 1.0f;

	/** 보물�??�률 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float TreasureRoomChance = 0.3f;

	/** 비�?�??�률 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float SecretRoomChance = 0.1f;

	/** 복도 구불구불??(0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float CorridorWindiness = 0.5f;

	/** 루프 ?�용 (?�환 경로) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bAllowLoops = true;
};

/**
 * ?�전 ?�성�?
 * ?�로?��????�전???�성?�는 ?�틸리티 ?�래??
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDungeonGenerator : public UObject
{
	GENERATED_BODY()

public:
	/** ?�전 ?�이?�웃 ?�성 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateDungeonLayout(const FDungeonGenerationSettings& Settings, int32 Seed = 0);

	/** 무한 ?�전 ?�이?�웃 ?�성 (층별) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateInfiniteDungeonFloor(int32 FloorNumber, const FInfiniteDungeonFloor& FloorInfo);

	/** ?�전 ?�이?�웃???�드???�폰 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator", meta = (WorldContext = "WorldContextObject"))
	AHarmoniaDungeonInstance* SpawnDungeonFromLayout(UObject* WorldContextObject, const FDungeonLayout& Layout, const UDungeonDataAsset* DungeonData);

	/** ?�전 검�?(?�결?? ?�달 가?�성 ?? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	bool ValidateDungeonLayout(const FDungeonLayout& Layout);

protected:
	/** �??�성 */
	TArray<FDungeonRoom> GenerateRooms(const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** 복도 ?�성 */
	TArray<FDungeonCorridor> GenerateCorridors(const TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** �??�???�당 */
	void AssignRoomTypes(TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** �?겹침 체크 */
	bool IsRoomOverlapping(const FDungeonRoom& Room, const TArray<FDungeonRoom>& ExistingRooms) const;

	/** 최단 경로 찾기 (A*) */
	TArray<FIntPoint> FindPath(FIntPoint Start, FIntPoint End, const FDungeonGenerationSettings& Settings) const;

	/** �??�결??체크 */
	bool AreAllRoomsConnected(const TArray<FDungeonRoom>& Rooms) const;

	/** ?�이??계산 (?�작?�으로�??�의 거리 기반) */
	void CalculateRoomDifficulty(TArray<FDungeonRoom>& Rooms);

	/** 몬스????계산 */
	void CalculateMonsterCounts(TArray<FDungeonRoom>& Rooms, int32 FloorNumber = 1);
};
