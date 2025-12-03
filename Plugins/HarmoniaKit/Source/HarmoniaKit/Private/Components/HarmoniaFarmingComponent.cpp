// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaFarmingComponent.h"

UHarmoniaFarmingComponent::UHarmoniaFarmingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	LastUpdateTime = FDateTime::Now();
}

void UHarmoniaFarmingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaFarmingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FDateTime CurrentTime = FDateTime::Now();
	FTimespan TimeSinceLastUpdate = CurrentTime - LastUpdateTime;

	// 10초마???�물 ?�장 ?�데?�트
	if (TimeSinceLastUpdate.GetTotalSeconds() >= 10.0)
	{
		for (auto& PlotPair : FarmPlots)
		{
			UpdateCropGrowth(PlotPair.Key, PlotPair.Value);
		}

		LastUpdateTime = CurrentTime;
	}
}

bool UHarmoniaFarmingComponent::PlantCrop(FName CropID, FName PlotID)
{
	if (!CropDatabase.Contains(CropID))
	{
		return false;
	}

	// �?�� 존재?�는지 ?�인
	if (!PlotSoilData.Contains(PlotID))
	{
		return false;
	}

	// ?��? ?�물???�는지 ?�인
	if (FarmPlots.Contains(PlotID))
	{
		return false;
	}

	const FCropData& CropData = CropDatabase[CropID];

	// 계절 ?�인
	if (!CanGrowInCurrentSeason(CropID))
	{
		return false;
	}

	// ?�물 ?�스?�스 ?�성
	FCropInstance NewCrop;
	NewCrop.CropID = CropID;
	NewCrop.CurrentStage = ECropGrowthStage::Seed;
	NewCrop.PlantedTime = FDateTime::Now();
	NewCrop.StageStartTime = FDateTime::Now();
	NewCrop.LastWateredTime = FDateTime::Now();
	NewCrop.Soil = PlotSoilData[PlotID];
	NewCrop.QualityScore = 50.0f;

	FarmPlots.Add(PlotID, NewCrop);

	OnCropPlanted.Broadcast(CropID, NewCrop);

	return true;
}

FHarvestResult UHarmoniaFarmingComponent::HarvestCrop(FName PlotID)
{
	FHarvestResult Result;

	if (!FarmPlots.Contains(PlotID))
	{
		return Result;
	}

	FCropInstance& Crop = FarmPlots[PlotID];

	if (Crop.CurrentStage != ECropGrowthStage::Harvest)
	{
		return Result;
	}

	if (!CropDatabase.Contains(Crop.CropID))
	{
		return Result;
	}

	const FCropData& CropData = CropDatabase[Crop.CropID];

	// ?�확 결과 계산
	Result = CalculateHarvestResult(Crop, CropData);

	// 경험�??�득
	AddFarmingExperience(Result.Experience);

	OnCropHarvested.Broadcast(Result);

	// ?�수??가?�한 ?�물?��? ?�인
	if (CropData.bReharvestablle)
	{
		Crop.CurrentStage = ECropGrowthStage::Growing;
		Crop.StageStartTime = FDateTime::Now();
		Crop.HarvestCount++;
	}
	else
	{
		// ?�물 ?�거
		FarmPlots.Remove(PlotID);
	}

	return Result;
}

bool UHarmoniaFarmingComponent::WaterCrop(FName PlotID)
{
	if (!FarmPlots.Contains(PlotID))
	{
		return false;
	}

	FCropInstance& Crop = FarmPlots[PlotID];

	// ?�분 증�?
	Crop.Soil.MoistureLevel = FMath::Min(100.0f, Crop.Soil.MoistureLevel + WaterMoistureIncrease);
	Crop.LastWateredTime = FDateTime::Now();

	OnCropWatered.Broadcast(Crop.CropID, Crop.Soil.MoistureLevel);

	return true;
}

bool UHarmoniaFarmingComponent::ApplyFertilizer(FName PlotID, const FFertilizerData& Fertilizer)
{
	if (!FarmPlots.Contains(PlotID))
	{
		return false;
	}

	FCropInstance& Crop = FarmPlots[PlotID];

	Crop.Soil.bFertilized = true;
	Crop.Soil.FertilizerDuration = Fertilizer.Duration;
	Crop.Soil.GrowthSpeedBonus = Fertilizer.GrowthSpeedBonus;
	Crop.Soil.YieldBonus = Fertilizer.YieldBonus;

	OnFertilizerApplied.Broadcast(Crop.CropID, Fertilizer);

	return true;
}

void UHarmoniaFarmingComponent::RemoveCrop(FName PlotID)
{
	FarmPlots.Remove(PlotID);
}

bool UHarmoniaFarmingComponent::GetCropInstance(FName PlotID, FCropInstance& OutCrop) const
{
	if (const FCropInstance* Crop = FarmPlots.Find(PlotID))
	{
		OutCrop = *Crop;
		return true;
	}
	return false;
}

