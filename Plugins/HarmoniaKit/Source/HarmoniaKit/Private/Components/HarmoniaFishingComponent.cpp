// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaFishingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

UHarmoniaFishingComponent::UHarmoniaFishingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHarmoniaFishingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaFishingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsFishing && !bMinigameActive)
	{
		// 물고기�? 물기�?기다�?
		TimeUntilBite -= DeltaTime;
		if (TimeUntilBite <= 0.0f)
		{
			OnFishBiteDetected();
		}
	}
}

bool UHarmoniaFishingComponent::StartFishing(UFishingSpotData* FishingSpot)
{
	if (!FishingSpot || bIsFishing)
	{
		return false;
	}

	// ?�벨 체크 (base class??Level ?�용)
	if (Level < FishingSpot->MinimumFishingLevel)
	{
		return false;
	}

	CurrentFishingSpot = FishingSpot;
	bIsFishing = true;
	bMinigameActive = false;
	FishingStartTime = GetWorld()->GetTimeSeconds();

	// 물고기�? 물기까�? ?�덤 ?�간 ?�정
	TimeUntilBite = FMath::RandRange(MinBiteTime, MaxBiteTime);

	// ???�성??
	SetComponentTickEnabled(true);

	OnFishingStarted.Broadcast(FishingSpot);

	return true;
}

void UHarmoniaFishingComponent::CancelFishing()
{
	if (!bIsFishing)
	{
		return;
	}

	bIsFishing = false;
	bMinigameActive = false;
	CurrentFishingSpot = nullptr;
	SetComponentTickEnabled(false);

	OnFishingCancelled.Broadcast();
}

void UHarmoniaFishingComponent::OnFishBiteDetected()
{
	if (!bIsFishing || bMinigameActive || !CurrentFishingSpot)
	{
		return;
	}

	bMinigameActive = true;
	CurrentMinigameSettings = CurrentFishingSpot->MinigameSettings;
	MinigameProgress = 0.0f;

	OnFishBite.Broadcast();
}

void UHarmoniaFishingComponent::CompleteFishingMinigame(bool bSuccess, float PerformanceScore)
{
	if (!bMinigameActive)
	{
		return;
	}

	bMinigameActive = false;

	if (bSuccess)
	{
		// 물고�??�택 �??�성
		FCaughtFish CaughtFish = SelectFishFromSpawnTable();

		// ?�능 ?�수???�라 ?�질 조정
		CaughtFish.QualityScore = FMath::Clamp(PerformanceScore * 100.0f, 0.0f, 100.0f);

		// ?�벽???�기?��? ?�인
		bool bPerfectCatch = PerformanceScore >= 0.95f;

		// ?�감???�록
		RegisterFishToCollection(CaughtFish);

		// 경험�?계산
		int32 BaseExp = static_cast<int32>(CaughtFish.QualityScore);
		if (bPerfectCatch)
		{
			BaseExp = FMath::CeilToInt(BaseExp * CurrentMinigameSettings.PerfectBonus);
		}
		else if (bSuccess)
		{
			BaseExp = FMath::CeilToInt(BaseExp * CurrentMinigameSettings.SuccessBonus);
		}

		// AddFishingExperience calls base class AddExperience
		AddFishingExperience(BaseExp);

		OnFishCaught.Broadcast(CaughtFish, bPerfectCatch);
	}
	else
	{
		OnFishEscaped.Broadcast();
	}

	// ?�시 종료
	CancelFishing();
}

// Note: AddFishingExperience is now a wrapper in the header that calls base class AddExperience()
// Note: GetExperienceForNextLevel is now provided by base class

void UHarmoniaFishingComponent::RegisterFishToCollection(const FCaughtFish& Fish)
{
	FishCollection.Add(Fish);

	// 최고 기록 ?�데?�트
	if (!BestCatchRecords.Contains(Fish.FishID))
	{
		BestCatchRecords.Add(Fish.FishID, Fish);
	}
	else
	{
		FCaughtFish& BestRecord = BestCatchRecords[Fish.FishID];
		// 무게 기�??�로 최고 기록 갱신
		if (Fish.Weight > BestRecord.Weight)
		{
			BestRecord = Fish;
		}
	}
}

bool UHarmoniaFishingComponent::GetBestCatchRecord(FName FishID, FCaughtFish& OutBestCatch) const
{
	if (const FCaughtFish* Record = BestCatchRecords.Find(FishID))
	{
		OutBestCatch = *Record;
		return true;
	}
	return false;
}

FCaughtFish UHarmoniaFishingComponent::SelectFishFromSpawnTable()
{
	if (!CurrentFishingSpot || CurrentFishingSpot->SpawnTable.Num() == 0)
	{
		return FCaughtFish();
	}

	// ?�벨 ?�구?�항??만족?�는 물고기만 ?�터�?(base class??Level ?�용)
	TArray<FFishingSpotSpawnEntry> ValidFish;
	for (const FFishingSpotSpawnEntry& Entry : CurrentFishingSpot->SpawnTable)
	{
		if (Level >= Entry.MinFishingLevel)
		{
			ValidFish.Add(Entry);
		}
	}

	if (ValidFish.Num() == 0)
	{
		return FCaughtFish();
	}

	// ?�률 기반 ?�택
	float TotalWeight = 0.0f;
	for (const FFishingSpotSpawnEntry& Entry : ValidFish)
	{
		TotalWeight += Entry.SpawnChance;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (const FFishingSpotSpawnEntry& Entry : ValidFish)
	{
		CurrentWeight += Entry.SpawnChance;
		if (RandomValue <= CurrentWeight)
		{
			// 물고�??�이??찾기
			if (const FFishData* FishData = FishDatabase.Find(Entry.FishID))
			{
				return GenerateFish(Entry.FishID, *FishData);
			}
		}
	}

	// ?�백: �?번째 물고�??�택
	if (const FFishData* FishData = FishDatabase.Find(ValidFish[0].FishID))
	{
		return GenerateFish(ValidFish[0].FishID, *FishData);
	}

	return FCaughtFish();
}

FCaughtFish UHarmoniaFishingComponent::GenerateFish(FName FishID, const FFishData& FishData)
{
	FCaughtFish Result;
	Result.FishID = FishID;
	Result.Size = FMath::RandRange(FishData.MinSize, FishData.MaxSize);
	Result.Weight = FMath::RandRange(FishData.MinWeight, FishData.MaxWeight);
	Result.CaughtTime = FDateTime::Now();
	
	if (const AActor* Owner = GetOwner())
	{
		Result.CaughtLocation = Owner->GetActorLocation();
	}

	Result.QualityScore = 50.0f; // 기본�? 미니게임 ?�능???�라 조정

	return Result;
}

// Note: CheckAndProcessLevelUp is now provided by base class

void UHarmoniaFishingComponent::OnActivityComplete()
{
	// Base class implementation handles common cleanup
	Super::OnActivityComplete();

	// Fishing-specific completion logic can be added here if needed
}

void UHarmoniaFishingComponent::OnLevelUpInternal(int32 NewLevel)
{
	// Broadcast fishing-specific level up event
	OnFishingLevelUp.Broadcast(NewLevel, 1);
}
