// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaGameServiceDefinitions.generated.h"

/**
 * Platform types supported by the game service
 */
UENUM(BlueprintType)
enum class EHarmoniaPlatform : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Steam UMETA(DisplayName = "Steam"),
	Epic UMETA(DisplayName = "Epic Games"),
	Xbox UMETA(DisplayName = "Xbox"),
	PlayStation UMETA(DisplayName = "PlayStation"),
	Switch UMETA(DisplayName = "Nintendo Switch"),
	GOG UMETA(DisplayName = "GOG")
};

/**
 * Achievement unlock status
 */
UENUM(BlueprintType)
enum class EHarmoniaAchievementState : uint8
{
	Locked UMETA(DisplayName = "Locked"),
	InProgress UMETA(DisplayName = "In Progress"),
	Unlocked UMETA(DisplayName = "Unlocked"),
	Hidden UMETA(DisplayName = "Hidden"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Achievement category types
 */
UENUM(BlueprintType)
enum class EHarmoniaAchievementCategory : uint8
{
	Story UMETA(DisplayName = "Story"),
	Combat UMETA(DisplayName = "Combat"),
	Exploration UMETA(DisplayName = "Exploration"),
	Collection UMETA(DisplayName = "Collection"),
	Boss UMETA(DisplayName = "Boss"),
	Challenge UMETA(DisplayName = "Challenge"),
	Social UMETA(DisplayName = "Social"),
	Progression UMETA(DisplayName = "Progression"),
	Secret UMETA(DisplayName = "Secret")
};

/**
 * Cloud save sync status
 */
UENUM(BlueprintType)
enum class EHarmoniaCloudSaveStatus : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Uploading UMETA(DisplayName = "Uploading"),
	Downloading UMETA(DisplayName = "Downloading"),
	Synced UMETA(DisplayName = "Synced"),
	Conflict UMETA(DisplayName = "Conflict"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Leaderboard sort methods
 */
UENUM(BlueprintType)
enum class EHarmoniaLeaderboardSort : uint8
{
	Ascending UMETA(DisplayName = "Ascending"),
	Descending UMETA(DisplayName = "Descending")
};

/**
 * Leaderboard update method
 */
UENUM(BlueprintType)
enum class EHarmoniaLeaderboardUpdateMethod : uint8
{
	KeepBest UMETA(DisplayName = "Keep Best"),
	ForceUpdate UMETA(DisplayName = "Force Update")
};

/**
 * Leaderboard time range for queries
 */
UENUM(BlueprintType)
enum class EHarmoniaLeaderboardTimeRange : uint8
{
	AllTime UMETA(DisplayName = "All Time"),
	Daily UMETA(DisplayName = "Daily"),
	Weekly UMETA(DisplayName = "Weekly"),
	Monthly UMETA(DisplayName = "Monthly")
};

/**
 * DLC/Content ownership status
 */
UENUM(BlueprintType)
enum class EHarmoniaDLCOwnershipStatus : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Owned UMETA(DisplayName = "Owned"),
	NotOwned UMETA(DisplayName = "Not Owned"),
	Pending UMETA(DisplayName = "Pending"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Session join availability
 */
UENUM(BlueprintType)
enum class EHarmoniaSessionJoinability : uint8
{
	Public UMETA(DisplayName = "Public"),
	FriendsOnly UMETA(DisplayName = "Friends Only"),
	InviteOnly UMETA(DisplayName = "Invite Only"),
	Closed UMETA(DisplayName = "Closed")
};

/**
 * Achievement data structure for platform service integration
 */
USTRUCT(BlueprintType)
struct HARMONIAGAMESERVICE_API FHarmoniaServiceAchievementData
{
	GENERATED_BODY()

	/** Unique achievement identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName AchievementId;

	/** Display name of the achievement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText Title;

	/** Description of how to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText Description;

	/** Current unlock state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EHarmoniaAchievementState State = EHarmoniaAchievementState::Locked;

	/** Current progress (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	float Progress = 0.0f;

	/** Category type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EHarmoniaAchievementCategory Category = EHarmoniaAchievementCategory::Story;

	/** When it was unlocked (if unlocked) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FDateTime UnlockTime;

	/** Is this a hidden achievement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsHidden = false;

	/** Icon path or texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FSoftObjectPath IconPath;

	/** XP or reward points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	int32 RewardPoints = 0;

	FHarmoniaServiceAchievementData()
		: AchievementId(NAME_None)
		, State(EHarmoniaAchievementState::Locked)
		, Progress(0.0f)
		, Category(EHarmoniaAchievementCategory::Story)
		, UnlockTime(0)
		, bIsHidden(false)
		, RewardPoints(0)
	{
	}
};

/**
 * Cloud save data structure
 */
USTRUCT(BlueprintType)
struct HARMONIAGAMESERVICE_API FHarmoniaCloudSaveData
{
	GENERATED_BODY()

	/** Save slot name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString SlotName;

	/** Platform this save is from */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	EHarmoniaPlatform Platform = EHarmoniaPlatform::Unknown;

	/** Last modified timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FDateTime LastModified;

	/** Size in bytes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	int64 SizeBytes = 0;

	/** Current sync status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	EHarmoniaCloudSaveStatus Status = EHarmoniaCloudSaveStatus::Idle;

	/** Cloud version/revision number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	int32 CloudRevision = 0;

	/** Local version/revision number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	int32 LocalRevision = 0;

	FHarmoniaCloudSaveData()
		: Platform(EHarmoniaPlatform::Unknown)
		, LastModified(0)
		, SizeBytes(0)
		, Status(EHarmoniaCloudSaveStatus::Idle)
		, CloudRevision(0)
		, LocalRevision(0)
	{
	}
};

/**
 * Leaderboard entry structure
 */
USTRUCT(BlueprintType)
struct HARMONIAGAMESERVICE_API FHarmoniaLeaderboardEntry
{
	GENERATED_BODY()

	/** User's unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString UserId;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString DisplayName;

	/** Rank/position (1-based) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Rank = 0;

	/** Score/time value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 Score = 0;

	/** Platform of the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	EHarmoniaPlatform Platform = EHarmoniaPlatform::Unknown;

	/** When this score was achieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FDateTime Timestamp;

	/** Additional metadata (JSON string) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString MetaData;

	FHarmoniaLeaderboardEntry()
		: Rank(0)
		, Score(0)
		, Platform(EHarmoniaPlatform::Unknown)
		, Timestamp(0)
	{
	}
};

/**
 * Player statistics structure
 */
USTRUCT(BlueprintType)
struct HARMONIAGAMESERVICE_API FHarmoniaPlayerStatistics
{
	GENERATED_BODY()

	/** Total playtime in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int64 TotalPlaytimeSeconds = 0;

	/** Number of deaths */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 DeathCount = 0;

	/** Total monsters killed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 MonstersKilled = 0;

	/** Bosses defeated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 BossesDefeated = 0;

	/** Items collected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 ItemsCollected = 0;

	/** Distance traveled in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int64 DistanceTraveled = 0;

	/** Damage dealt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int64 TotalDamageDealt = 0;

	/** Damage taken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int64 TotalDamageTaken = 0;

	/** Custom stat map for additional tracking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	TMap<FName, int64> CustomStats;

	FHarmoniaPlayerStatistics()
		: TotalPlaytimeSeconds(0)
		, DeathCount(0)
		, MonstersKilled(0)
		, BossesDefeated(0)
		, ItemsCollected(0)
		, DistanceTraveled(0)
		, TotalDamageDealt(0)
		, TotalDamageTaken(0)
	{
	}
};

/**
 * DLC/Content data structure
 */
USTRUCT(BlueprintType)
struct HARMONIAGAMESERVICE_API FHarmoniaDLCData
{
	GENERATED_BODY()

	/** DLC unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	FName DLCId;

	/** DLC display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	FText DisplayName;

	/** Ownership status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	EHarmoniaDLCOwnershipStatus OwnershipStatus = EHarmoniaDLCOwnershipStatus::Unknown;

	/** Platform-specific app ID or SKU */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	TMap<EHarmoniaPlatform, FString> PlatformIds;

	/** Is the DLC currently installed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	bool bIsInstalled = false;

	/** Content unlock tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	FGameplayTagContainer UnlockTags;

	FHarmoniaDLCData()
		: DLCId(NAME_None)
		, OwnershipStatus(EHarmoniaDLCOwnershipStatus::Unknown)
		, bIsInstalled(false)
	{
	}
};

/**
 * Cross-play session data
 */
USTRUCT(BlueprintType)
struct HARMONIAGAMESERVICE_API FHarmoniaCrossPlaySession
{
	GENERATED_BODY()

	/** Session unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString SessionId;

	/** Session name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString SessionName;

	/** Host user ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString HostUserId;

	/** Host platform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	EHarmoniaPlatform HostPlatform = EHarmoniaPlatform::Unknown;

	/** Current player count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 CurrentPlayers = 0;

	/** Maximum player count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 MaxPlayers = 4;

	/** Joinability setting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	EHarmoniaSessionJoinability Joinability = EHarmoniaSessionJoinability::Public;

	/** Map/level name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString MapName;

	/** Game mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString GameMode;

	/** Average ping in ms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 PingMs = 0;

	/** Session creation time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FDateTime CreatedTime;

	FHarmoniaCrossPlaySession()
		: HostPlatform(EHarmoniaPlatform::Unknown)
		, CurrentPlayers(0)
		, MaxPlayers(4)
		, Joinability(EHarmoniaSessionJoinability::Public)
		, PingMs(0)
		, CreatedTime(0)
	{
	}
};
