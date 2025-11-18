# HarmoniaGameService - Game Service Platform Integration

**Version:** 1.0.0
**Module Type:** Runtime
**Platforms:** Steam, Epic Games, Xbox, PlayStation, Nintendo Switch, GOG

---

## Overview

HarmoniaGameService is a comprehensive game service platform integration layer for Unreal Engine 5.7. It provides a unified API for achievements, cloud saves, leaderboards, statistics tracking, DLC management, and cross-play session management across multiple platforms.

### Key Features

✅ **Achievement System** - Unlock and track achievements across Steam, Epic, Xbox, and other platforms
✅ **Cloud Save Sync** - Automatic cloud save synchronization with conflict resolution
✅ **Leaderboards** - Boss clear times, high scores, and global rankings
✅ **Statistics Tracking** - Playtime, deaths, monster kills, and custom stats
✅ **DLC Management** - Ownership verification and content unlocking
✅ **Cross-Play Sessions** - Create and join cross-platform multiplayer sessions

---

## Architecture

### Subsystem Pattern

HarmoniaGameService uses Unreal Engine's `UGameInstanceSubsystem` pattern, ensuring the service persists for the entire game session.

```cpp
UHarmoniaGameService* GameService = UGameInstance->GetSubsystem<UHarmoniaGameService>();
```

### Platform Detection

The subsystem automatically detects the current platform on initialization:

- **Steam** - Steamworks API
- **Epic Games** - Epic Online Services (EOS)
- **Xbox** - GDK (Game Development Kit)
- **PlayStation** - PS4/PS5 SDK
- **Nintendo Switch** - Switch SDK
- **GOG** - GOG Galaxy SDK

---

## API Reference

### Achievements

#### Unlock Achievement

```cpp
// Unlock a single achievement
GameService->UnlockAchievement(FName("FirstBlood"));
```

#### Update Progressive Achievement

```cpp
// Update progress for progressive achievements (0.0 to 1.0)
GameService->UpdateAchievementProgress(FName("MonsterSlayer"), 0.75f);
```

#### Query Achievements

```cpp
// Query all achievements from platform
GameService->QueryAchievements();

// Listen for completion
GameService->OnAchievementsQueried.AddDynamic(this, &UMyClass::OnAchievementsLoaded);

void UMyClass::OnAchievementsLoaded(const TArray<FHarmoniaAchievementData>& Achievements, bool bSuccess)
{
    for (const FHarmoniaAchievementData& Achievement : Achievements)
    {
        UE_LOG(LogTemp, Log, TEXT("Achievement: %s - %.0f%%"), *Achievement.Title.ToString(), Achievement.Progress * 100.0f);
    }
}
```

#### Get Cached Achievement Data

```cpp
FHarmoniaAchievementData AchievementData;
if (GameService->GetAchievementData(FName("BossSlayer"), AchievementData))
{
    UE_LOG(LogTemp, Log, TEXT("Progress: %.2f%%"), AchievementData.Progress * 100.0f);
}
```

---

### Cloud Saves

#### Upload Save to Cloud

```cpp
TArray<uint8> SaveData = /* your serialized save data */;
GameService->UploadCloudSave(TEXT("SaveSlot1"), SaveData);

// Listen for completion
GameService->OnCloudSaveSynced.AddDynamic(this, &UMyClass::OnSaveUploaded);
```

#### Download Save from Cloud

```cpp
GameService->DownloadCloudSave(TEXT("SaveSlot1"));

// Listen for completion
GameService->OnCloudSaveSynced.AddDynamic(this, &UMyClass::OnSaveDownloaded);
```

#### Auto-Sync

```cpp
// Automatically sync (compares timestamps and resolves conflicts)
GameService->SyncCloudSave(TEXT("SaveSlot1"));
```

#### Check Sync Status

```cpp
EHarmoniaCloudSaveStatus Status = GameService->GetCloudSaveStatus(TEXT("SaveSlot1"));

switch (Status)
{
    case EHarmoniaCloudSaveStatus::Synced:
        UE_LOG(LogTemp, Log, TEXT("Save is up to date"));
        break;
    case EHarmoniaCloudSaveStatus::Uploading:
        UE_LOG(LogTemp, Log, TEXT("Upload in progress..."));
        break;
    case EHarmoniaCloudSaveStatus::Conflict:
        UE_LOG(LogTemp, Warning, TEXT("Save conflict detected!"));
        break;
}
```

---

### Leaderboards

#### Upload Score

