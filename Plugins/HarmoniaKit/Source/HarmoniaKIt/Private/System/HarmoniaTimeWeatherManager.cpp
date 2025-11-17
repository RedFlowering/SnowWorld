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
	// Prevent spam: check if enough time has passed since last broadcast
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastTimeChangeBroadcast < MinBroadcastInterval)
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadcastTimeChange ignored - called too frequently (%.2fs since last call)"),
			CurrentTime - LastTimeChangeBroadcast);
		return;
	}

	// Prevent duplicate broadcasts: check if values actually changed
	if (NewTime == CurrentTimeOfDay && Hour == CurrentHour && Minute == CurrentMinute)
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadcastTimeChange ignored - no change in values"));
		return;
	}

	LastTimeChangeBroadcast = CurrentTime;

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
	// Prevent spam: check if enough time has passed since last broadcast
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastWeatherChangeBroadcast < MinBroadcastInterval)
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadcastWeatherChange ignored - called too frequently (%.2fs since last call)"),
			CurrentTime - LastWeatherChangeBroadcast);
		return;
	}

	// Clamp intensity
	float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	// Prevent duplicate broadcasts: check if values actually changed
	if (NewWeather == CurrentWeather && FMath::IsNearlyEqual(ClampedIntensity, CurrentWeatherIntensity, 0.01f))
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadcastWeatherChange ignored - no significant change in values"));
		return;
	}

	LastWeatherChangeBroadcast = CurrentTime;

	CurrentWeather = NewWeather;
	CurrentWeatherIntensity = ClampedIntensity;

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
	// Prevent spam: check if enough time has passed since last broadcast
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHourChangeBroadcast < MinBroadcastInterval)
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadcastHourChange ignored - called too frequently (%.2fs since last call)"),
			CurrentTime - LastHourChangeBroadcast);
		return;
	}

	int32 NormalizedHour = Hour % 24;
	int32 NormalizedMinute = Minute % 60;

	// Prevent duplicate broadcasts: check if values actually changed
	if (NormalizedHour == CurrentHour && NormalizedMinute == CurrentMinute)
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadcastHourChange ignored - no change in values"));
		return;
	}

	LastHourChangeBroadcast = CurrentTime;

	CurrentHour = NormalizedHour;
	CurrentMinute = NormalizedMinute;

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
	// Prevent spam: check if enough time has passed since last broadcast
	// Note: This has a shorter interval since it's called internally by other broadcast functions
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastCombinationBroadcast < (MinBroadcastInterval * 0.5f))
	{
		return; // Silently ignore for internal calls
	}

	LastCombinationBroadcast = CurrentTime;

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
