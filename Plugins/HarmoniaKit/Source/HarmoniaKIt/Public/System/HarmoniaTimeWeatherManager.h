// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HarmoniaTimeWeatherManager.generated.h"

/**
 * Time of day enumeration
 */
UENUM(BlueprintType)
enum class EHarmoniaTimeOfDay : uint8
{
	Dawn		UMETA(DisplayName = "Dawn (04:00-06:00)"),
	Morning		UMETA(DisplayName = "Morning (06:00-12:00)"),
	Afternoon	UMETA(DisplayName = "Afternoon (12:00-17:00)"),
	Dusk		UMETA(DisplayName = "Dusk (17:00-19:00)"),
	Night		UMETA(DisplayName = "Night (19:00-04:00)")
};

/**
 * Weather type enumeration
 */
UENUM(BlueprintType)
enum class EHarmoniaWeatherType : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Cloudy		UMETA(DisplayName = "Cloudy"),
	Rainy		UMETA(DisplayName = "Rainy"),
	Stormy		UMETA(DisplayName = "Stormy"),
	Foggy		UMETA(DisplayName = "Foggy"),
	Snowy		UMETA(DisplayName = "Snowy"),
	Blizzard	UMETA(DisplayName = "Blizzard")
};

/**
 * Time change information
 */
USTRUCT(BlueprintType)
struct FHarmoniaTimeChangeInfo
{
	GENERATED_BODY()

	/** Previous time of day */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	EHarmoniaTimeOfDay PreviousTime;

	/** New time of day */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	EHarmoniaTimeOfDay NewTime;

	/** Current hour (0-23) */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 CurrentHour;

	/** Current minute (0-59) */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 CurrentMinute;

	FHarmoniaTimeChangeInfo()
		: PreviousTime(EHarmoniaTimeOfDay::Morning)
		, NewTime(EHarmoniaTimeOfDay::Morning)
		, CurrentHour(12)
		, CurrentMinute(0)
	{
	}
};

/**
 * Weather change information
 */
USTRUCT(BlueprintType)
struct FHarmoniaWeatherChangeInfo
{
	GENERATED_BODY()

	/** Previous weather type */
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	EHarmoniaWeatherType PreviousWeather;

	/** New weather type */
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	EHarmoniaWeatherType NewWeather;

	/** Weather intensity (0.0 - 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float Intensity;

	/** Weather transition duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float TransitionDuration;

	FHarmoniaWeatherChangeInfo()
		: PreviousWeather(EHarmoniaWeatherType::Clear)
		, NewWeather(EHarmoniaWeatherType::Clear)
		, Intensity(0.0f)
		, TransitionDuration(0.0f)
	{
	}
};

/**
 * Delegates for time and weather changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, const FHarmoniaTimeChangeInfo&, TimeInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, const FHarmoniaWeatherChangeInfo&, WeatherInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHourChanged, int32, Hour, int32, Minute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTimeWeatherCombinationChanged, EHarmoniaTimeOfDay, TimeOfDay, EHarmoniaWeatherType, Weather, float, WeatherIntensity);

/**
 * World subsystem for managing time and weather delegates
 * This class provides only delegate system for Blueprint integration
 * Actual time/weather logic should be implemented in Blueprint
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaTimeWeatherManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaTimeWeatherManager();

	// ==================== Subsystem Overrides ====================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==================== Events ====================

	/** Called when time of day changes (e.g., Morning to Afternoon) */
	UPROPERTY(BlueprintAssignable, Category = "Time & Weather")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	/** Called when weather changes */
	UPROPERTY(BlueprintAssignable, Category = "Time & Weather")
	FOnWeatherChanged OnWeatherChanged;

	/** Called every in-game hour */
	UPROPERTY(BlueprintAssignable, Category = "Time & Weather")
	FOnHourChanged OnHourChanged;

	/** Called when both time and weather need to be considered (for spawn systems, etc.) */
	UPROPERTY(BlueprintAssignable, Category = "Time & Weather")
	FOnTimeWeatherCombinationChanged OnTimeWeatherCombinationChanged;

	// ==================== Blueprint Callable Functions ====================
	// These should be called from Blueprint time/weather system

