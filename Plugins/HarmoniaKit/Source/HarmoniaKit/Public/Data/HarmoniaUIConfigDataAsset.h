// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Components/HarmoniaStatusEffectComponent.h"
#include "HarmoniaUIConfigDataAsset.generated.h"

/**
 * Status Effect UI ?�상 ?�정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectColorConfig
{
	GENERATED_BODY()

	/** 버프 ?�과 ?�두�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor BuffBorderColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** ?�버???�과 ?�두�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor DebuffBorderColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

	/** 중립 ?�과 ?�두�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor NeutralBorderColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	/** 만료 ?�박 ?�과 강조 ?�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ExpiringSoonColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);

	/** ?�택 가???�과 강조 ?�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor StackableEffectColor = FLinearColor(0.6f, 0.4f, 1.0f, 1.0f);
};

/**
 * Status Effect UI ?�?�밍 ?�정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectTimingConfig
{
	GENERATED_BODY()

	/** "�?만료" 경고 ?�시 ?�계�?(�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float ExpiringSoonThreshold = 3.0f;

	/** UI ?�데?�트 주기 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float UpdateInterval = 0.1f;

	/** ?�과 추�? ???�니메이??지???�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AddAnimationDuration = 0.3f;

	/** ?�과 ?�거 ???�니메이??지???�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float RemoveAnimationDuration = 0.2f;
};

/**
 * 미니�??�상 ?�정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMinimapColorConfig
{
	GENERATED_BODY()

	/** ?�레?�어 ?�이�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FLinearColor PlayerIconColor = FLinearColor::White;

	/** ?�군 ?�이�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor AllyIconColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	/** ???�이�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor EnemyIconColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	/** 중립 NPC ?�이�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor NeutralIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** ?�스??목표 ?�이�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor QuestObjectiveColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** POI (관??지?? ?�이�??�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor POIColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f);
};

/**
 * ?�드 마커 ?�상 ?�정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaWorldMarkerColorConfig
{
	GENERATED_BODY()

	/** 기본 주요 ?�스??마커 ?�상 (골드) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor PrimaryQuestColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** 보조 ?�스??마커 ?�상 (?�버) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor SecondaryQuestColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	/** ?�호?�용 가??객체 ?�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	FLinearColor InteractableColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** ?�험 지??마커 ?�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning")
	FLinearColor DangerColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** ?�망 지???�이???�수 마커 ?�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	FLinearColor CorpseRunColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);
};

/**
 * UI ?�정 ?�이???�셋
 * 
 * Status Effect, 미니�? ?�드 마커 ??UI 관???�정??
 * ?�이???�리�?방식?�로 관리합?�다.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaUIConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaUIConfigDataAsset();

	// ============================================================================
	// Status Effect UI ?�정
	// ============================================================================

	/** ?�태 ?�과 ?�상 ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Colors")
	FHarmoniaStatusEffectColorConfig StatusEffectColors;

	/** ?�태 ?�과 ?�?�밍 ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Timing")
	FHarmoniaStatusEffectTimingConfig StatusEffectTiming;

	/** 미리 ?�의???�태 ?�과 ?�정 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Definitions")
	TArray<FHarmoniaStatusEffectConfig> PredefinedEffectConfigs;

	// ============================================================================
	// 미니�??�정
	// ============================================================================

	/** 미니�??�상 ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Colors")
	FHarmoniaMinimapColorConfig MinimapColors;

	// ============================================================================
	// ?�드 마커 ?�정
	// ============================================================================

	/** ?�드 마커 ?�상 ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMarker|Colors")
	FHarmoniaWorldMarkerColorConfig WorldMarkerColors;

	// ============================================================================
	// ?�틸리티 ?�수
	// ============================================================================

	/** ?�과 ?�?�에 ?�른 ?�두�??�상 반환 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	FLinearColor GetBorderColorForType(EHarmoniaStatusEffectType Type) const;

	/** ?�과 ?�그�?미리 ?�의???�정 찾기 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool GetPredefinedEffectConfig(FGameplayTag EffectTag, FHarmoniaStatusEffectConfig& OutConfig) const;
};
