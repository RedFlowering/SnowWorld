// Copyright 2025 Snow Game Studio.

#include "HarmoniaGameService.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "OnlineSessionSettings.h"
#include "Engine/World.h"
#include "TimerManager.h"

UHarmoniaGameService::UHarmoniaGameService()
{
}

void UHarmoniaGameService::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Initializing..."));

	// Detect current platform
	DetectCurrentPlatform();

	// Initialize session search
	SessionSearch = MakeShared<FOnlineSessionSearch>();

	// Setup auto-upload timer for statistics
	if (bEnableStatistics && StatUploadIntervalSeconds > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				StatUploadTimerHandle,
				this,
				&UHarmoniaGameService::UploadStatistics,
				StatUploadIntervalSeconds,
				true
			);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Initialized on platform: %d"), (int32)CurrentPlatform);
}

void UHarmoniaGameService::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Deinitializing..."));

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StatUploadTimerHandle);
	}

	// Clear all delegate handles
	if (IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem())
	{
		if (IOnlineAchievementsPtr Achievements = OSS->GetAchievementsInterface())
		{
			Achievements->ClearOnAchievementUnlockedDelegate_Handle(AchievementWriteDelegateHandle);
		}

		if (IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface())
		{
			UserCloud->ClearOnEnumerateUserFilesCompleteDelegate_Handle(CloudSaveReadDelegateHandle);
			UserCloud->ClearOnWriteUserFileCompleteDelegate_Handle(CloudSaveWriteDelegateHandle);
			UserCloud->ClearOnDeleteUserFileCompleteDelegate_Handle(CloudSaveDeleteDelegateHandle);
		}

		if (IOnlineSessionPtr Sessions = OSS->GetSessionInterface())
		{
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
		}
	}

	Super::Deinitialize();
}

bool UHarmoniaGameService::ShouldCreateSubsystem(UObject* Outer) const
{
	// Always create this subsystem
	return true;
}

// ==================== PLATFORM DETECTION ====================

void UHarmoniaGameService::DetectCurrentPlatform()
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (!OSS)
	{
		CurrentPlatform = EHarmoniaPlatform::Unknown;
		return;
	}

	FName SubsystemName = OSS->GetSubsystemName();

	if (SubsystemName == TEXT("Steam"))
	{
		CurrentPlatform = EHarmoniaPlatform::Steam;
	}
	else if (SubsystemName == TEXT("EOS") || SubsystemName == TEXT("EOSPlus"))
	{
		CurrentPlatform = EHarmoniaPlatform::Epic;
	}
	else if (SubsystemName == TEXT("GDK") || SubsystemName == TEXT("Live"))
	{
		CurrentPlatform = EHarmoniaPlatform::Xbox;
	}
	else if (SubsystemName == TEXT("PS4") || SubsystemName == TEXT("PS5"))
	{
		CurrentPlatform = EHarmoniaPlatform::PlayStation;
	}
	else if (SubsystemName == TEXT("Switch"))
	{
		CurrentPlatform = EHarmoniaPlatform::Switch;
	}
	else if (SubsystemName == TEXT("GOG"))
	{
		CurrentPlatform = EHarmoniaPlatform::GOG;
	}
	else
	{
		CurrentPlatform = EHarmoniaPlatform::Unknown;
	}
}

EHarmoniaPlatform UHarmoniaGameService::GetCurrentPlatform() const
{
	return CurrentPlatform;
}

bool UHarmoniaGameService::IsPlatformAvailable(EHarmoniaPlatform Platform) const
{
	IOnlineSubsystem* OSS = GetPlatformSubsystem(Platform);
	return OSS != nullptr;
}

IOnlineSubsystem* UHarmoniaGameService::GetPlatformSubsystem(EHarmoniaPlatform Platform) const
{
	FName SubsystemName;

	switch (Platform)
	{
	case EHarmoniaPlatform::Steam:
		SubsystemName = TEXT("Steam");
		break;
	case EHarmoniaPlatform::Epic:
		SubsystemName = TEXT("EOS");
		break;
	case EHarmoniaPlatform::Xbox:
		SubsystemName = TEXT("GDK");
		break;
	case EHarmoniaPlatform::PlayStation:
		SubsystemName = TEXT("PS5");
		break;
	case EHarmoniaPlatform::Switch:
		SubsystemName = TEXT("Switch");
		break;
	case EHarmoniaPlatform::GOG:
		SubsystemName = TEXT("GOG");
		break;
	default:
		return nullptr;
	}

	return IOnlineSubsystem::Get(SubsystemName);
}

IOnlineSubsystem* UHarmoniaGameService::GetCurrentPlatformSubsystem() const
{
	return IOnlineSubsystem::Get();
}

