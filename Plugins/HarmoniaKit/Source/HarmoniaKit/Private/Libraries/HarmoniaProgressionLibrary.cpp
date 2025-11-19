// Copyright 2025 Snow Game Studio.

#include "Libraries/HarmoniaProgressionLibrary.h"
#include "Components/HarmoniaProgressionComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

//~ Component Access

UHarmoniaProgressionComponent* UHarmoniaProgressionLibrary::GetProgressionComponent(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UHarmoniaProgressionComponent>();
}

UHarmoniaProgressionComponent* UHarmoniaProgressionLibrary::GetProgressionComponentFromPlayerState(APlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return nullptr;
	}

	return PlayerState->FindComponentByClass<UHarmoniaProgressionComponent>();
}

UHarmoniaProgressionComponent* UHarmoniaProgressionLibrary::GetLocalPlayerProgressionComponent(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	APlayerState* PS = PC->GetPlayerState<APlayerState>();
	if (!PS)
	{
		return nullptr;
	}

	return GetProgressionComponentFromPlayerState(PS);
}

//~ Experience & Leveling Helpers

int32 UHarmoniaProgressionLibrary::CalculateTotalExperienceForLevelRange(int32 StartLevel, int32 EndLevel)
{
	int32 TotalExp = 0;
	for (int32 Level = StartLevel; Level < EndLevel; ++Level)
	{
		// Use same formula as progression component
		const float BaseExp = 100.0f;
		const float Exponent = 1.5f;
		const float Multiplier = 1.1f;
		TotalExp += FMath::RoundToInt(BaseExp * FMath::Pow(Level, Exponent) * Multiplier);
	}
	return TotalExp;
}

int32 UHarmoniaProgressionLibrary::GetLevelFromTotalExperience(int32 TotalExperience)
{
	int32 Level = 1;
	int32 AccumulatedExp = 0;

	while (AccumulatedExp < TotalExperience && Level < 100)
	{
		const float BaseExp = 100.0f;
		const float Exponent = 1.5f;
		const float Multiplier = 1.1f;
		AccumulatedExp += FMath::RoundToInt(BaseExp * FMath::Pow(Level, Exponent) * Multiplier);
		Level++;
	}

	return Level - 1;
}

FText UHarmoniaProgressionLibrary::FormatExperienceText(int32 CurrentExp, int32 RequiredExp)
{
	return FText::Format(
		FText::FromString("{0} / {1}"),
		FText::AsNumber(CurrentExp),
		FText::AsNumber(RequiredExp)
	);
}

int32 UHarmoniaProgressionLibrary::CalculateEnemyExperienceReward(int32 EnemyLevel, int32 PlayerLevel, float BaseReward)
{
	// Calculate exp with level difference modifier
	const int32 LevelDiff = EnemyLevel - PlayerLevel;
	float Multiplier = 1.0f;

	if (LevelDiff > 0)
	{
		// Higher level enemies give bonus exp
		Multiplier = 1.0f + (LevelDiff * 0.1f);
	}
	else if (LevelDiff < -5)
	{
		// Much lower level enemies give reduced exp
		Multiplier = FMath::Max(0.1f, 1.0f + (LevelDiff * 0.1f));
	}

	return FMath::RoundToInt(BaseReward * EnemyLevel * Multiplier);
}

//~ Class System Helpers

FText UHarmoniaProgressionLibrary::GetClassName(EHarmoniaCharacterClass ClassType)
{
	switch (ClassType)
	{
	case EHarmoniaCharacterClass::None: return FText::FromString("No Class");
	case EHarmoniaCharacterClass::Warrior: return FText::FromString("Warrior");
	case EHarmoniaCharacterClass::Rogue: return FText::FromString("Rogue");
	case EHarmoniaCharacterClass::Mage: return FText::FromString("Mage");
	case EHarmoniaCharacterClass::Berserker: return FText::FromString("Berserker");
	case EHarmoniaCharacterClass::Knight: return FText::FromString("Knight");
	case EHarmoniaCharacterClass::Paladin: return FText::FromString("Paladin");
	case EHarmoniaCharacterClass::Assassin: return FText::FromString("Assassin");
	case EHarmoniaCharacterClass::Ranger: return FText::FromString("Ranger");
	case EHarmoniaCharacterClass::Duelist: return FText::FromString("Duelist");
	case EHarmoniaCharacterClass::Sorcerer: return FText::FromString("Sorcerer");
	case EHarmoniaCharacterClass::Cleric: return FText::FromString("Cleric");
	case EHarmoniaCharacterClass::Warlock: return FText::FromString("Warlock");
	default: return FText::FromString("Unknown");
	}
}

