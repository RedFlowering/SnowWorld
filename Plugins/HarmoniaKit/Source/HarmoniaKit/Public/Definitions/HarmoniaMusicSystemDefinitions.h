// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaMusicSystemDefinitions.generated.h"

/**
 * ?�기 ?�??
 */
UENUM(BlueprintType)
enum class EInstrumentType : uint8
{
	Lute		UMETA(DisplayName = "Lute"),		// 류트
	Flute		UMETA(DisplayName = "Flute"),		// ?�루??
	Harp		UMETA(DisplayName = "Harp"),		// ?�프
	Drum		UMETA(DisplayName = "Drum"),		// ?�럼
	Horn		UMETA(DisplayName = "Horn"),		// ?�른
	Violin		UMETA(DisplayName = "Violin"),		// 바이?�린
	Piano		UMETA(DisplayName = "Piano"),		// ?�아??
	Guitar		UMETA(DisplayName = "Guitar")		// 기�?
};

/**
 * ?�악 ?�르
 */
UENUM(BlueprintType)
enum class EMusicGenre : uint8
{
	Classical	UMETA(DisplayName = "Classical"),	// ?�래??
	Folk		UMETA(DisplayName = "Folk"),		// 민속
	Battle		UMETA(DisplayName = "Battle"),		// ?�투
	Healing		UMETA(DisplayName = "Healing"),		// 치유
	Buff		UMETA(DisplayName = "Buff"),		// 버프
	Debuff		UMETA(DisplayName = "Debuff")		// ?�버??
};

/**
 * ?�주 ?�질
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
 * ?�악 버프 ?�과
 */
USTRUCT(BlueprintType)
struct FMusicBuffEffect
{
	GENERATED_BODY()

	/** 버프 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** 버프 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** 체력 ?�복/�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRegenPerSecond = 0.0f;

	/** 마나 ?�복/�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRegenPerSecond = 0.0f;

	/** ?�태미나 ?�복/�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRegenPerSecond = 0.0f;

	/** 공격??증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** 방어??증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** ?�동?�도 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** ?�리?�컬 ?�률 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** 쿨다??감소 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CooldownReduction = 0.0f;

	/** 경험�??�득??증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** 범위 ???�용 (m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float EffectRadius = 1000.0f;

	/** 버프 지?�시�?(�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 60.0f;

	/** ?�군?�게�??�용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAllyOnly = true;

	/** ?�신?�게???�용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAffectSelf = true;
};

/**
 * ?�보 ?�이??
 */
USTRUCT(BlueprintType)
struct FMusicSheetData
{
	GENERATED_BODY()

	/** �??�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName MusicName;

	/** �??�목 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Title;

	/** �??�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Description;

	/** ?�악 ?�르 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	EMusicGenre Genre = EMusicGenre::Classical;

	/** ?�요 ?�기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<EInstrumentType> RequiredInstruments;

	/** 최소 ?�주 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 MinPerformanceLevel = 1;

	/** ?�이??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 Difficulty = 5;

	/** ?�주 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float PerformanceDuration = 30.0f;

	/** ?�악 버프 ?�과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FMusicBuffEffect BuffEffect;

	/** ?�질�?버프 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TMap<EPerformanceQuality, float> QualityMultipliers;

	/** 경험�?보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 ExperienceReward = 25;

	/** ?�악 ?�운??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<class USoundBase> MusicSound;

	/** ?�보 ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?�겨�??�보 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bHidden = false;
};

/**
 * ?�기 ?�이??
 */
USTRUCT(BlueprintType)
struct FInstrumentData
{
	GENERATED_BODY()

	/** ?�기 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	FName InstrumentName;

	/** ?�기 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	EInstrumentType InstrumentType = EInstrumentType::Lute;

	/** ?�기 ?�급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 InstrumentTier = 1;

	/** ?�주 ?�질 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float QualityBonus = 0.0f;

	/** 버프 ?�과 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float BuffEffectBonus = 0.0f;

	/** 버프 범위 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float RangeBonus = 0.0f;

	/** ?�구??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 Durability = 100;

	/** 최�? ?�구??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 MaxDurability = 100;

	/** ?�기 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	TSoftObjectPtr<UStaticMesh> InstrumentMesh;
};

/**
 * ?�주 결과
 */
USTRUCT(BlueprintType)
struct FPerformanceResult
{
	GENERATED_BODY()

	/** ?�주??�?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName MusicID;

	/** ?�주 ?�질 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	EPerformanceQuality Quality = EPerformanceQuality::Good;

	/** ?�용??버프 ?�과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FMusicBuffEffect AppliedBuff;

	/** ?�향받�? ?�터 ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 AffectedActorCount = 0;

	/** ?�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** ?�벽???�주 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * 리듬 ?�트 (미니게임??
 */
USTRUCT(BlueprintType)
struct FRhythmNote
{
	GENERATED_BODY()

	/** ?�트 ?�??(0-3: ?�하좌우 ?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 NoteType = 0;

	/** ?�트 ?�?�밍 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float Timing = 0.0f;

	/** ?�벽???�?�밍 ?�용 범위 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float PerfectWindow = 0.05f;

	/** 좋�? ?�?�밍 ?�용 범위 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float GoodWindow = 0.15f;
};

/**
 * ?�주 ?�성
 */
USTRUCT(BlueprintType)
struct FPerformanceTrait
{
	GENERATED_BODY()

	/** ?�성 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** ?�성 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** ?�주 ?�질 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** 버프 ?�과 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** 버프 지?�시�?증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** 버프 범위 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RangeBonus = 0.0f;

	/** 경험�?보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** ?�코�??�률 (%) - 버프가 2�?지??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float EncoreChance = 0.0f;
};

/**
 * ?�보 ?�이???�이�?Row
 * ?�디?�에??DataTable�??�보�?관리할 ???�용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FMusicSheetDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?�보 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName MusicID;

	/** ?�시 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText DisplayName;

	/** ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Description;

	/** ?�악 ?�르 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	EMusicGenre Genre = EMusicGenre::Classical;

	/** ?�환 ?�기 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Requirement")
	TArray<EInstrumentType> CompatibleInstruments;

	/** ?�이??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Difficulty")
	int32 Difficulty = 1;

	/** ?�주 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Difficulty")
	float Duration = 30.0f;

	/** 최소 ?�악 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Requirement")
	int32 MinMusicLevel = 1;

	/** BPM (분당 비트?? 리듬 게임?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Rhythm")
	int32 BPM = 120;

	/** ?�트 ?�턴 (�??�위 ?�?�스?�프) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Rhythm")
	TArray<float> NotePattern;

	/** 버프 ?�과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	FMusicBuffEffect BuffEffect;

	/** 버프 지?�시�?(�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	float BuffDuration = 60.0f;

	/** 버프 범위 (주�? ?�레?�어/NPC?�게 ?�용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	float BuffRadius = 1000.0f;

	/** ?�공 ?�주???�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Economy")
	int32 ExperienceGain = 10;

	/** ?�디???�셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Audio")
	TSoftObjectPtr<USoundBase> AudioAsset;

	/** ?�이�??�보 ?�스�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?��?/?�설 ?�보 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bIsRare = false;
};