// ==================== ACHIEVEMENTS ====================

void UHarmoniaGameService::UnlockAchievement(FName AchievementId)
{
	if (!bEnableAchievements)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HarmoniaGameService] Achievements are disabled"));
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		UE_LOG(LogTemp, Error, TEXT("[HarmoniaGameService] No online subsystem available"));
		OnAchievementUnlocked.Broadcast(AchievementId, false);
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[HarmoniaGameService] No identity interface"));
		OnAchievementUnlocked.Broadcast(AchievementId, false);
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[HarmoniaGameService] No valid user ID"));
		OnAchievementUnlocked.Broadcast(AchievementId, false);
		return;
	}

	IOnlineAchievementsPtr Achievements = OSS->GetAchievementsInterface();
	if (!Achievements.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[HarmoniaGameService] No achievements interface"));
		OnAchievementUnlocked.Broadcast(AchievementId, false);
		return;
	}

	// Create achievement write object
	FOnlineAchievementsWriteRef WriteObject = MakeShared<FOnlineAchievementsWrite>();
	WriteObject->SetFloatStat(*AchievementId.ToString(), 100.0f);

	// Setup completion delegate
	FOnAchievementsWrittenDelegate WriteDelegate = FOnAchievementsWrittenDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnAchievementWriteComplete
	);

	// Write achievement
	Achievements->WriteAchievements(*UserId, WriteObject, WriteDelegate);

	// Update local cache
	if (FHarmoniaAchievementData* CachedAchievement = AchievementCache.Find(AchievementId))
	{
		CachedAchievement->State = EHarmoniaAchievementState::Unlocked;
		CachedAchievement->Progress = 1.0f;
		CachedAchievement->UnlockTime = FDateTime::UtcNow();
	}
	else
	{
		FHarmoniaAchievementData NewAchievement;
		NewAchievement.AchievementId = AchievementId;
		NewAchievement.State = EHarmoniaAchievementState::Unlocked;
		NewAchievement.Progress = 1.0f;
		NewAchievement.UnlockTime = FDateTime::UtcNow();
		AchievementCache.Add(AchievementId, NewAchievement);
	}

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Unlocking achievement: %s"), *AchievementId.ToString());
}

void UHarmoniaGameService::UpdateAchievementProgress(FName AchievementId, float Progress)
{
	if (!bEnableAchievements)
	{
		return;
	}

	// Clamp progress
	Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		return;
	}

	IOnlineAchievementsPtr Achievements = OSS->GetAchievementsInterface();
	if (!Achievements.IsValid())
	{
		return;
	}

	// Create achievement write object
	FOnlineAchievementsWriteRef WriteObject = MakeShared<FOnlineAchievementsWrite>();
	WriteObject->SetFloatStat(*AchievementId.ToString(), Progress * 100.0f);

	// Setup completion delegate
	FOnAchievementsWrittenDelegate WriteDelegate = FOnAchievementsWrittenDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnAchievementWriteComplete
	);

	// Write achievement progress
	Achievements->WriteAchievements(*UserId, WriteObject, WriteDelegate);

	// Update local cache
	if (FHarmoniaAchievementData* CachedAchievement = AchievementCache.Find(AchievementId))
	{
		CachedAchievement->Progress = Progress;
		CachedAchievement->State = (Progress >= 1.0f) ? EHarmoniaAchievementState::Unlocked : EHarmoniaAchievementState::InProgress;

		if (Progress >= 1.0f)
		{
			CachedAchievement->UnlockTime = FDateTime::UtcNow();
		}
	}
	else
	{
		FHarmoniaAchievementData NewAchievement;
		NewAchievement.AchievementId = AchievementId;
		NewAchievement.Progress = Progress;
		NewAchievement.State = (Progress >= 1.0f) ? EHarmoniaAchievementState::Unlocked : EHarmoniaAchievementState::InProgress;

		if (Progress >= 1.0f)
		{
			NewAchievement.UnlockTime = FDateTime::UtcNow();
		}

		AchievementCache.Add(AchievementId, NewAchievement);
	}

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Achievement %s progress: %.2f%%"), *AchievementId.ToString(), Progress * 100.0f);
}

