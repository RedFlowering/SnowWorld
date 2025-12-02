// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Components/HarmoniaStatusEffectComponent.h"
#include "HarmoniaUIConfigDataAsset.generated.h"

/**
 * Status Effect UI ?‰ìƒ ?¤ì •
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectColorConfig
{
	GENERATED_BODY()

	/** ë²„í”„ ?¨ê³¼ ?Œë‘ë¦??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor BuffBorderColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** ?”ë²„???¨ê³¼ ?Œë‘ë¦??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor DebuffBorderColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

	/** ì¤‘ë¦½ ?¨ê³¼ ?Œë‘ë¦??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor NeutralBorderColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	/** ë§Œë£Œ ?„ë°• ?¨ê³¼ ê°•ì¡° ?‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ExpiringSoonColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);

	/** ?¤íƒ ê°€???¨ê³¼ ê°•ì¡° ?‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor StackableEffectColor = FLinearColor(0.6f, 0.4f, 1.0f, 1.0f);
};

/**
 * Status Effect UI ?€?´ë° ?¤ì •
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectTimingConfig
{
	GENERATED_BODY()

	/** "ê³?ë§Œë£Œ" ê²½ê³  ?œì‹œ ?„ê³„ê°?(ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float ExpiringSoonThreshold = 3.0f;

	/** UI ?…ë°?´íŠ¸ ì£¼ê¸° (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float UpdateInterval = 0.1f;

	/** ?¨ê³¼ ì¶”ê? ??? ë‹ˆë©”ì´??ì§€???œê°„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AddAnimationDuration = 0.3f;

	/** ?¨ê³¼ ?œê±° ??? ë‹ˆë©”ì´??ì§€???œê°„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float RemoveAnimationDuration = 0.2f;
};

/**
 * ë¯¸ë‹ˆë§??‰ìƒ ?¤ì •
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMinimapColorConfig
{
	GENERATED_BODY()

	/** ?Œë ˆ?´ì–´ ?„ì´ì½??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FLinearColor PlayerIconColor = FLinearColor::White;

	/** ?„êµ° ?„ì´ì½??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor AllyIconColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	/** ???„ì´ì½??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor EnemyIconColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	/** ì¤‘ë¦½ NPC ?„ì´ì½??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor NeutralIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** ?˜ìŠ¤??ëª©í‘œ ?„ì´ì½??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor QuestObjectiveColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** POI (ê´€??ì§€?? ?„ì´ì½??‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor POIColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f);
};

/**
 * ?”ë“œ ë§ˆì»¤ ?‰ìƒ ?¤ì •
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaWorldMarkerColorConfig
{
	GENERATED_BODY()

	/** ê¸°ë³¸ ì£¼ìš” ?˜ìŠ¤??ë§ˆì»¤ ?‰ìƒ (ê³¨ë“œ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor PrimaryQuestColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** ë³´ì¡° ?˜ìŠ¤??ë§ˆì»¤ ?‰ìƒ (?¤ë²„) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor SecondaryQuestColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	/** ?í˜¸?‘ìš© ê°€??ê°ì²´ ?‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	FLinearColor InteractableColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** ?„í—˜ ì§€??ë§ˆì»¤ ?‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning")
	FLinearColor DangerColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** ?¬ë§ ì§€???„ì´???Œìˆ˜ ë§ˆì»¤ ?‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	FLinearColor CorpseRunColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);
};

/**
 * UI ?¤ì • ?°ì´???ì…‹
 * 
 * Status Effect, ë¯¸ë‹ˆë§? ?”ë“œ ë§ˆì»¤ ??UI ê´€???¤ì •??
 * ?°ì´???œë¦¬ë¸?ë°©ì‹?¼ë¡œ ê´€ë¦¬í•©?ˆë‹¤.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaUIConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaUIConfigDataAsset();

	// ============================================================================
	// Status Effect UI ?¤ì •
	// ============================================================================

	/** ?íƒœ ?¨ê³¼ ?‰ìƒ ?¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Colors")
	FHarmoniaStatusEffectColorConfig StatusEffectColors;

	/** ?íƒœ ?¨ê³¼ ?€?´ë° ?¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Timing")
	FHarmoniaStatusEffectTimingConfig StatusEffectTiming;

	/** ë¯¸ë¦¬ ?•ì˜???íƒœ ?¨ê³¼ ?¤ì • ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Definitions")
	TArray<FHarmoniaStatusEffectConfig> PredefinedEffectConfigs;

	// ============================================================================
	// ë¯¸ë‹ˆë§??¤ì •
	// ============================================================================

	/** ë¯¸ë‹ˆë§??‰ìƒ ?¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Colors")
	FHarmoniaMinimapColorConfig MinimapColors;

	// ============================================================================
	// ?”ë“œ ë§ˆì»¤ ?¤ì •
	// ============================================================================

	/** ?”ë“œ ë§ˆì»¤ ?‰ìƒ ?¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMarker|Colors")
	FHarmoniaWorldMarkerColorConfig WorldMarkerColors;

	// ============================================================================
	// ? í‹¸ë¦¬í‹° ?¨ìˆ˜
	// ============================================================================

	/** ?¨ê³¼ ?€?…ì— ?°ë¥¸ ?Œë‘ë¦??‰ìƒ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	FLinearColor GetBorderColorForType(EHarmoniaStatusEffectType Type) const;

	/** ?¨ê³¼ ?œê·¸ë¡?ë¯¸ë¦¬ ?•ì˜???¤ì • ì°¾ê¸° */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool GetPredefinedEffectConfig(FGameplayTag EffectTag, FHarmoniaStatusEffectConfig& OutConfig) const;
};
