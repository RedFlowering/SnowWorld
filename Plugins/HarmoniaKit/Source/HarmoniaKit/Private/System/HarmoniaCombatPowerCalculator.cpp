// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/HarmoniaCombatPowerCalculator.h"

TMap<EHarmoniaElementType, TArray<EHarmoniaElementType>> UHarmoniaCombatPowerCalculator::ElementalWeaknesses;
TMap<EHarmoniaElementType, TArray<EHarmoniaElementType>> UHarmoniaCombatPowerCalculator::ElementalStrengths;
bool UHarmoniaCombatPowerCalculator::bAffinitiesInitialized = false;

FHarmoniaCombatPowerResult UHarmoniaCombatPowerCalculator::CalculateCombatPower(const FHarmoniaCombatPowerParams& Params)
{
	if (!bAffinitiesInitialized)
	{
		InitializeElementalAffinities();
	}

	FHarmoniaCombatPowerResult Result;

	// 1. Elemental affinity
	Result.ElementalMultiplier = GetElementalAffinity(Params.AttackerElement, Params.DefenderElement);

	// 2. Level difference
	Result.LevelMultiplier = GetLevelMultiplier(Params.LevelDifference);

	// 3. Terrain
	Result.TerrainMultiplier = GetTerrainMultiplier(Params.AttackerElement, Params.TerrainType);

	// 4. Health percentage
	Result.HealthMultiplier = FMath::Lerp(0.5f, 1.0f, Params.HealthPercentage);

	// 5. Buffs/Debuffs
	float BuffBonus = Params.BuffCount * 0.1f;
	float DebuffPenalty = Params.DebuffCount * 0.1f;
	Result.BuffMultiplier = 1.0f + BuffBonus - DebuffPenalty;
	Result.BuffMultiplier = FMath::Max(0.3f, Result.BuffMultiplier); // Minimum 30%

	// 6. Group size
	Result.GroupMultiplier = 1.0f + (Params.GroupSize - 1) * 0.3f;

	// Calculate final power
	Result.FinalPower = Params.BasePower
		* Result.ElementalMultiplier
		* Result.LevelMultiplier
		* Result.TerrainMultiplier
		* Result.HealthMultiplier
		* Result.BuffMultiplier
		* Result.GroupMultiplier;

	// Generate breakdown text
	Result.BreakdownText = FString::Printf(
		TEXT("Base: %.0f\n")
		TEXT("Elemental: x%.2f (%s vs %s)\n")
		TEXT("Level: x%.2f (Diff: %d)\n")
		TEXT("Terrain: x%.2f (%s)\n")
		TEXT("Health: x%.2f (%.0f%%)\n")
		TEXT("Buffs: x%.2f (+%d -%d)\n")
		TEXT("Group: x%.2f (%d members)\n")
		TEXT("= %.0f Total Power"),
		Params.BasePower,
		Result.ElementalMultiplier, *ElementToString(Params.AttackerElement), *ElementToString(Params.DefenderElement),
		Result.LevelMultiplier, Params.LevelDifference,
		Result.TerrainMultiplier, *TerrainToString(Params.TerrainType),
		Result.HealthMultiplier, Params.HealthPercentage * 100.0f,
		Result.BuffMultiplier, Params.BuffCount, Params.DebuffCount,
		Result.GroupMultiplier, Params.GroupSize,
		Result.FinalPower
	);

	return Result;
}

float UHarmoniaCombatPowerCalculator::GetElementalAffinity(EHarmoniaElementType Attacker, EHarmoniaElementType Defender)
{
	if (Attacker == EHarmoniaElementType::None || Defender == EHarmoniaElementType::None)
	{
		return 1.0f;
	}

	// Check strengths
	if (ElementalStrengths.Contains(Attacker))
	{
		const TArray<EHarmoniaElementType>& Strengths = ElementalStrengths[Attacker];
		if (Strengths.Contains(Defender))
		{
			return 2.0f; // Super effective
		}
	}

	// Check weaknesses
	if (ElementalWeaknesses.Contains(Attacker))
	{
		const TArray<EHarmoniaElementType>& Weaknesses = ElementalWeaknesses[Attacker];
		if (Weaknesses.Contains(Defender))
		{
			return 0.5f; // Not very effective
		}
	}

	return 1.0f; // Neutral
}

