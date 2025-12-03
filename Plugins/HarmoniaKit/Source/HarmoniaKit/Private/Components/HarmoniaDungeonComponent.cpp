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

	// 컴포?�트 참조 찾기
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

	// 진행 중이�??�패 처리
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

		// ??�� ?�록
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

	// ??�� 목록??추�?
	DungeonRankings.Add(NewEntry);

	// ?�수 ?�으�??�렬
	DungeonRankings.Sort([](const FDungeonRankingEntry& A, const FDungeonRankingEntry& B)
	{
		return A.Score > B.Score;
	});

	// ?�위 100개만 ?��?
	if (DungeonRankings.Num() > 100)
	{
		DungeonRankings.SetNum(100);
	}

	// 개인 최고 기록 갱신
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

	// ?�제 보상 지�?로직?� 게임 ?�스?�에 ?�라 구현
	// ?? ?�벤?�리 컴포?�트, 경험�??�스???�과 ?�동
}

FDungeonReward UHarmoniaDungeonComponent::CalculateReward() const
{
	FDungeonReward Reward;

	if (!CurrentDungeon)
	{
		return Reward;
	}

	// ?�이?�별 보상 가?�오�?
	if (const FDungeonReward* BaseReward = CurrentDungeon->Rewards.Find(CurrentDifficulty))
	{
		Reward = *BaseReward;
	}

	// 챌린지 모디?�이???�용
	for (const FDungeonChallengeModifier& Modifier : ActiveChallengeModifiers)
	{
		Reward.ExperienceReward = FMath::RoundToInt(Reward.ExperienceReward * Modifier.ScoreMultiplier);
		Reward.GoldReward = FMath::RoundToInt(Reward.GoldReward * Modifier.ScoreMultiplier);
		Reward.RewardMultiplier *= Modifier.ScoreMultiplier;
	}

	// 무한 ?�전?� 층수???�라 보상 증�?
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
	// ?�티 ?�스?�과 ?�동 ?�요
	// ?�재???�시�?1 반환
	return 1;
}

TMap<ERaidRole, int32> UHarmoniaDungeonComponent::GetRoleComposition() const
{
	TMap<ERaidRole, int32> Composition;

	// ?�티/?�이???�스?�과 ?�동?�여 ??���??�원 계산
	// ?�재???�시 구현
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
			CompleteDungeon(false); // ?�간 초과�??�패
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

	// ?�벨 ?�인
	int32 PlayerLevel = GetPlayerLevel();
	if (PlayerLevel < Req.MinLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateRequirements: Player level %d is below minimum %d"), PlayerLevel, Req.MinLevel);
		return false;
	}
	
	// ?�티 ?�기 ?�인
	int32 PartySize = GetPartySize();
	if (PartySize < Req.MinPartySize || PartySize > Req.MaxPartySize)
	{
		return false;
	}

	// ?�요 ?�이???�인
	if (!HasRequiredItems(Req.RequiredItemIDs))
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateRequirements: Missing required items"));
		return false;
	}

	// ?�요 ?�스???�인
	// Note: ?�스???�스?�이 구현?�면 ?�기???�인
	// ?�재??RequiredQuestIDs가 비어?�으�??�과
	if (Req.RequiredQuestIDs.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateRequirements: Quest validation not yet implemented"));
		// TODO: ?�스???�스??구현 ???�동
	}

	return true;
}

int32 UHarmoniaDungeonComponent::CalculateScore() const
{
	if (!CurrentDungeon)
	{
		return 0;
	}

	int32 Score = 10000; // 기본 ?�수

	// ?�간 보너??
	if (CurrentDungeon->TimeLimit > 0.0f)
	{
		float TimeRatio = RemainingTime / CurrentDungeon->TimeLimit;
		Score += FMath::RoundToInt(5000.0f * TimeRatio);
	}

	// ?�이??보너??
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

	// 챌린지 모디?�이??보너??
	for (const FDungeonChallengeModifier& Modifier : ActiveChallengeModifiers)
	{
		Score = FMath::RoundToInt(Score * Modifier.ScoreMultiplier);
	}

	// 무한 ?�전 층수 보너??
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
	
	// ?�로그레??컴포?�트가 ?�으�?기본�?1 반환
	return 1;
}

bool UHarmoniaDungeonComponent::HasRequiredItems(const TArray<FName>& ItemIDs) const
{
	if (ItemIDs.Num() == 0)
	{
		return true; // ?�요 ?�이?�이 ?�으�??�과
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

	// 먼�? 모든 ?�이?�이 ?�는지 ?�인
	if (!HasRequiredItems(ItemIDs))
	{
		return false;
	}

	// ?�이???�비
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
