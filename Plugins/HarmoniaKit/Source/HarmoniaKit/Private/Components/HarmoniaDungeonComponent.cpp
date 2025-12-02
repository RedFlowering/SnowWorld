// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#include "Components/HarmoniaDungeonComponent.h"
#include "Components/HarmoniaProgressionComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UHarmoniaDungeonComponent::UHarmoniaDungeonComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	CurrentDungeon = nullptr;
	CurrentDifficulty = EDungeonDifficulty::Normal;
	CurrentDungeonState = EDungeonState::NotStarted;
	RemainingTime = 0.0f;
	ElapsedTime = 0.0f;
	CurrentFloor = 0;
	HighestFloorReached = 0;
	CurrentScore = 0;
}

void UHarmoniaDungeonComponent::BeginPlay()
{
	Super::BeginPlay();

	// Ïª¥Ìè¨?åÌä∏ Ï∞∏Ï°∞ Ï∞æÍ∏∞
	if (AActor* Owner = GetOwner())
	{
		ProgressionComponent = Owner->FindComponentByClass<UHarmoniaProgressionComponent>();
		InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
	}
}

void UHarmoniaDungeonComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentDungeonState == EDungeonState::InProgress)
	{
		UpdateTimer(DeltaTime);
	}
}

bool UHarmoniaDungeonComponent::EnterDungeon(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty)
{
	if (!DungeonData)
	{
		return false;
	}

	if (!CanEnterDungeon(DungeonData))
	{
		return false;
	}

	CurrentDungeon = DungeonData;
	CurrentDifficulty = Difficulty;
	CurrentDungeonState = EDungeonState::NotStarted;
	ElapsedTime = 0.0f;
	CurrentScore = 0;

	if (DungeonData->TimeLimit > 0.0f)
	{
		RemainingTime = DungeonData->TimeLimit;
	}
	else
	{
		RemainingTime = -1.0f;
	}

	if (DungeonData->DungeonType == EDungeonType::Infinite)
	{
		CurrentFloor = 1;
	}

	SetComponentTickEnabled(true);

	return true;
}

void UHarmoniaDungeonComponent::ExitDungeon(bool bSaveProgress)
{
	if (!CurrentDungeon)
	{
		return;
	}

	// ÏßÑÌñâ Ï§ëÏù¥Î©??§Ìå® Ï≤òÎ¶¨
	if (CurrentDungeonState == EDungeonState::InProgress)
	{
		SetDungeonState(EDungeonState::Failed);
	}

	if (bSaveProgress && CurrentDungeon->DungeonType == EDungeonType::Infinite)
	{
		if (CurrentFloor > HighestFloorReached)
		{
			HighestFloorReached = CurrentFloor;
		}
	}

	CurrentDungeon = nullptr;
	CurrentDungeonState = EDungeonState::NotStarted;
	RemainingTime = 0.0f;
	ElapsedTime = 0.0f;
	CurrentFloor = 0;
	CurrentScore = 0;
	ActiveChallengeModifiers.Empty();

	SetComponentTickEnabled(false);
}

bool UHarmoniaDungeonComponent::CanEnterDungeon(const UDungeonDataAsset* DungeonData) const
{
	if (!DungeonData)
	{
		return false;
	}

	return ValidateRequirements(DungeonData);
}

void UHarmoniaDungeonComponent::StartDungeon()
{
	if (!CurrentDungeon || CurrentDungeonState != EDungeonState::NotStarted)
	{
		return;
	}

	SetDungeonState(EDungeonState::InProgress);
}

void UHarmoniaDungeonComponent::CompleteDungeon(bool bSuccess)
{
	if (!CurrentDungeon)
	{
		return;
	}

	EDungeonState NewState = bSuccess ? EDungeonState::Completed : EDungeonState::Failed;
	SetDungeonState(NewState);

	if (bSuccess)
	{
		GrantDungeonReward();

		// ??Çπ ?±Î°ù
		if (APlayerState* PS = Cast<APlayerState>(GetOwner()))
		{
			SubmitRanking(PS->GetPlayerName(), ElapsedTime, CurrentScore);
		}
	}

	OnDungeonCompleted.Broadcast(bSuccess);
}

