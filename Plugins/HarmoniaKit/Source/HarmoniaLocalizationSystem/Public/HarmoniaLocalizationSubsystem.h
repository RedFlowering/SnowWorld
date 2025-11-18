// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HarmoniaLocalizationTypes.h"
#include "Engine/DataTable.h"
#include "HarmoniaLocalizationSubsystem.generated.h"

/**
 * Harmonia Localization Subsystem
 *
 * Features:
 * - Dynamic language switching (no restart required)
 * - Text table management (CSV/JSON based)
 * - Automatic font switching per language
 * - Audio localization (language-specific voice/SFX)
 * - Date/Time/Number formatting (region-specific)
 * - RTL (Right-to-Left) support
 * - Missing translation detection
 * - Context-based translation
 */
UCLASS()
class HARMONIALOCALIZATIONSYSTEM_API UHarmoniaLocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Language Management
	// ========================================

	/**
	 * Get current language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	EHarmoniaLanguage GetCurrentLanguage() const { return CurrentLanguage; }

	/**
	 * Set current language (applies immediately, no restart required)
	 * @param NewLanguage - Language to switch to
	 * @param bSaveToConfig - Save to config file
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void SetCurrentLanguage(EHarmoniaLanguage NewLanguage, bool bSaveToConfig = true);

	/**
	 * Get language settings for a specific language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FHarmoniaLanguageSettings GetLanguageSettings(EHarmoniaLanguage Language) const;

	/**
	 * Check if language is RTL
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	bool IsRTLLanguage(EHarmoniaLanguage Language) const;

	/**
	 * Get text direction for current language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	ETextDirection GetCurrentTextDirection() const;

	/**
	 * Get font for current language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	UFont* GetCurrentLanguageFont() const;

	// ========================================
	// Text Translation
	// ========================================

	/**
	 * Get localized text by key
	 * @param TextKey - Text key to look up
	 * @param Context - Translation context (for context-based translation)
	 * @param bMarkMissingTranslation - If true, missing translations will be marked with [MISSING]
	 * @return Localized text
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FText GetLocalizedText(FName TextKey, ETranslationContext Context = ETranslationContext::None, bool bMarkMissingTranslation = true);

	/**
	 * Get localized text with format args
	 * Example: GetLocalizedTextFormatted("Welcome", {{"PlayerName", "John"}})
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FText GetLocalizedTextFormatted(FName TextKey, const TMap<FString, FString>& FormatArgs, ETranslationContext Context = ETranslationContext::None);

	/**
	 * Check if translation exists for key
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	bool HasTranslation(FName TextKey, EHarmoniaLanguage Language, ETranslationContext Context = ETranslationContext::None) const;

	/**
	 * Reload localization data tables
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void ReloadLocalizationData();

	/**
	 * Add localization data table at runtime
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void AddLocalizationDataTable(UDataTable* DataTable);

	/**
	 * Remove localization data table
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void RemoveLocalizationDataTable(UDataTable* DataTable);

	// ========================================
	// Audio Localization
	// ========================================

	/**
	 * Get localized audio by key
	 * @param AudioKey - Audio key to look up
	 * @return Localized audio for current language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	USoundBase* GetLocalizedAudio(FName AudioKey);

	/**
	 * Get localized audio for specific language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	USoundBase* GetLocalizedAudioForLanguage(FName AudioKey, EHarmoniaLanguage Language);

	/**
	 * Register localized audio
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void RegisterLocalizedAudio(const FHarmoniaLocalizedAudio& LocalizedAudio);

	// ========================================
	// Number/Date/Time Formatting
	// ========================================

	/**
	 * Format number according to current language settings
	 * Example: 1234.56 -> "1,234.56" (English) or "1.234,56" (German)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FString FormatNumber(float Number, int32 DecimalPlaces = 2) const;

	/**
	 * Format integer with thousands separators
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FString FormatInteger(int32 Number) const;

	/**
	 * Format currency
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FString FormatCurrency(float Amount) const;

	/**
	 * Format date according to current language settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FString FormatDate(const FDateTime& DateTime) const;

	/**
	 * Format time according to current language settings (12/24 hour)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FString FormatTime(const FDateTime& DateTime) const;

	/**
	 * Format date and time
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	FString FormatDateTime(const FDateTime& DateTime) const;

	// ========================================
	// Missing Translation Detection
	// ========================================

	/**
	 * Get list of missing translations for current language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	TArray<FName> GetMissingTranslations() const;

	/**
	 * Export missing translations to CSV file
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	bool ExportMissingTranslationsToCSV(const FString& FilePath) const;

	/**
	 * Enable/disable missing translation tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void SetMissingTranslationTracking(bool bEnabled);

	/**
	 * Clear missing translation list
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void ClearMissingTranslations();

	// ========================================
	// Configuration
	// ========================================

	/**
	 * Set language settings for a specific language
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	void SetLanguageSettings(EHarmoniaLanguage Language, const FHarmoniaLanguageSettings& Settings);

	/**
	 * Load localization config from file
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	bool LoadLocalizationConfig(const FString& ConfigPath);

	/**
	 * Save localization config to file
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Localization")
	bool SaveLocalizationConfig(const FString& ConfigPath) const;

	// ========================================
	// Delegates
	// ========================================

	/** Called when language changes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Localization")
	FOnLanguageChanged OnLanguageChanged;

	/** Called when missing translation is detected */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Localization")
	FOnMissingTranslation OnMissingTranslation;

private:
	/**
	 * Load default language settings
	 */
	void LoadDefaultLanguageSettings();

	/**
	 * Load saved language from config
	 */
	void LoadSavedLanguage();

	/**
	 * Apply language change
	 */
	void ApplyLanguageChange(EHarmoniaLanguage NewLanguage);

	/**
	 * Get text for specific language from entry
	 */
	FText GetTextForLanguage(const FHarmoniaLocalizationTextEntry* Entry, EHarmoniaLanguage Language) const;

	/**
	 * Get audio for specific language from entry
	 */
	TSoftObjectPtr<USoundBase> GetAudioForLanguage(const FHarmoniaLocalizedAudio* Entry, EHarmoniaLanguage Language) const;

	/**
	 * Mark translation as missing
	 */
	void MarkTranslationAsMissing(FName TextKey, EHarmoniaLanguage Language, ETranslationContext Context);

	/**
	 * Format number with custom settings
	 */
	FString FormatNumberInternal(double Number, int32 DecimalPlaces, const FHarmoniaLanguageSettings& Settings) const;

	// ========================================
	// Data
	// ========================================

	/** Current language */
	UPROPERTY()
	EHarmoniaLanguage CurrentLanguage = EHarmoniaLanguage::English;

	/** Language settings map */
	UPROPERTY()
	TMap<EHarmoniaLanguage, FHarmoniaLanguageSettings> LanguageSettingsMap;

	/** Localization data tables */
	UPROPERTY()
	TArray<UDataTable*> LocalizationDataTables;

	/** Cached localization entries for faster lookup */
	TMap<FName, FHarmoniaLocalizationTextEntry*> LocalizationCache;

	/** Audio localization map */
	UPROPERTY()
	TMap<FName, FHarmoniaLocalizedAudio> AudioLocalizationMap;

	/** Missing translations tracker */
	TSet<FName> MissingTranslations;

	/** Missing translation tracking enabled */
	bool bTrackMissingTranslations = true;
};
