// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HarmoniaMusicSystemDefinitions.generated.h"

/**
 * 악기 타입
 */
UENUM(BlueprintType)
enum class EInstrumentType : uint8
{
	Lute		UMETA(DisplayName = "Lute"),		// 류트
	Flute		UMETA(DisplayName = "Flute"),		// 플루트
	Harp		UMETA(DisplayName = "Harp"),		// 하프
	Drum		UMETA(DisplayName = "Drum"),		// 드럼
	Horn		UMETA(DisplayName = "Horn"),		// 호른
	Violin		UMETA(DisplayName = "Violin"),		// 바이올린
	Piano		UMETA(DisplayName = "Piano"),		// 피아노
	Guitar		UMETA(DisplayName = "Guitar")		// 기타
};

/**
 * 음악 장르
 */
UENUM(BlueprintType)
enum class EMusicGenre : uint8
{
	Classical	UMETA(DisplayName = "Classical"),	// 클래식
	Folk		UMETA(DisplayName = "Folk"),		// 민속
	Battle		UMETA(DisplayName = "Battle"),		// 전투
	Healing		UMETA(DisplayName = "Healing"),		// 치유
	Buff		UMETA(DisplayName = "Buff"),		// 버프
	Debuff		UMETA(DisplayName = "Debuff")		// 디버프
};

/**
 * 연주 품질
 */
UENUM(BlueprintType)
enum class EPerformanceQuality : uint8
{
	Poor		UMETA(DisplayName = "Poor"),
	Fair		UMETA(DisplayName = "Fair"),
	Good		UMETA(DisplayName = "Good"),
	Great		UMETA(DisplayName = "Great"),
	Perfect		UMETA(DisplayName = "Perfect"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * 음악 버프 효과
 */
USTRUCT(BlueprintType)
struct FMusicBuffEffect
{
	GENERATED_BODY()

	/** 버프 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** 버프 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** 체력 회복/초 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRegenPerSecond = 0.0f;

	/** 마나 회복/초 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRegenPerSecond = 0.0f;

	/** 스태미나 회복/초 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRegenPerSecond = 0.0f;

	/** 공격력 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** 방어력 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** 이동속도 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** 크리티컬 확률 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** 쿨다운 감소 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CooldownReduction = 0.0f;

	/** 경험치 획득량 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** 범위 내 적용 (m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float EffectRadius = 1000.0f;

	/** 버프 지속시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 60.0f;

	/** 아군에게만 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAllyOnly = true;

	/** 자신에게도 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAffectSelf = true;
};

/**
 * 악보 데이터
 */
USTRUCT(BlueprintType)
struct FMusicSheetData
{
	GENERATED_BODY()

	/** 곡 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName MusicName;

	/** 곡 제목 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Title;

	/** 곡 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Description;

	/** 음악 장르 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	EMusicGenre Genre = EMusicGenre::Classical;

	/** 필요 악기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<EInstrumentType> RequiredInstruments;

	/** 최소 연주 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 MinPerformanceLevel = 1;

	/** 난이도 (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 Difficulty = 5;

	/** 연주 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float PerformanceDuration = 30.0f;

	/** 음악 버프 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FMusicBuffEffect BuffEffect;

	/** 품질별 버프 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TMap<EPerformanceQuality, float> QualityMultipliers;

	/** 경험치 보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 ExperienceReward = 25;

	/** 음악 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<class USoundBase> MusicSound;

	/** 악보 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 숨겨진 악보 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bHidden = false;
};

/**
 * 악기 데이터
 */
USTRUCT(BlueprintType)
struct FInstrumentData
{
	GENERATED_BODY()

	/** 악기 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	FName InstrumentName;

	/** 악기 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	EInstrumentType InstrumentType = EInstrumentType::Lute;

	/** 악기 등급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 InstrumentTier = 1;

	/** 연주 품질 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float QualityBonus = 0.0f;

	/** 버프 효과 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float BuffEffectBonus = 0.0f;

	/** 버프 범위 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float RangeBonus = 0.0f;

	/** 내구도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 Durability = 100;

	/** 최대 내구도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 MaxDurability = 100;

	/** 악기 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	TSoftObjectPtr<UStaticMesh> InstrumentMesh;
};

/**
 * 연주 결과
 */
USTRUCT(BlueprintType)
struct FPerformanceResult
{
	GENERATED_BODY()

	/** 연주한 곡 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName MusicID;

	/** 연주 품질 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	EPerformanceQuality Quality = EPerformanceQuality::Good;

	/** 적용된 버프 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FMusicBuffEffect AppliedBuff;

	/** 영향받은 액터 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 AffectedActorCount = 0;

	/** 획득 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** 완벽한 연주 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * 리듬 노트 (미니게임용)
 */
USTRUCT(BlueprintType)
struct FRhythmNote
{
	GENERATED_BODY()

	/** 노트 타입 (0-3: 상하좌우 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 NoteType = 0;

	/** 노트 타이밍 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float Timing = 0.0f;

	/** 완벽한 타이밍 허용 범위 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float PerfectWindow = 0.05f;

	/** 좋은 타이밍 허용 범위 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float GoodWindow = 0.15f;
};

/**
 * 연주 특성
 */
USTRUCT(BlueprintType)
struct FPerformanceTrait
{
	GENERATED_BODY()

	/** 특성 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** 특성 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** 연주 품질 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** 버프 효과 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** 버프 지속시간 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** 버프 범위 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RangeBonus = 0.0f;

	/** 경험치 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** 앙코르 확률 (%) - 버프가 2배 지속 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float EncoreChance = 0.0f;
};
