// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/HarmoniaDynamicDifficultySubsystem.h"
#include "HarmoniaDDAConfigDataAsset.generated.h"

/**
 * DDA (Dynamic Difficulty Adjustment) ê°€ì¤‘ì¹˜ ?¤ì •
 * ê°?ë©”íŠ¸ë¦?´ ?¤í‚¬ ?ˆì´?…ì— ë¯¸ì¹˜???í–¥?„ë? ?•ì˜
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAWeightConfig
{
	GENERATED_BODY()

	/** ?¬ë§ ?Ÿìˆ˜ ê°€ì¤‘ì¹˜ (?’ì„?˜ë¡ ?¬ë§???¤í‚¬??ë§ì´ ?í–¥) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeathWeight = 0.25f;

	/** ?¹ë¦¬ ???¨ì? ì²´ë ¥ ê°€ì¤‘ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VictoryHealthWeight = 0.15f;

	/** ?¨ë§ ?±ê³µë¥?ê°€ì¤‘ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ParryWeight = 0.20f;

	/** ?Œí”¼ ?±ê³µë¥?ê°€ì¤‘ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeWeight = 0.15f;

	/** ëª…ì¤‘ë¥?ê°€ì¤‘ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccuracyWeight = 0.10f;

	/** ?¼í•´ ë¹„ìœ¨ (ì£¼ëŠ” ?¼í•´/ë°›ëŠ” ?¼í•´) ê°€ì¤‘ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageRatioWeight = 0.15f;
};

/**
 * DDA ?„ê³„ê°??¤ì •
 * ê°??„ë¡œ?„ë¡œ ?„í™˜?˜ëŠ” ?¤í‚¬ ?ˆì´??ê¸°ì?
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAThresholdConfig
{
	GENERATED_BODY()

	/** Beginner ?„ë¡œ??ìµœë? ?¤í‚¬ ?ˆì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 BeginnerMaxRating = 25;

	/** Learning ?„ë¡œ??ìµœë? ?¤í‚¬ ?ˆì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 LearningMaxRating = 45;

	/** Standard ?„ë¡œ??ìµœë? ?¤í‚¬ ?ˆì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 StandardMaxRating = 65;

	/** Skilled ?„ë¡œ??ìµœë? ?¤í‚¬ ?ˆì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 SkilledMaxRating = 85;

	// 85 ?´ìƒ?€ Master ?„ë¡œ??
};

/**
 * ì¢Œì ˆ ê°ì? ?¤ì •
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaFrustrationConfig
{
	GENERATED_BODY()

	/** ì¢Œì ˆ ê°ì?ë¥??„í•œ ?°ì† ?¬ë§ ?„ê³„ê°?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 ConsecutiveDeathThreshold = 3;

	/** ì¢Œì ˆ ê°ì?ë¥??„í•œ ?œê°„???¬ë§ ?„ê³„ê°?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 DeathsPerHourThreshold = 5;

	/** ì¢Œì ˆ ???ìš©???¨ê²¨ì§?ë³´ë„ˆ??ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float FrustrationAssistMultiplier = 1.2f;

	/** ì¢Œì ˆ ???•ìƒ?”ë˜ê¸°ê¹Œì§€ ?„ìš”???¹ë¦¬ ?Ÿìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "10"))
	int32 RecoveryWinsRequired = 2;
};

/**
 * DDA ?¤ì • ?°ì´???ì…‹
 * 
 * ?™ì  ?œì´??ì¡°ì ˆ ?œìŠ¤?œì˜ ëª¨ë“  ?¤ì •???°ì´???œë¦¬ë¸?ë°©ì‹?¼ë¡œ ê´€ë¦¬í•©?ˆë‹¤.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDDAConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDDAConfigDataAsset();

	// ============================================================================
	// ê°€ì¤‘ì¹˜ ë°??„ê³„ê°?
	// ============================================================================

	/** ?¤í‚¬ ?ˆì´??ê³„ì‚° ê°€ì¤‘ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Weights")
	FHarmoniaDDAWeightConfig Weights;

	/** ?„ë¡œ???„í™˜ ?„ê³„ê°?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Thresholds")
	FHarmoniaDDAThresholdConfig Thresholds;

	/** ì¢Œì ˆ ê°ì? ?¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Frustration")
	FHarmoniaFrustrationConfig FrustrationConfig;

	// ============================================================================
	// DDA ?„ë¡œ??
	// ============================================================================

	/** Beginner ?„ë¡œ??(ê°€???¬ì?) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile BeginnerProfile;

	/** Learning ?„ë¡œ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile LearningProfile;

	/** Standard ?„ë¡œ??(ê¸°ë³¸) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile StandardProfile;

	/** Skilled ?„ë¡œ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile SkilledProfile;

	/** Master ?„ë¡œ??(ê°€???´ë ¤?€) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile MasterProfile;

	// ============================================================================
	// ê³ ê¸‰ ?¤ì •
	// ============================================================================

	/** ?¤í‚¬ ?ˆì´??ë³€???ë„ (0.1 = ?ë¦¼, 1.0 = ë¹ ë¦„) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SkillRatingChangeSpeed = 0.5f;

	/** ë©”íŠ¸ë¦?ê¸°ë¡ ë³´ê? ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "300", ClampMax = "7200"))
	float MetricsRetentionTime = 3600.0f; // 1?œê°„

	/** ë³´ìŠ¤?„ì—??DDA ?œì„±???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bEnableDDAForBosses = true;

	/** ë©€?°í”Œ?ˆì´?ì„œ DDA ê³„ì‚° ë°©ì‹ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bUseAverageInMultiplayer = true;

	// ============================================================================
	// ? í‹¸ë¦¬í‹° ?¨ìˆ˜
	// ============================================================================

	/** ?¤í‚¬ ?ˆì´?…ì— ?´ë‹¹?˜ëŠ” ?„ë¡œ??ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "DDA")
	const FHarmoniaDDAProfile& GetProfileForSkillRating(float SkillRating) const;

	/** ?„ë¡œ???´ë¦„ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "DDA")
	FString GetProfileName(float SkillRating) const;
};
