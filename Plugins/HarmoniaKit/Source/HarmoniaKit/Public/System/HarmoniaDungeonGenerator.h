// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

/**
 * @file HarmoniaDungeonGenerator.h
 * @brief Dungeon generation system for procedural dungeon layouts
 * @author Harmonia Team
 * 
 * Provides procedural dungeon generation with customizable room types,
 * corridors, and infinite dungeon floor support.
 */

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonGenerator.generated.h"

class UDungeonDataAsset;
class AHarmoniaDungeonInstance;

/**
 * @enum EDungeonRoomType
 * @brief Types of dungeon rooms
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
 * @struct FDungeonRoom
 * @brief Contains dungeon room information
 */
USTRUCT(BlueprintType)
struct FDungeonRoom
{
	GENERATED_BODY()

	/** Room type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	EDungeonRoomType RoomType = EDungeonRoomType::Normal;

	/** Room position (grid coordinates) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridPosition = FIntPoint::ZeroValue;

	/** Actual world position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector WorldPosition = FVector::ZeroVector;

	/** Room size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector RoomSize = FVector(2000.0f, 2000.0f, 400.0f);

	/** Connected rooms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FIntPoint> ConnectedRooms;

	/** Door directions (North, South, East, West) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> DoorDirections;

	/** Difficulty level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Difficulty = 1;

	/** Number of monsters to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 MonsterCount = 0;
};

/**
 * @struct FDungeonCorridor
 * @brief Contains dungeon corridor information
 */
USTRUCT(BlueprintType)
struct FDungeonCorridor
{
	GENERATED_BODY()

	/** Start room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint StartRoom = FIntPoint::ZeroValue;

	/** End room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint EndRoom = FIntPoint::ZeroValue;

	/** Path waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FVector> PathPoints;

	/** Corridor width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float Width = 400.0f;
};

/**
 * @struct FDungeonLayout
 * @brief Contains dungeon layout data
 */
USTRUCT(BlueprintType)
struct FDungeonLayout
{
	GENERATED_BODY()

	/** Dungeon seed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 Seed = 0;

	/** Room list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonRoom> Rooms;

	/** Corridor list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TArray<FDungeonCorridor> Corridors;

	/** Dungeon total size (grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FIntPoint GridSize = FIntPoint(10, 10);

	/** Tile size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	float TileSize = 2000.0f;
};

/**
 * @struct FDungeonGenerationSettings
 * @brief Configuration for dungeon generation
 */
USTRUCT(BlueprintType)
struct FDungeonGenerationSettings
{
	GENERATED_BODY()

	/** Dungeon size (grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint DungeonSize = FIntPoint(20, 20);

	/** Minimum room count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MinRoomCount = 5;

	/** Maximum room count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 MaxRoomCount = 15;

	/** Minimum room size (grid units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MinRoomSize = FIntPoint(3, 3);

	/** Maximum room size (grid units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FIntPoint MaxRoomSize = FIntPoint(7, 7);

	/** Boss room spawn chance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float BossRoomChance = 1.0f;

	/** Treasure room spawn chance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float TreasureRoomChance = 0.3f;

	/** Secret room spawn chance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float SecretRoomChance = 0.1f;

	/** Corridor windiness (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float CorridorWindiness = 0.5f;

	/** Allow loops (circular paths) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bAllowLoops = true;
};

/**
 * @class UHarmoniaDungeonGenerator
 * @brief Dungeon generator utility class
 * 
 * Generates procedural dungeons using configurable settings.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDungeonGenerator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Generates dungeon layout
	 * @param Settings Generation settings
	 * @param Seed Random seed (0 = random)
	 * @return Generated dungeon layout
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateDungeonLayout(const FDungeonGenerationSettings& Settings, int32 Seed = 0);

	/**
	 * @brief Generates infinite dungeon layout (per floor)
	 * @param FloorNumber Floor number
	 * @param FloorInfo Floor configuration
	 * @return Generated dungeon layout for the floor
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	FDungeonLayout GenerateInfiniteDungeonFloor(int32 FloorNumber, const FInfiniteDungeonFloor& FloorInfo);

	/**
	 * @brief Spawns dungeon layout into the world
	 * @param WorldContextObject World context
	 * @param Layout Dungeon layout to spawn
	 * @param DungeonData Dungeon data asset
	 * @return Spawned dungeon instance actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator", meta = (WorldContext = "WorldContextObject"))
	AHarmoniaDungeonInstance* SpawnDungeonFromLayout(UObject* WorldContextObject, const FDungeonLayout& Layout, const UDungeonDataAsset* DungeonData);

	/**
	 * @brief Validates dungeon layout (connectivity, reachability, etc.)
	 * @param Layout Dungeon layout to validate
	 * @return True if layout is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Generator")
	bool ValidateDungeonLayout(const FDungeonLayout& Layout);

protected:
	/**
	 * @brief Generates rooms
	 * @param Settings Generation settings
	 * @param RandomStream Random stream for generation
	 * @return Array of generated rooms
	 */
	TArray<FDungeonRoom> GenerateRooms(const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/**
	 * @brief Generates corridors
	 * @param Rooms Array of rooms to connect
	 * @param Settings Generation settings
	 * @param RandomStream Random stream for generation
	 * @return Array of generated corridors
	 */
	TArray<FDungeonCorridor> GenerateCorridors(const TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/**
	 * @brief Assigns room types
	 * @param Rooms Rooms to assign types to
	 * @param Settings Generation settings
	 * @param RandomStream Random stream for generation
	 */
	void AssignRoomTypes(TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream);

	/**
	 * @brief Checks room overlap
	 * @param Room Room to check
	 * @param ExistingRooms Existing rooms to check against
	 * @return True if room overlaps
	 */
	bool IsRoomOverlapping(const FDungeonRoom& Room, const TArray<FDungeonRoom>& ExistingRooms) const;

	/**
	 * @brief Finds shortest path (A*)
	 * @param Start Start position
	 * @param End End position
	 * @param Settings Generation settings
	 * @return Path waypoints
	 */
	TArray<FIntPoint> FindPath(FIntPoint Start, FIntPoint End, const FDungeonGenerationSettings& Settings) const;

	/**
	 * @brief Checks if all rooms are connected
	 * @param Rooms Rooms to check
	 * @return True if all rooms are connected
	 */
	bool AreAllRoomsConnected(const TArray<FDungeonRoom>& Rooms) const;

	/**
	 * @brief Calculates room difficulty (based on distance from start room)
	 * @param Rooms Rooms to calculate difficulty for
	 */
	void CalculateRoomDifficulty(TArray<FDungeonRoom>& Rooms);

	/**
	 * @brief Calculates monster counts
	 * @param Rooms Rooms to assign monster counts
	 * @param FloorNumber Current floor number
	 */
	void CalculateMonsterCounts(TArray<FDungeonRoom>& Rooms, int32 FloorNumber = 1);
};
