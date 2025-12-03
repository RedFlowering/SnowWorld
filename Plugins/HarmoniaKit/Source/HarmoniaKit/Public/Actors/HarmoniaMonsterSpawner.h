// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HarmoniaMonsterSpawner.generated.h"

class UHarmoniaMonsterData;
class AHarmoniaMonsterBase;

/**
 * Monster Spawn Configuration
 * Defines what and how to spawn
 */
USTRUCT(BlueprintType)
struct FHarmoniaMonsterSpawnConfig
{
	GENERATED_BODY()

	// Monster data to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<UHarmoniaMonsterData> MonsterData = nullptr;

	// Monster class (if not using HarmoniaMonsterBase)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AHarmoniaMonsterBase> MonsterClass;

	// Level range for spawned monsters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MinLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MaxLevel = 1;

	// Spawn weight (higher = more likely in random selection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 SpawnWeight = 1;

	// Maximum instances of this monster type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MaxInstances = 1;
};

/**
 * Spawn Mode
 */
UENUM(BlueprintType)
enum class EHarmoniaSpawnMode : uint8
{
	// Spawn once on begin play
	OnBeginPlay UMETA(DisplayName = "On Begin Play"),

	// Respawn when killed
	Respawn UMETA(DisplayName = "Respawn"),

	// Continuous spawning (wave-based)
	Continuous UMETA(DisplayName = "Continuous"),

	// Manual spawning only
	Manual UMETA(DisplayName = "Manual")
};

/**
 * Monster Spawned Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterSpawnedDelegate, AHarmoniaMonsterBase*, SpawnedMonster);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDespawnedDelegate, AHarmoniaMonsterBase*, DespawnedMonster);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllMonstersDefeatedDelegate);

/**
 * AHarmoniaMonsterSpawner
 *
 * Spawns and manages monsters
 *
 * Features:
 * - Multiple spawn modes (one-time, respawn, wave-based)
 * - Random monster selection from pool
 * - Level scaling
 * - Spawn cooldowns and delays
 * - Maximum monster limits
 * - Respawn on death
 * - Area/radius spawning
 * - Visual debug helpers
 *
 * Usage: Place in level, configure spawn settings, monsters spawn automatically or on demand
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaMonsterSpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Spawn mode
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
	EHarmoniaSpawnMode SpawnMode = EHarmoniaSpawnMode::OnBeginPlay;

	/**
	 * Monster configurations to spawn
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
	TArray<FHarmoniaMonsterSpawnConfig> MonsterConfigs;

	/**
	 * Maximum total monsters alive at once
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
	int32 MaxTotalMonsters = 5;

	/**
	 * Spawn radius (0 = exact location)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
	float SpawnRadius = 500.0f;

	/**
	 * Delay before first spawn
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
	float InitialSpawnDelay = 0.0f;

	/**
	 * Time between respawns
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Respawn", meta = (EditCondition = "SpawnMode == EHarmoniaSpawnMode::Respawn"))
	float RespawnDelay = 30.0f;

	/**
	 * Time between wave spawns
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Continuous", meta = (EditCondition = "SpawnMode == EHarmoniaSpawnMode::Continuous"))
	float WaveInterval = 60.0f;

	/**
	 * Number of monsters per wave
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Continuous", meta = (EditCondition = "SpawnMode == EHarmoniaSpawnMode::Continuous"))
	int32 MonstersPerWave = 3;

	/**
	 * Whether spawner is active
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
	bool bActive = true;

	/**
	 * Show debug spawn radius
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Debug")
	bool bShowDebugRadius = true;

	// ============================================================================
	// Spawning Functions
	// ============================================================================

	/**
	 * Spawn a monster
	 * @return Spawned monster actor (nullptr if failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	AHarmoniaMonsterBase* SpawnMonster();

	/**
	 * Spawn specific monster configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	AHarmoniaMonsterBase* SpawnMonsterFromConfig(const FHarmoniaMonsterSpawnConfig& Config);

	/**
	 * Despawn a monster
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void DespawnMonster(AHarmoniaMonsterBase* Monster);

	/**
	 * Despawn all monsters
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void DespawnAllMonsters();

	/**
	 * Activate spawner
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void ActivateSpawner();

	/**
	 * Deactivate spawner
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void DeactivateSpawner();

	/**
	 * Get all spawned monsters
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Spawner")
	TArray<AHarmoniaMonsterBase*> GetSpawnedMonsters() const { return SpawnedMonsters; }

	/**
	 * Get number of spawned monsters
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Spawner")
	int32 GetSpawnedMonsterCount() const { return SpawnedMonsters.Num(); }

	// ============================================================================
	// Delegates
	// ============================================================================

	/**
	 * Called when monster is spawned
	 */
	UPROPERTY(BlueprintAssignable, Category = "Spawner|Events")
	FOnMonsterSpawnedDelegate OnMonsterSpawned;

	/**
	 * Called when monster is despawned
	 */
	UPROPERTY(BlueprintAssignable, Category = "Spawner|Events")
	FOnMonsterDespawnedDelegate OnMonsterDespawned;

	/**
	 * Called when all monsters are defeated
	 */
	UPROPERTY(BlueprintAssignable, Category = "Spawner|Events")
	FOnAllMonstersDefeatedDelegate OnAllMonstersDefeated;

protected:
	/**
	 * Spawned monsters
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AHarmoniaMonsterBase>> SpawnedMonsters;

	/**
	 * Timer for initial spawn delay
	 */
	FTimerHandle InitialSpawnTimerHandle;

	/**
	 * Timer for respawn/wave spawning
	 */
	FTimerHandle SpawnTimerHandle;

	/**
	 * Current wave number (for continuous mode)
	 */
	int32 CurrentWave = 0;

	/**
	 * Initialize spawner
	 */
	virtual void InitializeSpawner();

	/**
	 * Get random spawn location
	 */
	virtual FVector GetRandomSpawnLocation() const;

	/**
	 * Select random monster config based on weight
	 */
	virtual FHarmoniaMonsterSpawnConfig SelectRandomMonsterConfig() const;

	/**
	 * Check if can spawn more monsters
	 */
	virtual bool CanSpawn() const;

	/**
	 * Called when spawned monster dies
	 */
	UFUNCTION()
	virtual void OnSpawnedMonsterDeath(AHarmoniaMonsterBase* Monster, AActor* Killer);

	/**
	 * Handle respawn logic
	 */
	virtual void HandleRespawn();

	/**
	 * Handle wave spawning logic
	 */
	virtual void HandleWaveSpawn();

	/**
	 * Cleanup invalid monster references
	 */
	virtual void CleanupMonsterReferences();
};
