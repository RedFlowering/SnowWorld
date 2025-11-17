// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "System/HarmoniaTimeWeatherManager.h"
#include "HarmoniaDynamicSpawnManager.generated.h"

class AHarmoniaMonsterSpawner;
class UHarmoniaMonsterData;

/**
 * Spawn modifier based on time/weather
 */
USTRUCT(BlueprintType)
struct FHarmoniaSpawnModifier
{
	GENERATED_BODY()

	/** Time of day for this modifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	EHarmoniaTimeOfDay TimeOfDay = EHarmoniaTimeOfDay::Morning;

	/** Weather type for this modifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	EHarmoniaWeatherType WeatherType = EHarmoniaWeatherType::Clear;

	/** Spawn rate multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float SpawnRateMultiplier = 1.0f;

	/** Monster count multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float MonsterCountMultiplier = 1.0f;

	/** Additional monster types to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	TArray<TSoftObjectPtr<UHarmoniaMonsterData>> AdditionalMonsterTypes;

	/** Apply only if both time and weather match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	bool bRequireBothConditions = false;

	FHarmoniaSpawnModifier()
		: SpawnRateMultiplier(1.0f)
		, MonsterCountMultiplier(1.0f)
		, bRequireBothConditions(false)
	{
	}
};

/**
 * Wave configuration for survival mode
 */
USTRUCT(BlueprintType)
struct FHarmoniaWaveConfig
{
	GENERATED_BODY()

	/** Wave number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 WaveNumber = 1;

	/** Monsters to spawn in this wave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<TSoftObjectPtr<UHarmoniaMonsterData>> MonsterTypes;

	/** Number of monsters per type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 MonstersPerType = 5;

	/** Delay before wave starts (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float WaveStartDelay = 10.0f;

	/** Time between monster spawns within wave (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnInterval = 2.0f;

	/** Boss monster to spawn at end of wave (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSoftObjectPtr<UHarmoniaMonsterData> BossMonster;

	FHarmoniaWaveConfig()
		: WaveNumber(1)
		, MonstersPerType(5)
		, WaveStartDelay(10.0f)
		, SpawnInterval(2.0f)
	{
	}
};

/**
 * Delegates for wave events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveCompleted, int32, WaveNumber, float, CompletionTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveFailed, int32, WaveNumber);

/**
 * Dynamic spawn manager that adjusts spawning based on:
 * - Time of day
 * - Weather conditions
 * - Party size (via party scaling)
 * - Wave-based survival mode
 */
UCLASS()
class HARMONIAKIT_API AHarmoniaDynamicSpawnManager : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaDynamicSpawnManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ==================== Configuration ====================

	/** Enable dynamic spawning based on time/weather */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Spawn")
	bool bEnableDynamicSpawn = true;

	/** Spawn modifiers for different time/weather combinations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Spawn")
	TArray<FHarmoniaSpawnModifier> SpawnModifiers;

	/** Spawners to manage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Spawn")
	TArray<AHarmoniaMonsterSpawner*> ManagedSpawners;

	/** Auto-find spawners in level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Spawn")
	bool bAutoFindSpawners = true;

	/** Base spawn interval (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Spawn")
	float BaseSpawnInterval = 30.0f;

	// ==================== Wave Mode ====================

	/** Enable wave-based survival mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Mode")
	bool bEnableWaveMode = false;

	/** Wave configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Mode")
	TArray<FHarmoniaWaveConfig> WaveConfigs;

	/** Current wave number */
	UPROPERTY(BlueprintReadOnly, Category = "Wave Mode")
	int32 CurrentWave = 0;

	/** Is wave active */
	UPROPERTY(BlueprintReadOnly, Category = "Wave Mode")
	bool bWaveActive = false;

	/** Auto-start waves on begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Mode")
	bool bAutoStartWaves = false;

	// ==================== Events ====================

	UPROPERTY(BlueprintAssignable, Category = "Wave Mode")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Wave Mode")
	FOnWaveCompleted OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Wave Mode")
	FOnAllWavesCompleted OnAllWavesCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Wave Mode")
	FOnWaveFailed OnWaveFailed;

	// ==================== Public API ====================

	/** Start wave mode */
	UFUNCTION(BlueprintCallable, Category = "Wave Mode")
	void StartWaveMode();

	/** Start specific wave */
	UFUNCTION(BlueprintCallable, Category = "Wave Mode")
	void StartWave(int32 WaveNumber);

	/** Stop wave mode */
	UFUNCTION(BlueprintCallable, Category = "Wave Mode")
	void StopWaveMode();

	/** Get current spawn rate multiplier */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Spawn")
	float GetCurrentSpawnRateMultiplier() const;

	/** Get current monster count multiplier */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Spawn")
	float GetCurrentMonsterCountMultiplier() const;

	/** Force refresh spawn modifiers */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Spawn")
	void RefreshSpawnModifiers();

protected:
	// ==================== Internal Functions ====================

	/** Find all spawners in level */
	void FindSpawnersInLevel();

	/** Apply current spawn modifiers to spawners */
	void ApplySpawnModifiers();

	/** Get applicable modifiers for current conditions */
	TArray<FHarmoniaSpawnModifier> GetApplicableModifiers() const;

	/** Handle time change */
	UFUNCTION()
	void OnTimeChanged(const FHarmoniaTimeChangeInfo& TimeInfo);

	/** Handle weather change */
	UFUNCTION()
	void OnWeatherChanged(const FHarmoniaWeatherChangeInfo& WeatherInfo);

	/** Update wave spawning */
	void UpdateWaveSpawning(float DeltaTime);

	/** Spawn wave monsters */
	void SpawnWaveMonsters(const FHarmoniaWaveConfig& WaveConfig);

	/** Check if current wave is complete */
	bool IsWaveComplete() const;

	/** Complete current wave */
	void CompleteWave();

	// ==================== State ====================

	/** Cached time/weather manager */
	UPROPERTY()
	UHarmoniaTimeWeatherManager* TimeWeatherManager = nullptr;

	/** Current spawn rate multiplier */
	float CurrentSpawnRateMultiplier = 1.0f;

	/** Current monster count multiplier */
	float CurrentMonsterCountMultiplier = 1.0f;

	/** Wave start time */
	float WaveStartTime = 0.0f;

	/** Wave spawn timer */
	float WaveSpawnTimer = 0.0f;

	/** Monsters spawned in current wave */
	UPROPERTY()
	TArray<AActor*> CurrentWaveMonsters;

	/** Index of next monster to spawn in wave */
	int32 WaveSpawnIndex = 0;
};
