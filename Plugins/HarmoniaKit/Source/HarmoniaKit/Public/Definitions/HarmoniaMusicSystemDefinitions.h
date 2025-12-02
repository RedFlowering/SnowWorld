// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaMusicSystemDefinitions.generated.h"

/**
 * ?…ê¸° ?€??
 */
UENUM(BlueprintType)
enum class EInstrumentType : uint8
{
	Lute		UMETA(DisplayName = "Lute"),		// ë¥˜íŠ¸
	Flute		UMETA(DisplayName = "Flute"),		// ?Œë£¨??
	Harp		UMETA(DisplayName = "Harp"),		// ?˜í”„
	Drum		UMETA(DisplayName = "Drum"),		// ?œëŸ¼
	Horn		UMETA(DisplayName = "Horn"),		// ?¸ë¥¸
	Violin		UMETA(DisplayName = "Violin"),		// ë°”ì´?¬ë¦°
	Piano		UMETA(DisplayName = "Piano"),		// ?¼ì•„??
	Guitar		UMETA(DisplayName = "Guitar")		// ê¸°í?
};

/**
 * ?Œì•… ?¥ë¥´
 */
UENUM(BlueprintType)
enum class EMusicGenre : uint8
{
	Classical	UMETA(DisplayName = "Classical"),	// ?´ë˜??
	Folk		UMETA(DisplayName = "Folk"),		// ë¯¼ì†
	Battle		UMETA(DisplayName = "Battle"),		// ?„íˆ¬
	Healing		UMETA(DisplayName = "Healing"),		// ì¹˜ìœ 
	Buff		UMETA(DisplayName = "Buff"),		// ë²„í”„
	Debuff		UMETA(DisplayName = "Debuff")		// ?”ë²„??
};

/**
 * ?°ì£¼ ?ˆì§ˆ
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
 * ?Œì•… ë²„í”„ ?¨ê³¼
 */
USTRUCT(BlueprintType)
struct FMusicBuffEffect
{
	GENERATED_BODY()

	/** ë²„í”„ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** ë²„í”„ ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** ì²´ë ¥ ?Œë³µ/ì´?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRegenPerSecond = 0.0f;

	/** ë§ˆë‚˜ ?Œë³µ/ì´?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRegenPerSecond = 0.0f;

	/** ?¤íƒœë¯¸ë‚˜ ?Œë³µ/ì´?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRegenPerSecond = 0.0f;

	/** ê³µê²©??ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** ë°©ì–´??ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** ?´ë™?ë„ ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** ?¬ë¦¬?°ì»¬ ?•ë¥  ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** ì¿¨ë‹¤??ê°ì†Œ (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CooldownReduction = 0.0f;

	/** ê²½í—˜ì¹??ë“??ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** ë²”ìœ„ ???ìš© (m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float EffectRadius = 1000.0f;

	/** ë²„í”„ ì§€?ì‹œê°?(ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 60.0f;

	/** ?„êµ°?ê²Œë§??ìš© */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAllyOnly = true;

	/** ?ì‹ ?ê²Œ???ìš© */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAffectSelf = true;
};

/**
 * ?…ë³´ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FMusicSheetData
{
	GENERATED_BODY()

	/** ê³??´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName MusicName;

	/** ê³??œëª© */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Title;

	/** ê³??¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Description;

	/** ?Œì•… ?¥ë¥´ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	EMusicGenre Genre = EMusicGenre::Classical;

	/** ?„ìš” ?…ê¸° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<EInstrumentType> RequiredInstruments;

	/** ìµœì†Œ ?°ì£¼ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 MinPerformanceLevel = 1;

	/** ?œì´??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 Difficulty = 5;

	/** ?°ì£¼ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float PerformanceDuration = 30.0f;

	/** ?Œì•… ë²„í”„ ?¨ê³¼ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FMusicBuffEffect BuffEffect;

	/** ?ˆì§ˆë³?ë²„í”„ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TMap<EPerformanceQuality, float> QualityMultipliers;

	/** ê²½í—˜ì¹?ë³´ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 ExperienceReward = 25;

	/** ?Œì•… ?¬ìš´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<class USoundBase> MusicSound;

	/** ?…ë³´ ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?¨ê²¨ì§??…ë³´ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bHidden = false;
};

/**
 * ?…ê¸° ?°ì´??
 */
USTRUCT(BlueprintType)
struct FInstrumentData
{
	GENERATED_BODY()

