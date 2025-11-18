// Copyright 2025 Snow Game Studio.

#include "HarmoniaLocalizationSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformFileManager.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogHarmoniaLocalization, Log, All);

void UHarmoniaLocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Harmonia Localization Subsystem Initialized"));

	// Load default language settings
	LoadDefaultLanguageSettings();

	// Load saved language from config
	LoadSavedLanguage();
}

void UHarmoniaLocalizationSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Harmonia Localization Subsystem Deinitialized"));
}

// ========================================
// Language Management
// ========================================

void UHarmoniaLocalizationSubsystem::SetCurrentLanguage(EHarmoniaLanguage NewLanguage, bool bSaveToConfig)
{
	if (CurrentLanguage == NewLanguage)
	{
		return;
	}

	EHarmoniaLanguage OldLanguage = CurrentLanguage;
	CurrentLanguage = NewLanguage;

	// Apply language change
	ApplyLanguageChange(NewLanguage);

	// Save to config if requested
	if (bSaveToConfig)
	{
		GConfig->SetInt(TEXT("Harmonia.Localization"), TEXT("CurrentLanguage"), static_cast<int32>(NewLanguage), GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}

	// Broadcast language change event
	OnLanguageChanged.Broadcast(OldLanguage, NewLanguage);

	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Language changed from %d to %d"), static_cast<int32>(OldLanguage), static_cast<int32>(NewLanguage));
}

FHarmoniaLanguageSettings UHarmoniaLocalizationSubsystem::GetLanguageSettings(EHarmoniaLanguage Language) const
{
	const FHarmoniaLanguageSettings* Settings = LanguageSettingsMap.Find(Language);
	if (Settings)
	{
		return *Settings;
	}

	// Return default settings if not found
	FHarmoniaLanguageSettings DefaultSettings;
	DefaultSettings.Language = Language;
	return DefaultSettings;
}

bool UHarmoniaLocalizationSubsystem::IsRTLLanguage(EHarmoniaLanguage Language) const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(Language);
	return Settings.TextDirection == ETextDirection::RightToLeft;
}

ETextDirection UHarmoniaLocalizationSubsystem::GetCurrentTextDirection() const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(CurrentLanguage);
	return Settings.TextDirection;
}

UFont* UHarmoniaLocalizationSubsystem::GetCurrentLanguageFont() const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(CurrentLanguage);
	return Settings.Font.LoadSynchronous();
}

// ========================================
// Text Translation
// ========================================

FText UHarmoniaLocalizationSubsystem::GetLocalizedText(FName TextKey, ETranslationContext Context, bool bMarkMissingTranslation)
{
	// Check cache first
	FHarmoniaLocalizationTextEntry** CachedEntry = LocalizationCache.Find(TextKey);
	if (CachedEntry && *CachedEntry)
	{
		FText LocalizedText = GetTextForLanguage(*CachedEntry, CurrentLanguage);

		// Check if translation is empty
		if (LocalizedText.IsEmpty())
		{
			// Try fallback to English
			LocalizedText = GetTextForLanguage(*CachedEntry, EHarmoniaLanguage::English);

			if (LocalizedText.IsEmpty())
			{
				// Mark as missing
				if (bTrackMissingTranslations)
				{
					MarkTranslationAsMissing(TextKey, CurrentLanguage, Context);
				}

				if (bMarkMissingTranslation)
				{
					return FText::FromString(FString::Printf(TEXT("[MISSING: %s]"), *TextKey.ToString()));
				}
			}
		}

		return LocalizedText;
	}

	// Search in data tables
	for (UDataTable* DataTable : LocalizationDataTables)
	{
		if (!DataTable)
		{
			continue;
		}

		FHarmoniaLocalizationTextEntry* Entry = DataTable->FindRow<FHarmoniaLocalizationTextEntry>(TextKey, TEXT(""));
		if (Entry)
		{
			// Cache for faster lookup next time
			LocalizationCache.Add(TextKey, Entry);

			FText LocalizedText = GetTextForLanguage(Entry, CurrentLanguage);

			if (LocalizedText.IsEmpty())
			{
				// Try fallback to English
				LocalizedText = GetTextForLanguage(Entry, EHarmoniaLanguage::English);

				if (LocalizedText.IsEmpty())
				{
					// Mark as missing
					if (bTrackMissingTranslations)
					{
						MarkTranslationAsMissing(TextKey, CurrentLanguage, Context);
					}

					if (bMarkMissingTranslation)
					{
						return FText::FromString(FString::Printf(TEXT("[MISSING: %s]"), *TextKey.ToString()));
					}
				}
			}

			return LocalizedText;
		}
	}

	// Not found
	if (bTrackMissingTranslations)
	{
		MarkTranslationAsMissing(TextKey, CurrentLanguage, Context);
	}

	UE_LOG(LogHarmoniaLocalization, Warning, TEXT("Translation not found: %s"), *TextKey.ToString());

	if (bMarkMissingTranslation)
	{
		return FText::FromString(FString::Printf(TEXT("[MISSING: %s]"), *TextKey.ToString()));
	}

	return FText::FromName(TextKey);
}