void UHarmoniaGameService::QueryAchievements()
{
	if (!bEnableAchievements)
	{
		OnAchievementsQueried.Broadcast(TArray<FHarmoniaAchievementData>(), false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnAchievementsQueried.Broadcast(TArray<FHarmoniaAchievementData>(), false);
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		OnAchievementsQueried.Broadcast(TArray<FHarmoniaAchievementData>(), false);
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		OnAchievementsQueried.Broadcast(TArray<FHarmoniaAchievementData>(), false);
		return;
	}

	IOnlineAchievementsPtr Achievements = OSS->GetAchievementsInterface();
	if (!Achievements.IsValid())
	{
		OnAchievementsQueried.Broadcast(TArray<FHarmoniaAchievementData>(), false);
		return;
	}

	// Setup completion delegate
	FOnQueryAchievementsCompleteDelegate QueryDelegate = FOnQueryAchievementsCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnAchievementQueryComplete
	);

	// Query achievements
	Achievements->QueryAchievements(*UserId, QueryDelegate);

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Querying achievements..."));
}

bool UHarmoniaGameService::GetAchievementData(FName AchievementId, FHarmoniaAchievementData& OutAchievement) const
{
	const FHarmoniaAchievementData* Found = AchievementCache.Find(AchievementId);
	if (Found)
	{
		OutAchievement = *Found;
		return true;
	}
	return false;
}

TArray<FHarmoniaAchievementData> UHarmoniaGameService::GetAllAchievements() const
{
	TArray<FHarmoniaAchievementData> Result;
	AchievementCache.GenerateValueArray(Result);
	return Result;
}

void UHarmoniaGameService::OnAchievementWriteComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Achievement write complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
}

void UHarmoniaGameService::OnAchievementQueryComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Achievement query complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	if (bWasSuccessful)
	{
		IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
		if (OSS)
		{
			IOnlineAchievementsPtr Achievements = OSS->GetAchievementsInterface();
			if (Achievements.IsValid())
			{
				TArray<FOnlineAchievement> AchievementData;
				Achievements->GetCachedAchievements(PlayerId, AchievementData);

				// Update cache
				for (const FOnlineAchievement& Achievement : AchievementData)
				{
					FName AchievementId(*Achievement.Id);

					FHarmoniaAchievementData& CachedData = AchievementCache.FindOrAdd(AchievementId);
					CachedData.AchievementId = AchievementId;
					CachedData.Progress = Achievement.Progress;

					if (Achievement.Progress >= 100.0f)
					{
						CachedData.State = EHarmoniaAchievementState::Unlocked;
					}
					else if (Achievement.Progress > 0.0f)
					{
						CachedData.State = EHarmoniaAchievementState::InProgress;
					}
					else
					{
						CachedData.State = EHarmoniaAchievementState::Locked;
					}
				}
			}
		}
	}

	OnAchievementsQueried.Broadcast(GetAllAchievements(), bWasSuccessful);
}

// ==================== CLOUD SAVE ====================

void UHarmoniaGameService::UploadCloudSave(const FString& SlotName, const TArray<uint8>& SaveData)
{
	if (!bEnableCloudSaves)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HarmoniaGameService] Cloud saves are disabled"));
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface();
	if (!UserCloud.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[HarmoniaGameService] No user cloud interface"));
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	// Setup completion delegate
	FOnWriteUserFileCompleteDelegate WriteDelegate = FOnWriteUserFileCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnCloudSaveWriteComplete
	);
	CloudSaveWriteDelegateHandle = UserCloud->AddOnWriteUserFileCompleteDelegate_Handle(0, WriteDelegate);

	// Write file to cloud
	bool bStarted = UserCloud->WriteUserFile(*UserId, SlotName, SaveData);

	if (bStarted)
	{
		// Update cache
		FHarmoniaCloudSaveData& CacheData = CloudSaveCache.FindOrAdd(SlotName);
		CacheData.SlotName = SlotName;
		CacheData.Platform = CurrentPlatform;
		CacheData.Status = EHarmoniaCloudSaveStatus::Uploading;
		CacheData.SizeBytes = SaveData.Num();
		CacheData.LastModified = FDateTime::UtcNow();
		CacheData.LocalRevision++;

		UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Uploading cloud save: %s (%d bytes)"), *SlotName, SaveData.Num());
	}
	else
	{
		UserCloud->ClearOnWriteUserFileCompleteDelegate_Handle(0, CloudSaveWriteDelegateHandle);
		OnCloudSaveSynced.Broadcast(SlotName, false);
	}
}

void UHarmoniaGameService::DownloadCloudSave(const FString& SlotName)
{
	if (!bEnableCloudSaves)
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface();
	if (!UserCloud.IsValid())
	{
		OnCloudSaveSynced.Broadcast(SlotName, false);
		return;
	}

	// Setup completion delegate
	FOnReadUserFileCompleteDelegate ReadDelegate = FOnReadUserFileCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnCloudSaveReadComplete
	);
	CloudSaveReadDelegateHandle = UserCloud->AddOnReadUserFileCompleteDelegate_Handle(0, ReadDelegate);

	// Read file from cloud
	bool bStarted = UserCloud->ReadUserFile(*UserId, SlotName);

	if (bStarted)
	{
		// Update cache
		FHarmoniaCloudSaveData& CacheData = CloudSaveCache.FindOrAdd(SlotName);
		CacheData.SlotName = SlotName;
		CacheData.Platform = CurrentPlatform;
		CacheData.Status = EHarmoniaCloudSaveStatus::Downloading;

		UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Downloading cloud save: %s"), *SlotName);
	}
	else
	{
		UserCloud->ClearOnReadUserFileCompleteDelegate_Handle(0, CloudSaveReadDelegateHandle);
		OnCloudSaveSynced.Broadcast(SlotName, false);
	}
}