FText UHarmoniaProgressionLibrary::GetClassDescription(EHarmoniaCharacterClass ClassType)
{
	switch (ClassType)
	{
	case EHarmoniaCharacterClass::Warrior:
		return FText::FromString("A balanced melee fighter with high survivability");
	case EHarmoniaCharacterClass::Rogue:
		return FText::FromString("A swift combatant specializing in critical strikes and evasion");
	case EHarmoniaCharacterClass::Mage:
		return FText::FromString("A spellcaster wielding powerful elemental magic");
	case EHarmoniaCharacterClass::Berserker:
		return FText::FromString("A wild warrior trading defense for devastating offense");
	case EHarmoniaCharacterClass::Knight:
		return FText::FromString("A heavily armored defender protecting allies");
	case EHarmoniaCharacterClass::Paladin:
		return FText::FromString("A holy warrior combining combat prowess with divine magic");
	case EHarmoniaCharacterClass::Assassin:
		return FText::FromString("A silent killer specializing in stealth and lethal precision");
	case EHarmoniaCharacterClass::Ranger:
		return FText::FromString("A skilled archer and tracker mastering ranged combat");
	case EHarmoniaCharacterClass::Duelist:
		return FText::FromString("A master swordsman excelling in one-on-one combat");
	case EHarmoniaCharacterClass::Sorcerer:
		return FText::FromString("An arcane master wielding raw magical power");
	case EHarmoniaCharacterClass::Cleric:
		return FText::FromString("A divine healer supporting allies with miracles");
	case EHarmoniaCharacterClass::Warlock:
		return FText::FromString("A dark caster wielding forbidden magic");
	default:
		return FText::FromString("No class selected");
	}
}

FLinearColor UHarmoniaProgressionLibrary::GetClassColor(EHarmoniaCharacterClass ClassType)
{
	// Color coding: Red for melee, Green for ranged, Blue for magic
	switch (ClassType)
	{
	case EHarmoniaCharacterClass::Warrior:
	case EHarmoniaCharacterClass::Berserker:
	case EHarmoniaCharacterClass::Knight:
	case EHarmoniaCharacterClass::Paladin:
		return FLinearColor::Red;

	case EHarmoniaCharacterClass::Rogue:
	case EHarmoniaCharacterClass::Assassin:
	case EHarmoniaCharacterClass::Ranger:
	case EHarmoniaCharacterClass::Duelist:
		return FLinearColor::Green;

	case EHarmoniaCharacterClass::Mage:
	case EHarmoniaCharacterClass::Sorcerer:
	case EHarmoniaCharacterClass::Cleric:
	case EHarmoniaCharacterClass::Warlock:
		return FLinearColor::Blue;

	default:
		return FLinearColor::Gray;
	}
}

bool UHarmoniaProgressionLibrary::IsMeleeClass(EHarmoniaCharacterClass ClassType)
{
	return ClassType == EHarmoniaCharacterClass::Warrior ||
		   ClassType == EHarmoniaCharacterClass::Berserker ||
		   ClassType == EHarmoniaCharacterClass::Knight ||
		   ClassType == EHarmoniaCharacterClass::Paladin ||
		   ClassType == EHarmoniaCharacterClass::Duelist;
}

bool UHarmoniaProgressionLibrary::IsRangedClass(EHarmoniaCharacterClass ClassType)
{
	return ClassType == EHarmoniaCharacterClass::Ranger;
}

bool UHarmoniaProgressionLibrary::IsMagicClass(EHarmoniaCharacterClass ClassType)
{
	return ClassType == EHarmoniaCharacterClass::Mage ||
		   ClassType == EHarmoniaCharacterClass::Sorcerer ||
		   ClassType == EHarmoniaCharacterClass::Cleric ||
		   ClassType == EHarmoniaCharacterClass::Warlock;
}

//~ Awakening System Helpers