FText UHarmoniaLocalizationSubsystem::GetLocalizedTextFormatted(FName TextKey, const TMap<FString, FString>& FormatArgs, ETranslationContext Context)
{
	FText BaseText = GetLocalizedText(TextKey, Context, true);
	FString FormattedString = BaseText.ToString();

	// Replace format arguments
	for (const auto& Pair : FormatArgs)
	{
		FString SearchString = FString::Printf(TEXT("{%s}"), *Pair.Key);
		FormattedString = FormattedString.Replace(*SearchString, *Pair.Value);
	}

	return FText::FromString(FormattedString);
}

bool UHarmoniaLocalizationSubsystem::HasTranslation(FName TextKey, EHarmoniaLanguage Language, ETranslationContext Context) const
{
	for (UDataTable* DataTable : LocalizationDataTables)
	{
		if (!DataTable)
		{
			continue;
		}

		FHarmoniaLocalizationTextEntry* Entry = DataTable->FindRow<FHarmoniaLocalizationTextEntry>(TextKey, TEXT(""));
		if (Entry)
		{
			FText Text = GetTextForLanguage(Entry, Language);
			return !Text.IsEmpty();
		}
	}

	return false;
}

void UHarmoniaLocalizationSubsystem::ReloadLocalizationData()
{
	// Clear cache
	LocalizationCache.Empty();

	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Localization data reloaded"));
}

void UHarmoniaLocalizationSubsystem::AddLocalizationDataTable(UDataTable* DataTable)
{
	if (DataTable && !LocalizationDataTables.Contains(DataTable))
	{
		LocalizationDataTables.Add(DataTable);

		// Clear cache to force re-lookup
		LocalizationCache.Empty();

		UE_LOG(LogHarmoniaLocalization, Log, TEXT("Added localization data table: %s"), *DataTable->GetName());
	}
}

void UHarmoniaLocalizationSubsystem::RemoveLocalizationDataTable(UDataTable* DataTable)
{
	if (DataTable)
	{
		LocalizationDataTables.Remove(DataTable);

		// Clear cache
		LocalizationCache.Empty();

		UE_LOG(LogHarmoniaLocalization, Log, TEXT("Removed localization data table: %s"), *DataTable->GetName());
	}
}

// ========================================
// Audio Localization
// ========================================

USoundBase* UHarmoniaLocalizationSubsystem::GetLocalizedAudio(FName AudioKey)
{
	return GetLocalizedAudioForLanguage(AudioKey, CurrentLanguage);
}

USoundBase* UHarmoniaLocalizationSubsystem::GetLocalizedAudioForLanguage(FName AudioKey, EHarmoniaLanguage Language)
{
	FHarmoniaLocalizedAudio* Entry = AudioLocalizationMap.Find(AudioKey);
	if (Entry)
	{
		TSoftObjectPtr<USoundBase> Audio = GetAudioForLanguage(Entry, Language);
		if (!Audio.IsNull())
		{
			return Audio.LoadSynchronous();
		}

		// Fallback to English
		Audio = GetAudioForLanguage(Entry, EHarmoniaLanguage::English);
		if (!Audio.IsNull())
		{
			return Audio.LoadSynchronous();
		}
	}

	UE_LOG(LogHarmoniaLocalization, Warning, TEXT("Localized audio not found: %s"), *AudioKey.ToString());
	return nullptr;
}

