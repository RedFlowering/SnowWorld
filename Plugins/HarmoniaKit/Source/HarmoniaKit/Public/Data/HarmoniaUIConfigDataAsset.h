// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaUIConfigDataAsset.h
 * @brief UI configuration data asset for visual and timing settings
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Components/HarmoniaStatusEffectComponent.h"
#include "HarmoniaUIConfigDataAsset.generated.h"

/**
 * @struct FHarmoniaStatusEffectColorConfig
 * @brief Status Effect UI color configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectColorConfig
{
	GENERATED_BODY()

	/** Buff effect border color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor BuffBorderColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** Debuff effect border color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor DebuffBorderColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

	/** Neutral effect border color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor NeutralBorderColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	/** Expiring soon effect highlight color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ExpiringSoonColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);

	/** Stackable effect highlight color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor StackableEffectColor = FLinearColor(0.6f, 0.4f, 1.0f, 1.0f);
};

/**
 * @struct FHarmoniaStatusEffectTimingConfig
 * @brief Status Effect UI timing configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectTimingConfig
{
	GENERATED_BODY()

	/** "Expiring soon" warning threshold in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float ExpiringSoonThreshold = 3.0f;

	/** UI update interval in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float UpdateInterval = 0.1f;

	/** Effect add animation duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AddAnimationDuration = 0.3f;

	/** Effect remove animation duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float RemoveAnimationDuration = 0.2f;
};

/**
 * @struct FHarmoniaMinimapColorConfig
 * @brief Minimap color configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMinimapColorConfig
{
	GENERATED_BODY()

	/** Player icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FLinearColor PlayerIconColor = FLinearColor::White;

	/** Ally icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor AllyIconColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	/** Enemy icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor EnemyIconColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	/** Neutral NPC icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor NeutralIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** Quest objective icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor QuestObjectiveColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** POI (Point of Interest) icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor POIColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f);
};

/**
 * @struct FHarmoniaWorldMarkerColorConfig
 * @brief World marker color configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaWorldMarkerColorConfig
{
	GENERATED_BODY()

	/** Primary quest marker color (gold) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor PrimaryQuestColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** Secondary quest marker color (silver) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor SecondaryQuestColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	/** Interactable object color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	FLinearColor InteractableColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** Danger zone marker color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning")
	FLinearColor DangerColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** Death location corpse run marker color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	FLinearColor CorpseRunColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);
};

/**
 * @class UHarmoniaUIConfigDataAsset
 * @brief UI configuration data asset
 * 
 * Manages Status Effect, minimap, and world marker UI settings in a data-driven manner.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaUIConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaUIConfigDataAsset();

	// ============================================================================
	// Status Effect UI Settings
	// ============================================================================

	/** Status effect color configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Colors")
	FHarmoniaStatusEffectColorConfig StatusEffectColors;

	/** Status effect timing configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Timing")
	FHarmoniaStatusEffectTimingConfig StatusEffectTiming;

	/** Predefined status effect configuration list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Definitions")
	TArray<FHarmoniaStatusEffectConfig> PredefinedEffectConfigs;

	// ============================================================================
	// Minimap Settings
	// ============================================================================

	/** Minimap color configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Colors")
	FHarmoniaMinimapColorConfig MinimapColors;

	// ============================================================================
	// World Marker Settings
	// ============================================================================

	/** World marker color configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMarker|Colors")
	FHarmoniaWorldMarkerColorConfig WorldMarkerColors;

	// ============================================================================
	// Utility Functions
	// ============================================================================

	/** Get border color for effect type */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	FLinearColor GetBorderColorForType(EHarmoniaStatusEffectType Type) const;

	/** Find predefined config by effect tag */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool GetPredefinedEffectConfig(FGameplayTag EffectTag, FHarmoniaStatusEffectConfig& OutConfig) const;
};
