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
		// 물고기가 물기를 기다림
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

	// 레벨 체크
	if (FishingLevel < FishingSpot->MinimumFishingLevel)
	{
		return false;
	}

	CurrentFishingSpot = FishingSpot;
	bIsFishing = true;
	bMinigameActive = false;
	FishingStartTime = GetWorld()->GetTimeSeconds();

	// 물고기가 물기까지 랜덤 시간 설정
	TimeUntilBite = FMath::RandRange(MinBiteTime, MaxBiteTime);

	// 틱 활성화
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
		// 물고기 선택 및 생성
		FCaughtFish CaughtFish = SelectFishFromSpawnTable();

		// 성능 점수에 따라 품질 조정
		CaughtFish.QualityScore = FMath::Clamp(PerformanceScore * 100.0f, 0.0f, 100.0f);

		// 완벽한 잡기인지 확인
		bool bPerfectCatch = PerformanceScore >= 0.95f;

		// 도감에 등록
		RegisterFishToCollection(CaughtFish);

		// 경험치 계산
		int32 BaseExp = static_cast<int32>(CaughtFish.QualityScore);
		if (bPerfectCatch)
		{
			BaseExp = FMath::CeilToInt(BaseExp * CurrentMinigameSettings.PerfectBonus);
		}
		else if (bSuccess)
		{
			BaseExp = FMath::CeilToInt(BaseExp * CurrentMinigameSettings.SuccessBonus);
		}

		AddFishingExperience(BaseExp);

		OnFishCaught.Broadcast(CaughtFish, bPerfectCatch);
	}
	else
	{
		OnFishEscaped.Broadcast();
	}

	// 낚시 종료
	CancelFishing();
}

void UHarmoniaFishingComponent::AddFishingExperience(int32 Amount)
{
	int32 ModifiedAmount = FMath::CeilToInt(Amount * ExperienceMultiplier);
	CurrentExperience += ModifiedAmount;

	CheckAndProcessLevelUp();
}

int32 UHarmoniaFishingComponent::GetExperienceForNextLevel() const
{
	return BaseExperiencePerLevel * FishingLevel;
}

void UHarmoniaFishingComponent::RegisterFishToCollection(const FCaughtFish& Fish)
{
	FishCollection.Add(Fish);

	// 최고 기록 업데이트
	if (!BestCatchRecords.Contains(Fish.FishID))
	{
		BestCatchRecords.Add(Fish.FishID, Fish);
	}
	else
	{
		FCaughtFish& BestRecord = BestCatchRecords[Fish.FishID];
		// 무게 기준으로 최고 기록 갱신
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

	// 레벨 요구사항을 만족하는 물고기만 필터링
	TArray<FFishingSpotSpawnEntry> ValidFish;
	for (const FFishingSpotSpawnEntry& Entry : CurrentFishingSpot->SpawnTable)
	{
		if (FishingLevel >= Entry.MinFishingLevel)
		{
			ValidFish.Add(Entry);
		}
	}

	if (ValidFish.Num() == 0)
	{
		return FCaughtFish();
	}

	// 확률 기반 선택
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
			// 물고기 데이터 찾기
			if (const FFishData* FishData = FishDatabase.Find(Entry.FishID))
			{
				return GenerateFish(Entry.FishID, *FishData);
			}
		}
	}

	// 폴백: 첫 번째 물고기 선택
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

	Result.QualityScore = 50.0f; // 기본값, 미니게임 성능에 따라 조정

	return Result;
}

void UHarmoniaFishingComponent::CheckAndProcessLevelUp()
{
	int32 ExpNeeded = GetExperienceForNextLevel();

	while (CurrentExperience >= ExpNeeded)
	{
		CurrentExperience -= ExpNeeded;
		FishingLevel++;

		OnFishingLevelUp.Broadcast(FishingLevel, 1);

		ExpNeeded = GetExperienceForNextLevel();
	}
}
