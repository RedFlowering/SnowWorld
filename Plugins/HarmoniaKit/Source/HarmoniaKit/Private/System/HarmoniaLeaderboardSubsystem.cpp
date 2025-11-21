// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaLeaderboardSubsystem.h"
#include "HarmoniaKit.h"

void UHarmoniaLeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaLeaderboardSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaLeaderboardSubsystem::SubmitScore(const FHarmoniaID& LeaderboardId, int64 Score)
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("Submitting Score %lld to Leaderboard %s"), Score, *LeaderboardId.ToString());
	
	// TODO: Find leaderboard definition and get Steam Leaderboard Name
	// InternalSubmitScore(SteamLeaderboardName, Score);
}

void UHarmoniaLeaderboardSubsystem::GetTopEntries(const FHarmoniaID& LeaderboardId, int32 Count)
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("Requesting Top %d Entries for Leaderboard %s"), Count, *LeaderboardId.ToString());
	// TODO: Request from Steam/Server
}

void UHarmoniaLeaderboardSubsystem::GetFriendsEntries(const FHarmoniaID& LeaderboardId)
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("Requesting Friends Entries for Leaderboard %s"), *LeaderboardId.ToString());
	// TODO: Request from Steam/Server
}

void UHarmoniaLeaderboardSubsystem::InternalSubmitScore(const FString& LeaderboardName, int64 Score)
{
	// Steamworks integration placeholder
}