bool UHarmoniaFarmingComponent::CanHarvest(FName PlotID) const
{
	if (const FCropInstance* Crop = FarmPlots.Find(PlotID))
	{
		return Crop->CurrentStage == ECropGrowthStage::Harvest;
	}
	return false;
}

void UHarmoniaFarmingComponent::AddFarmPlot(FName PlotID, const FSoilData& SoilData)
{
	PlotSoilData.Add(PlotID, SoilData);
}

void UHarmoniaFarmingComponent::RemoveFarmPlot(FName PlotID)
{
	PlotSoilData.Remove(PlotID);
	FarmPlots.Remove(PlotID);
}

TArray<FName> UHarmoniaFarmingComponent::GetAllPlots() const
{
	TArray<FName> AllPlots;
	PlotSoilData.GetKeys(AllPlots);
	return AllPlots;
}

TArray<FName> UHarmoniaFarmingComponent::GetEmptyPlots() const
{
	TArray<FName> EmptyPlots;

	for (const auto& PlotPair : PlotSoilData)
	{
		if (!FarmPlots.Contains(PlotPair.Key))
		{
			EmptyPlots.Add(PlotPair.Key);
		}
	}

	return EmptyPlots;
}

void UHarmoniaFarmingComponent::AddFarmingExperience(int32 Amount)
{
	int32 ModifiedAmount = FMath::CeilToInt(Amount * ExperienceMultiplier);
	CurrentExperience += ModifiedAmount;

	CheckAndProcessLevelUp();
}

void UHarmoniaFarmingComponent::SetCurrentSeason(ESeason Season)
{
	CurrentSeason = Season;

	// 계절??맞�? ?�는 ?�물?� ?�장 중�? ?�는 ?�널???�용 가??
	// ?�요??구현
}

bool UHarmoniaFarmingComponent::CanGrowInCurrentSeason(FName CropID) const
{
	if (!CropDatabase.Contains(CropID))
	{
		return false;
	}

	const FCropData& CropData = CropDatabase[CropID];

	// 모든 계절???�라???�물
	if (CropData.GrowingSeasons.Contains(ESeason::AllSeasons))
	{
		return true;
	}

	return CropData.GrowingSeasons.Contains(CurrentSeason);
}

void UHarmoniaFarmingComponent::UpdateCropGrowth(FName PlotID, FCropInstance& Crop)
{
	if (!CropDatabase.Contains(Crop.CropID))
	{
		return;
	}

	const FCropData& CropData = CropDatabase[Crop.CropID];

	// ?�장 가???��? ?�인
	if (!CanCropGrow(Crop, CropData))
	{
		return;
	}

	FDateTime CurrentTime = FDateTime::Now();
	FTimespan TimeSinceStageStart = CurrentTime - Crop.StageStartTime;
	FTimespan TimeSinceWatered = CurrentTime - Crop.LastWateredTime;

	// ?�분 감소
	float HoursSinceWatered = TimeSinceWatered.GetTotalHours();
	Crop.Soil.MoistureLevel = FMath::Max(0.0f, Crop.Soil.MoistureLevel - (MoistureDecayPerHour * HoursSinceWatered / 10.0f));

	// 비료 ?�과 감소
	if (Crop.Soil.bFertilized)
	{
		Crop.Soil.FertilizerDuration -= TimeSinceStageStart.GetTotalSeconds();
		if (Crop.Soil.FertilizerDuration <= 0.0f)
		{
			Crop.Soil.bFertilized = false;
			Crop.Soil.GrowthSpeedBonus = 0.0f;
			Crop.Soil.YieldBonus = 0.0f;
		}
	}

	// ?�장 ?�도 계산
	float GrowthSpeedMultiplier = 1.0f;

	// 비료 보너??
	if (Crop.Soil.bFertilized)
	{
		GrowthSpeedMultiplier += Crop.Soil.GrowthSpeedBonus / 100.0f;
	}

	// ?�분 ?�널??
	if (Crop.Soil.MoistureLevel < 30.0f)
	{
		GrowthSpeedMultiplier *= (1.0f - LowMoisturePenalty / 100.0f);
	}

	// ?�장 ?�계 ?�인 �??�데?�트
	ECropGrowthStage OldStage = Crop.CurrentStage;
	ECropGrowthStage NewStage = OldStage;

	float RequiredTime = 0.0f;
	switch (Crop.CurrentStage)
	{
	case ECropGrowthStage::Seed:
		RequiredTime = CropData.SproutTime / GrowthSpeedMultiplier;
		if (TimeSinceStageStart.GetTotalSeconds() >= RequiredTime)
		{
			NewStage = ECropGrowthStage::Sprout;
		}
		break;

	case ECropGrowthStage::Sprout:
		RequiredTime = CropData.GrowingTime / GrowthSpeedMultiplier;
		if (TimeSinceStageStart.GetTotalSeconds() >= RequiredTime)
		{
			NewStage = ECropGrowthStage::Growing;
		}
		break;

	case ECropGrowthStage::Growing:
		RequiredTime = CropData.MaturityTime / GrowthSpeedMultiplier;
		if (TimeSinceStageStart.GetTotalSeconds() >= RequiredTime)
		{
			NewStage = ECropGrowthStage::Mature;
		}
		break;

	case ECropGrowthStage::Mature:
		// 바로 ?�확 가???�태�??�환
		NewStage = ECropGrowthStage::Harvest;
		break;

	case ECropGrowthStage::Harvest:
		// ?�확 ?�간 초과 ?�인
		if (TimeSinceStageStart.GetTotalSeconds() >= CropData.HarvestWindow)
		{
			NewStage = ECropGrowthStage::Withered;
		}
		break;

	case ECropGrowthStage::Withered:
		// ?�들?� ?�태 ?��?
		break;
	}

	if (NewStage != OldStage)
	{
		Crop.CurrentStage = NewStage;
		Crop.StageStartTime = CurrentTime;

		// ?�질 ?�수 ?�데?�트
		Crop.QualityScore = CalculateCropQuality(Crop, CropData);

		OnCropGrowthStageChanged.Broadcast(Crop.CropID, NewStage);
	}
}

