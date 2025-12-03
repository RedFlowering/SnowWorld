// Copyright 2025 Snow Game Studio.

#include "HarmoniaLocalizationBlueprintLibrary.h"
#include "HarmoniaLocalizationSubsystem.h"
#include "Engine/Engine.h"

UHarmoniaLocalizationSubsystem* UHarmoniaLocalizationBlueprintLibrary::GetLocalizationSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UHarmoniaLocalizationSubsystem>();
}

EHarmoniaLanguage UHarmoniaLocalizationBlueprintLibrary::GetCurrentLanguage(const UObject* WorldContextObject)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->GetCurrentLanguage();
	}
	return EHarmoniaLanguage::English;
}

void UHarmoniaLocalizationBlueprintLibrary::SetCurrentLanguage(const UObject* WorldContextObject, EHarmoniaLanguage NewLanguage, bool bSaveToConfig)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		Subsystem->SetCurrentLanguage(NewLanguage, bSaveToConfig);
	}
}

FText UHarmoniaLocalizationBlueprintLibrary::GetLocalizedText(const UObject* WorldContextObject, FName TextKey, ETranslationContext Context)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->GetLocalizedText(TextKey, Context);
	}
	return FText::FromName(TextKey);
}

FText UHarmoniaLocalizationBlueprintLibrary::GetLocalizedTextFormatted(const UObject* WorldContextObject, FName TextKey, const TMap<FString, FString>& FormatArgs)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->GetLocalizedTextFormatted(TextKey, FormatArgs);
	}
	return FText::FromName(TextKey);
}

USoundBase* UHarmoniaLocalizationBlueprintLibrary::GetLocalizedAudio(const UObject* WorldContextObject, FName AudioKey)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->GetLocalizedAudio(AudioKey);
	}
	return nullptr;
}

FString UHarmoniaLocalizationBlueprintLibrary::FormatNumber(const UObject* WorldContextObject, float Number, int32 DecimalPlaces)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->FormatNumber(Number, DecimalPlaces);
	}
	return FString::Printf(TEXT("%.*f"), DecimalPlaces, Number);
}

FString UHarmoniaLocalizationBlueprintLibrary::FormatCurrency(const UObject* WorldContextObject, float Amount)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->FormatCurrency(Amount);
	}
	return FString::Printf(TEXT("$%.2f"), Amount);
}

FString UHarmoniaLocalizationBlueprintLibrary::FormatDate(const UObject* WorldContextObject, FDateTime DateTime)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->FormatDate(DateTime);
	}
	return DateTime.ToString();
}

FString UHarmoniaLocalizationBlueprintLibrary::FormatTime(const UObject* WorldContextObject, FDateTime DateTime)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->FormatTime(DateTime);
	}
	return DateTime.ToString();
}

FString UHarmoniaLocalizationBlueprintLibrary::FormatDateTime(const UObject* WorldContextObject, FDateTime DateTime)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->FormatDateTime(DateTime);
	}
	return DateTime.ToString();
}

bool UHarmoniaLocalizationBlueprintLibrary::IsCurrentLanguageRTL(const UObject* WorldContextObject)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->IsRTLLanguage(Subsystem->GetCurrentLanguage());
	}
	return false;
}

ETextDirection UHarmoniaLocalizationBlueprintLibrary::GetCurrentTextDirection(const UObject* WorldContextObject)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->GetCurrentTextDirection();
	}
	return ETextDirection::LeftToRight;
}

UFont* UHarmoniaLocalizationBlueprintLibrary::GetCurrentLanguageFont(const UObject* WorldContextObject)
{
	UHarmoniaLocalizationSubsystem* Subsystem = GetLocalizationSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->GetCurrentLanguageFont();
	}
	return nullptr;
}

FString UHarmoniaLocalizationBlueprintLibrary::GetLanguageDisplayName(EHarmoniaLanguage Language)
{
	switch (Language)
	{
	case EHarmoniaLanguage::English:
		return TEXT("English");
	case EHarmoniaLanguage::Korean:
		return TEXT("한국어");
	case EHarmoniaLanguage::Japanese:
		return TEXT("日本語");
	case EHarmoniaLanguage::ChineseSimplified:
		return TEXT("简体中文");
	case EHarmoniaLanguage::ChineseTraditional:
		return TEXT("繁體中文");
	case EHarmoniaLanguage::Spanish:
		return TEXT("Español");
	case EHarmoniaLanguage::French:
		return TEXT("Français");
	case EHarmoniaLanguage::German:
		return TEXT("Deutsch");
	case EHarmoniaLanguage::Russian:
		return TEXT("Русский");
	case EHarmoniaLanguage::Portuguese:
		return TEXT("Português");
	case EHarmoniaLanguage::Italian:
		return TEXT("Italiano");
	case EHarmoniaLanguage::Arabic:
		return TEXT("العربية");
	case EHarmoniaLanguage::Turkish:
		return TEXT("Türkçe");
	case EHarmoniaLanguage::Polish:
		return TEXT("Polski");
	case EHarmoniaLanguage::Thai:
		return TEXT("ไทย");
	case EHarmoniaLanguage::Vietnamese:
		return TEXT("Tiếng Việt");
	default:
		return TEXT("Unknown");
	}
}

TArray<EHarmoniaLanguage> UHarmoniaLocalizationBlueprintLibrary::GetAllLanguages()
{
	TArray<EHarmoniaLanguage> Languages;
	Languages.Add(EHarmoniaLanguage::English);
	Languages.Add(EHarmoniaLanguage::Korean);
	Languages.Add(EHarmoniaLanguage::Japanese);
	Languages.Add(EHarmoniaLanguage::ChineseSimplified);
	Languages.Add(EHarmoniaLanguage::ChineseTraditional);
	Languages.Add(EHarmoniaLanguage::Spanish);
	Languages.Add(EHarmoniaLanguage::French);
	Languages.Add(EHarmoniaLanguage::German);
	Languages.Add(EHarmoniaLanguage::Russian);
	Languages.Add(EHarmoniaLanguage::Portuguese);
	Languages.Add(EHarmoniaLanguage::Italian);
	Languages.Add(EHarmoniaLanguage::Arabic);
	Languages.Add(EHarmoniaLanguage::Turkish);
	Languages.Add(EHarmoniaLanguage::Polish);
	Languages.Add(EHarmoniaLanguage::Thai);
	Languages.Add(EHarmoniaLanguage::Vietnamese);
	return Languages;
}
