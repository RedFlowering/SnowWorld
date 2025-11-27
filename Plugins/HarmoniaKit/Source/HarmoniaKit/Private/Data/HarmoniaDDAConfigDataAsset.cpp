// Copyright 2025 Snow Game Studio.

#include "Data/HarmoniaDDAConfigDataAsset.h"

UHarmoniaDDAConfigDataAsset::UHarmoniaDDAConfigDataAsset()
{
	// 기본 ?�로???�정
	
	// Beginner - 가???��?
	BeginnerProfile.ProfileName = TEXT("Beginner");
	BeginnerProfile.BaseParameters.EnemyHealthMultiplier = 0.7f;
	BeginnerProfile.BaseParameters.EnemyDamageMultiplier = 0.6f;
	BeginnerProfile.BaseParameters.EnemyAggressionMultiplier = 0.6f;
	BeginnerProfile.BaseParameters.PlayerDamageMultiplier = 1.3f;
	BeginnerProfile.BaseParameters.PlayerDefenseMultiplier = 1.4f;
	BeginnerProfile.BaseParameters.ParryWindowMultiplier = 1.5f;
	BeginnerProfile.BaseParameters.DodgeIFrameMultiplier = 1.5f;
	BeginnerProfile.BaseParameters.HealingMultiplier = 1.5f;
	BeginnerProfile.BaseParameters.ItemDropRateMultiplier = 1.5f;
	BeginnerProfile.BaseParameters.AIDelayBetweenAttacks = 0.5f;
	
	// Learning - ?��?
	LearningProfile.ProfileName = TEXT("Learning");
	LearningProfile.BaseParameters.EnemyHealthMultiplier = 0.85f;
	LearningProfile.BaseParameters.EnemyDamageMultiplier = 0.8f;
	LearningProfile.BaseParameters.EnemyAggressionMultiplier = 0.8f;
	LearningProfile.BaseParameters.PlayerDamageMultiplier = 1.15f;
	LearningProfile.BaseParameters.PlayerDefenseMultiplier = 1.2f;
	LearningProfile.BaseParameters.ParryWindowMultiplier = 1.25f;
	LearningProfile.BaseParameters.DodgeIFrameMultiplier = 1.25f;
	LearningProfile.BaseParameters.HealingMultiplier = 1.25f;
	LearningProfile.BaseParameters.ItemDropRateMultiplier = 1.25f;
	LearningProfile.BaseParameters.AIDelayBetweenAttacks = 0.25f;
	
	// Standard - 기본
	StandardProfile.ProfileName = TEXT("Standard");
	StandardProfile.BaseParameters.EnemyHealthMultiplier = 1.0f;
	StandardProfile.BaseParameters.EnemyDamageMultiplier = 1.0f;
	StandardProfile.BaseParameters.EnemyAggressionMultiplier = 1.0f;
	StandardProfile.BaseParameters.PlayerDamageMultiplier = 1.0f;
	StandardProfile.BaseParameters.PlayerDefenseMultiplier = 1.0f;
	StandardProfile.BaseParameters.ParryWindowMultiplier = 1.0f;
	StandardProfile.BaseParameters.DodgeIFrameMultiplier = 1.0f;
	StandardProfile.BaseParameters.HealingMultiplier = 1.0f;
	StandardProfile.BaseParameters.ItemDropRateMultiplier = 1.0f;
	StandardProfile.BaseParameters.AIDelayBetweenAttacks = 0.0f;
	
	// Skilled - ?�려?�
	SkilledProfile.ProfileName = TEXT("Skilled");
	SkilledProfile.BaseParameters.EnemyHealthMultiplier = 1.15f;
	SkilledProfile.BaseParameters.EnemyDamageMultiplier = 1.2f;
	SkilledProfile.BaseParameters.EnemyAggressionMultiplier = 1.2f;
	SkilledProfile.BaseParameters.PlayerDamageMultiplier = 0.9f;
	SkilledProfile.BaseParameters.PlayerDefenseMultiplier = 0.9f;
	SkilledProfile.BaseParameters.ParryWindowMultiplier = 0.85f;
	SkilledProfile.BaseParameters.DodgeIFrameMultiplier = 0.9f;
	SkilledProfile.BaseParameters.HealingMultiplier = 0.9f;
	SkilledProfile.BaseParameters.ItemDropRateMultiplier = 1.1f;
	SkilledProfile.BaseParameters.AIDelayBetweenAttacks = 0.0f;
	
	// Master - 가???�려?�
	MasterProfile.ProfileName = TEXT("Master");
	MasterProfile.BaseParameters.EnemyHealthMultiplier = 1.3f;
	MasterProfile.BaseParameters.EnemyDamageMultiplier = 1.4f;
	MasterProfile.BaseParameters.EnemyAggressionMultiplier = 1.4f;
	MasterProfile.BaseParameters.PlayerDamageMultiplier = 0.8f;
	MasterProfile.BaseParameters.PlayerDefenseMultiplier = 0.8f;
	MasterProfile.BaseParameters.ParryWindowMultiplier = 0.7f;
	MasterProfile.BaseParameters.DodgeIFrameMultiplier = 0.8f;
	MasterProfile.BaseParameters.HealingMultiplier = 0.8f;
	MasterProfile.BaseParameters.ItemDropRateMultiplier = 1.3f;
	MasterProfile.BaseParameters.AIDelayBetweenAttacks = 0.0f;
}

const FHarmoniaDDAProfile& UHarmoniaDDAConfigDataAsset::GetProfileForSkillRating(float SkillRating) const
{
	if (SkillRating <= Thresholds.BeginnerMaxRating)
	{
		return BeginnerProfile;
	}
	else if (SkillRating <= Thresholds.LearningMaxRating)
	{
		return LearningProfile;
	}
	else if (SkillRating <= Thresholds.StandardMaxRating)
	{
		return StandardProfile;
	}
	else if (SkillRating <= Thresholds.SkilledMaxRating)
	{
		return SkilledProfile;
	}
	else
	{
		return MasterProfile;
	}
}

FString UHarmoniaDDAConfigDataAsset::GetProfileName(float SkillRating) const
{
	return GetProfileForSkillRating(SkillRating).ProfileName.ToString();
}
