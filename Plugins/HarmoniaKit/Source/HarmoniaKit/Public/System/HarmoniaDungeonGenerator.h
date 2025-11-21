// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonGenerator.generated.h"

class UDungeonDataAsset;
class AHarmoniaDungeonInstance;

/**
 * 던전 방 타입
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
 * 던전 방 정보
 */
USTRUCT(BlueprintType)
struct FDungeonRoom
{
	GENERATED_BODY()

	/** 방 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	EDungeonRoomType RoomType = EDungeonRoomType::Normal;

	/** 방 위치 (그리드 좌표) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridPosition;

	/** 실제 월드 위치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector WorldPosition;

	/** 방 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector RoomSize = FVector(2000.0f, 2000.0f, 400.0f);

	/** 연결된 방들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FIntPoint> ConnectedRooms;

	/** 문 방향 (북, 남, 동, 서) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> DoorDirections;

	/** 난이도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Difficulty = 1;

	/** 스폰할 몬스터 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 MonsterCount = 0;
};

/**
 * 던전 복도 정보
 */
USTRUCT(BlueprintType)
struct FDungeonCorridor
{
	GENERATED_BODY()

	/** 시작 방 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint StartRoom;

	/** 끝 방 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint EndRoom;

	/** 경로 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> PathPoints;

	/** 복도 너비 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float Width = 400.0f;
};

/**
 * 던전 레이아웃 데이터
 */
USTRUCT(BlueprintType)
struct FDungeonLayout
{
	GENERATED_BODY()

	/** 던전 시드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Seed = 0;

	/** 방 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonRoom> Rooms;

	/** 복도 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonCorridor> Corridors;

	/** 던전 전체 크기 (그리드) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridSize = FIntPoint(10, 10);

	/** 타일 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float TileSize = 2000.0f;
};

/**
 * 던전 생성 설정
 */
USTRUCT(BlueprintType)
struct FDungeonGenerationSettings
{
	GENERATED_BODY()

	/** 던전 크기 (그리드) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint DungeonSize = FIntPoint(20, 20);

	/** 최소 방 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MinRoomCount = 5;

	/** 최대 방 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MaxRoomCount = 15;

	/** 최소 방 크기 (그리드 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MinRoomSize = FIntPoint(3, 3);

	/** 최대 방 크기 (그리드 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MaxRoomSize = FIntPoint(7, 7);

	/** 보스방 확률 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float BossRoomChance = 1.0f;

	/** 보물방 확률 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float TreasureRoomChance = 0.3f;

	/** 비밀방 확률 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float SecretRoomChance = 0.1f;

	/** 복도 구불구불함 (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float CorridorWindiness = 0.5f;

	/** 루프 허용 (순환 경로) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bAllowLoops = true;
};

/**
 * 던전 생성기
 * 프로시저럴 던전을 생성하는 유틸리티 클래스
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDungeonGenerator : public UObject
{
	GENERATED_BODY()

public:
	/** 던전 레이아웃 생성 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateDungeonLayout(const FDungeonGenerationSettings& Settings, int32 Seed = 0);

	/** 무한 던전 레이아웃 생성 (층별) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateInfiniteDungeonFloor(int32 FloorNumber, const FInfiniteDungeonFloor& FloorInfo);

	/** 던전 레이아웃을 월드에 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator", meta = (WorldContext = "WorldContextObject"))
	AHarmoniaDungeonInstance* SpawnDungeonFromLayout(UObject* WorldContextObject, const FDungeonLayout& Layout, const UDungeonDataAsset* DungeonData);

	/** 던전 검증 (연결성, 도달 가능성 등) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	bool ValidateDungeonLayout(const FDungeonLayout& Layout);

protected:
	/** 방 생성 */
	TArray<FDungeonRoom> GenerateRooms(const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** 복도 생성 */
	TArray<FDungeonCorridor> GenerateCorridors(const TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** 방 타입 할당 */
	void AssignRoomTypes(TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** 방 겹침 체크 */
	bool IsRoomOverlapping(const FDungeonRoom& Room, const TArray<FDungeonRoom>& ExistingRooms) const;

	/** 최단 경로 찾기 (A*) */
	TArray<FIntPoint> FindPath(FIntPoint Start, FIntPoint End, const FDungeonGenerationSettings& Settings) const;

	/** 방 연결성 체크 */
	bool AreAllRoomsConnected(const TArray<FDungeonRoom>& Rooms) const;

	/** 난이도 계산 (시작점으로부터의 거리 기반) */
	void CalculateRoomDifficulty(TArray<FDungeonRoom>& Rooms);

	/** 몬스터 수 계산 */
	void CalculateMonsterCounts(TArray<FDungeonRoom>& Rooms, int32 FloorNumber = 1);
};
