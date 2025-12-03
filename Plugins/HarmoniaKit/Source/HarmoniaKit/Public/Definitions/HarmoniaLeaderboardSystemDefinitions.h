// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLeaderboardSystemDefinitions.h
 * @brief Leaderboard system type definitions and data structures
 * 
 * Contains enums and structs for the leaderboard system including
 * sort orders, value types, and entry data structures.
 */

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaLeaderboardSystemDefinitions.generated.h"

/**
 * @enum ELeaderboardSortOrder
 * @brief Leaderboard sort order
 */
UENUM(BlueprintType)
enum class ELeaderboardSortOrder : uint8
{
	Ascending		UMETA(DisplayName = "Ascending"),		// Ascending (lower is better, e.g. clear time)
	Descending		UMETA(DisplayName = "Descending"),		// Descending (higher is better, e.g. score)
	MAX				UMETA(Hidden)
};

/**
 * @enum ELeaderboardType
 * @brief Leaderboard value type
 */
UENUM(BlueprintType)
enum class ELeaderboardType : uint8
{
	Numeric			UMETA(DisplayName = "Numeric"),			// Numeric (score)
	Time			UMETA(DisplayName = "Time"),			// Time (in seconds)
	MAX				UMETA(Hidden)
};

/**
 * Leaderboard definition structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaLeaderboardData : public FTableRowBase
{
	GENERATED_BODY()

	// Leaderboard ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FHarmoniaID LeaderboardId;

	// Display Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FText DisplayName;

	// Sort Order
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ELeaderboardSortOrder SortOrder = ELeaderboardSortOrder::Descending;

	// Value Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ELeaderboardType Type = ELeaderboardType::Numeric;

	// Steam Leaderboard Name (for integration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString SteamLeaderboardName;

	FHarmoniaLeaderboardData()
		: LeaderboardId()
		, DisplayName()
		, SortOrder(ELeaderboardSortOrder::Descending)
		, Type(ELeaderboardType::Numeric)
		, SteamLeaderboardName()
	{}
};

/**
 * Leaderboard entry structure (local version)
 * Note: For cross-platform entries, use FHarmoniaLeaderboardEntry in HarmoniaGameServiceDefinitions
 */
USTRUCT(BlueprintType)
struct FHarmoniaLocalLeaderboardEntry
{
	GENERATED_BODY()

	// Rank (1-based)
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int32 Rank = 0;

	// Player Name
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	FString PlayerName;

	// Score/Value
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int64 Score = 0;

	// Extra Data (optional)
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	FString ExtraData;

	FHarmoniaLocalLeaderboardEntry()
		: Rank(0)
		, PlayerName()
		, Score(0)
		, ExtraData()
	{}
};

