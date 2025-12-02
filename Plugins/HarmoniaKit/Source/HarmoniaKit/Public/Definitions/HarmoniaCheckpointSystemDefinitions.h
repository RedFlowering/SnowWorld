// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaCheckpointSystemDefinitions.generated.h"

/**
 * Checkpoint State
 * ì²´í¬?¬ì¸???íƒœ
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointState : uint8
{
	// ë¹„í™œ?±í™” - ?„ì§ ë°œê²¬?˜ì? ëª»í•¨
	Inactive UMETA(DisplayName = "Inactive"),

	// ?œì„±?”ë¨ - ë°œê²¬?ˆì?ë§?ê³µëª…?˜ì? ?ŠìŒ
	Activated UMETA(DisplayName = "Activated"),

	// ê³µëª… ì¤?- ?„ì¬ ?Œë ˆ?´ì–´ê°€ ê³µëª…?˜ê³  ?ˆìŒ
	Resonating UMETA(DisplayName = "Resonating")
};

/**
 * Resonance Frequency Type
 * ê³µëª… ì£¼íŒŒ???€??- ê°??¬ë¦¬?¤íƒˆ??ê³ ìœ ???¹ì„±
 */
UENUM(BlueprintType)
enum class EHarmoniaResonanceFrequency : uint8
{
	// ?¸ë¥¸ ê³µëª… - ì°¨ë¶„?˜ê³  ?‰í™”ë¡œìš´
	Azure UMETA(DisplayName = "Azure (Blue)"),

	// ë¶‰ì? ê³µëª… - ?´ì •?ì´ê³?ê°•ë ¬??
	Crimson UMETA(DisplayName = "Crimson (Red)"),

	// ?¹ìƒ‰ ê³µëª… - ?ëª…?¥ê³¼ ?Œë³µ
	Verdant UMETA(DisplayName = "Verdant (Green)"),

	// ê¸ˆìƒ‰ ê³µëª… - ? ì„±?˜ê³  ê³ ê???
	Aurum UMETA(DisplayName = "Aurum (Gold)"),

	// ë³´ë¼ ê³µëª… - ? ë¹„ë¡?³  ë§ˆë²•?ì¸
	Violet UMETA(DisplayName = "Violet (Purple)"),

	// ?°ìƒ‰ ê³µëª… - ?œìˆ˜?˜ê³  ì¤‘ë¦½?ì¸
	Luminous UMETA(DisplayName = "Luminous (White)")
};

/**
 * Checkpoint Upgrade Type
 * ì²´í¬?¬ì¸??ê°•í™” ?€??
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointUpgradeType : uint8
{
	// ?Œë³µ??ì¦ê?
	EnhancedRestoration UMETA(DisplayName = "Enhanced Restoration"),

	// ê³µëª… ?ë„ ì¦ê?
	FasterResonance UMETA(DisplayName = "Faster Resonance"),

	// ?”ë ˆ?¬íŠ¸ ë¹„ìš© ê°ì†Œ
	ReducedTeleportCost UMETA(DisplayName = "Reduced Teleport Cost"),

	// ê³µëª… ë²”ìœ„ ì¦ê? (ì£¼ë? ?Œë ˆ?´ì–´???Œë³µ)
	ExtendedRange UMETA(DisplayName = "Extended Range"),

	// ë¶€????ì¶”ê? ë³´ë„ˆ??
	RespawnBonus UMETA(DisplayName = "Respawn Bonus")
};

/**
 * Checkpoint Data
 * ì²´í¬?¬ì¸???€???°ì´??
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointData
{
	GENERATED_BODY()

	// ì²´í¬?¬ì¸??ê³ ìœ  ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	// ì²´í¬?¬ì¸???´ë¦„ (UI ?œì‹œ??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	// ì²´í¬?¬ì¸???¤ëª…
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointDescription;

	// ê³µëª… ì£¼íŒŒ??(?‰ìƒ/?¹ì„±)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	// ì²´í¬?¬ì¸???„ì¹˜
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FVector Location = FVector::ZeroVector;

	// ì²´í¬?¬ì¸???Œì „
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	// ?œì„±???¬ë?
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	bool bActivated = false;

	// ë§ˆì?ë§?ê³µëª… ?œê°„
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FDateTime LastResonanceTime;

	// ê°•í™” ?ˆë²¨
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
 * Resonance Result
 * ê³µëª… ê²°ê³¼
 */
USTRUCT(BlueprintType)
struct FHarmoniaResonanceResult
{
	GENERATED_BODY()

	// ?±ê³µ ?¬ë?
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bSuccess = false;

	// ?Œë³µ??ì²´ë ¥
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float HealthRestored = 0.0f;

	// ?Œë³µ??ë§ˆë‚˜/?¤íƒœë¯¸ë‚˜
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float ResourceRestored = 0.0f;

	// ë¦¬ìŠ¤?°ëœ ????
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	int32 EnemiesRespawned = 0;

	// ê²Œì„ ?€???¬ë?
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bGameSaved = false;

	// ?¤íŒ¨ ?¬ìœ 
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	FText FailureReason;
};

/**
 * Teleport Result
 * ?”ë ˆ?¬íŠ¸ ê²°ê³¼
 */
USTRUCT(BlueprintType)
struct FHarmoniaTeleportResult
{
	GENERATED_BODY()

	// ?±ê³µ ?¬ë?
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	bool bSuccess = false;

	// ?”ë ˆ?¬íŠ¸??ì²´í¬?¬ì¸??ID
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FName DestinationCheckpointID;

	// ?Œëª¨??ë¦¬ì†Œ??(?Œìš¸, ë§ˆë‚˜ ??
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	int32 ResourceCost = 0;

	// ?¤íŒ¨ ?¬ìœ 
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FText FailureReason;
};

/**
 * Checkpoint Configuration
 * ì²´í¬?¬ì¸???¤ì •
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointConfig
{
	GENERATED_BODY()

	// ê³µëª… ??ì²´ë ¥ ?Œë³µë¥?(0.0 ~ 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRestorationRate = 1.0f;

	// ê³µëª… ??ë¦¬ì†Œ???Œë³µë¥?(0.0 ~ 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ResourceRestorationRate = 1.0f;

	// ê³µëª… ?Œìš” ?œê°„ (ì´?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.1"))
	float ResonanceDuration = 3.0f;

	// ê³µëª… ì¤??€ì§ì´ë©?ì·¨ì†Œ ?¬ë?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnMovement = true;

	// ê³µëª… ì¤??¼ê²© ??ì·¨ì†Œ ?¬ë?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnDamage = true;

	// ?”ë ˆ?¬íŠ¸ ê¸°ë³¸ ë¹„ìš© (?Œìš¸)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 BaseTeleportCost = 100;

	// ?”ë ˆ?¬íŠ¸ ê±°ë¦¬??ì¶”ê? ë¹„ìš©
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float TeleportCostPerDistance = 0.1f;

	// ì£½ì—ˆ????ë§ˆì?ë§?ì²´í¬?¬ì¸?¸ì—??ë¦¬ìŠ¤??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnAtLastCheckpoint = true;

	// ê³µëª… ??ì£¼ë? ??ë¦¬ìŠ¤??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnEnemiesOnResonance = true;

	// ê³µëª… ???ë™ ?€??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAutoSaveOnResonance = true;

	// ê³µëª… ë²”ìœ„ (ë©€?°í”Œ?ˆì´ ??ì£¼ë? ?Œë ˆ?´ì–´???Œë³µ)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ResonanceRange = 500.0f;

	FHarmoniaCheckpointConfig()
	{
	}
};