void UHarmoniaDungeonComponent::SetDungeonState(EDungeonState NewState)
{
	if (CurrentDungeonState == NewState)
	{
		return;
	}

	EDungeonState OldState = CurrentDungeonState;
	CurrentDungeonState = NewState;

	OnDungeonStateChanged.Broadcast(OldState, NewState);
}

void UHarmoniaDungeonComponent::AdvanceToNextFloor()
{
	if (!CurrentDungeon || CurrentDungeon->DungeonType != EDungeonType::Infinite)
	{
		return;
	}

	int32 OldFloor = CurrentFloor;
	CurrentFloor++;

	if (CurrentFloor > HighestFloorReached)
	{
		HighestFloorReached = CurrentFloor;
	}

	OnInfiniteDungeonFloorChanged.Broadcast(OldFloor, CurrentFloor);
}

void UHarmoniaDungeonComponent::ApplyChallengeModifier(const FDungeonChallengeModifier& Modifier)
{
	ActiveChallengeModifiers.Add(Modifier);
}

void UHarmoniaDungeonComponent::RemoveChallengeModifier(const FDungeonChallengeModifier& Modifier)
{
	ActiveChallengeModifiers.Remove(Modifier);
}

void UHarmoniaDungeonComponent::SubmitRanking(const FString& PlayerName, float ClearTime, int32 Score)
{
	if (!CurrentDungeon)
	{
		return;
	}

	FDungeonRankingEntry NewEntry;
	NewEntry.PlayerName = PlayerName;
	NewEntry.ClearTime = ClearTime;
	NewEntry.Score = Score;
	NewEntry.Difficulty = CurrentDifficulty;
	NewEntry.ClearDate = FDateTime::Now();

	// ??Çπ Î™©Î°ù??Ï∂îÍ?
	DungeonRankings.Add(NewEntry);

	// ?êÏàò ?úÏúºÎ°??ïÎ†¨
	DungeonRankings.Sort([](const FDungeonRankingEntry& A, const FDungeonRankingEntry& B)
	{
		return A.Score > B.Score;
	});

	// ?ÅÏúÑ 100Í∞úÎßå ?†Ï?
	if (DungeonRankings.Num() > 100)
	{
		DungeonRankings.SetNum(100);
	}

	// Í∞úÏù∏ ÏµúÍ≥† Í∏∞Î°ù Í∞±Ïã†
	if (Score > PersonalBestRecord.Score || PersonalBestRecord.Score == 0)
	{
		PersonalBestRecord = NewEntry;
	}

	OnDungeonRankingUpdated.Broadcast(NewEntry);
}

TArray<FDungeonRankingEntry> UHarmoniaDungeonComponent::GetDungeonRankings(int32 TopCount) const
{
	TArray<FDungeonRankingEntry> Result;

	int32 Count = FMath::Min(TopCount, DungeonRankings.Num());
	for (int32 i = 0; i < Count; i++)
	{
		Result.Add(DungeonRankings[i]);
	}

	return Result;
}

FDungeonRankingEntry UHarmoniaDungeonComponent::GetPersonalBest() const
{
	return PersonalBestRecord;
}

void UHarmoniaDungeonComponent::GrantDungeonReward()
{
	if (!CurrentDungeon)
	{
		return;
	}

	FDungeonReward Reward = CalculateReward();

	// ?§Ï†ú Î≥¥ÏÉÅ ÏßÄÍ∏?Î°úÏßÅ?Ä Í≤åÏûÑ ?úÏä§?úÏóê ?∞Îùº Íµ¨ÌòÑ
	// ?? ?∏Î≤§?†Î¶¨ Ïª¥Ìè¨?åÌä∏, Í≤ΩÌóòÏπ??úÏä§???±Í≥º ?∞Îèô
}

