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

	// Check level (using base class Level)
	if (Level < FishingSpot->MinimumFishingLevel)
	{
		return false;
	}

	CurrentFishingSpot = FishingSpot;
	bIsFishing = true;
	bMinigameActive = false;
	FishingStartTime = GetWorld()->GetTimeSeconds();

	// Set random time until fish bites
	TimeUntilBite = FMath::RandRange(MinBiteTime, MaxBiteTime);

	// Activate rod
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
		// Select fish and calculate
		FCaughtFish CaughtFish = SelectFishFromSpawnTable();

		// Adjust quality based on skill factor
		CaughtFish.QualityScore = FMath::Clamp(PerformanceScore * 100.0f, 0.0f, 100.0f);

		// Check if perfect timing
		bool bPerfectCatch = PerformanceScore >= 0.95f;

		// Record to encyclopedia
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

	// End fishing
	CancelFishing();
}

// Note: AddFishingExperience is now a wrapper in the header that calls base class AddExperience()
// Note: GetExperienceForNextLevel is now provided by base class

void UHarmoniaFishingComponent::RegisterFishToCollection(const FCaughtFish& Fish)
{
	FishCollection.Add(Fish);

	// Update best record
	if (!BestCatchRecords.Contains(Fish.FishID))
	{
		BestCatchRecords.Add(Fish.FishID, Fish);
	}
	else
	{
		FCaughtFish& BestRecord = BestCatchRecords[Fish.FishID];
		// Update best record based on weight
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

	// Filter fish that meet level requirements (using base class Level)
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

	// Probability-based selection
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
			// Find fish data
			if (const FFishData* FishData = FishDatabase.Find(Entry.FishID))
			{
				return GenerateFish(Entry.FishID, *FishData);
			}
		}
	}

	// Fallback: Select first fish
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

	Result.QualityScore = 50.0f; // Default, adjust based on minigame performance

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
