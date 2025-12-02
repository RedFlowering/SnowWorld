// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaGatheringComponent.h"
#include "Kismet/KismetMathLibrary.h"

UHarmoniaGatheringComponent::UHarmoniaGatheringComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// 기본 ?�벨 초기??
	GatheringLevels.Add(EGatheringResourceType::Mineral, 1);
	GatheringLevels.Add(EGatheringResourceType::Herb, 1);
	GatheringLevels.Add(EGatheringResourceType::Wood, 1);
	GatheringLevels.Add(EGatheringResourceType::Fiber, 1);
	GatheringLevels.Add(EGatheringResourceType::Stone, 1);
	GatheringLevels.Add(EGatheringResourceType::Crystal, 1);
	GatheringLevels.Add(EGatheringResourceType::Flower, 1);
	GatheringLevels.Add(EGatheringResourceType::Mushroom, 1);

	// 경험�?초기??
	GatheringExperience.Add(EGatheringResourceType::Mineral, 0);
	GatheringExperience.Add(EGatheringResourceType::Herb, 0);
	GatheringExperience.Add(EGatheringResourceType::Wood, 0);
	GatheringExperience.Add(EGatheringResourceType::Fiber, 0);
	GatheringExperience.Add(EGatheringResourceType::Stone, 0);
	GatheringExperience.Add(EGatheringResourceType::Crystal, 0);
	GatheringExperience.Add(EGatheringResourceType::Flower, 0);
	GatheringExperience.Add(EGatheringResourceType::Mushroom, 0);
}

void UHarmoniaGatheringComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaGatheringComponent::OnActivityComplete()
{
	Super::OnActivityComplete();

	// Gathering-specific completion logic
	bIsGathering = false;
}

void UHarmoniaGatheringComponent::OnLevelUpInternal(int32 NewLevel)
{
	Super::OnLevelUpInternal(NewLevel);

	// Gathering-specific level up logic
	// Could add resource type-specific bonuses here
}

void UHarmoniaGatheringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsGathering)
	{
		float ElapsedTime = GetWorld()->GetTimeSeconds() - GatheringStartTime;
		if (ElapsedTime >= RequiredGatheringTime)
		{
			CompleteGathering();
		}
	}
}

bool UHarmoniaGatheringComponent::StartGathering(FName ResourceID, AActor* TargetNode)
{
	if (bIsGathering || !ResourceDatabase.Contains(ResourceID))
	{
		return false;
	}

	const FGatheringResourceData& ResourceData = ResourceDatabase[ResourceID];

	// ?�벨 체크
	int32 CurrentLevel = GetGatheringLevel(ResourceData.ResourceType);
	if (CurrentLevel < ResourceData.MinGatheringLevel)
	{
		return false;
	}

	// ?�구 체크
	if (ResourceData.RequiredTool != EGatheringToolType::None)
	{
		if (!HasToolEquipped() || EquippedTool.ToolType != ResourceData.RequiredTool)
		{
			return false;
		}

		if (EquippedTool.ToolTier < ResourceData.MinToolTier)
		{
			return false;
		}
	}

	CurrentResourceID = ResourceID;
	bIsGathering = true;
	TargetGatheringNode = TargetNode;
	GatheringStartTime = GetWorld()->GetTimeSeconds();
	RequiredGatheringTime = CalculateGatheringTime(ResourceData.GatheringTime);

	SetComponentTickEnabled(true);

	OnGatheringStarted.Broadcast(ResourceID, RequiredGatheringTime, ResourceData.ResourceType);

	return true;
}

void UHarmoniaGatheringComponent::CancelGathering()
{
	if (!bIsGathering)
	{
		return;
	}

	bIsGathering = false;
	CurrentResourceID = NAME_None;
	TargetGatheringNode = nullptr;
	SetComponentTickEnabled(false);

	OnGatheringCancelled.Broadcast();
}

float UHarmoniaGatheringComponent::GetGatheringProgress() const
{
	if (!bIsGathering || RequiredGatheringTime <= 0.0f)
	{
		return 0.0f;
	}

	float ElapsedTime = GetWorld()->GetTimeSeconds() - GatheringStartTime;
	return FMath::Clamp(ElapsedTime / RequiredGatheringTime, 0.0f, 1.0f);
}