```cpp
// Upload boss clear time (in milliseconds)
int64 ClearTimeMs = 123456;
GameService->UploadLeaderboardScore(
    FName("Boss_DragonKing_ClearTime"),
    ClearTimeMs,
    EHarmoniaLeaderboardUpdateMethod::KeepBest
);

// Listen for completion
GameService->OnLeaderboardScoreUploaded.AddDynamic(this, &UMyClass::OnScoreUploaded);
```

#### Query Global Leaderboard

```cpp
// Query top 10 global entries
GameService->QueryLeaderboard(
    FName("Boss_DragonKing_ClearTime"),
    1,      // Start rank
    10,     // Entry count
    EHarmoniaLeaderboardTimeRange::AllTime
);

// Listen for results
GameService->OnLeaderboardQueried.AddDynamic(this, &UMyClass::OnLeaderboardLoaded);

void UMyClass::OnLeaderboardLoaded(const TArray<FHarmoniaLeaderboardEntry>& Entries, bool bSuccess)
{
    for (const FHarmoniaLeaderboardEntry& Entry : Entries)
    {
        UE_LOG(LogTemp, Log, TEXT("%d. %s - %lld ms"), Entry.Rank, *Entry.DisplayName, Entry.Score);
    }
}
```

#### Query Around Player

```cpp
// Query 5 entries above and below the current player
GameService->QueryLeaderboardAroundPlayer(FName("Boss_DragonKing_ClearTime"), 5);
```

#### Query Friends Only

```cpp
// Query leaderboard with only friends
GameService->QueryLeaderboardFriends(FName("Boss_DragonKing_ClearTime"));
```

---

### Statistics

#### Track Statistics

```cpp
// Increment stats
GameService->IncrementStat(FName("Deaths"), 1);
GameService->IncrementStat(FName("MonstersKilled"), 1);
GameService->IncrementStat(FName("DamageDealt"), 150);

// Set stats directly
GameService->SetStat(FName("BossesDefeated"), 5);
```

#### Get Statistics

```cpp
int64 DeathCount = GameService->GetStat(FName("Deaths"));
UE_LOG(LogTemp, Log, TEXT("Total Deaths: %lld"), DeathCount);

// Get all statistics
FHarmoniaPlayerStatistics Stats = GameService->GetPlayerStatistics();
UE_LOG(LogTemp, Log, TEXT("Playtime: %lld seconds"), Stats.TotalPlaytimeSeconds);
UE_LOG(LogTemp, Log, TEXT("Monsters Killed: %d"), Stats.MonstersKilled);
```

#### Upload/Query Statistics

```cpp
// Upload local stats to platform (called automatically every 5 minutes by default)
GameService->UploadStatistics();

// Query stats from platform
GameService->QueryStatistics();

// Listen for completion
GameService->OnStatisticsUpdated.AddDynamic(this, &UMyClass::OnStatsUpdated);
```

#### Built-in Statistics

| Stat Name | Type | Description |
|-----------|------|-------------|
| `TotalPlaytime` | int64 | Total playtime in seconds |
| `Deaths` | int32 | Number of deaths |
| `MonstersKilled` | int32 | Total monsters killed |
| `BossesDefeated` | int32 | Total bosses defeated |
| `ItemsCollected` | int32 | Total items collected |
| `DistanceTraveled` | int64 | Distance traveled in cm |
| `DamageDealt` | int64 | Total damage dealt |
| `DamageTaken` | int64 | Total damage taken |

You can also track custom stats using the `CustomStats` map.

---

### DLC Management

#### Check DLC Ownership

```cpp
GameService->CheckDLCOwnership(FName("DLC_SeasonPass1"));

// Listen for result
GameService->OnDLCOwnershipChecked.AddDynamic(this, &UMyClass::OnDLCChecked);

void UMyClass::OnDLCChecked(FName DLCId, EHarmoniaDLCOwnershipStatus Status)
{
    if (Status == EHarmoniaDLCOwnershipStatus::Owned)
    {
        UE_LOG(LogTemp, Log, TEXT("Player owns %s"), *DLCId.ToString());
        // Unlock DLC content
    }
}
```

#### Check Ownership (Cached)

```cpp
if (GameService->IsDLCOwned(FName("DLC_SeasonPass1")))
{
    // Enable DLC features
}
```

#### Get DLC Data

```cpp
FHarmoniaDLCData DLCData;
if (GameService->GetDLCData(FName("DLC_SeasonPass1"), DLCData))
{
    UE_LOG(LogTemp, Log, TEXT("DLC: %s"), *DLCData.DisplayName.ToString());
    UE_LOG(LogTemp, Log, TEXT("Installed: %s"), DLCData.bIsInstalled ? TEXT("Yes") : TEXT("No"));
}
```