FText UHarmoniaProgressionLibrary::GetAwakeningTierName(EHarmoniaAwakeningTier Tier)
{
	switch (Tier)
	{
	case EHarmoniaAwakeningTier::Base: return FText::FromString("Base");
	case EHarmoniaAwakeningTier::FirstAwakening: return FText::FromString("First Awakening");
	case EHarmoniaAwakeningTier::SecondAwakening: return FText::FromString("Second Awakening");
	case EHarmoniaAwakeningTier::ThirdAwakening: return FText::FromString("Third Awakening");
	case EHarmoniaAwakeningTier::Transcendent: return FText::FromString("Transcendent");
	default: return FText::FromString("Unknown");
	}
}

FLinearColor UHarmoniaProgressionLibrary::GetAwakeningTierColor(EHarmoniaAwakeningTier Tier)
{
	switch (Tier)
	{
	case EHarmoniaAwakeningTier::Base: return FLinearColor::White;
	case EHarmoniaAwakeningTier::FirstAwakening: return FLinearColor(0.0f, 1.0f, 0.5f); // Cyan
	case EHarmoniaAwakeningTier::SecondAwakening: return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	case EHarmoniaAwakeningTier::ThirdAwakening: return FLinearColor(1.0f, 0.0f, 1.0f); // Magenta
	case EHarmoniaAwakeningTier::Transcendent: return FLinearColor(1.0f, 0.84f, 0.0f); // Gold
	default: return FLinearColor::Gray;
	}
}

float UHarmoniaProgressionLibrary::GetAwakeningStatMultiplier(EHarmoniaAwakeningTier Tier)
{
	// 20% per awakening tier
	return 1.0f + (static_cast<int32>(Tier) * 0.2f);
}

//~ Skill Tree Helpers

FText UHarmoniaProgressionLibrary::GetSkillTreeCategoryName(EHarmoniaSkillTreeCategory Category)
{
	switch (Category)
	{
	case EHarmoniaSkillTreeCategory::SwordMastery: return FText::FromString("Sword Mastery");
	case EHarmoniaSkillTreeCategory::AxeMastery: return FText::FromString("Axe Mastery");
	case EHarmoniaSkillTreeCategory::SpearMastery: return FText::FromString("Spear Mastery");
	case EHarmoniaSkillTreeCategory::BowMastery: return FText::FromString("Bow Mastery");
	case EHarmoniaSkillTreeCategory::MagicMastery: return FText::FromString("Magic Mastery");
	case EHarmoniaSkillTreeCategory::OffensiveCombat: return FText::FromString("Offensive Combat");
	case EHarmoniaSkillTreeCategory::DefensiveCombat: return FText::FromString("Defensive Combat");
	case EHarmoniaSkillTreeCategory::StealthTactics: return FText::FromString("Stealth Tactics");
	case EHarmoniaSkillTreeCategory::Survival: return FText::FromString("Survival");
	case EHarmoniaSkillTreeCategory::Crafting: return FText::FromString("Crafting");
	case EHarmoniaSkillTreeCategory::Social: return FText::FromString("Social");
	default: return FText::FromString("Unknown");
	}
}

FLinearColor UHarmoniaProgressionLibrary::GetSkillTreeCategoryColor(EHarmoniaSkillTreeCategory Category)
{
	switch (Category)
	{
	case EHarmoniaSkillTreeCategory::SwordMastery:
	case EHarmoniaSkillTreeCategory::AxeMastery:
	case EHarmoniaSkillTreeCategory::SpearMastery:
		return FLinearColor::Red;

	case EHarmoniaSkillTreeCategory::BowMastery:
	case EHarmoniaSkillTreeCategory::StealthTactics:
		return FLinearColor::Green;

	case EHarmoniaSkillTreeCategory::MagicMastery:
		return FLinearColor::Blue;

	case EHarmoniaSkillTreeCategory::OffensiveCombat:
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange

	case EHarmoniaSkillTreeCategory::DefensiveCombat:
		return FLinearColor(0.0f, 0.5f, 1.0f); // Light Blue

	case EHarmoniaSkillTreeCategory::Survival:
	case EHarmoniaSkillTreeCategory::Crafting:
		return FLinearColor(0.5f, 0.5f, 0.0f); // Yellow

	case EHarmoniaSkillTreeCategory::Social:
		return FLinearColor(1.0f, 0.0f, 1.0f); // Magenta

	default:
		return FLinearColor::Gray;
	}
}

