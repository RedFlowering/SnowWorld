// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaMusicSystemDefinitions.h
 * @brief Music system type definitions and data structures
 * 
 * Contains enums, structs, and data assets for the music performance system
 * including instruments, music genres, buff effects, and rhythm game mechanics.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaMusicSystemDefinitions.generated.h"

/**
 * @enum EInstrumentType
 * @brief Types of musical instruments
 */
UENUM(BlueprintType)
enum class EInstrumentType : uint8
{
	Lute		UMETA(DisplayName = "Lute"),		// Lute
	Flute		UMETA(DisplayName = "Flute"),		// Flute
	Harp		UMETA(DisplayName = "Harp"),		// Harp
	Drum		UMETA(DisplayName = "Drum"),		// Drum
	Horn		UMETA(DisplayName = "Horn"),		// Horn
	Violin		UMETA(DisplayName = "Violin"),		// Violin
	Piano		UMETA(DisplayName = "Piano"),		// Piano
	Guitar		UMETA(DisplayName = "Guitar")		// Guitar
};

/**
 * @enum EMusicGenre
 * @brief Music genre types
 */
UENUM(BlueprintType)
enum class EMusicGenre : uint8
{
	Classical	UMETA(DisplayName = "Classical"),	// Classical
	Folk		UMETA(DisplayName = "Folk"),		// Folk
	Battle		UMETA(DisplayName = "Battle"),		// Battle
	Healing		UMETA(DisplayName = "Healing"),		// Healing
	Buff		UMETA(DisplayName = "Buff"),		// Buff
	Debuff		UMETA(DisplayName = "Debuff")		// Debuff
};

/**
 * @enum EPerformanceQuality
 * @brief Performance quality grades
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
 * @struct FMusicBuffEffect
 * @brief Music buff effect data
 */
USTRUCT(BlueprintType)
struct FMusicBuffEffect
{
	GENERATED_BODY()

	/** Buff name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** Buff description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** Health regeneration per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRegenPerSecond = 0.0f;

	/** Mana regeneration per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRegenPerSecond = 0.0f;

	/** Stamina regeneration per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRegenPerSecond = 0.0f;

	/** Attack power increase percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** Defense power increase percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** Movement speed increase percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** Critical chance increase percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** Cooldown reduction percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CooldownReduction = 0.0f;

	/** Experience gain bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** Effect radius in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float EffectRadius = 1000.0f;

	/** Buff duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 60.0f;

	/** Apply to allies only */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAllyOnly = true;

	/** Apply to self */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bAffectSelf = true;
};

/**
 * @struct FMusicSheetData
 * @brief Music sheet data structure
 */
USTRUCT(BlueprintType)
struct FMusicSheetData
{
	GENERATED_BODY()

	/** Song name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName MusicName;

	/** Song title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Title;

	/** Song description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Description;

	/** Music genre */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	EMusicGenre Genre = EMusicGenre::Classical;

	/** Required instruments to perform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<EInstrumentType> RequiredInstruments;

	/** Minimum performance level required */

	/** Minimum performance level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 MinPerformanceLevel = 1;

	/** Difficulty level (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 Difficulty = 5;

	/** Performance duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float PerformanceDuration = 30.0f;

	/** Music buff effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FMusicBuffEffect BuffEffect;

	/** Quality-based buff multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TMap<EPerformanceQuality, float> QualityMultipliers;

	/** Experience reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	int32 ExperienceReward = 25;

	/** Music sound asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<class USoundBase> MusicSound;

	/** Music sheet icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Hidden/secret music flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bHidden = false;
};

/**
 * @struct FInstrumentData
 * @brief Instrument data structure
 */
USTRUCT(BlueprintType)
struct FInstrumentData
{
	GENERATED_BODY()

	/** Instrument name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	FName InstrumentName;

	/** Type of instrument */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	EInstrumentType InstrumentType = EInstrumentType::Lute;

	/** Instrument tier level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 InstrumentTier = 1;

	/** Performance quality bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float QualityBonus = 0.0f;

	/** Buff effect bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float BuffEffectBonus = 0.0f;

	/** Buff range bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	float RangeBonus = 0.0f;

	/** Current durability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 Durability = 100;

	/** Maximum durability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	int32 MaxDurability = 100;

	/** Instrument mesh asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instrument")
	TSoftObjectPtr<UStaticMesh> InstrumentMesh;
};

/**
 * @struct FPerformanceResult
 * @brief Performance result data
 */
USTRUCT(BlueprintType)
struct FPerformanceResult
{
	GENERATED_BODY()

	/** Performed music identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName MusicID;

	/** Performance quality achieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	EPerformanceQuality Quality = EPerformanceQuality::Good;

	/** Applied buff effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FMusicBuffEffect AppliedBuff;

	/** Number of affected actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 AffectedActorCount = 0;

	/** Experience gained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** Perfect performance achieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * @struct FRhythmNote
 * @brief Rhythm note data for minigame
 */
USTRUCT(BlueprintType)
struct FRhythmNote
{
	GENERATED_BODY()

	/** Note type (0-3: up/down/left/right etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 NoteType = 0;

	/** Note timing in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float Timing = 0.0f;

	/** Perfect timing tolerance window (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float PerfectWindow = 0.05f;

	/** Good timing tolerance window (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float GoodWindow = 0.15f;
};

/**
 * @struct FPerformanceTrait
 * @brief Performance passive skill/trait
 */
USTRUCT(BlueprintType)
struct FPerformanceTrait
{
	GENERATED_BODY()

	/** Trait name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** Trait description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** Performance quality bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** Buff effect bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** Buff duration bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** Buff range bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RangeBonus = 0.0f;

	/** Experience bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** Encore chance percentage - buff lasts twice as long */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float EncoreChance = 0.0f;
};

/**
 * @struct FMusicSheetDataTableRow
 * @brief Music sheet data table row for editor DataTable management
 * 
 * Used for managing music sheets in DataTable format within the editor.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FMusicSheetDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Music sheet unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName MusicID;

	/** Display name of the music */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText DisplayName;

	/** Description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText Description;

	/** Music genre */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	EMusicGenre Genre = EMusicGenre::Classical;

	/** Compatible instrument types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Requirement")
	TArray<EInstrumentType> CompatibleInstruments;

	/** Difficulty level (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Difficulty")
	int32 Difficulty = 1;

	/** Performance duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Difficulty")
	float Duration = 30.0f;

	/** Minimum music level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Requirement")
	int32 MinMusicLevel = 1;

	/** BPM (beats per minute) for rhythm game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Rhythm")
	int32 BPM = 120;

	/** Note pattern (timestamps in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Rhythm")
	TArray<float> NotePattern;

	/** Buff effect applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	FMusicBuffEffect BuffEffect;

	/** Buff duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	float BuffDuration = 60.0f;

	/** Buff radius (applied to nearby players/NPCs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Buff")
	float BuffRadius = 1000.0f;

	/** Experience gained on successful performance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Economy")
	int32 ExperienceGain = 10;

	/** Audio asset reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Audio")
	TSoftObjectPtr<USoundBase> AudioAsset;

	/** Icon texture reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Rare/legendary music flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bIsRare = false;
};