	/**
	 * Call this from Blueprint when time of day changes
	 * @param PreviousTime The previous time of day
	 * @param NewTime The new time of day
	 * @param Hour Current hour (0-23)
	 * @param Minute Current minute (0-59)
	 */
	UFUNCTION(BlueprintCallable, Category = "Time & Weather")
	void BroadcastTimeChange(EHarmoniaTimeOfDay PreviousTime, EHarmoniaTimeOfDay NewTime, int32 Hour, int32 Minute);

	/**
	 * Call this from Blueprint when weather changes
	 * @param PreviousWeather The previous weather type
	 * @param NewWeather The new weather type
	 * @param Intensity Weather intensity (0.0 - 1.0)
	 * @param TransitionDuration Duration of weather transition in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Time & Weather")
	void BroadcastWeatherChange(EHarmoniaWeatherType PreviousWeather, EHarmoniaWeatherType NewWeather, float Intensity = 1.0f, float TransitionDuration = 10.0f);

	/**
	 * Call this from Blueprint every in-game hour
	 * @param Hour Current hour (0-23)
	 * @param Minute Current minute (0-59)
	 */
	UFUNCTION(BlueprintCallable, Category = "Time & Weather")
	void BroadcastHourChange(int32 Hour, int32 Minute);

	/**
	 * Call this when you want to notify systems of the current time/weather combination
	 * Useful for spawn systems that need both pieces of information
	 * @param TimeOfDay Current time of day
	 * @param Weather Current weather
	 * @param WeatherIntensity Current weather intensity (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Time & Weather")
	void BroadcastTimeWeatherCombination(EHarmoniaTimeOfDay TimeOfDay, EHarmoniaWeatherType Weather, float WeatherIntensity = 1.0f);

	// ==================== Query Functions ====================

	/** Get current time of day */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	EHarmoniaTimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

	/** Get current weather */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	EHarmoniaWeatherType GetCurrentWeather() const { return CurrentWeather; }

	/** Get current weather intensity */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	float GetCurrentWeatherIntensity() const { return CurrentWeatherIntensity; }

	/** Get current hour */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	int32 GetCurrentHour() const { return CurrentHour; }

	/** Get current minute */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	int32 GetCurrentMinute() const { return CurrentMinute; }

	/** Check if it's currently nighttime */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	bool IsNightTime() const { return CurrentTimeOfDay == EHarmoniaTimeOfDay::Night; }

	/** Check if it's currently daytime */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	bool IsDayTime() const { return CurrentTimeOfDay == EHarmoniaTimeOfDay::Morning || CurrentTimeOfDay == EHarmoniaTimeOfDay::Afternoon; }

	/** Check if current weather is stormy */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	bool IsStormyWeather() const { return CurrentWeather == EHarmoniaWeatherType::Stormy || CurrentWeather == EHarmoniaWeatherType::Blizzard; }

	/** Check if current weather is rainy */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	bool IsRainyWeather() const { return CurrentWeather == EHarmoniaWeatherType::Rainy; }

	/** Check if current weather is snowy */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	bool IsSnowyWeather() const { return CurrentWeather == EHarmoniaWeatherType::Snowy || CurrentWeather == EHarmoniaWeatherType::Blizzard; }

	/** Convert hour to time of day enum */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	static EHarmoniaTimeOfDay HourToTimeOfDay(int32 Hour);

	/** Get time of day as localized string */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	static FString TimeOfDayToString(EHarmoniaTimeOfDay TimeOfDay);

	/** Get weather as localized string */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time & Weather")
	static FString WeatherToString(EHarmoniaWeatherType Weather);

protected:
	// ==================== State ====================

	/** Current time of day */
	UPROPERTY(BlueprintReadOnly, Category = "Time & Weather")
	EHarmoniaTimeOfDay CurrentTimeOfDay;

	/** Current weather */
	UPROPERTY(BlueprintReadOnly, Category = "Time & Weather")
	EHarmoniaWeatherType CurrentWeather;

	/** Current weather intensity */
	UPROPERTY(BlueprintReadOnly, Category = "Time & Weather")
	float CurrentWeatherIntensity;

	/** Current hour (0-23) */
	UPROPERTY(BlueprintReadOnly, Category = "Time & Weather")
	int32 CurrentHour;

	/** Current minute (0-59) */
	UPROPERTY(BlueprintReadOnly, Category = "Time & Weather")
	int32 CurrentMinute;
};
