// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaBaseLifeContentComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UHarmoniaBaseLifeContentComponent::UHarmoniaBaseLifeContentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHarmoniaBaseLifeContentComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaBaseLifeContentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsActivityActive)
	{
		UpdateActivityTiming(DeltaTime);
	}
}

// ====================================
// Activity State Management
// ====================================

bool UHarmoniaBaseLifeContentComponent::StartActivity()
{
	if (bIsActivityActive)
	{
		return false;
	}

	bIsActivityActive = true;
	ActivityStartTime = GetWorld()->GetTimeSeconds();

	// Enable tick for activity updates
	SetComponentTickEnabled(true);

	OnActivityStarted.Broadcast();
	return true;
}

void UHarmoniaBaseLifeContentComponent::CancelActivity()
{
	if (!bIsActivityActive)
	{
		return;
	}

	bIsActivityActive = false;
	RequiredActivityTime = 0.0f;

	// Disable tick when not active
	SetComponentTickEnabled(false);

	OnActivityCancelled.Broadcast();
}

float UHarmoniaBaseLifeContentComponent::GetActivityProgress() const
{
	if (!bIsActivityActive || RequiredActivityTime <= 0.0f)
	{
		return 0.0f;
	}

	const float ElapsedTime = GetElapsedActivityTime();
	return FMath::Clamp(ElapsedTime / RequiredActivityTime, 0.0f, 1.0f);
}

// ====================================
// Level and Experience System
// ====================================

void UHarmoniaBaseLifeContentComponent::AddExperience(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	// Apply multiplier
	const int32 AdjustedAmount = FMath::RoundToInt(Amount * ExperienceMultiplier);
	CurrentExperience += AdjustedAmount;

	// Check for level up
	CheckAndProcessLevelUp();
}

int32 UHarmoniaBaseLifeContentComponent::GetExperienceForNextLevel() const
{
	return CalculateExperienceForLevel(Level + 1);
}

int32 UHarmoniaBaseLifeContentComponent::CalculateExperienceForLevel(int32 TargetLevel) const
{
	if (TargetLevel <= 1)
	{
		return 0;
	}

	// Experience formula: BaseExp * (Level ^ Exponent)
	const float LevelFactor = FMath::Pow(static_cast<float>(TargetLevel), ExperienceCurveExponent);
	return FMath::RoundToInt(BaseExperiencePerLevel * LevelFactor);
}

// ====================================
// Protected Functions
// ====================================

void UHarmoniaBaseLifeContentComponent::OnActivityComplete()
{
	if (!bIsActivityActive)
	{
		return;
	}

	bIsActivityActive = false;
	RequiredActivityTime = 0.0f;

	// Disable tick when not active
	SetComponentTickEnabled(false);

	OnActivityCompleted.Broadcast();
}

void UHarmoniaBaseLifeContentComponent::OnLevelUpInternal(int32 NewLevel)
{
	// Subclasses can override for additional logic
}

void UHarmoniaBaseLifeContentComponent::CheckAndProcessLevelUp()
{
	int32 RequiredExp = GetExperienceForNextLevel();

	while (CurrentExperience >= RequiredExp && RequiredExp > 0)
	{
		CurrentExperience -= RequiredExp;
		Level++;

		// Calculate skill points (can be customized by subclasses)
		const int32 SkillPoints = 1;

		// Broadcast level up event
		OnLevelUp.Broadcast(Level, SkillPoints);

		// Call subclass hook
		OnLevelUpInternal(Level);

		// Update required exp for next level
		RequiredExp = GetExperienceForNextLevel();
	}
}

float UHarmoniaBaseLifeContentComponent::GetElapsedActivityTime() const
{
	if (!bIsActivityActive || !GetWorld())
	{
		return 0.0f;
	}

	return GetWorld()->GetTimeSeconds() - ActivityStartTime;
}

// ====================================
// Private Functions
// ====================================

void UHarmoniaBaseLifeContentComponent::UpdateActivityTiming(float DeltaTime)
{
	// Auto-complete activity when time is up
	if (RequiredActivityTime > 0.0f)
	{
		const float ElapsedTime = GetElapsedActivityTime();
		if (ElapsedTime >= RequiredActivityTime)
		{
			OnActivityComplete();
		}
	}
}