FDungeonReward UHarmoniaDungeonComponent::CalculateReward() const
{
	FDungeonReward Reward;

	if (!CurrentDungeon)
	{
		return Reward;
	}

	// ?úÏù¥?ÑÎ≥Ñ Î≥¥ÏÉÅ Í∞Ä?∏Ïò§Í∏?
	if (const FDungeonReward* BaseReward = CurrentDungeon->Rewards.Find(CurrentDifficulty))
	{
		Reward = *BaseReward;
	}

	// Ï±åÎ¶∞ÏßÄ Î™®Îîî?åÏù¥???ÅÏö©
	for (const FDungeonChallengeModifier& Modifier : ActiveChallengeModifiers)
	{
		Reward.ExperienceReward = FMath::RoundToInt(Reward.ExperienceReward * Modifier.ScoreMultiplier);
		Reward.GoldReward = FMath::RoundToInt(Reward.GoldReward * Modifier.ScoreMultiplier);
		Reward.RewardMultiplier *= Modifier.ScoreMultiplier;
	}

	// Î¨¥Ìïú ?òÏ†Ñ?Ä Ï∏µÏàò???∞Îùº Î≥¥ÏÉÅ Ï¶ùÍ?
	if (CurrentDungeon->DungeonType == EDungeonType::Infinite)
	{
		float FloorMultiplier = 1.0f + (CurrentFloor - 1) * 0.1f;
		Reward.ExperienceReward = FMath::RoundToInt(Reward.ExperienceReward * FloorMultiplier);
		Reward.GoldReward = FMath::RoundToInt(Reward.GoldReward * FloorMultiplier);
	}

	return Reward;
}

int32 UHarmoniaDungeonComponent::GetPartySize() const
{
	// ?åÌã∞ ?úÏä§?úÍ≥º ?∞Îèô ?ÑÏöî
	// ?ÑÏû¨???ÑÏãúÎ°?1 Î∞òÌôò
	return 1;
}

TMap<ERaidRole, int32> UHarmoniaDungeonComponent::GetRoleComposition() const
{
	TMap<ERaidRole, int32> Composition;

	// ?åÌã∞/?àÏù¥???úÏä§?úÍ≥º ?∞Îèô?òÏó¨ ??ï†Î≥??∏Ïõê Í≥ÑÏÇ∞
	// ?ÑÏû¨???ÑÏãú Íµ¨ÌòÑ
	Composition.Add(ERaidRole::Tank, 1);
	Composition.Add(ERaidRole::Healer, 1);
	Composition.Add(ERaidRole::DPS, 3);

	return Composition;
}

void UHarmoniaDungeonComponent::UpdateTimer(float DeltaTime)
{
	ElapsedTime += DeltaTime;

	if (RemainingTime > 0.0f)
	{
		RemainingTime -= DeltaTime;

		if (RemainingTime <= 0.0f)
		{
			RemainingTime = 0.0f;
			CompleteDungeon(false); // ?úÍ∞Ñ Ï¥àÍ≥ºÎ°??§Ìå®
		}

		OnDungeonTimeUpdate.Broadcast(RemainingTime, CurrentDungeon->TimeLimit);
	}
}

bool UHarmoniaDungeonComponent::ValidateRequirements(const UDungeonDataAsset* DungeonData) const
{
	if (!DungeonData)
	{
		return false;
	}

	const FDungeonRequirement& Req = DungeonData->Requirements;

	// ?àÎ≤® ?ïÏù∏
	int32 PlayerLevel = GetPlayerLevel();
	if (PlayerLevel < Req.MinLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateRequirements: Player level %d is below minimum %d"), PlayerLevel, Req.MinLevel);
		return false;
	}
	
	// ?åÌã∞ ?¨Í∏∞ ?ïÏù∏
	int32 PartySize = GetPartySize();
	if (PartySize < Req.MinPartySize || PartySize > Req.MaxPartySize)
	{
		return false;
	}

	// ?ÑÏöî ?ÑÏù¥???ïÏù∏
	if (!HasRequiredItems(Req.RequiredItemIDs))
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateRequirements: Missing required items"));
		return false;
	}

	// ?ÑÏöî ?òÏä§???ïÏù∏
	// Note: ?òÏä§???úÏä§?úÏù¥ Íµ¨ÌòÑ?òÎ©¥ ?¨Í∏∞???ïÏù∏
	// ?ÑÏû¨??RequiredQuestIDsÍ∞Ä ÎπÑÏñ¥?àÏúºÎ©??µÍ≥º
	if (Req.RequiredQuestIDs.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateRequirements: Quest validation not yet implemented"));
		// TODO: ?òÏä§???úÏä§??Íµ¨ÌòÑ ???∞Îèô
	}

	return true;
}

