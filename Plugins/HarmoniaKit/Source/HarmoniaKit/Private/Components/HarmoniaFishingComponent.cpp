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
		// Î¨ºÍ≥†Í∏∞Í? Î¨ºÍ∏∞Î•?Í∏∞Îã§Î¶?
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

	// ?àÎ≤® Ï≤¥ÌÅ¨ (base class??Level ?¨Ïö©)
	if (Level < FishingSpot->MinimumFishingLevel)
	{
		return false;
	}

	CurrentFishingSpot = FishingSpot;
	bIsFishing = true;
	bMinigameActive = false;
	FishingStartTime = GetWorld()->GetTimeSeconds();

	// Î¨ºÍ≥†Í∏∞Í? Î¨ºÍ∏∞ÍπåÏ? ?úÎç§ ?úÍ∞Ñ ?§Ï†ï
	TimeUntilBite = FMath::RandRange(MinBiteTime, MaxBiteTime);

	// ???úÏÑ±??
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
		// Î¨ºÍ≥†Í∏??†ÌÉù Î∞??ùÏÑ±
		FCaughtFish CaughtFish = SelectFishFromSpawnTable();

		// ?±Îä• ?êÏàò???∞Îùº ?àÏßà Ï°∞Ï†ï
		CaughtFish.QualityScore = FMath::Clamp(PerformanceScore * 100.0f, 0.0f, 100.0f);

		// ?ÑÎ≤Ω???°Í∏∞?∏Ï? ?ïÏù∏
		bool bPerfectCatch = PerformanceScore >= 0.95f;

		// ?ÑÍ∞ê???±Î°ù
		RegisterFishToCollection(CaughtFish);

		// Í≤ΩÌóòÏπ?Í≥ÑÏÇ∞
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

	// ?öÏãú Ï¢ÖÎ£å
	CancelFishing();
}

// Note: AddFishingExperience is now a wrapper in the header that calls base class AddExperience()
// Note: GetExperienceForNextLevel is now provided by base class

void UHarmoniaFishingComponent::RegisterFishToCollection(const FCaughtFish& Fish)
{
	FishCollection.Add(Fish);

	// ÏµúÍ≥† Í∏∞Î°ù ?ÖÎç∞?¥Ìä∏
	if (!BestCatchRecords.Contains(Fish.FishID))
	{
		BestCatchRecords.Add(Fish.FishID, Fish);
	}
	else
	{
		FCaughtFish& BestRecord = BestCatchRecords[Fish.FishID];
		// Î¨¥Í≤å Í∏∞Ï??ºÎ°ú ÏµúÍ≥† Í∏∞Î°ù Í∞±Ïã†
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

	// ?àÎ≤® ?îÍµ¨?¨Ìï≠??ÎßåÏ°±?òÎäî Î¨ºÍ≥†Í∏∞Îßå ?ÑÌÑ∞Îß?(base class??Level ?¨Ïö©)
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

	// ?ïÎ•† Í∏∞Î∞ò ?†ÌÉù
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
			// Î¨ºÍ≥†Í∏??∞Ïù¥??Ï∞æÍ∏∞
			if (const FFishData* FishData = FishDatabase.Find(Entry.FishID))
			{
				return GenerateFish(Entry.FishID, *FishData);
			}
		}
	}

	// ?¥Î∞±: Ï≤?Î≤àÏß∏ Î¨ºÍ≥†Í∏??†ÌÉù
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

	Result.QualityScore = 50.0f; // Í∏∞Î≥∏Í∞? ÎØ∏ÎãàÍ≤åÏûÑ ?±Îä•???∞Îùº Ï°∞Ï†ï

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