float UHarmoniaCombatPowerCalculator::GetLevelMultiplier(int32 LevelDifference)
{
	// Each level difference is worth 10%
	// Capped at +/- 50%
	float Multiplier = 1.0f + (LevelDifference * 0.1f);
	return FMath::Clamp(Multiplier, 0.5f, 1.5f);
}

float UHarmoniaCombatPowerCalculator::GetTerrainMultiplier(EHarmoniaElementType Element, EHarmoniaTerrainType Terrain)
{
	// Favorable terrain gives 20% bonus
	switch (Element)
	{
	case EHarmoniaElementType::Fire:
		if (Terrain == EHarmoniaTerrainType::Desert)
			return 1.2f;
		if (Terrain == EHarmoniaTerrainType::Water || Terrain == EHarmoniaTerrainType::Snow)
			return 0.8f;
		break;

	case EHarmoniaElementType::Water:
		if (Terrain == EHarmoniaTerrainType::Water)
			return 1.3f;
		if (Terrain == EHarmoniaTerrainType::Desert)
			return 0.8f;
		break;

	case EHarmoniaElementType::Ice:
		if (Terrain == EHarmoniaTerrainType::Snow)
			return 1.3f;
		if (Terrain == EHarmoniaTerrainType::Desert)
			return 0.7f;
		break;

	case EHarmoniaElementType::Lightning:
		if (Terrain == EHarmoniaTerrainType::HighGround)
			return 1.2f;
		if (Terrain == EHarmoniaTerrainType::Water)
			return 1.4f; // Lightning in water is devastating
		break;

	case EHarmoniaElementType::Earth:
		if (Terrain == EHarmoniaTerrainType::Cave || Terrain == EHarmoniaTerrainType::Forest)
			return 1.2f;
		break;

	case EHarmoniaElementType::Wind:
		if (Terrain == EHarmoniaTerrainType::HighGround)
			return 1.3f;
		if (Terrain == EHarmoniaTerrainType::Cave)
			return 0.8f;
		break;

	case EHarmoniaElementType::Dark:
		if (Terrain == EHarmoniaTerrainType::Cave)
			return 1.3f;
		break;

	case EHarmoniaElementType::Poison:
		if (Terrain == EHarmoniaTerrainType::Forest)
			return 1.2f;
		break;

	default:
		break;
	}

	return 1.0f;
}

bool UHarmoniaCombatPowerCalculator::IsElementStrongAgainst(EHarmoniaElementType Attacker, EHarmoniaElementType Defender)
{
	return GetElementalAffinity(Attacker, Defender) > 1.0f;
}