void UHarmoniaGameService::SyncCloudSave(const FString& SlotName)
{
	// For now, just download - full sync logic would compare timestamps and resolve conflicts
	DownloadCloudSave(SlotName);
}

void UHarmoniaGameService::DeleteCloudSave(const FString& SlotName)
{
	if (!bEnableCloudSaves)
	{
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		return;
	}

	IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface();
	if (!UserCloud.IsValid())
	{
		return;
	}

	// Setup completion delegate
	FOnDeleteUserFileCompleteDelegate DeleteDelegate = FOnDeleteUserFileCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnCloudSaveDeleteComplete
	);
	CloudSaveDeleteDelegateHandle = UserCloud->AddOnDeleteUserFileCompleteDelegate_Handle(0, DeleteDelegate);

	// Delete file from cloud
	UserCloud->DeleteUserFile(*UserId, SlotName, true, true);

	// Remove from cache
	CloudSaveCache.Remove(SlotName);
	DownloadedSaveData.Remove(SlotName);

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Deleting cloud save: %s"), *SlotName);
}

EHarmoniaCloudSaveStatus UHarmoniaGameService::GetCloudSaveStatus(const FString& SlotName) const
{
	const FHarmoniaCloudSaveData* Found = CloudSaveCache.Find(SlotName);
	if (Found)
	{
		return Found->Status;
	}
	return EHarmoniaCloudSaveStatus::Idle;
}

void UHarmoniaGameService::OnCloudSaveWriteComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Cloud save write complete: %s - %s"),
		*FileName, bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	if (FHarmoniaCloudSaveData* CacheData = CloudSaveCache.Find(FileName))
	{
		CacheData->Status = bWasSuccessful ? EHarmoniaCloudSaveStatus::Synced : EHarmoniaCloudSaveStatus::Error;
		if (bWasSuccessful)
		{
			CacheData->CloudRevision = CacheData->LocalRevision;
		}
	}

	OnCloudSaveSynced.Broadcast(FileName, bWasSuccessful);

	// Clear delegate
	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (OSS)
	{
		IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface();
		if (UserCloud.IsValid())
		{
			UserCloud->ClearOnWriteUserFileCompleteDelegate_Handle(0, CloudSaveWriteDelegateHandle);
		}
	}
}

void UHarmoniaGameService::OnCloudSaveReadComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Cloud save read complete: %s - %s"),
		*FileName, bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	if (bWasSuccessful)
	{
		IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
		if (OSS)
		{
			IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface();
			if (UserCloud.IsValid())
			{
				TArray<uint8> FileData;
				if (UserCloud->GetFileContents(UserId, FileName, FileData))
				{
					DownloadedSaveData.Add(FileName, FileData);

					if (FHarmoniaCloudSaveData* CacheData = CloudSaveCache.Find(FileName))
					{
						CacheData->Status = EHarmoniaCloudSaveStatus::Synced;
						CacheData->SizeBytes = FileData.Num();
						CacheData->CloudRevision++;
					}

					UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Downloaded %d bytes for %s"), FileData.Num(), *FileName);
				}
			}
		}
	}
	else
	{
		if (FHarmoniaCloudSaveData* CacheData = CloudSaveCache.Find(FileName))
		{
			CacheData->Status = EHarmoniaCloudSaveStatus::Error;
		}
	}

	OnCloudSaveSynced.Broadcast(FileName, bWasSuccessful);

	// Clear delegate
	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (OSS)
	{
		IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface();
		if (UserCloud.IsValid())
		{
			UserCloud->ClearOnReadUserFileCompleteDelegate_Handle(0, CloudSaveReadDelegateHandle);
		}
	}
}

void UHarmoniaGameService::OnCloudSaveDeleteComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Cloud save delete complete: %s - %s"),
		*FileName, bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	// Clear delegate
	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (OSS)
	{
		IOnlineUserCloudPtr UserCloud = OSS->GetUserCloudInterface();
		if (UserCloud.IsValid())
		{
			UserCloud->ClearOnDeleteUserFileCompleteDelegate_Handle(0, CloudSaveDeleteDelegateHandle);
		}
	}
}

