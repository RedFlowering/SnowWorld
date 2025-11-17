// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/HarmoniaTimeWeatherManager.h"

UHarmoniaTimeWeatherManager::UHarmoniaTimeWeatherManager()
{
	CurrentTimeOfDay = EHarmoniaTimeOfDay::Morning;
	CurrentWeather = EHarmoniaWeatherType::Clear;
	CurrentWeatherIntensity = 0.0f;
	CurrentHour = 12;
	CurrentMinute = 0;
}

void UHarmoniaTimeWeatherManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTimeWeatherManager initialized"));
}

void UHarmoniaTimeWeatherManager::Deinitialize()
{
	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTimeWeatherManager deinitialized"));
}

void UHarmoniaTimeWeatherManager::BroadcastTimeChange(EHarmoniaTimeOfDay PreviousTime, EHarmoniaTimeOfDay NewTime, int32 Hour, int32 Minute)
{
	CurrentTimeOfDay = NewTime;
	CurrentHour = Hour;
	CurrentMinute = Minute;

	FHarmoniaTimeChangeInfo TimeInfo;
	TimeInfo.PreviousTime = PreviousTime;
	TimeInfo.NewTime = NewTime;
	TimeInfo.CurrentHour = Hour;
	TimeInfo.CurrentMinute = Minute;

	OnTimeOfDayChanged.Broadcast(TimeInfo);

	// Also broadcast the combination change
	BroadcastTimeWeatherCombination(NewTime, CurrentWeather, CurrentWeatherIntensity);

	UE_LOG(LogTemp, Log, TEXT("Time changed: %s -> %s (%02d:%02d)"),
		*TimeOfDayToString(PreviousTime),
		*TimeOfDayToString(NewTime),
		Hour, Minute);
}

void UHarmoniaTimeWeatherManager::BroadcastWeatherChange(EHarmoniaWeatherType PreviousWeather, EHarmoniaWeatherType NewWeather, float Intensity, float TransitionDuration)
{
	CurrentWeather = NewWeather;
	CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	FHarmoniaWeatherChangeInfo WeatherInfo;
	WeatherInfo.PreviousWeather = PreviousWeather;
	WeatherInfo.NewWeather = NewWeather;
	WeatherInfo.Intensity = CurrentWeatherIntensity;
	WeatherInfo.TransitionDuration = TransitionDuration;

	OnWeatherChanged.Broadcast(WeatherInfo);

	// Also broadcast the combination change
	BroadcastTimeWeatherCombination(CurrentTimeOfDay, NewWeather, CurrentWeatherIntensity);

	UE_LOG(LogTemp, Log, TEXT("Weather changed: %s -> %s (Intensity: %.2f, Transition: %.1fs)"),
		*WeatherToString(PreviousWeather),
		*WeatherToString(NewWeather),
		CurrentWeatherIntensity,
		TransitionDuration);
}

void UHarmoniaTimeWeatherManager::BroadcastHourChange(int32 Hour, int32 Minute)
{
	CurrentHour = Hour % 24;
	CurrentMinute = Minute % 60;

	OnHourChanged.Broadcast(CurrentHour, CurrentMinute);

	// Check if time of day changed
	EHarmoniaTimeOfDay NewTimeOfDay = HourToTimeOfDay(CurrentHour);
	if (NewTimeOfDay != CurrentTimeOfDay)
	{
		EHarmoniaTimeOfDay PreviousTimeOfDay = CurrentTimeOfDay;
		CurrentTimeOfDay = NewTimeOfDay;

		FHarmoniaTimeChangeInfo TimeInfo;
		TimeInfo.PreviousTime = PreviousTimeOfDay;
		TimeInfo.NewTime = NewTimeOfDay;
		TimeInfo.CurrentHour = CurrentHour;
		TimeInfo.CurrentMinute = CurrentMinute;

		OnTimeOfDayChanged.Broadcast(TimeInfo);
	}
}

void UHarmoniaTimeWeatherManager::BroadcastTimeWeatherCombination(EHarmoniaTimeOfDay TimeOfDay, EHarmoniaWeatherType Weather, float WeatherIntensity)
{
	OnTimeWeatherCombinationChanged.Broadcast(TimeOfDay, Weather, WeatherIntensity);
}

EHarmoniaTimeOfDay UHarmoniaTimeWeatherManager::HourToTimeOfDay(int32 Hour)
{
	Hour = Hour % 24;

	if (Hour >= 4 && Hour < 6)
	{
		return EHarmoniaTimeOfDay::Dawn;
	}
	else if (Hour >= 6 && Hour < 12)
	{
		return EHarmoniaTimeOfDay::Morning;
	}
	else if (Hour >= 12 && Hour < 17)
	{
		return EHarmoniaTimeOfDay::Afternoon;
	}
	else if (Hour >= 17 && Hour < 19)
	{
		return EHarmoniaTimeOfDay::Dusk;
	}
	else
	{
		return EHarmoniaTimeOfDay::Night;
	}
}

FString UHarmoniaTimeWeatherManager::TimeOfDayToString(EHarmoniaTimeOfDay TimeOfDay)
{
	switch (TimeOfDay)
	{
	case EHarmoniaTimeOfDay::Dawn:
		return TEXT("Dawn");
	case EHarmoniaTimeOfDay::Morning:
		return TEXT("Morning");
	case EHarmoniaTimeOfDay::Afternoon:
		return TEXT("Afternoon");
	case EHarmoniaTimeOfDay::Dusk:
		return TEXT("Dusk");
	case EHarmoniaTimeOfDay::Night:
		return TEXT("Night");
	default:
		return TEXT("Unknown");
	}
}

FString UHarmoniaTimeWeatherManager::WeatherToString(EHarmoniaWeatherType Weather)
{
	switch (Weather)
	{
	case EHarmoniaWeatherType::Clear:
		return TEXT("Clear");
	case EHarmoniaWeatherType::Cloudy:
		return TEXT("Cloudy");
	case EHarmoniaWeatherType::Rainy:
		return TEXT("Rainy");
	case EHarmoniaWeatherType::Stormy:
		return TEXT("Stormy");
	case EHarmoniaWeatherType::Foggy:
		return TEXT("Foggy");
	case EHarmoniaWeatherType::Snowy:
		return TEXT("Snowy");
	case EHarmoniaWeatherType::Blizzard:
		return TEXT("Blizzard");
	default:
		return TEXT("Unknown");
	}
}
