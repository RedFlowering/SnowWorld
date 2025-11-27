// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Components/HarmoniaStatusEffectComponent.h"
#include "HarmoniaUIConfigDataAsset.generated.h"

/**
 * Status Effect UI 색상 설정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectColorConfig
{
	GENERATED_BODY()

	/** 버프 효과 테두리 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor BuffBorderColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** 디버프 효과 테두리 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor DebuffBorderColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

	/** 중립 효과 테두리 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor NeutralBorderColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	/** 만료 임박 효과 강조 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ExpiringSoonColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);

	/** 스택 가능 효과 강조 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor StackableEffectColor = FLinearColor(0.6f, 0.4f, 1.0f, 1.0f);
};

/**
 * Status Effect UI 타이밍 설정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectTimingConfig
{
	GENERATED_BODY()

	/** "곧 만료" 경고 표시 임계값 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float ExpiringSoonThreshold = 3.0f;

	/** UI 업데이트 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float UpdateInterval = 0.1f;

	/** 효과 추가 시 애니메이션 지속 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AddAnimationDuration = 0.3f;

	/** 효과 제거 시 애니메이션 지속 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float RemoveAnimationDuration = 0.2f;
};

/**
 * 미니맵 색상 설정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMinimapColorConfig
{
	GENERATED_BODY()

	/** 플레이어 아이콘 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FLinearColor PlayerIconColor = FLinearColor::White;

	/** 아군 아이콘 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor AllyIconColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	/** 적 아이콘 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor EnemyIconColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	/** 중립 NPC 아이콘 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor NeutralIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** 퀘스트 목표 아이콘 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor QuestObjectiveColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** POI (관심 지점) 아이콘 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FLinearColor POIColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f);
};

/**
 * 월드 마커 색상 설정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaWorldMarkerColorConfig
{
	GENERATED_BODY()

	/** 기본 주요 퀘스트 마커 색상 (골드) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor PrimaryQuestColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);

	/** 보조 퀘스트 마커 색상 (실버) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor SecondaryQuestColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	/** 상호작용 가능 객체 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	FLinearColor InteractableColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** 위험 지역 마커 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning")
	FLinearColor DangerColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** 사망 지점/아이템 회수 마커 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	FLinearColor CorpseRunColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);
};

/**
 * UI 설정 데이터 에셋
 * 
 * Status Effect, 미니맵, 월드 마커 등 UI 관련 설정을 
 * 데이터 드리븐 방식으로 관리합니다.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaUIConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaUIConfigDataAsset();

	// ============================================================================
	// Status Effect UI 설정
	// ============================================================================

	/** 상태 효과 색상 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Colors")
	FHarmoniaStatusEffectColorConfig StatusEffectColors;

	/** 상태 효과 타이밍 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Timing")
	FHarmoniaStatusEffectTimingConfig StatusEffectTiming;

	/** 미리 정의된 상태 효과 설정 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Definitions")
	TArray<FHarmoniaStatusEffectConfig> PredefinedEffectConfigs;

	// ============================================================================
	// 미니맵 설정
	// ============================================================================

	/** 미니맵 색상 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Colors")
	FHarmoniaMinimapColorConfig MinimapColors;

	// ============================================================================
	// 월드 마커 설정
	// ============================================================================

	/** 월드 마커 색상 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMarker|Colors")
	FHarmoniaWorldMarkerColorConfig WorldMarkerColors;

	// ============================================================================
	// 유틸리티 함수
	// ============================================================================

	/** 효과 타입에 따른 테두리 색상 반환 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	FLinearColor GetBorderColorForType(EHarmoniaStatusEffectType Type) const;

	/** 효과 태그로 미리 정의된 설정 찾기 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool GetPredefinedEffectConfig(FGameplayTag EffectTag, FHarmoniaStatusEffectConfig& OutConfig) const;
};