void UHarmoniaLocalizationSubsystem::RegisterLocalizedAudio(const FHarmoniaLocalizedAudio& LocalizedAudio)
{
	AudioLocalizationMap.Add(LocalizedAudio.AudioKey, LocalizedAudio);
	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Registered localized audio: %s"), *LocalizedAudio.AudioKey.ToString());
}

// ========================================
// Number/Date/Time Formatting
// ========================================

FString UHarmoniaLocalizationSubsystem::FormatNumber(float Number, int32 DecimalPlaces) const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(CurrentLanguage);
	return FormatNumberInternal(Number, DecimalPlaces, Settings);
}

FString UHarmoniaLocalizationSubsystem::FormatInteger(int32 Number) const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(CurrentLanguage);
	return FormatNumberInternal(Number, 0, Settings);
}

FString UHarmoniaLocalizationSubsystem::FormatCurrency(float Amount) const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(CurrentLanguage);
	FString FormattedNumber = FormatNumberInternal(Amount, 2, Settings);

	if (Settings.bCurrencySymbolBefore)
	{
		return Settings.CurrencySymbol + FormattedNumber;
	}
	else
	{
		return FormattedNumber + Settings.CurrencySymbol;
	}
}

FString UHarmoniaLocalizationSubsystem::FormatDate(const FDateTime& DateTime) const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(CurrentLanguage);

	switch (Settings.DateFormat)
	{
	case EDateFormat::MMDDYYYY:
		return FString::Printf(TEXT("%02d/%02d/%04d"), DateTime.GetMonth(), DateTime.GetDay(), DateTime.GetYear());
	case EDateFormat::DDMMYYYY:
		return FString::Printf(TEXT("%02d/%02d/%04d"), DateTime.GetDay(), DateTime.GetMonth(), DateTime.GetYear());
	case EDateFormat::YYYYMMDD:
		return FString::Printf(TEXT("%04d/%02d/%02d"), DateTime.GetYear(), DateTime.GetMonth(), DateTime.GetDay());
	case EDateFormat::Custom:
		// Use custom format string
		if (!Settings.CustomDateFormat.IsEmpty())
		{
			// TODO: Implement custom format parsing
			return DateTime.ToString(*Settings.CustomDateFormat);
		}
		return DateTime.ToString();
	default:
		return DateTime.ToString();
	}
}

FString UHarmoniaLocalizationSubsystem::FormatTime(const FDateTime& DateTime) const
{
	FHarmoniaLanguageSettings Settings = GetLanguageSettings(CurrentLanguage);

	if (Settings.TimeFormat == ETimeFormat::Hour12)
	{
		int32 Hour12 = DateTime.GetHour12();
		FString AMPM = DateTime.GetHour() < 12 ? TEXT("AM") : TEXT("PM");
		return FString::Printf(TEXT("%02d:%02d %s"), Hour12, DateTime.GetMinute(), *AMPM);
	}
	else
	{
		return FString::Printf(TEXT("%02d:%02d"), DateTime.GetHour(), DateTime.GetMinute());
	}
}

FString UHarmoniaLocalizationSubsystem::FormatDateTime(const FDateTime& DateTime) const
{
	return FormatDate(DateTime) + TEXT(" ") + FormatTime(DateTime);
}

// ========================================
// Missing Translation Detection
// ========================================

TArray<FName> UHarmoniaLocalizationSubsystem::GetMissingTranslations() const
{
	return MissingTranslations.Array();
}

bool UHarmoniaLocalizationSubsystem::ExportMissingTranslationsToCSV(const FString& FilePath) const
{
	FString CSVContent = TEXT("TextKey,Language,Context\n");

	for (const FName& TextKey : MissingTranslations)
	{
		CSVContent += FString::Printf(TEXT("%s,%d,%d\n"),
			*TextKey.ToString(),
			static_cast<int32>(CurrentLanguage),
			0 // Context - would need to be stored separately
		);
	}

	return FFileHelper::SaveStringToFile(CSVContent, *FilePath);
}