int32 UHarmoniaDungeonComponent::CalculateScore() const
{
	if (!CurrentDungeon)
	{
		return 0;
	}

	int32 Score = 10000; // Í∏∞Î≥∏ ?êÏàò

	// ?úÍ∞Ñ Î≥¥ÎÑà??
	if (CurrentDungeon->TimeLimit > 0.0f)
	{
		float TimeRatio = RemainingTime / CurrentDungeon->TimeLimit;
		Score += FMath::RoundToInt(5000.0f * TimeRatio);
	}

	// ?úÏù¥??Î≥¥ÎÑà??
	switch (CurrentDifficulty)
	{
	case EDungeonDifficulty::Normal:
		Score *= 1;
		break;
	case EDungeonDifficulty::Hard:
		Score *= 2;
		break;
	case EDungeonDifficulty::Expert:
		Score *= 3;
		break;
	case EDungeonDifficulty::Master:
		Score *= 4;
		break;
	case EDungeonDifficulty::Nightmare:
		Score *= 5;
		break;
	}

	// Ï±åÎ¶∞ÏßÄ Î™®Îîî?åÏù¥??Î≥¥ÎÑà??
	for (const FDungeonChallengeModifier& Modifier : ActiveChallengeModifiers)
	{
		Score = FMath::RoundToInt(Score * Modifier.ScoreMultiplier);
	}

	// Î¨¥Ìïú ?òÏ†Ñ Ï∏µÏàò Î≥¥ÎÑà??
	if (CurrentDungeon->DungeonType == EDungeonType::Infinite)
	{
		Score += CurrentFloor * 1000;
	}

	return Score;
}

int32 UHarmoniaDungeonComponent::GetPlayerLevel() const
{
	if (ProgressionComponent)
	{
		return ProgressionComponent->CurrentLevel;
	}
	
	// ?ÑÎ°úÍ∑∏Î†à??Ïª¥Ìè¨?åÌä∏Í∞Ä ?ÜÏúºÎ©?Í∏∞Î≥∏Í∞?1 Î∞òÌôò
	return 1;
}

bool UHarmoniaDungeonComponent::HasRequiredItems(const TArray<FName>& ItemIDs) const
{
	if (ItemIDs.Num() == 0)
	{
		return true; // ?ÑÏöî ?ÑÏù¥?úÏù¥ ?ÜÏúºÎ©??µÍ≥º
	}

	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasRequiredItems: No inventory component found"));
		return false;
	}

	for (const FName& ItemID : ItemIDs)
	{
		FHarmoniaID HarmoniaID(ItemID);
		if (InventoryComponent->GetTotalCount(HarmoniaID) < 1)
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaDungeonComponent::ConsumeRequiredItems(const TArray<FName>& ItemIDs)
{
	if (ItemIDs.Num() == 0)
	{
		return true;
	}

	if (!InventoryComponent)
	{
		return false;
	}

	// Î®ºÏ? Î™®Îì† ?ÑÏù¥?úÏù¥ ?àÎäîÏßÄ ?ïÏù∏
	if (!HasRequiredItems(ItemIDs))
	{
		return false;
	}

	// ?ÑÏù¥???åÎπÑ
	for (const FName& ItemID : ItemIDs)
	{
		FHarmoniaID HarmoniaID(ItemID);
		if (!InventoryComponent->RemoveItem(HarmoniaID, 1, -1.0f))
		{
			UE_LOG(LogTemp, Error, TEXT("ConsumeRequiredItems: Failed to remove item %s"), *ItemID.ToString());
			return false;
		}
	}

	return true;
}
