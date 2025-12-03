// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaObjectPoolConfigDataAsset.h
 * @brief Object pool configuration data asset
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "System/HarmoniaObjectPoolSubsystem.h"
#include "HarmoniaObjectPoolConfigDataAsset.generated.h"

/**
 * @struct FHarmoniaPoolGlobalSettings
 * @brief Object pool global settings
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPoolGlobalSettings
{
	GENERATED_BODY()

	/** Enable global pooling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bEnablePooling = true;

	/** Enable statistics collection (debug, recommended to disable in production) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bCollectStatistics = true;

	/** Pool shrink check interval in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global", meta = (ClampMin = "1.0", ClampMax = "300.0"))
	float ShrinkCheckInterval = 30.0f;

	/** Enable pool warm-up (pre-create on game start) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bWarmUpOnStart = true;

	/** Hidden location for deactivated actors from pool (offscreen) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	FVector HiddenLocation = FVector(0.0f, 0.0f, -100000.0f);
};

/**
 * @class UHarmoniaObjectPoolConfigDataAsset
 * @brief Object pool configuration data asset
 * 
 * Manages global settings and predefined pool configurations for the object pooling system.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaObjectPoolConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaObjectPoolConfigDataAsset();

	/** Global pool settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Global")
	FHarmoniaPoolGlobalSettings GlobalSettings;

	/** Predefined pool configuration list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Configs")
	TArray<FHarmoniaPoolConfig> PoolConfigs;

	/** Find pool config by tag */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByTag(FGameplayTag PoolTag, FHarmoniaPoolConfig& OutConfig) const;

	/** Find pool config by class */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByClass(TSubclassOf<AActor> ActorClass, FHarmoniaPoolConfig& OutConfig) const;
};