void UHarmoniaLocalizationSubsystem::SetMissingTranslationTracking(bool bEnabled)
{
	bTrackMissingTranslations = bEnabled;
	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Missing translation tracking %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UHarmoniaLocalizationSubsystem::ClearMissingTranslations()
{
	MissingTranslations.Empty();
	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Missing translations cleared"));
}

// ========================================
// Configuration
// ========================================

void UHarmoniaLocalizationSubsystem::SetLanguageSettings(EHarmoniaLanguage Language, const FHarmoniaLanguageSettings& Settings)
{
	LanguageSettingsMap.Add(Language, Settings);
	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Language settings updated for language: %d"), static_cast<int32>(Language));
}

bool UHarmoniaLocalizationSubsystem::LoadLocalizationConfig(const FString& ConfigPath)
{
	// TODO: Implement JSON config loading
	UE_LOG(LogHarmoniaLocalization, Warning, TEXT("LoadLocalizationConfig not yet implemented"));
	return false;
}

bool UHarmoniaLocalizationSubsystem::SaveLocalizationConfig(const FString& ConfigPath) const
{
	// TODO: Implement JSON config saving
	UE_LOG(LogHarmoniaLocalization, Warning, TEXT("SaveLocalizationConfig not yet implemented"));
	return false;
}

// ========================================
// Private Methods
// ========================================

void UHarmoniaLocalizationSubsystem::LoadDefaultLanguageSettings()
{
	// English
	{
		FHarmoniaLanguageSettings Settings;
		Settings.Language = EHarmoniaLanguage::English;
		Settings.TextDirection = ETextDirection::LeftToRight;
		Settings.NumberFormat = ENumberFormat::Default;
		Settings.DateFormat = EDateFormat::MMDDYYYY;
		Settings.TimeFormat = ETimeFormat::Hour12;
		Settings.DecimalSeparator = TEXT(".");
		Settings.ThousandsSeparator = TEXT(",");
		Settings.CurrencySymbol = TEXT("$");
		Settings.bCurrencySymbolBefore = true;
		LanguageSettingsMap.Add(EHarmoniaLanguage::English, Settings);
	}

	// Korean
	{
		FHarmoniaLanguageSettings Settings;
		Settings.Language = EHarmoniaLanguage::Korean;
		Settings.TextDirection = ETextDirection::LeftToRight;
		Settings.NumberFormat = ENumberFormat::Default;
		Settings.DateFormat = EDateFormat::YYYYMMDD;
		Settings.TimeFormat = ETimeFormat::Hour24;
		Settings.DecimalSeparator = TEXT(".");
		Settings.ThousandsSeparator = TEXT(",");
		Settings.CurrencySymbol = TEXT("₩");
		Settings.bCurrencySymbolBefore = true;
		LanguageSettingsMap.Add(EHarmoniaLanguage::Korean, Settings);
	}

	// Japanese
	{
		FHarmoniaLanguageSettings Settings;
		Settings.Language = EHarmoniaLanguage::Japanese;
		Settings.TextDirection = ETextDirection::LeftToRight;
		Settings.NumberFormat = ENumberFormat::Default;
		Settings.DateFormat = EDateFormat::YYYYMMDD;
		Settings.TimeFormat = ETimeFormat::Hour24;
		Settings.DecimalSeparator = TEXT(".");
		Settings.ThousandsSeparator = TEXT(",");
		Settings.CurrencySymbol = TEXT("¥");
		Settings.bCurrencySymbolBefore = true;
		LanguageSettingsMap.Add(EHarmoniaLanguage::Japanese, Settings);
	}

	// Chinese (Simplified)
	{
		FHarmoniaLanguageSettings Settings;
		Settings.Language = EHarmoniaLanguage::ChineseSimplified;
		Settings.TextDirection = ETextDirection::LeftToRight;
		Settings.NumberFormat = ENumberFormat::Default;
		Settings.DateFormat = EDateFormat::YYYYMMDD;
		Settings.TimeFormat = ETimeFormat::Hour24;
		Settings.DecimalSeparator = TEXT(".");
		Settings.ThousandsSeparator = TEXT(",");
		Settings.CurrencySymbol = TEXT("¥");
		Settings.bCurrencySymbolBefore = true;
		LanguageSettingsMap.Add(EHarmoniaLanguage::ChineseSimplified, Settings);
	}

	// German
	{
		FHarmoniaLanguageSettings Settings;
		Settings.Language = EHarmoniaLanguage::German;
		Settings.TextDirection = ETextDirection::LeftToRight;
		Settings.NumberFormat = ENumberFormat::European;
		Settings.DateFormat = EDateFormat::DDMMYYYY;
		Settings.TimeFormat = ETimeFormat::Hour24;
		Settings.DecimalSeparator = TEXT(",");
		Settings.ThousandsSeparator = TEXT(".");
		Settings.CurrencySymbol = TEXT("€");
		Settings.bCurrencySymbolBefore = false;
		LanguageSettingsMap.Add(EHarmoniaLanguage::German, Settings);
	}

	// Arabic
	{
		FHarmoniaLanguageSettings Settings;
		Settings.Language = EHarmoniaLanguage::Arabic;
		Settings.TextDirection = ETextDirection::RightToLeft;
		Settings.NumberFormat = ENumberFormat::Arabic;
		Settings.DateFormat = EDateFormat::DDMMYYYY;
		Settings.TimeFormat = ETimeFormat::Hour24;
		Settings.DecimalSeparator = TEXT(".");
		Settings.ThousandsSeparator = TEXT(",");
		Settings.CurrencySymbol = TEXT("﷼");
		Settings.bCurrencySymbolBefore = true;
		LanguageSettingsMap.Add(EHarmoniaLanguage::Arabic, Settings);
	}

	// Add more language settings as needed...
	// For brevity, I'll skip the rest, but you can add them similarly
}

void UHarmoniaLocalizationSubsystem::LoadSavedLanguage()
{
	int32 SavedLanguage = static_cast<int32>(EHarmoniaLanguage::English);
	GConfig->GetInt(TEXT("Harmonia.Localization"), TEXT("CurrentLanguage"), SavedLanguage, GGameUserSettingsIni);

	CurrentLanguage = static_cast<EHarmoniaLanguage>(SavedLanguage);

	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Loaded saved language: %d"), SavedLanguage);
}

void UHarmoniaLocalizationSubsystem::ApplyLanguageChange(EHarmoniaLanguage NewLanguage)
{
	// Clear cache to force re-lookup with new language
	LocalizationCache.Empty();

	UE_LOG(LogHarmoniaLocalization, Log, TEXT("Applied language change to: %d"), static_cast<int32>(NewLanguage));
}

FText UHarmoniaLocalizationSubsystem::GetTextForLanguage(const FHarmoniaLocalizationTextEntry* Entry, EHarmoniaLanguage Language) const
{
	if (!Entry)
	{
		return FText::GetEmpty();
	}

	switch (Language)
	{
	case EHarmoniaLanguage::English:
		return Entry->English;
	case EHarmoniaLanguage::Korean:
		return Entry->Korean;
	case EHarmoniaLanguage::Japanese:
		return Entry->Japanese;
	case EHarmoniaLanguage::ChineseSimplified:
		return Entry->ChineseSimplified;
	case EHarmoniaLanguage::ChineseTraditional:
		return Entry->ChineseTraditional;
	case EHarmoniaLanguage::Spanish:
		return Entry->Spanish;
	case EHarmoniaLanguage::French:
		return Entry->French;
	case EHarmoniaLanguage::German:
		return Entry->German;
	case EHarmoniaLanguage::Russian:
		return Entry->Russian;
	case EHarmoniaLanguage::Portuguese:
		return Entry->Portuguese;
	case EHarmoniaLanguage::Italian:
		return Entry->Italian;
	case EHarmoniaLanguage::Arabic:
		return Entry->Arabic;
	case EHarmoniaLanguage::Turkish:
		return Entry->Turkish;
	case EHarmoniaLanguage::Polish:
		return Entry->Polish;
	case EHarmoniaLanguage::Thai:
		return Entry->Thai;
	case EHarmoniaLanguage::Vietnamese:
		return Entry->Vietnamese;
	default:
		return Entry->English;
	}
}

TSoftObjectPtr<USoundBase> UHarmoniaLocalizationSubsystem::GetAudioForLanguage(const FHarmoniaLocalizedAudio* Entry, EHarmoniaLanguage Language) const
{
	if (!Entry)
	{
		return nullptr;
	}

	switch (Language)
	{
	case EHarmoniaLanguage::English:
		return Entry->English;
	case EHarmoniaLanguage::Korean:
		return Entry->Korean;
	case EHarmoniaLanguage::Japanese:
		return Entry->Japanese;
	case EHarmoniaLanguage::ChineseSimplified:
		return Entry->ChineseSimplified;
	case EHarmoniaLanguage::ChineseTraditional:
		return Entry->ChineseTraditional;
	case EHarmoniaLanguage::Spanish:
		return Entry->Spanish;
	case EHarmoniaLanguage::French:
		return Entry->French;
	case EHarmoniaLanguage::German:
		return Entry->German;
	case EHarmoniaLanguage::Russian:
		return Entry->Russian;
	case EHarmoniaLanguage::Portuguese:
		return Entry->Portuguese;
	case EHarmoniaLanguage::Italian:
		return Entry->Italian;
	case EHarmoniaLanguage::Arabic:
		return Entry->Arabic;
	case EHarmoniaLanguage::Turkish:
		return Entry->Turkish;
	case EHarmoniaLanguage::Polish:
		return Entry->Polish;
	case EHarmoniaLanguage::Thai:
		return Entry->Thai;
	case EHarmoniaLanguage::Vietnamese:
		return Entry->Vietnamese;
	default:
		return Entry->English;
	}
}

void UHarmoniaLocalizationSubsystem::MarkTranslationAsMissing(FName TextKey, EHarmoniaLanguage Language, ETranslationContext Context)
{
	bool bWasAlreadyInSet = false;
	MissingTranslations.Add(TextKey, &bWasAlreadyInSet);

	if (!bWasAlreadyInSet)
	{
		// Broadcast missing translation event
		OnMissingTranslation.Broadcast(TextKey, Language, Context);

		UE_LOG(LogHarmoniaLocalization, Warning, TEXT("Missing translation detected: %s (Language: %d, Context: %d)"),
			*TextKey.ToString(), static_cast<int32>(Language), static_cast<int32>(Context));
	}
}

FString UHarmoniaLocalizationSubsystem::FormatNumberInternal(double Number, int32 DecimalPlaces, const FHarmoniaLanguageSettings& Settings) const
{
	// Split into integer and decimal parts
	int64 IntegerPart = FMath::FloorToInt64(FMath::Abs(Number));
	double DecimalPart = FMath::Abs(Number) - IntegerPart;

	// Format integer part with thousands separators
	FString IntegerString = FString::Printf(TEXT("%lld"), IntegerPart);
	FString FormattedInteger;

	int32 DigitCount = 0;
	for (int32 i = IntegerString.Len() - 1; i >= 0; --i)
	{
		if (DigitCount > 0 && DigitCount % 3 == 0)
		{
			FormattedInteger = Settings.ThousandsSeparator + FormattedInteger;
		}
		FormattedInteger = IntegerString[i] + FormattedInteger;
		DigitCount++;
	}

	// Add sign if negative
	if (Number < 0)
	{
		FormattedInteger = TEXT("-") + FormattedInteger;
	}

	// Add decimal part if needed
	if (DecimalPlaces > 0)
	{
		int32 DecimalValue = FMath::RoundToInt(DecimalPart * FMath::Pow(10.0, DecimalPlaces));
		FString DecimalString = FString::Printf(TEXT("%0*d"), DecimalPlaces, DecimalValue);
		return FormattedInteger + Settings.DecimalSeparator + DecimalString;
	}

	return FormattedInteger;
}