bool UHarmoniaGatheringComponent::CanGatherResource(FName ResourceID) const
{
	if (!ResourceDatabase.Contains(ResourceID))
	{
		return false;
	}

	const FGatheringResourceData& ResourceData = ResourceDatabase[ResourceID];

	// ?�벨 체크
	int32 CurrentLevel = GetGatheringLevel(ResourceData.ResourceType);
	if (CurrentLevel < ResourceData.MinGatheringLevel)
	{
		return false;
	}

	// ?�구 체크
	if (ResourceData.RequiredTool != EGatheringToolType::None)
	{
		if (!HasToolEquipped() || EquippedTool.ToolType != ResourceData.RequiredTool)
		{
			return false;
		}

		if (EquippedTool.ToolTier < ResourceData.MinToolTier)
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaGatheringComponent::EquipTool(const FGatheringToolData& Tool)
{
	EquippedTool = Tool;
}

void UHarmoniaGatheringComponent::UnequipTool()
{
	EquippedTool = FGatheringToolData();
}

bool UHarmoniaGatheringComponent::HasToolEquipped() const
{
	return EquippedTool.ToolType != EGatheringToolType::None;
}

void UHarmoniaGatheringComponent::ReduceToolDurability(int32 Amount)
{
	if (!HasToolEquipped())
	{
		return;
	}

	EquippedTool.Durability = FMath::Max(0, EquippedTool.Durability - Amount);

	OnToolDurabilityChanged.Broadcast(EquippedTool.ToolName, EquippedTool.Durability);

	// ?�구?��? 0???�면 ?�구 ?�제
	if (EquippedTool.Durability <= 0)
	{
		UnequipTool();
	}
}

void UHarmoniaGatheringComponent::AddGatheringExperience(int32 Amount, EGatheringResourceType ResourceType)
{
	int32 ModifiedAmount = FMath::CeilToInt(Amount * ExperienceMultiplier);

	if (int32* CurrentExp = GatheringExperience.Find(ResourceType))
	{
		*CurrentExp += ModifiedAmount;
		CheckAndProcessLevelUp(ResourceType);
	}
}

int32 UHarmoniaGatheringComponent::GetGatheringLevel(EGatheringResourceType ResourceType) const
{
	if (const int32* FoundLevel = GatheringLevels.Find(ResourceType))
	{
		return *FoundLevel;
	}
	return 1;
}

int32 UHarmoniaGatheringComponent::GetCurrentExperience(EGatheringResourceType ResourceType) const
{
	if (const int32* Exp = GatheringExperience.Find(ResourceType))
	{
		return *Exp;
	}
	return 0;
}

int32 UHarmoniaGatheringComponent::GetExperienceForNextLevel(EGatheringResourceType ResourceType) const
{
	int32 CurrentLevel = GetGatheringLevel(ResourceType);
	return BaseExperiencePerLevel * CurrentLevel;
}

void UHarmoniaGatheringComponent::AddTrait(const FGatheringTrait& Trait)
{
	// 중복 체크
	for (const FGatheringTrait& ExistingTrait : ActiveTraits)
	{
		if (ExistingTrait.TraitName == Trait.TraitName)
		{
			return;
		}
	}

	ActiveTraits.Add(Trait);
}

void UHarmoniaGatheringComponent::RemoveTrait(FName TraitName)
{
	ActiveTraits.RemoveAll([TraitName](const FGatheringTrait& Trait)
	{
		return Trait.TraitName == TraitName;
	});
}

float UHarmoniaGatheringComponent::GetTotalGatheringSpeedBonus() const
{
	float TotalBonus = 0.0f;

	// ?�성 보너??
	for (const FGatheringTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.GatheringSpeedBonus;
	}

	// ?�구 보너??
	if (HasToolEquipped())
	{
		TotalBonus += (EquippedTool.GatheringSpeedMultiplier - 1.0f) * 100.0f;
	}

	return TotalBonus;
}

float UHarmoniaGatheringComponent::GetTotalYieldBonus() const
{
	float TotalBonus = 0.0f;

	// ?�성 보너??
	for (const FGatheringTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.YieldBonus;
	}

	// ?�구 보너??
	if (HasToolEquipped())
	{
		TotalBonus += EquippedTool.YieldBonus * 100.0f;
	}

	return TotalBonus;
}

float UHarmoniaGatheringComponent::GetCriticalChance() const
{
	float TotalChance = BaseCriticalChance;

	for (const FGatheringTrait& Trait : ActiveTraits)
	{
		TotalChance += Trait.CriticalChanceBonus;
	}

	return FMath::Clamp(TotalChance, 0.0f, 100.0f);
}

void UHarmoniaGatheringComponent::CompleteGathering()
{
	if (!ResourceDatabase.Contains(CurrentResourceID))
	{
		CancelGathering();
		return;
	}

	const FGatheringResourceData& ResourceData = ResourceDatabase[CurrentResourceID];

	// 채집 결과 계산
	FGatheringResult Result = CalculateGatheringResult(ResourceData);

	// 경험�??�득
	AddGatheringExperience(Result.Experience, ResourceData.ResourceType);

	// ?�구 ?�구??감소
	if (HasToolEquipped())
	{
		int32 DurabilityLoss = 1;
		
		// ?�성?�로 ?�구??감소???�용
		for (const FGatheringTrait& Trait : ActiveTraits)
		{
			DurabilityLoss = FMath::CeilToInt(DurabilityLoss * (1.0f - Trait.DurabilityReduction / 100.0f));
		}

		ReduceToolDurability(FMath::Max(1, DurabilityLoss));
	}

	OnGatheringCompleted.Broadcast(Result);

	// 채집 종료
	bIsGathering = false;
	CurrentResourceID = NAME_None;
	TargetGatheringNode = nullptr;
	SetComponentTickEnabled(false);
}

FGatheringResult UHarmoniaGatheringComponent::CalculateGatheringResult(const FGatheringResourceData& ResourceData)
{
	FGatheringResult Result;
	Result.ResourceID = CurrentResourceID;

	// ?�리?�컬 체크
	float CritChance = GetCriticalChance();
	Result.bCriticalGather = FMath::FRand() * 100.0f <= CritChance;

	// ?�득??계산
	int32 BaseYield = FMath::RandRange(ResourceData.MinYield, ResourceData.MaxYield);
	float YieldMultiplier = 1.0f + (GetTotalYieldBonus() / 100.0f);

	if (Result.bCriticalGather)
	{
		YieldMultiplier *= CriticalYieldMultiplier;
	}

	Result.Quantity = FMath::CeilToInt(BaseYield * YieldMultiplier);

	// 경험�?계산
	float ExpMultiplier = 1.0f;
	for (const FGatheringTrait& Trait : ActiveTraits)
	{
		ExpMultiplier += Trait.ExperienceBonus / 100.0f;
	}

	Result.Experience = FMath::CeilToInt(ResourceData.ExperienceReward * ExpMultiplier);

	return Result;
}

void UHarmoniaGatheringComponent::CheckAndProcessLevelUp(EGatheringResourceType ResourceType)
{
	int32* CurrentExp = GatheringExperience.Find(ResourceType);
	int32* CurrentLevel = GatheringLevels.Find(ResourceType);

	if (!CurrentExp || !CurrentLevel)
	{
		return;
	}

	int32 ExpNeeded = GetExperienceForNextLevel(ResourceType);

	while (*CurrentExp >= ExpNeeded)
	{
		*CurrentExp -= ExpNeeded;
		(*CurrentLevel)++;

		OnGatheringLevelUp.Broadcast(*CurrentLevel, 1);

		ExpNeeded = GetExperienceForNextLevel(ResourceType);
	}
}

float UHarmoniaGatheringComponent::CalculateGatheringTime(float BaseTime) const
{
	float SpeedBonus = GetTotalGatheringSpeedBonus();
	float TimeMultiplier = 1.0f / (1.0f + SpeedBonus / 100.0f);

	return BaseTime * TimeMultiplier;
}