// ==================== LEADERBOARDS ====================

void UHarmoniaGameService::UploadLeaderboardScore(FName LeaderboardId, int64 Score, EHarmoniaLeaderboardUpdateMethod UpdateMethod)
{
	if (!bEnableLeaderboards)
	{
		OnLeaderboardScoreUploaded.Broadcast(LeaderboardId, Score, false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnLeaderboardScoreUploaded.Broadcast(LeaderboardId, Score, false);
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		OnLeaderboardScoreUploaded.Broadcast(LeaderboardId, Score, false);
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		OnLeaderboardScoreUploaded.Broadcast(LeaderboardId, Score, false);
		return;
	}

	IOnlineLeaderboardsPtr Leaderboards = OSS->GetLeaderboardsInterface();
	if (!Leaderboards.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[HarmoniaGameService] No leaderboards interface"));
		OnLeaderboardScoreUploaded.Broadcast(LeaderboardId, Score, false);
		return;
	}

	// Create leaderboard write object
	FOnlineLeaderboardWrite WriteObject;
	WriteObject.LeaderboardNames.Add(LeaderboardId);

	FVariantData ScoreData;
	ScoreData.SetValue(Score);
	WriteObject.SetIntStat(LeaderboardId, Score);

	// Use KeepBest by default (most leaderboard systems)
	WriteObject.RatedStat = LeaderboardId;
	WriteObject.DisplayFormat = (UpdateMethod == EHarmoniaLeaderboardUpdateMethod::KeepBest) ?
		ELeaderboardFormat::Number : ELeaderboardFormat::Number;

	// Setup completion delegate
	FOnLeaderboardFlushCompleteDelegate FlushDelegate = FOnLeaderboardFlushCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnLeaderboardWriteComplete
	);

	// Write score
	bool bResult = Leaderboards->FlushLeaderboards(FName(TEXT("GameSession")));

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Uploading leaderboard score: %s = %lld"), *LeaderboardId.ToString(), Score);

	// For now, broadcast success if write was initiated
	OnLeaderboardScoreUploaded.Broadcast(LeaderboardId, Score, bResult);
}

void UHarmoniaGameService::QueryLeaderboard(FName LeaderboardId, int32 StartRank, int32 EntryCount, EHarmoniaLeaderboardTimeRange TimeRange)
{
	if (!bEnableLeaderboards)
	{
		OnLeaderboardQueried.Broadcast(TArray<FHarmoniaLeaderboardEntry>(), false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnLeaderboardQueried.Broadcast(TArray<FHarmoniaLeaderboardEntry>(), false);
		return;
	}

	IOnlineLeaderboardsPtr Leaderboards = OSS->GetLeaderboardsInterface();
	if (!Leaderboards.IsValid())
	{
		OnLeaderboardQueried.Broadcast(TArray<FHarmoniaLeaderboardEntry>(), false);
		return;
	}

	// Create read object
	FOnlineLeaderboardReadRef ReadObject = MakeShared<FOnlineLeaderboardRead>();
	ReadObject->LeaderboardName = LeaderboardId;

	// Setup completion delegate
	FOnLeaderboardReadCompleteDelegate ReadDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnLeaderboardReadComplete
	);

	// Read leaderboard
	Leaderboards->ReadLeaderboards(TArray<FUniqueNetIdRef>(), ReadObject, ReadDelegate);

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Querying leaderboard: %s"), *LeaderboardId.ToString());
}

void UHarmoniaGameService::QueryLeaderboardAroundPlayer(FName LeaderboardId, int32 Range)
{
	QueryLeaderboard(LeaderboardId, 1, Range * 2 + 1);
}

void UHarmoniaGameService::QueryLeaderboardFriends(FName LeaderboardId)
{
	QueryLeaderboard(LeaderboardId, 1, 100);
}

void UHarmoniaGameService::OnLeaderboardWriteComplete(const FName& SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Leaderboard write complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
}

void UHarmoniaGameService::OnLeaderboardReadComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Leaderboard read complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	TArray<FHarmoniaLeaderboardEntry> Entries;
	OnLeaderboardQueried.Broadcast(Entries, bWasSuccessful);
}

// ==================== STATISTICS ====================

void UHarmoniaGameService::IncrementStat(FName StatName, int64 Amount)
{
	int64 CurrentValue = GetStat(StatName);
	SetStat(StatName, CurrentValue + Amount);
}

