// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HarmoniaCombatPowerCalculator.generated.h"

/**
 * Element types for affinity system
 */
UENUM(BlueprintType)
enum class EHarmoniaElementType : uint8
{
	None		UMETA(DisplayName = "None"),
	Fire		UMETA(DisplayName = "Fire"),
	Water		UMETA(DisplayName = "Water"),
	Ice			UMETA(DisplayName = "Ice"),
	Lightning	UMETA(DisplayName = "Lightning"),
	Earth		UMETA(DisplayName = "Earth"),
	Wind		UMETA(DisplayName = "Wind"),
	Light		UMETA(DisplayName = "Light"),
	Dark		UMETA(DisplayName = "Dark"),
	Poison		UMETA(DisplayName = "Poison")
};

/**
 * Terrain types
 */
UENUM(BlueprintType)
enum class EHarmoniaTerrainType : uint8
{
	Flat		UMETA(DisplayName = "Flat"),
	HighGround	UMETA(DisplayName = "High Ground"),
	LowGround	UMETA(DisplayName = "Low Ground"),
	Water		UMETA(DisplayName = "Water"),
	Forest		UMETA(DisplayName = "Forest"),
	Cave		UMETA(DisplayName = "Cave"),
	Desert		UMETA(DisplayName = "Desert"),
	Snow		UMETA(DisplayName = "Snow")
};

/**
 * Combat power calculation parameters
 */
USTRUCT(BlueprintType)
struct FHarmoniaCombatPowerParams
{
	GENERATED_BODY()

	/** Base combat power (from stats) */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	float BasePower = 100.0f;

	/** Attacker element */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	EHarmoniaElementType AttackerElement = EHarmoniaElementType::None;

	/** Defender element */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	EHarmoniaElementType DefenderElement = EHarmoniaElementType::None;

	/** Level difference (positive = attacker higher level) */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	int32 LevelDifference = 0;

	/** Current terrain type */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	EHarmoniaTerrainType TerrainType = EHarmoniaTerrainType::Flat;

	/** Health percentage (0.0 - 1.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	float HealthPercentage = 1.0f;

	/** Number of active buffs */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	int32 BuffCount = 0;

	/** Number of active debuffs */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	int32 DebuffCount = 0;

	/** Group size */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	int32 GroupSize = 1;

	/** Is on favorable terrain for element */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Power")
	bool bOnFavorableTerrain = false;
};

/**
 * Combat power calculation result
 */
USTRUCT(BlueprintType)
struct FHarmoniaCombatPowerResult
{
	GENERATED_BODY()

	/** Final calculated combat power */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float FinalPower = 100.0f;

	/** Elemental affinity multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float ElementalMultiplier = 1.0f;

	/** Level difference multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float LevelMultiplier = 1.0f;

	/** Terrain multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float TerrainMultiplier = 1.0f;

	/** Health multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float HealthMultiplier = 1.0f;

	/** Buff/Debuff multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float BuffMultiplier = 1.0f;

	/** Group size multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float GroupMultiplier = 1.0f;

	/** Breakdown text for debugging */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString BreakdownText;
};

/**
 * Static calculator for combat power with advanced modifiers
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCombatPowerCalculator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Calculate combat power with all modifiers
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat Power")
	static FHarmoniaCombatPowerResult CalculateCombatPower(const FHarmoniaCombatPowerParams& Params);

	/**
	 * Get elemental affinity multiplier
	 * @return 2.0 for super effective, 0.5 for not effective, 1.0 for neutral
	 */
	UFUNCTION(BlueprintPure, Category = "Combat Power")
	static float GetElementalAffinity(EHarmoniaElementType Attacker, EHarmoniaElementType Defender);

	/**
	 * Get level difference multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "Combat Power")
	static float GetLevelMultiplier(int32 LevelDifference);

	/**
	 * Get terrain multiplier for element
	 */
	UFUNCTION(BlueprintPure, Category = "Combat Power")
	static float GetTerrainMultiplier(EHarmoniaElementType Element, EHarmoniaTerrainType Terrain);

	/**
	 * Check if element is strong against another
	 */
	UFUNCTION(BlueprintPure, Category = "Combat Power")
	static bool IsElementStrongAgainst(EHarmoniaElementType Attacker, EHarmoniaElementType Defender);

	/**
	 * Get element name as string
	 */
	UFUNCTION(BlueprintPure, Category = "Combat Power")
	static FString ElementToString(EHarmoniaElementType Element);

	/**
	 * Get terrain name as string
	 */
	UFUNCTION(BlueprintPure, Category = "Combat Power")
	static FString TerrainToString(EHarmoniaTerrainType Terrain);

protected:
	/**
	 * Initialize elemental affinity table
	 * Fire > Ice > Earth > Lightning > Water > Fire
	 * Light <> Dark
	 * Wind > Poison
	 */
	static void InitializeElementalAffinities();

	/** Elemental affinity lookup table */
	static TMap<EHarmoniaElementType, TArray<EHarmoniaElementType>> ElementalWeaknesses;
	static TMap<EHarmoniaElementType, TArray<EHarmoniaElementType>> ElementalStrengths;
	static bool bAffinitiesInitialized;
};
