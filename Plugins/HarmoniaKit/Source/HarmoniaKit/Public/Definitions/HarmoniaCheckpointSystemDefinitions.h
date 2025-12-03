// Copyright 2025 Snow Game Studio.

#pragma once

/**
 * @file HarmoniaCheckpointSystemDefinitions.h
 * @brief Checkpoint system definitions and data structures
 * @author Harmonia Team
 * 
 * Defines checkpoint states, resonance frequencies, and upgrade types
 * for the crystal resonator save point system.
 */

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaCheckpointSystemDefinitions.generated.h"

/**
 * @enum EHarmoniaCheckpointState
 * @brief Checkpoint state
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointState : uint8
{
	/** Inactive - not yet discovered */
	Inactive UMETA(DisplayName = "Inactive"),

	/** Activated - discovered but not resonating */
	Activated UMETA(DisplayName = "Activated"),

	/** Resonating - player is currently resonating */
	Resonating UMETA(DisplayName = "Resonating")
};

/**
 * @enum EHarmoniaResonanceFrequency
 * @brief Resonance frequency type - unique attribute of each crystal
 */
UENUM(BlueprintType)
enum class EHarmoniaResonanceFrequency : uint8
{
	/** Azure resonance - calm and peaceful */
	Azure UMETA(DisplayName = "Azure (Blue)"),

	/** Crimson resonance - passionate and intense */
	Crimson UMETA(DisplayName = "Crimson (Red)"),

	/** Verdant resonance - life and healing */
	Verdant UMETA(DisplayName = "Verdant (Green)"),

	/** Aurum resonance - sacred and noble */
	Aurum UMETA(DisplayName = "Aurum (Gold)"),

	/** Violet resonance - mysterious and magical */
	Violet UMETA(DisplayName = "Violet (Purple)"),

	/** Luminous resonance - pure and neutral */
	Luminous UMETA(DisplayName = "Luminous (White)")
};

/**
 * @enum EHarmoniaCheckpointUpgradeType
 * @brief Checkpoint upgrade types
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointUpgradeType : uint8
{
	/** Increased healing */
	EnhancedRestoration UMETA(DisplayName = "Enhanced Restoration"),

	/** Faster resonance speed */
	FasterResonance UMETA(DisplayName = "Faster Resonance"),

	/** Reduced teleport cost */
	ReducedTeleportCost UMETA(DisplayName = "Reduced Teleport Cost"),

	/** Extended resonance range (heals nearby players) */
	ExtendedRange UMETA(DisplayName = "Extended Range"),

	/** Additional bonus on respawn */
	RespawnBonus UMETA(DisplayName = "Respawn Bonus")
};

/**
 * @struct FHarmoniaCheckpointData
 * @brief Checkpoint data structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointData
{
	GENERATED_BODY()

	/** Checkpoint unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	/** Checkpoint name (for UI display) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	/** Checkpoint description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointDescription;

	/** Resonance frequency (visual/attribute) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	/** Checkpoint location */
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FVector Location = FVector::ZeroVector;

	/** Checkpoint rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Activated status */
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	bool bActivated = false;

	/** Last resonance time */
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FDateTime LastResonanceTime;

	/** Upgrade levels */
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	TMap<EHarmoniaCheckpointUpgradeType, int32> UpgradeLevels;

	FHarmoniaCheckpointData()
	{
		CheckpointID = NAME_None;
		CheckpointName = FText::GetEmpty();
		CheckpointDescription = FText::GetEmpty();
	}

	bool operator==(const FHarmoniaCheckpointData& Other) const
	{
		return CheckpointID == Other.CheckpointID;
	}
};

/**
 * @struct FHarmoniaResonanceResult
 * @brief Resonance result structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaResonanceResult
{
	GENERATED_BODY()

	/** Success status */
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bSuccess = false;

	/** Health restored */
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float HealthRestored = 0.0f;

	/** Resource restored (mana/stamina) */
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float ResourceRestored = 0.0f;

	/** Number of enemies respawned */
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	int32 EnemiesRespawned = 0;

	/** Game saved status */
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bGameSaved = false;

	/** Failure reason */
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	FText FailureReason;
};

/**
 * @struct FHarmoniaTeleportResult
 * @brief Teleport result structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaTeleportResult
{
	GENERATED_BODY()

	/** Success status */
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	bool bSuccess = false;

	/** Teleport destination checkpoint ID */
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FName DestinationCheckpointID;

	/** Resource cost (souls, mana, etc.) */
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	int32 ResourceCost = 0;

	/** Failure reason */
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FText FailureReason;
};

/**
 * @struct FHarmoniaCheckpointConfig
 * @brief Checkpoint configuration structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointConfig
{
	GENERATED_BODY()

	/** Health restoration rate on resonance (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRestorationRate = 1.0f;

	/** Resource restoration rate on resonance (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ResourceRestorationRate = 1.0f;

	/** Resonance duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.1"))
	float ResonanceDuration = 3.0f;

	/** Cancel resonance on movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnMovement = true;

	/** Cancel resonance on damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnDamage = true;

	/** Base teleport cost (souls) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 BaseTeleportCost = 100;

	/** Additional cost per distance for teleport */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float TeleportCostPerDistance = 0.1f;

	/** Respawn at last checkpoint on death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnAtLastCheckpoint = true;

	/** Respawn enemies on resonance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnEnemiesOnResonance = true;

	/** Auto-save on resonance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAutoSaveOnResonance = true;

	/** Resonance range (heals nearby players in multiplayer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ResonanceRange = 500.0f;

	FHarmoniaCheckpointConfig()
	{
	}
};
