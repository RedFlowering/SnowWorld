// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaLocalizationTypes.h"
#include "HarmoniaLocalizationBlueprintLibrary.generated.h"

class UHarmoniaLocalizationSubsystem;

/**
 * Blueprint Function Library for Harmonia Localization System
 * Provides convenient access to localization functions from blueprints
 */
UCLASS()
class HARMONIALOCALIZATIONSYSTEM_API UHarmoniaLocalizationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get Localization Subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaLocalizationSubsystem* GetLocalizationSubsystem(const UObject* WorldContextObject);

	/**
	 * Get current language
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static EHarmoniaLanguage GetCurrentLanguage(const UObject* WorldContextObject);

	/**
	 * Set current language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static void SetCurrentLanguage(const UObject* WorldContextObject, EHarmoniaLanguage NewLanguage, bool bSaveToConfig = true);

	/**
	 * Get localized text by key
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static FText GetLocalizedText(const UObject* WorldContextObject, FName TextKey, ETranslationContext Context = ETranslationContext::None);

	/**
	 * Get localized text with format args
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static FText GetLocalizedTextFormatted(const UObject* WorldContextObject, FName TextKey, const TMap<FString, FString>& FormatArgs);

	/**
	 * Get localized audio
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static USoundBase* GetLocalizedAudio(const UObject* WorldContextObject, FName AudioKey);

	/**
	 * Format number according to current language settings
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static FString FormatNumber(const UObject* WorldContextObject, float Number, int32 DecimalPlaces = 2);

	/**
	 * Format currency according to current language settings
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static FString FormatCurrency(const UObject* WorldContextObject, float Amount);

	/**
	 * Format date according to current language settings
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static FString FormatDate(const UObject* WorldContextObject, FDateTime DateTime);

	/**
	 * Format time according to current language settings
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static FString FormatTime(const UObject* WorldContextObject, FDateTime DateTime);

	/**
	 * Format date and time according to current language settings
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static FString FormatDateTime(const UObject* WorldContextObject, FDateTime DateTime);

	/**
	 * Check if current language is RTL
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static bool IsCurrentLanguageRTL(const UObject* WorldContextObject);

	/**
	 * Get current text direction
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static ETextDirection GetCurrentTextDirection(const UObject* WorldContextObject);

	/**
	 * Get current language font
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization", meta = (WorldContext = "WorldContextObject"))
	static UFont* GetCurrentLanguageFont(const UObject* WorldContextObject);

	/**
	 * Convert language enum to display name
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization")
	static FString GetLanguageDisplayName(EHarmoniaLanguage Language);

	/**
	 * Get all available languages
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Localization")
	static TArray<EHarmoniaLanguage> GetAllLanguages();
};
