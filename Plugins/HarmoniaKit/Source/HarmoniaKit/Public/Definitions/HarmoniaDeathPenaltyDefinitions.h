// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaShopSystemDefinitions.h"
#include "HarmoniaDeathPenaltyDefinitions.generated.h"

// EHarmoniaCurrencyType is defined as an alias for ECurrencyType in HarmoniaShopSystemDefinitions.h

/**
 * Death penalty severity levels
 */
UENUM(BlueprintType)
enum class EHarmoniaDeathPenaltySeverity : uint8
{
	None UMETA(DisplayName = "None"),
	Light UMETA(DisplayName = "Light"),
	Medium UMETA(DisplayName = "Medium"),
	Heavy UMETA(DisplayName = "Heavy"),
	Severe UMETA(DisplayName = "Severe"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Player state after death
 */
UENUM(BlueprintType)
enum class EHarmoniaPlayerDeathState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Ethereal UMETA(DisplayName = "Ethereal"),
	Corrupted UMETA(DisplayName = "Corrupted")
};

/**
 * Configuration for a single currency type
 */
USTRUCT(BlueprintType)
struct FHarmoniaCurrencyAmount
{
	GENERATED_BODY()

	/** Type of currency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	EHarmoniaCurrencyType CurrencyType = EHarmoniaCurrencyType::None;

	/** Amount of this currency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency", meta = (ClampMin = "0"))
	int32 Amount = 0;

	FHarmoniaCurrencyAmount()
		: CurrencyType(EHarmoniaCurrencyType::None)
		, Amount(0)
	{
	}

	FHarmoniaCurrencyAmount(EHarmoniaCurrencyType InType, int32 InAmount)
		: CurrencyType(InType)
		, Amount(InAmount)
	{
	}

	bool IsValid() const
	{
		return CurrencyType != EHarmoniaCurrencyType::None && Amount > 0;
	}
};

/**
 * Configuration for attribute penalties on death
 */
USTRUCT(BlueprintType)
struct FHarmoniaDeathAttributePenalty
{
	GENERATED_BODY()

	/** Health multiplier when in ethereal state (1.0 = no change, 0.7 = -30%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthMultiplier = 0.7f;

	/** Damage multiplier when in ethereal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageMultiplier = 0.7f;

	/** Stamina regen multiplier when in ethereal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StaminaRegenMultiplier = 0.7f;

	/** Movement speed multiplier when in ethereal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovementSpeedMultiplier = 0.85f;

	/** Max health penalty for consecutive deaths (recoverable when resting) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalties", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float MaxHealthPenaltyPerDeath = 0.1f;

	/** Maximum stack of max health penalty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalties", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxHealthPenaltyStacks = 3;

	FHarmoniaDeathAttributePenalty()
		: HealthMultiplier(0.7f)
		, DamageMultiplier(0.7f)
		, StaminaRegenMultiplier(0.7f)
		, MovementSpeedMultiplier(0.85f)
		, MaxHealthPenaltyPerDeath(0.1f)
		, MaxHealthPenaltyStacks(3)
	{
	}
};

/**
 * Configuration for time decay mechanics
 */
USTRUCT(BlueprintType)
struct FHarmoniaTimeDecayConfig
{
	GENERATED_BODY()

	/** Enable time decay system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Decay")
	bool bEnableTimeDecay = true;

	/** Time in seconds before decay starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Decay", meta = (ClampMin = "0.0", EditCondition = "bEnableTimeDecay"))
	float DecayStartTime = 60.0f;

	/** How often decay is applied (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Decay", meta = (ClampMin = "1.0", EditCondition = "bEnableTimeDecay"))
	float DecayInterval = 300.0f; // 5 minutes

	/** Percentage lost per decay interval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Decay", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableTimeDecay"))
	float DecayPercentage = 0.1f; // 10%

	/** Time window for fast recovery bonus (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Decay", meta = (ClampMin = "0.0"))
	float FastRecoveryWindow = 30.0f;

	/** Bonus percentage for fast recovery */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Decay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FastRecoveryBonus = 0.2f; // +20%

	FHarmoniaTimeDecayConfig()
		: bEnableTimeDecay(true)
		, DecayStartTime(60.0f)
		, DecayInterval(300.0f)
		, DecayPercentage(0.1f)
		, FastRecoveryWindow(30.0f)
		, FastRecoveryBonus(0.2f)
	{
	}
};

/**
 * Configuration for Memory Resonance - enemies near dropped currencies get buffed
 */
USTRUCT(BlueprintType)
struct FHarmoniaMemoryResonanceConfig
{
	GENERATED_BODY()

	/** Enable memory resonance system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Resonance")
	bool bEnableMemoryResonance = true;

	/** Radius around memory echo that affects enemies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Resonance", meta = (ClampMin = "0.0", EditCondition = "bEnableMemoryResonance"))
	float ResonanceRadius = 2000.0f; // 20 meters

	/** Damage buff percentage for enemies in resonance range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Resonance", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableMemoryResonance"))
	float EnemyDamageBuffPercentage = 0.1f; // +10%

	/** Health buff percentage for enemies in resonance range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Resonance", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableMemoryResonance"))
	float EnemyHealthBuffPercentage = 0.1f; // +10%

	/** Visual effect intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Resonance", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableMemoryResonance"))
	float ResonanceEffectIntensity = 0.5f;

	FHarmoniaMemoryResonanceConfig()
		: bEnableMemoryResonance(true)
		, ResonanceRadius(2000.0f)
		, EnemyDamageBuffPercentage(0.1f)
		, EnemyHealthBuffPercentage(0.1f)
		, ResonanceEffectIntensity(0.5f)
	{
	}
};

/**
 * Percentage of each currency to drop on death
 */
USTRUCT(BlueprintType)
struct FHarmoniaCurrencyDropConfig
{
	GENERATED_BODY()

	/** Currency type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Config")
	EHarmoniaCurrencyType CurrencyType = EHarmoniaCurrencyType::None;

	/** Percentage to drop on death (0.0 = none, 1.0 = all) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropPercentage = 1.0f;

	/** Percentage permanently lost on double death (0.0 = none recovered, 1.0 = all lost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PermanentLossPercentage = 1.0f;

	FHarmoniaCurrencyDropConfig()
		: CurrencyType(EHarmoniaCurrencyType::None)
		, DropPercentage(1.0f)
		, PermanentLossPercentage(1.0f)
	{
	}
};

/**
 * Complete death penalty configuration
 */
USTRUCT(BlueprintType)
struct FHarmoniaDeathPenaltyConfig
{
	GENERATED_BODY()

	/** Drop configurations for each currency type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	TArray<FHarmoniaCurrencyDropConfig> CurrencyDropConfigs;

	/** Attribute penalties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalties")
	FHarmoniaDeathAttributePenalty AttributePenalties;

	/** Time decay configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Decay")
	FHarmoniaTimeDecayConfig TimeDecayConfig;

	/** Memory resonance configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Resonance")
	FHarmoniaMemoryResonanceConfig MemoryResonanceConfig;

	/** Allow recovery from another player's memory echo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	bool bAllowOtherPlayerRecovery = false;

	/** Percentage of another player's memory that can be stolen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bAllowOtherPlayerRecovery"))
	float OtherPlayerRecoveryPercentage = 0.1f; // 10%

	FHarmoniaDeathPenaltyConfig()
		: bAllowOtherPlayerRecovery(false)
		, OtherPlayerRecoveryPercentage(0.1f)
	{
	}
};