//~ Prestige Helpers

FText UHarmoniaProgressionLibrary::FormatPrestigeLevelText(int32 PrestigeLevel)
{
	if (PrestigeLevel == 0)
	{
		return FText::FromString("Normal");
	}

	return FText::Format(FText::FromString("NG+{0}"), FText::AsNumber(PrestigeLevel));
}

FText UHarmoniaProgressionLibrary::GetPrestigeRankName(int32 PrestigeLevel)
{
	if (PrestigeLevel == 0) return FText::FromString("Novice");
	if (PrestigeLevel == 1) return FText::FromString("Veteran");
	if (PrestigeLevel == 2) return FText::FromString("Elite");
	if (PrestigeLevel == 3) return FText::FromString("Master");
	if (PrestigeLevel == 4) return FText::FromString("Grandmaster");
	if (PrestigeLevel >= 5) return FText::FromString("Legend");

	return FText::FromString("Unknown");
}

FLinearColor UHarmoniaProgressionLibrary::GetPrestigeColor(int32 PrestigeLevel)
{
	if (PrestigeLevel == 0) return FLinearColor::White;
	if (PrestigeLevel == 1) return FLinearColor::Green;
	if (PrestigeLevel == 2) return FLinearColor::Blue;
	if (PrestigeLevel == 3) return FLinearColor(1.0f, 0.0f, 1.0f); // Magenta
	if (PrestigeLevel == 4) return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	if (PrestigeLevel >= 5) return FLinearColor(1.0f, 0.84f, 0.0f); // Gold

	return FLinearColor::Gray;
}

//~ Stat Calculation Helpers

float UHarmoniaProgressionLibrary::CalculateStatContribution(FGameplayTag StatTag, int32 StatValue)
{
	// Placeholder - should be customized based on actual stat system
	return StatValue * 1.0f;
}

TMap<FGameplayTag, int32> UHarmoniaProgressionLibrary::GetRecommendedStatsForClass(EHarmoniaCharacterClass ClassType, int32 Level)
{
	TMap<FGameplayTag, int32> RecommendedStats;

	// Placeholder - should be customized based on class balance
	// This would distribute stat points based on class archetype

	return RecommendedStats;
}

//~ Progression Validation

bool UHarmoniaProgressionLibrary::ValidateSkillNodePrerequisites(const FHarmoniaSkillNode& Node, const TArray<FSkillNodeInvestment>& UnlockedNodes)
{
	for (const FName& PrereqID : Node.PrerequisiteNodeIDs)
	{
		bool bFound = UnlockedNodes.ContainsByPredicate([PrereqID](const FSkillNodeInvestment& Investment)
		{
			return Investment.NodeID == PrereqID;
		});

		if (!bFound)
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaProgressionLibrary::MeetsClassRequirements(int32 PlayerLevel, EHarmoniaCharacterClass CurrentClass, EHarmoniaCharacterClass TargetClass)
{
	// Placeholder - should check actual class progression requirements
	return true;
}

//~ UI Formatting Helpers

FText UHarmoniaProgressionLibrary::FormatLargeNumber(int64 Number)
{
	if (Number >= 1000000000)
	{
		return FText::Format(FText::FromString("{0}B"), FText::AsNumber(Number / 1000000000.0f));
	}
	else if (Number >= 1000000)
	{
		return FText::Format(FText::FromString("{0}M"), FText::AsNumber(Number / 1000000.0f));
	}
	else if (Number >= 1000)
	{
		return FText::Format(FText::FromString("{0}K"), FText::AsNumber(Number / 1000.0f));
	}

	return FText::AsNumber(Number);
}

FLinearColor UHarmoniaProgressionLibrary::GetProgressBarColor(float Percentage)
{
	if (Percentage < 0.33f)
	{
		return FLinearColor::Red;
	}
	else if (Percentage < 0.66f)
	{
		return FLinearColor::Yellow;
	}
	else
	{
		return FLinearColor::Green;
	}
}

FText UHarmoniaProgressionLibrary::CreateStatChangeText(FGameplayTag StatTag, int32 Change)
{
	const FString Sign = Change >= 0 ? "+" : "";
	return FText::Format(
		FText::FromString("{0}{1} {2}"),
		FText::FromString(Sign),
		FText::AsNumber(Change),
		FText::FromName(StatTag.GetTagName())
	);
}