float UHarmoniaFarmingComponent::CalculateCropQuality(const FCropInstance& Crop, const FCropData& CropData) const
{
	float Quality = 50.0f;

	// ?�분 ?�벨???�른 ?�질
	if (Crop.Soil.MoistureLevel >= 70.0f)
	{
		Quality += 20.0f;
	}
	else if (Crop.Soil.MoistureLevel >= 40.0f)
	{
		Quality += 10.0f;
	}
	else
	{
		Quality -= 10.0f;
	}

	// 비료 ?�용 ?��?
	if (Crop.Soil.bFertilized)
	{
		Quality += 15.0f;
	}

	// ?�양 ?�질
	switch (Crop.Soil.Quality)
	{
	case ESoilQuality::Poor:
		Quality -= 10.0f;
		break;
	case ESoilQuality::Good:
		Quality += 10.0f;
		break;
	case ESoilQuality::Excellent:
		Quality += 20.0f;
		break;
	case ESoilQuality::Perfect:
		Quality += 30.0f;
		break;
	default:
		break;
	}

	return FMath::Clamp(Quality, 0.0f, 100.0f);
}

FHarvestResult UHarmoniaFarmingComponent::CalculateHarvestResult(const FCropInstance& Crop, const FCropData& CropData)
{
	FHarvestResult Result;
	Result.CropID = Crop.CropID;
	Result.Quality = Crop.QualityScore;

	// ?�확??계산
	int32 BaseYield = FMath::RandRange(CropData.MinYield, CropData.MaxYield);
	float YieldMultiplier = 1.0f;

	// ?�질 보너??
	YieldMultiplier += (Crop.QualityScore - 50.0f) / 100.0f;

	// 비료 보너??
	if (Crop.Soil.bFertilized)
	{
		YieldMultiplier += Crop.Soil.YieldBonus / 100.0f;
	}

	Result.Quantity = FMath::Max(1, FMath::CeilToInt(BaseYield * YieldMultiplier));

	// 경험�?계산
	Result.Experience = FMath::CeilToInt(CropData.ExperienceReward * (Crop.QualityScore / 50.0f));

	// ?�앗 ?�득
	if (FMath::FRand() * 100.0f <= SeedDropChance)
	{
		Result.bGotSeeds = true;
		Result.SeedCount = FMath::RandRange(1, 3);
	}

	return Result;
}

void UHarmoniaFarmingComponent::CheckAndProcessLevelUp()
{
	int32 ExpNeeded = GetExperienceForNextLevel();

	while (CurrentExperience >= ExpNeeded)
	{
		CurrentExperience -= ExpNeeded;
		FarmingLevel++;

		OnFarmingLevelUp.Broadcast(FarmingLevel, 1);

		ExpNeeded = GetExperienceForNextLevel();
	}
}

bool UHarmoniaFarmingComponent::CanCropGrow(const FCropInstance& Crop, const FCropData& CropData) const
{
	// 계절 ?�인
	if (!CropData.GrowingSeasons.Contains(ESeason::AllSeasons) &&
		!CropData.GrowingSeasons.Contains(CurrentSeason))
	{
		return false;
	}

	// ?�들?�으�??�장 불�?
	if (Crop.CurrentStage == ECropGrowthStage::Withered)
	{
		return false;
	}

	return true;
}