void UHarmoniaCombatPowerCalculator::InitializeElementalAffinities()
{
	if (bAffinitiesInitialized)
	{
		return;
	}

	ElementalStrengths.Empty();
	ElementalWeaknesses.Empty();

	// Fire beats Ice, Wind
	ElementalStrengths.Add(EHarmoniaElementType::Fire, { EHarmoniaElementType::Ice, EHarmoniaElementType::Wind });
	ElementalWeaknesses.Add(EHarmoniaElementType::Fire, { EHarmoniaElementType::Water, EHarmoniaElementType::Earth });

	// Water beats Fire, Earth
	ElementalStrengths.Add(EHarmoniaElementType::Water, { EHarmoniaElementType::Fire, EHarmoniaElementType::Earth });
	ElementalWeaknesses.Add(EHarmoniaElementType::Water, { EHarmoniaElementType::Lightning, EHarmoniaElementType::Ice });

	// Ice beats Earth, Water
	ElementalStrengths.Add(EHarmoniaElementType::Ice, { EHarmoniaElementType::Earth, EHarmoniaElementType::Water });
	ElementalWeaknesses.Add(EHarmoniaElementType::Ice, { EHarmoniaElementType::Fire, EHarmoniaElementType::Lightning });

	// Lightning beats Water, Wind
	ElementalStrengths.Add(EHarmoniaElementType::Lightning, { EHarmoniaElementType::Water, EHarmoniaElementType::Wind });
	ElementalWeaknesses.Add(EHarmoniaElementType::Lightning, { EHarmoniaElementType::Earth, EHarmoniaElementType::Ice });

	// Earth beats Lightning, Fire
	ElementalStrengths.Add(EHarmoniaElementType::Earth, { EHarmoniaElementType::Lightning, EHarmoniaElementType::Fire });
	ElementalWeaknesses.Add(EHarmoniaElementType::Earth, { EHarmoniaElementType::Water, EHarmoniaElementType::Ice });

	// Wind beats Earth, Poison
	ElementalStrengths.Add(EHarmoniaElementType::Wind, { EHarmoniaElementType::Earth, EHarmoniaElementType::Poison });
	ElementalWeaknesses.Add(EHarmoniaElementType::Wind, { EHarmoniaElementType::Fire, EHarmoniaElementType::Lightning });

	// Light beats Dark
	ElementalStrengths.Add(EHarmoniaElementType::Light, { EHarmoniaElementType::Dark });
	ElementalWeaknesses.Add(EHarmoniaElementType::Light, { EHarmoniaElementType::Dark });

	// Dark beats Light
	ElementalStrengths.Add(EHarmoniaElementType::Dark, { EHarmoniaElementType::Light });
	ElementalWeaknesses.Add(EHarmoniaElementType::Dark, { EHarmoniaElementType::Light });

	// Poison beats Water
	ElementalStrengths.Add(EHarmoniaElementType::Poison, { EHarmoniaElementType::Water });
	ElementalWeaknesses.Add(EHarmoniaElementType::Poison, { EHarmoniaElementType::Wind, EHarmoniaElementType::Fire });

	bAffinitiesInitialized = true;
}

FString UHarmoniaCombatPowerCalculator::ElementToString(EHarmoniaElementType Element)
{
	switch (Element)
	{
	case EHarmoniaElementType::None: return TEXT("None");
	case EHarmoniaElementType::Fire: return TEXT("Fire");
	case EHarmoniaElementType::Water: return TEXT("Water");
	case EHarmoniaElementType::Ice: return TEXT("Ice");
	case EHarmoniaElementType::Lightning: return TEXT("Lightning");
	case EHarmoniaElementType::Earth: return TEXT("Earth");
	case EHarmoniaElementType::Wind: return TEXT("Wind");
	case EHarmoniaElementType::Light: return TEXT("Light");
	case EHarmoniaElementType::Dark: return TEXT("Dark");
	case EHarmoniaElementType::Poison: return TEXT("Poison");
	default: return TEXT("Unknown");
	}
}

FString UHarmoniaCombatPowerCalculator::TerrainToString(EHarmoniaTerrainType Terrain)
{
	switch (Terrain)
	{
	case EHarmoniaTerrainType::Flat: return TEXT("Flat");
	case EHarmoniaTerrainType::HighGround: return TEXT("High Ground");
	case EHarmoniaTerrainType::LowGround: return TEXT("Low Ground");
	case EHarmoniaTerrainType::Water: return TEXT("Water");
	case EHarmoniaTerrainType::Forest: return TEXT("Forest");
	case EHarmoniaTerrainType::Cave: return TEXT("Cave");
	case EHarmoniaTerrainType::Desert: return TEXT("Desert");
	case EHarmoniaTerrainType::Snow: return TEXT("Snow");
	default: return TEXT("Unknown");
	}
}
