// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonGenerator.generated.h"

class UDungeonDataAsset;
class AHarmoniaDungeonInstance;

/**
 * ?˜ì „ ë°??€??
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
 * ?˜ì „ ë°??•ë³´
 */
USTRUCT(BlueprintType)
struct FDungeonRoom
{
	GENERATED_BODY()

	/** ë°??€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	EDungeonRoomType RoomType = EDungeonRoomType::Normal;

	/** ë°??„ì¹˜ (ê·¸ë¦¬??ì¢Œí‘œ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridPosition;

	/** ?¤ì œ ?”ë“œ ?„ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector WorldPosition;

	/** ë°??¬ê¸° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector RoomSize = FVector(2000.0f, 2000.0f, 400.0f);

	/** ?°ê²°??ë°©ë“¤ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FIntPoint> ConnectedRooms;

	/** ë¬?ë°©í–¥ (ë¶? ?? ?? ?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> DoorDirections;

	/** ?œì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Difficulty = 1;

	/** ?¤í°??ëª¬ìŠ¤????*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 MonsterCount = 0;
};

/**
 * ?˜ì „ ë³µë„ ?•ë³´
 */
USTRUCT(BlueprintType)
struct FDungeonCorridor
{
	GENERATED_BODY()

	/** ?œì‘ ë°?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint StartRoom;

	/** ??ë°?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint EndRoom;

	/** ê²½ë¡œ ?¬ì¸??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> PathPoints;

	/** ë³µë„ ?ˆë¹„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float Width = 400.0f;
};

/**
 * ?˜ì „ ?ˆì´?„ì›ƒ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FDungeonLayout
{
	GENERATED_BODY()

	/** ?˜ì „ ?œë“œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Seed = 0;

	/** ë°?ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonRoom> Rooms;

	/** ë³µë„ ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonCorridor> Corridors;

	/** ?˜ì „ ?„ì²´ ?¬ê¸° (ê·¸ë¦¬?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridSize = FIntPoint(10, 10);

	/** ?€???¬ê¸° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float TileSize = 2000.0f;
};

/**
 * ?˜ì „ ?ì„± ?¤ì •
 */
USTRUCT(BlueprintType)
struct FDungeonGenerationSettings
{
	GENERATED_BODY()

	/** ?˜ì „ ?¬ê¸° (ê·¸ë¦¬?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint DungeonSize = FIntPoint(20, 20);

	/** ìµœì†Œ ë°?ê°œìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MinRoomCount = 5;

	/** ìµœë? ë°?ê°œìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MaxRoomCount = 15;

	/** ìµœì†Œ ë°??¬ê¸° (ê·¸ë¦¬???¨ìœ„) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MinRoomSize = FIntPoint(3, 3);

	/** ìµœë? ë°??¬ê¸° (ê·¸ë¦¬???¨ìœ„) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MaxRoomSize = FIntPoint(7, 7);

	/** ë³´ìŠ¤ë°??•ë¥  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float BossRoomChance = 1.0f;

	/** ë³´ë¬¼ë°??•ë¥  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float TreasureRoomChance = 0.3f;

	/** ë¹„ë?ë°??•ë¥  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float SecretRoomChance = 0.1f;

	/** ë³µë„ êµ¬ë¶ˆêµ¬ë¶ˆ??(0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float CorridorWindiness = 0.5f;

	/** ë£¨í”„ ?ˆìš© (?œí™˜ ê²½ë¡œ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bAllowLoops = true;
};

/**
 * ?˜ì „ ?ì„±ê¸?
 * ?„ë¡œ?œì????˜ì „???ì„±?˜ëŠ” ? í‹¸ë¦¬í‹° ?´ë˜??
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDungeonGenerator : public UObject
{
	GENERATED_BODY()

public:
	/** ?˜ì „ ?ˆì´?„ì›ƒ ?ì„± */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateDungeonLayout(const FDungeonGenerationSettings& Settings, int32 Seed = 0);

	/** ë¬´í•œ ?˜ì „ ?ˆì´?„ì›ƒ ?ì„± (ì¸µë³„) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateInfiniteDungeonFloor(int32 FloorNumber, const FInfiniteDungeonFloor& FloorInfo);

	/** ?˜ì „ ?ˆì´?„ì›ƒ???”ë“œ???¤í° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator", meta = (WorldContext = "WorldContextObject"))
	AHarmoniaDungeonInstance* SpawnDungeonFromLayout(UObject* WorldContextObject, const FDungeonLayout& Layout, const UDungeonDataAsset* DungeonData);

	/** ?˜ì „ ê²€ì¦?(?°ê²°?? ?„ë‹¬ ê°€?¥ì„± ?? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	bool ValidateDungeonLayout(const FDungeonLayout& Layout);

protected:
	/** ë°??ì„± */
	TArray<FDungeonRoom> GenerateRooms(const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** ë³µë„ ?ì„± */
	TArray<FDungeonCorridor> GenerateCorridors(const TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** ë°??€??? ë‹¹ */
	void AssignRoomTypes(TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/** ë°?ê²¹ì¹¨ ì²´í¬ */
	bool IsRoomOverlapping(const FDungeonRoom& Room, const TArray<FDungeonRoom>& ExistingRooms) const;

	/** ìµœë‹¨ ê²½ë¡œ ì°¾ê¸° (A*) */
	TArray<FIntPoint> FindPath(FIntPoint Start, FIntPoint End, const FDungeonGenerationSettings& Settings) const;

	/** ë°??°ê²°??ì²´í¬ */
	bool AreAllRoomsConnected(const TArray<FDungeonRoom>& Rooms) const;

	/** ?œì´??ê³„ì‚° (?œì‘?ìœ¼ë¡œë??°ì˜ ê±°ë¦¬ ê¸°ë°˜) */
	void CalculateRoomDifficulty(TArray<FDungeonRoom>& Rooms);

	/** ëª¬ìŠ¤????ê³„ì‚° */
	void CalculateMonsterCounts(TArray<FDungeonRoom>& Rooms, int32 FloorNumber = 1);
};