---

### Cross-Play Sessions

#### Create Session

```cpp
GameService->CreateCrossPlaySession(
    TEXT("MySession"),
    4,  // Max players
    EHarmoniaSessionJoinability::Public
);

// Listen for completion
GameService->OnSessionCreated.AddDynamic(this, &UMyClass::OnSessionCreated);
```

#### Find Sessions

```cpp
GameService->FindCrossPlaySessions();
```

#### Join Session

```cpp
GameService->JoinCrossPlaySession(TEXT("SessionId"));

// Listen for completion
GameService->OnSessionJoined.AddDynamic(this, &UMyClass::OnSessionJoined);
```

#### Leave Session

```cpp
GameService->LeaveCrossPlaySession();
```

#### Get Current Session

```cpp
FHarmoniaCrossPlaySession CurrentSession;
if (GameService->GetCurrentSession(CurrentSession))
{
    UE_LOG(LogTemp, Log, TEXT("Session: %s"), *CurrentSession.SessionName);
    UE_LOG(LogTemp, Log, TEXT("Players: %d/%d"), CurrentSession.CurrentPlayers, CurrentSession.MaxPlayers);
}
```

---

## Configuration

### DefaultGame.ini

```ini
[/Script/HarmoniaGameService.HarmoniaGameService]
bEnableAchievements=True
bEnableCloudSaves=True
bEnableLeaderboards=True
bEnableStatistics=True
bEnableCrossPlay=True
StatUploadIntervalSeconds=300.0
```

### Platform-Specific Setup

#### Steam

1. Add Steam App ID to `DefaultEngine.ini`:

```ini
[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480
GameServerQueryPort=27015
bRelaunchInSteam=false
bVACEnabled=0
bAllowP2PPacketRelay=true
P2PConnectionTimeout=90
```

2. Configure achievements in Steamworks partner portal
3. Set up leaderboards in Steamworks

#### Epic Games

1. Configure EOS in `DefaultEngine.ini`:

```ini
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientId=YOUR_CLIENT_ID
ClientSecret=YOUR_CLIENT_SECRET
```

2. Configure achievements in Epic Games Dev Portal
3. Set up leaderboards and stats

#### Xbox

1. Add GDK configuration to `DefaultEngine.ini`:

```ini
[OnlineSubsystem]
DefaultPlatformService=GDK

[OnlineSubsystemGDK]
bEnabled=true
```

2. Configure achievements in Partner Center
3. Set up leaderboards and stats in Xbox services

---

## Blueprint Integration

All functions are exposed to Blueprint and can be called from visual scripting:

### Blueprint Example: Unlock Achievement

```
Event BeginPlay
    ├─ Get Game Instance
    │   └─ Get Subsystem (HarmoniaGameService)
    │       └─ Unlock Achievement (Achievement ID: "FirstBlood")
```

### Blueprint Example: Upload Leaderboard Score

```
On Boss Defeated
    ├─ Get Clear Time (milliseconds)
    ├─ Get Game Instance
    │   └─ Get Subsystem (HarmoniaGameService)
    │       └─ Upload Leaderboard Score
    │           ├─ Leaderboard ID: "Boss_DragonKing_ClearTime"
    │           ├─ Score: [Clear Time]
    │           └─ Update Method: Keep Best
```

---

## Events and Delegates