	/** ?…ê¸° ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	FName InstrumentName;

	/** ?…ê¸° ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	EInstrumentType InstrumentType = EInstrumentType::Lute;

	/** ?…ê¸° ?±ê¸‰ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 InstrumentTier = 1;

	/** ?°ì£¼ ?ˆì§ˆ ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float QualityBonus = 0.0f;

	/** ë²„í”„ ?¨ê³¼ ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float BuffEffectBonus = 0.0f;

	/** ë²„í”„ ë²”ìœ„ ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float RangeBonus = 0.0f;

	/** ?´êµ¬??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 Durability = 100;

	/** ìµœë? ?´êµ¬??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 MaxDurability = 100;

	/** ?…ê¸° ë©”ì‹œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	TSoftObjectPtr<UStaticMesh> InstrumentMesh;
};

/**
 * ?°ì£¼ ê²°ê³¼
 */
USTRUCT(BlueprintType)
struct FPerformanceResult
{
	GENERATED_BODY()

	/** ?°ì£¼??ê³?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName MusicID;

	/** ?°ì£¼ ?ˆì§ˆ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	EPerformanceQuality Quality = EPerformanceQuality::Good;

	/** ?ìš©??ë²„í”„ ?¨ê³¼ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FMusicBuffEffect AppliedBuff;

	/** ?í–¥ë°›ì? ?¡í„° ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 AffectedActorCount = 0;

	/** ?ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** ?„ë²½???°ì£¼ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * ë¦¬ë“¬ ?¸íŠ¸ (ë¯¸ë‹ˆê²Œì„??
 */
USTRUCT(BlueprintType)
struct FRhythmNote
{
	GENERATED_BODY()

	/** ?¸íŠ¸ ?€??(0-3: ?í•˜ì¢Œìš° ?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 NoteType = 0;

	/** ?¸íŠ¸ ?€?´ë° (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float Timing = 0.0f;

	/** ?„ë²½???€?´ë° ?ˆìš© ë²”ìœ„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float PerfectWindow = 0.05f;

	/** ì¢‹ì? ?€?´ë° ?ˆìš© ë²”ìœ„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float GoodWindow = 0.15f;
};

/**
 * ?°ì£¼ ?¹ì„±
 */
USTRUCT(BlueprintType)
struct FPerformanceTrait
{
	GENERATED_BODY()

	/** ?¹ì„± ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** ?¹ì„± ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** ?°ì£¼ ?ˆì§ˆ ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** ë²„í”„ ?¨ê³¼ ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** ë²„í”„ ì§€?ì‹œê°?ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** ë²„í”„ ë²”ìœ„ ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RangeBonus = 0.0f;

	/** ê²½í—˜ì¹?ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** ?™ì½”ë¥??•ë¥  (%) - ë²„í”„ê°€ 2ë°?ì§€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float EncoreChance = 0.0f;
};

/**
 * ?…ë³´ ?°ì´???Œì´ë¸?Row
 * ?ë””?°ì—??DataTableë¡??…ë³´ë¥?ê´€ë¦¬í•  ???¬ìš©
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FMusicSheetDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?…ë³´ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName MusicID;

	/** ?œì‹œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText DisplayName;

	/** ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Description;

	/** ?Œì•… ?¥ë¥´ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	EMusicGenre Genre = EMusicGenre::Classical;

	/** ?¸í™˜ ?…ê¸° ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Requirement")
	TArray<EInstrumentType> CompatibleInstruments;

	/** ?œì´??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Difficulty")
	int32 Difficulty = 1;

	/** ?°ì£¼ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Difficulty")
	float Duration = 30.0f;

	/** ìµœì†Œ ?Œì•… ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Requirement")
	int32 MinMusicLevel = 1;

	/** BPM (ë¶„ë‹¹ ë¹„íŠ¸?? ë¦¬ë“¬ ê²Œì„?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Rhythm")
	int32 BPM = 120;

	/** ?¸íŠ¸ ?¨í„´ (ì´??¨ìœ„ ?€?„ìŠ¤?¬í”„) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Rhythm")
	TArray<float> NotePattern;

	/** ë²„í”„ ?¨ê³¼ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	FMusicBuffEffect BuffEffect;

	/** ë²„í”„ ì§€?ì‹œê°?(ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	float BuffDuration = 60.0f;

	/** ë²„í”„ ë²”ìœ„ (ì£¼ë? ?Œë ˆ?´ì–´/NPC?ê²Œ ?ìš©) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	float BuffRadius = 1000.0f;

	/** ?±ê³µ ?°ì£¼???ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Economy")
	int32 ExperienceGain = 10;

	/** ?¤ë””???ì…‹ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Audio")
	TSoftObjectPtr<USoundBase> AudioAsset;

	/** ?„ì´ì½??…ë³´ ?ìŠ¤ì²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?¬ê?/?„ì„¤ ?…ë³´ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bIsRare = false;
};
