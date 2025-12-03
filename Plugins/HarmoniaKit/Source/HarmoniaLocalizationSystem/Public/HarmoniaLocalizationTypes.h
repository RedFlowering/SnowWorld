// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Engine/Font.h"
#include "HarmoniaLocalizationTypes.generated.h"

/**
 * Supported Languages
 */
UENUM(BlueprintType)
enum class EHarmoniaLanguage : uint8
{
	English       UMETA(DisplayName = "English"),
	Korean        UMETA(DisplayName = "한국어"),
	Japanese      UMETA(DisplayName = "日本語"),
	ChineseSimplified UMETA(DisplayName = "简体中文"),
	ChineseTraditional UMETA(DisplayName = "繁體中文"),
	Spanish       UMETA(DisplayName = "Español"),
	French        UMETA(DisplayName = "Français"),
	German        UMETA(DisplayName = "Deutsch"),
	Russian       UMETA(DisplayName = "Русский"),
	Portuguese    UMETA(DisplayName = "Português"),
	Italian       UMETA(DisplayName = "Italiano"),
	Arabic        UMETA(DisplayName = "العربية"),
	Turkish       UMETA(DisplayName = "Türkçe"),
	Polish        UMETA(DisplayName = "Polski"),
	Thai          UMETA(DisplayName = "ไทย"),
	Vietnamese    UMETA(DisplayName = "Tiếng Việt"),
};

/**
 * Text Direction (for RTL support)
 */
UENUM(BlueprintType)
enum class ETextDirection : uint8
{
	LeftToRight   UMETA(DisplayName = "Left to Right (LTR)"),
	RightToLeft   UMETA(DisplayName = "Right to Left (RTL)"),
};

/**
 * Number Format Style
 */
UENUM(BlueprintType)
enum class ENumberFormat : uint8
{
	Default       UMETA(DisplayName = "Default (1,234.56)"),
	European      UMETA(DisplayName = "European (1.234,56)"),
	Indian        UMETA(DisplayName = "Indian (12,34.56)"),
	Arabic        UMETA(DisplayName = "Arabic (١٬٢٣٤٫٥٦)"),
};

/**
 * Date Format Style
 */
UENUM(BlueprintType)
enum class EDateFormat : uint8
{
	MMDDYYYY      UMETA(DisplayName = "MM/DD/YYYY"),
	DDMMYYYY      UMETA(DisplayName = "DD/MM/YYYY"),
	YYYYMMDD      UMETA(DisplayName = "YYYY/MM/DD"),
	Custom        UMETA(DisplayName = "Custom"),
};

/**
 * Time Format Style
 */
UENUM(BlueprintType)
enum class EHarmoniaTimeFormat : uint8
{
	Hour12        UMETA(DisplayName = "12-Hour (AM/PM)"),
	Hour24        UMETA(DisplayName = "24-Hour"),
};

/**
 * Translation Context (for context-based translation)
 */
UENUM(BlueprintType)
enum class ETranslationContext : uint8
{
	None          UMETA(DisplayName = "None"),
	UI            UMETA(DisplayName = "User Interface"),
	Dialogue      UMETA(DisplayName = "Dialogue"),
	Item          UMETA(DisplayName = "Item"),
	Quest         UMETA(DisplayName = "Quest"),
	Skill         UMETA(DisplayName = "Skill"),
	Character     UMETA(DisplayName = "Character"),
	Tutorial      UMETA(DisplayName = "Tutorial"),
	Error         UMETA(DisplayName = "Error Message"),
	Notification  UMETA(DisplayName = "Notification"),
	Custom        UMETA(DisplayName = "Custom"),
};

/**
 * Localization Text Entry (DataTable Row)
 */
USTRUCT(BlueprintType)
struct FHarmoniaLocalizationTextEntry : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique text key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FName TextKey;

	/** English (default) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText English;

	/** Korean */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Korean;

	/** Japanese */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Japanese;

	/** Chinese (Simplified) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText ChineseSimplified;

	/** Chinese (Traditional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText ChineseTraditional;

	/** Spanish */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Spanish;

	/** French */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText French;

	/** German */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText German;

	/** Russian */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Russian;

	/** Portuguese */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Portuguese;

	/** Italian */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Italian;

	/** Arabic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Arabic;

	/** Turkish */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Turkish;

	/** Polish */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Polish;

	/** Thai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Thai;

	/** Vietnamese */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText Vietnamese;

	/** Translation context for context-based translation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	ETranslationContext Context = ETranslationContext::None;

	/** Notes for translators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FString TranslatorNotes;

	FHarmoniaLocalizationTextEntry()
		: TextKey(NAME_None)
		, Context(ETranslationContext::None)
	{
	}
};

/**
 * Language Settings
 */
USTRUCT(BlueprintType)
struct FHarmoniaLanguageSettings
{
	GENERATED_BODY()

	/** Language */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	EHarmoniaLanguage Language = EHarmoniaLanguage::English;

	/** Text Direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	ETextDirection TextDirection = ETextDirection::LeftToRight;

	/** Font for this language */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<UFont> Font;

	/** Number format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	ENumberFormat NumberFormat = ENumberFormat::Default;

	/** Date format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	EDateFormat DateFormat = EDateFormat::MMDDYYYY;

	/** Time format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	EHarmoniaTimeFormat TimeFormat = EHarmoniaTimeFormat::Hour12;

	/** Custom date format string (if DateFormat is Custom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FString CustomDateFormat;

	/** Decimal separator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FString DecimalSeparator = TEXT(".");

	/** Thousands separator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FString ThousandsSeparator = TEXT(",");

	/** Currency symbol */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FString CurrencySymbol = TEXT("$");

	/** Currency symbol position (before or after) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	bool bCurrencySymbolBefore = true;

	FHarmoniaLanguageSettings()
	{
	}
};

/**
 * Audio Localization Entry
 */
USTRUCT(BlueprintType)
struct FHarmoniaLocalizedAudio
{
	GENERATED_BODY()

	/** Audio key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FName AudioKey;

	/** English audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> English;

	/** Korean audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Korean;

	/** Japanese audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Japanese;

	/** Chinese (Simplified) audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> ChineseSimplified;

	/** Chinese (Traditional) audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> ChineseTraditional;

	/** Spanish audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Spanish;

	/** French audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> French;

	/** German audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> German;

	/** Russian audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Russian;

	/** Portuguese audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Portuguese;

	/** Italian audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Italian;

	/** Arabic audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Arabic;

	/** Turkish audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Turkish;

	/** Polish audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Polish;

	/** Thai audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Thai;

	/** Vietnamese audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TSoftObjectPtr<USoundBase> Vietnamese;

	FHarmoniaLocalizedAudio()
		: AudioKey(NAME_None)
	{
	}
};

/**
 * Delegate for language change event
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLanguageChanged, EHarmoniaLanguage, OldLanguage, EHarmoniaLanguage, NewLanguage);

/**
 * Delegate for missing translation detection
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMissingTranslation, FName, TextKey, EHarmoniaLanguage, Language, ETranslationContext, Context);