void UHarmoniaGameService::SetStat(FName StatName, int64 Value)
{
	if (!bEnableStatistics)
	{
		return;
	}

	// Update local stats
	if (StatName == TEXT("TotalPlaytime"))
	{
		PlayerStats.TotalPlaytimeSeconds = Value;
	}
	else if (StatName == TEXT("Deaths"))
	{
		PlayerStats.DeathCount = Value;
	}
	else if (StatName == TEXT("MonstersKilled"))
	{
		PlayerStats.MonstersKilled = Value;
	}
	else if (StatName == TEXT("BossesDefeated"))
	{
		PlayerStats.BossesDefeated = Value;
	}
	else if (StatName == TEXT("ItemsCollected"))
	{
		PlayerStats.ItemsCollected = Value;
	}
	else if (StatName == TEXT("DistanceTraveled"))
	{
		PlayerStats.DistanceTraveled = Value;
	}
	else if (StatName == TEXT("DamageDealt"))
	{
		PlayerStats.TotalDamageDealt = Value;
	}
	else if (StatName == TEXT("DamageTaken"))
	{
		PlayerStats.TotalDamageTaken = Value;
	}
	else
	{
		// Custom stat
		PlayerStats.CustomStats.Add(StatName, Value);
	}

	UE_LOG(LogTemp, Verbose, TEXT("[HarmoniaGameService] Stat updated: %s = %lld"), *StatName.ToString(), Value);
}

int64 UHarmoniaGameService::GetStat(FName StatName) const
{
	if (StatName == TEXT("TotalPlaytime"))
	{
		return PlayerStats.TotalPlaytimeSeconds;
	}
	else if (StatName == TEXT("Deaths"))
	{
		return PlayerStats.DeathCount;
	}
	else if (StatName == TEXT("MonstersKilled"))
	{
		return PlayerStats.MonstersKilled;
	}
	else if (StatName == TEXT("BossesDefeated"))
	{
		return PlayerStats.BossesDefeated;
	}
	else if (StatName == TEXT("ItemsCollected"))
	{
		return PlayerStats.ItemsCollected;
	}
	else if (StatName == TEXT("DistanceTraveled"))
	{
		return PlayerStats.DistanceTraveled;
	}
	else if (StatName == TEXT("DamageDealt"))
	{
		return PlayerStats.TotalDamageDealt;
	}
	else if (StatName == TEXT("DamageTaken"))
	{
		return PlayerStats.TotalDamageTaken;
	}
	else
	{
		const int64* Found = PlayerStats.CustomStats.Find(StatName);
		return Found ? *Found : 0;
	}
}

FHarmoniaPlayerStatistics UHarmoniaGameService::GetPlayerStatistics() const
{
	return PlayerStats;
}