All async operations provide delegate callbacks:

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnAchievementUnlocked` | `FName AchievementId, bool bSuccess` | Achievement unlock completed |
| `OnAchievementsQueried` | `TArray<FHarmoniaAchievementData> Achievements, bool bSuccess` | Achievement query completed |
| `OnCloudSaveSynced` | `FString SlotName, bool bSuccess` | Cloud save operation completed |
| `OnLeaderboardScoreUploaded` | `FName LeaderboardId, int64 Score, bool bSuccess` | Leaderboard score uploaded |
| `OnLeaderboardQueried` | `TArray<FHarmoniaLeaderboardEntry> Entries, bool bSuccess` | Leaderboard query completed |
| `OnStatisticsUpdated` | `FHarmoniaPlayerStatistics Stats, bool bSuccess` | Statistics sync completed |
| `OnDLCOwnershipChecked` | `FName DLCId, EHarmoniaDLCOwnershipStatus Status` | DLC ownership verified |
| `OnSessionCreated` | `FString SessionId, bool bSuccess` | Session creation completed |
| `OnSessionJoined` | `FString SessionId, bool bSuccess` | Session join completed |

---

## Usage Examples

### Example 1: Boss Fight Achievement System

```cpp
void ABossCharacter::OnDeath()
{
    UGameInstance* GI = GetGameInstance();
    UHarmoniaGameService* GameService = GI->GetSubsystem<UHarmoniaGameService>();

    // Unlock achievement
    GameService->UnlockAchievement(FName("Boss_DragonKing_Defeated"));

    // Upload clear time to leaderboard
    int64 ClearTimeMs = FMath::FloorToInt(BattleDuration * 1000.0f);
    GameService->UploadLeaderboardScore(
        FName("Boss_DragonKing_ClearTime"),
        ClearTimeMs,
        EHarmoniaLeaderboardUpdateMethod::KeepBest
    );

    // Update statistics
    GameService->IncrementStat(FName("BossesDefeated"), 1);
}
```

### Example 2: Auto-Save to Cloud

```cpp
void UMyGameInstance::SaveGame()
{
    // Serialize save data
    TArray<uint8> SaveData;
    FMemoryWriter Writer(SaveData);
    // ... serialize your game state

    // Upload to cloud
    UHarmoniaGameService* GameService = GetSubsystem<UHarmoniaGameService>();
    GameService->UploadCloudSave(TEXT("Slot1"), SaveData);
}
```

### Example 3: Progressive Achievement

```cpp
void APlayerCharacter::OnMonsterKilled()
{
    UHarmoniaGameService* GameService = GetGameInstance()->GetSubsystem<UHarmoniaGameService>();

    GameService->IncrementStat(FName("MonstersKilled"), 1);

    int64 TotalKills = GameService->GetStat(FName("MonstersKilled"));

    // Update progressive achievement (0-1000 kills)
    float Progress = FMath::Clamp(TotalKills / 1000.0f, 0.0f, 1.0f);
    GameService->UpdateAchievementProgress(FName("MonsterSlayer"), Progress);
}
```

---

## Best Practices

### 1. Cache Queries

Don't query the platform API repeatedly. Use cached data:

```cpp
// ❌ Don't do this every frame
FHarmoniaAchievementData Data;
GameService->GetAchievementData(FName("MyAchievement"), Data);

// ✅ Query once, cache results
void AMyActor::BeginPlay()
{
    GameService->QueryAchievements();
}
```

### 2. Batch Statistics Updates

Update stats locally and upload in batches:

```cpp
// ✅ Update locally
GameService->IncrementStat(FName("Deaths"), 1);
GameService->IncrementStat(FName("MonstersKilled"), 5);

// Upload automatically every 5 minutes (default), or manually
GameService->UploadStatistics();
```

### 3. Handle Failures Gracefully

Always check operation success:

```cpp
void UMyClass::OnCloudSaveSynced(const FString& SlotName, bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Save synced successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Save sync failed - using local save"));
        // Fallback to local save
    }
}
```

### 4. Platform-Agnostic Code

Write platform-agnostic code - let HarmoniaGameService handle platform differences:

```cpp
// ✅ Works on all platforms
GameService->UnlockAchievement(FName("MyAchievement"));

// ❌ Don't use platform-specific APIs directly
SteamAPI_UnlockAchievement("MyAchievement");
```

---

## Debugging

### Enable Verbose Logging

In `DefaultEngine.ini`:

```ini
[Core.Log]
LogTemp=Verbose
```

### Log Output

HarmoniaGameService logs all operations:

```
[HarmoniaGameService] Initializing...
[HarmoniaGameService] Initialized on platform: Steam
[HarmoniaGameService] Unlocking achievement: FirstBlood
[HarmoniaGameService] Achievement write complete: Success
[HarmoniaGameService] Uploading cloud save: Slot1 (12345 bytes)
[HarmoniaGameService] Cloud save write complete: Slot1 - Success
```

---

## Performance Considerations

1. **Async Operations** - All platform API calls are asynchronous and non-blocking
2. **Local Caching** - Achievement and stat data is cached locally to minimize API calls
3. **Auto-Upload** - Statistics are uploaded automatically every 5 minutes (configurable)
4. **Memory Efficient** - Save data is streamed, not loaded entirely into memory

---

## Platform Compatibility

| Feature | Steam | Epic | Xbox | PlayStation | Switch | GOG |
|---------|-------|------|------|-------------|--------|-----|
| Achievements | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Cloud Saves | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Leaderboards | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ |
| Statistics | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ |
| DLC Check | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Cross-Play | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ⚠️ |

✅ Fully Supported | ⚠️ Partial Support | ❌ Not Supported

---

## License

Copyright 2025 Snow Game Studio. All rights reserved.

---

## Support

For issues and feature requests, please contact the development team or refer to the HarmoniaKit documentation.