void UHarmoniaGameService::UploadStatistics()
{
	if (!bEnableStatistics)
	{
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		return;
	}

	IOnlineStatsPtr Stats = OSS->GetStatsInterface();
	if (!Stats.IsValid())
	{
		return;
	}

	// Create stats write object
	TArray<FOnlineStatsUserUpdatedStats> StatsToWrite;
	FOnlineStatsUserUpdatedStats& UserStats = StatsToWrite.AddDefaulted_GetRef();
	UserStats.Account = UserId.ToSharedRef();

	// Add all stats
	UserStats.Stats.Add(TEXT("TotalPlaytime"), FOnlineStatUpdate(PlayerStats.TotalPlaytimeSeconds, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	UserStats.Stats.Add(TEXT("Deaths"), FOnlineStatUpdate(PlayerStats.DeathCount, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	UserStats.Stats.Add(TEXT("MonstersKilled"), FOnlineStatUpdate(PlayerStats.MonstersKilled, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	UserStats.Stats.Add(TEXT("BossesDefeated"), FOnlineStatUpdate(PlayerStats.BossesDefeated, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	UserStats.Stats.Add(TEXT("ItemsCollected"), FOnlineStatUpdate(PlayerStats.ItemsCollected, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	UserStats.Stats.Add(TEXT("DistanceTraveled"), FOnlineStatUpdate(PlayerStats.DistanceTraveled, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	UserStats.Stats.Add(TEXT("DamageDealt"), FOnlineStatUpdate(PlayerStats.TotalDamageDealt, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	UserStats.Stats.Add(TEXT("DamageTaken"), FOnlineStatUpdate(PlayerStats.TotalDamageTaken, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));

	// Add custom stats
	for (const auto& Pair : PlayerStats.CustomStats)
	{
		UserStats.Stats.Add(Pair.Key.ToString(), FOnlineStatUpdate(Pair.Value, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
	}

	// Setup completion delegate
	FOnlineStatsUpdateStatsCompleteDelegate WriteDelegate = FOnlineStatsUpdateStatsCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnStatsWriteComplete
	);

	// Write stats
	Stats->UpdateStats(UserId.ToSharedRef(), StatsToWrite, WriteDelegate);

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Uploading statistics..."));
}

void UHarmoniaGameService::QueryStatistics()
{
	if (!bEnableStatistics)
	{
		OnStatisticsUpdated.Broadcast(PlayerStats, false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnStatisticsUpdated.Broadcast(PlayerStats, false);
		return;
	}

	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		OnStatisticsUpdated.Broadcast(PlayerStats, false);
		return;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		OnStatisticsUpdated.Broadcast(PlayerStats, false);
		return;
	}

	IOnlineStatsPtr Stats = OSS->GetStatsInterface();
	if (!Stats.IsValid())
	{
		OnStatisticsUpdated.Broadcast(PlayerStats, false);
		return;
	}

	// Setup completion delegate
	TArray<FUniqueNetIdRef> Players;
	Players.Add(UserId.ToSharedRef());

	TArray<FString> StatNames;
	StatNames.Add(TEXT("TotalPlaytime"));
	StatNames.Add(TEXT("Deaths"));
	StatNames.Add(TEXT("MonstersKilled"));
	StatNames.Add(TEXT("BossesDefeated"));
	StatNames.Add(TEXT("ItemsCollected"));
	StatNames.Add(TEXT("DistanceTraveled"));
	StatNames.Add(TEXT("DamageDealt"));
	StatNames.Add(TEXT("DamageTaken"));

	FOnlineStatsQueryUserStatsCompleteDelegate ReadDelegate = FOnlineStatsQueryUserStatsCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnStatsReadComplete
	);

	// Query stats
	Stats->QueryStats(UserId.ToSharedRef(), Players, StatNames, ReadDelegate);

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Querying statistics..."));
}

void UHarmoniaGameService::OnStatsWriteComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Stats write complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	OnStatisticsUpdated.Broadcast(PlayerStats, bWasSuccessful);
}

void UHarmoniaGameService::OnStatsReadComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Stats read complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	if (bWasSuccessful)
	{
		IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
		if (OSS)
		{
			IOnlineStatsPtr Stats = OSS->GetStatsInterface();
			if (Stats.IsValid())
			{
				TArray<TSharedRef<const FOnlineStatsUserStats>> UsersStats;
				Stats->GetStats(TArray<FUniqueNetIdRef>{PlayerId.AsShared()}, UsersStats);

				if (UsersStats.Num() > 0)
				{
					for (const auto& StatPair : UsersStats[0]->Stats)
					{
						FName StatName(*StatPair.Key);
						FVariantData Value = StatPair.Value;

						int64 IntValue = 0;
						Value.GetValue(IntValue);

						SetStat(StatName, IntValue);
					}
				}
			}
		}
	}

	OnStatisticsUpdated.Broadcast(PlayerStats, bWasSuccessful);
}

// ==================== DLC / CONTENT OWNERSHIP ====================

void UHarmoniaGameService::CheckDLCOwnership(FName DLCId)
{
	// Platform-specific implementation would go here
	// For now, we'll use a simple cache-based system

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Checking DLC ownership: %s"), *DLCId.ToString());

	// Simulate ownership check
	FHarmoniaDLCData& DLCData = DLCCache.FindOrAdd(DLCId);
	DLCData.DLCId = DLCId;
	DLCData.OwnershipStatus = EHarmoniaDLCOwnershipStatus::Unknown;

	// In a real implementation, query the platform API
	OnDLCOwnershipChecked.Broadcast(DLCId, DLCData.OwnershipStatus);
}

bool UHarmoniaGameService::IsDLCOwned(FName DLCId) const
{
	const FHarmoniaDLCData* Found = DLCCache.Find(DLCId);
	return Found && Found->OwnershipStatus == EHarmoniaDLCOwnershipStatus::Owned;
}

bool UHarmoniaGameService::GetDLCData(FName DLCId, FHarmoniaDLCData& OutDLCData) const
{
	const FHarmoniaDLCData* Found = DLCCache.Find(DLCId);
	if (Found)
	{
		OutDLCData = *Found;
		return true;
	}
	return false;
}

void UHarmoniaGameService::QueryOwnedDLC()
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Querying owned DLC..."));

	// Platform-specific implementation would query the store API
	// For now, this is a stub
}

// ==================== CROSS-PLAY SESSIONS ====================

void UHarmoniaGameService::CreateCrossPlaySession(const FString& SessionName, int32 MaxPlayers, EHarmoniaSessionJoinability Joinability)
{
	if (!bEnableCrossPlay)
	{
		OnSessionCreated.Broadcast(TEXT(""), false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnSessionCreated.Broadcast(TEXT(""), false);
		return;
	}

	IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		OnSessionCreated.Broadcast(TEXT(""), false);
		return;
	}

	// Create session settings
	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = MaxPlayers;
	SessionSettings.NumPrivateConnections = 0;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bShouldAdvertise = (Joinability == EHarmoniaSessionJoinability::Public);
	SessionSettings.bAllowJoinInProgress = (Joinability != EHarmoniaSessionJoinability::Closed);
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = (Joinability == EHarmoniaSessionJoinability::Public || Joinability == EHarmoniaSessionJoinability::FriendsOnly);

	// Enable cross-play
	SessionSettings.bUseLobbiesIfAvailable = true;

	// Setup completion delegate
	FOnCreateSessionCompleteDelegate CreateDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnCreateSessionComplete
	);
	CreateSessionDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(CreateDelegate);

	// Create session
	bool bStarted = Sessions->CreateSession(0, FName(*SessionName), SessionSettings);

	if (!bStarted)
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
		OnSessionCreated.Broadcast(TEXT(""), false);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Creating cross-play session: %s"), *SessionName);
	}
}

void UHarmoniaGameService::FindCrossPlaySessions()
{
	if (!bEnableCrossPlay)
	{
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		return;
	}

	IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}

	// Setup search parameters
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->bIsLanQuery = false;

	// Setup completion delegate
	FOnFindSessionsCompleteDelegate FindDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(
		this, &UHarmoniaGameService::OnFindSessionsComplete
	);
	FindSessionsDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(FindDelegate);

	// Start search
	bool bStarted = Sessions->FindSessions(0, SessionSearch.ToSharedRef());

	if (!bStarted)
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Finding cross-play sessions..."));
	}
}

void UHarmoniaGameService::JoinCrossPlaySession(const FString& SessionId)
{
	if (!bEnableCrossPlay)
	{
		OnSessionJoined.Broadcast(SessionId, false);
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		OnSessionJoined.Broadcast(SessionId, false);
		return;
	}

	IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		OnSessionJoined.Broadcast(SessionId, false);
		return;
	}

	// For now, join the first result from search
	if (SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
	{
		// Setup completion delegate
		FOnJoinSessionCompleteDelegate JoinDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &UHarmoniaGameService::OnJoinSessionComplete
		);
		JoinSessionDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(JoinDelegate);

		// Join session
		bool bStarted = Sessions->JoinSession(0, FName(*SessionId), SessionSearch->SearchResults[0]);

		if (!bStarted)
		{
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
			OnSessionJoined.Broadcast(SessionId, false);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Joining session: %s"), *SessionId);
		}
	}
	else
	{
		OnSessionJoined.Broadcast(SessionId, false);
	}
}

void UHarmoniaGameService::LeaveCrossPlaySession()
{
	if (!bEnableCrossPlay)
	{
		return;
	}

	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (!OSS)
	{
		return;
	}

	IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}

	// Destroy the session
	Sessions->DestroySession(FName(TEXT("GameSession")));

	// Clear current session
	CurrentSession = FHarmoniaCrossPlaySession();

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Left cross-play session"));
}

bool UHarmoniaGameService::GetCurrentSession(FHarmoniaCrossPlaySession& OutSession) const
{
	if (!CurrentSession.SessionId.IsEmpty())
	{
		OutSession = CurrentSession;
		return true;
	}
	return false;
}

void UHarmoniaGameService::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Session creation complete: %s - %s"),
		*SessionName.ToString(), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	if (bWasSuccessful)
	{
		CurrentSession.SessionId = SessionName.ToString();
		CurrentSession.SessionName = SessionName.ToString();
		CurrentSession.HostPlatform = CurrentPlatform;
		CurrentSession.CreatedTime = FDateTime::UtcNow();
	}

	OnSessionCreated.Broadcast(SessionName.ToString(), bWasSuccessful);

	// Clear delegate
	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (OSS)
	{
		IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
		}
	}
}

void UHarmoniaGameService::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bWasSuccessful = (Result == EOnJoinSessionCompleteResult::Success);

	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Session join complete: %s - %s"),
		*SessionName.ToString(), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	if (bWasSuccessful)
	{
		CurrentSession.SessionId = SessionName.ToString();
		CurrentSession.SessionName = SessionName.ToString();
	}

	OnSessionJoined.Broadcast(SessionName.ToString(), bWasSuccessful);

	// Clear delegate
	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (OSS)
	{
		IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
		}
	}
}

void UHarmoniaGameService::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameService] Session search complete: %s - Found %d sessions"),
		bWasSuccessful ? TEXT("Success") : TEXT("Failed"),
		SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : 0);

	// Clear delegate
	IOnlineSubsystem* OSS = GetCurrentPlatformSubsystem();
	if (OSS)
	{
		IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
		}
	}
}
