// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actors/HarmoniaDynamicSpawnManager.h"
#include "Actors/HarmoniaMonsterSpawner.h"
#include "System/HarmoniaTimeWeatherManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AHarmoniaDynamicSpawnManager::AHarmoniaDynamicSpawnManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f; // Tick every second

	bEnableDynamicSpawn = true;
	bAutoFindSpawners = true;
	BaseSpawnInterval = 30.0f;

	bEnableWaveMode = false;
	CurrentWave = 0;
	bWaveActive = false;
	bAutoStartWaves = false;

	CurrentSpawnRateMultiplier = 1.0f;
	CurrentMonsterCountMultiplier = 1.0f;
}

void AHarmoniaDynamicSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	// Get time/weather manager
	TimeWeatherManager = GetWorld()->GetSubsystem<UHarmoniaTimeWeatherManager>();
	if (TimeWeatherManager)
	{
		// Bind to time/weather events
		TimeWeatherManager->OnTimeOfDayChanged.AddDynamic(this, &AHarmoniaDynamicSpawnManager::OnTimeChanged);
		TimeWeatherManager->OnWeatherChanged.AddDynamic(this, &AHarmoniaDynamicSpawnManager::OnWeatherChanged);
	}

	// Find spawners if configured
	if (bAutoFindSpawners)
	{
		FindSpawnersInLevel();
	}

	// Apply initial modifiers
	ApplySpawnModifiers();

	// Start wave mode if configured
	if (bEnableWaveMode && bAutoStartWaves)
	{
		StartWaveMode();
	}
}

void AHarmoniaDynamicSpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Unbind events
	if (TimeWeatherManager)
	{
		TimeWeatherManager->OnTimeOfDayChanged.RemoveDynamic(this, &AHarmoniaDynamicSpawnManager::OnTimeChanged);
		TimeWeatherManager->OnWeatherChanged.RemoveDynamic(this, &AHarmoniaDynamicSpawnManager::OnWeatherChanged);
	}

	ManagedSpawners.Empty();
	CurrentWaveMonsters.Empty();
}

void AHarmoniaDynamicSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bEnableWaveMode && bWaveActive)
	{
		UpdateWaveSpawning(DeltaTime);
	}
}

void AHarmoniaDynamicSpawnManager::FindSpawnersInLevel()
{
	TArray<AActor*> FoundSpawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHarmoniaMonsterSpawner::StaticClass(), FoundSpawners);

	ManagedSpawners.Empty();

	for (AActor* Actor : FoundSpawners)
	{
		AHarmoniaMonsterSpawner* Spawner = Cast<AHarmoniaMonsterSpawner>(Actor);
		if (Spawner)
		{
			ManagedSpawners.Add(Spawner);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DynamicSpawnManager found %d spawners"), ManagedSpawners.Num());
}

void AHarmoniaDynamicSpawnManager::ApplySpawnModifiers()
{
	if (!bEnableDynamicSpawn)
	{
		return;
	}

	TArray<FHarmoniaSpawnModifier> Modifiers = GetApplicableModifiers();

	// Calculate combined multipliers
	CurrentSpawnRateMultiplier = 1.0f;
	CurrentMonsterCountMultiplier = 1.0f;

	for (const FHarmoniaSpawnModifier& Modifier : Modifiers)
	{
		CurrentSpawnRateMultiplier *= Modifier.SpawnRateMultiplier;
		CurrentMonsterCountMultiplier *= Modifier.MonsterCountMultiplier;
	}

	// Apply to spawners
	for (AHarmoniaMonsterSpawner* Spawner : ManagedSpawners)
	{
		if (!Spawner)
		{
			continue;
		}

		// Adjust spawn intervals and counts
		// This is a simplified version - actual implementation would depend on spawner API
		// Spawner->SetSpawnRateMultiplier(CurrentSpawnRateMultiplier);
	}

	UE_LOG(LogTemp, Log, TEXT("Spawn modifiers applied: Rate x%.2f, Count x%.2f"),
		CurrentSpawnRateMultiplier, CurrentMonsterCountMultiplier);
}

TArray<FHarmoniaSpawnModifier> AHarmoniaDynamicSpawnManager::GetApplicableModifiers() const
{
	TArray<FHarmoniaSpawnModifier> Result;

	if (!TimeWeatherManager)
	{
		return Result;
	}

	EHarmoniaTimeOfDay CurrentTime = TimeWeatherManager->GetCurrentTimeOfDay();
	EHarmoniaWeatherType CurrentWeather = TimeWeatherManager->GetCurrentWeather();

	for (const FHarmoniaSpawnModifier& Modifier : SpawnModifiers)
	{
		bool bTimeMatches = (Modifier.TimeOfDay == CurrentTime);
		bool bWeatherMatches = (Modifier.WeatherType == CurrentWeather);

		if (Modifier.bRequireBothConditions)
		{
			if (bTimeMatches && bWeatherMatches)
			{
				Result.Add(Modifier);
			}
		}
		else
		{
			if (bTimeMatches || bWeatherMatches)
			{
				Result.Add(Modifier);
			}
		}
	}

	return Result;
}

void AHarmoniaDynamicSpawnManager::OnTimeChanged(const FHarmoniaTimeChangeInfo& TimeInfo)
{
	UE_LOG(LogTemp, Log, TEXT("Time changed to %s - updating spawn modifiers"),
		*UHarmoniaTimeWeatherManager::TimeOfDayToString(TimeInfo.NewTime));

	ApplySpawnModifiers();
}

void AHarmoniaDynamicSpawnManager::OnWeatherChanged(const FHarmoniaWeatherChangeInfo& WeatherInfo)
{
	UE_LOG(LogTemp, Log, TEXT("Weather changed to %s - updating spawn modifiers"),
		*UHarmoniaTimeWeatherManager::WeatherToString(WeatherInfo.NewWeather));

	ApplySpawnModifiers();
}

void AHarmoniaDynamicSpawnManager::StartWaveMode()
{
	if (WaveConfigs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start wave mode - no wave configs!"));
		return;
	}

	CurrentWave = 1;
	StartWave(CurrentWave);

	UE_LOG(LogTemp, Log, TEXT("Wave mode started"));
}

void AHarmoniaDynamicSpawnManager::StartWave(int32 WaveNumber)
{
	if (WaveNumber < 1 || WaveNumber > WaveConfigs.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid wave number: %d"), WaveNumber);
		return;
	}

	CurrentWave = WaveNumber;
	bWaveActive = true;
	WaveStartTime = GetWorld()->GetTimeSeconds();
	WaveSpawnTimer = WaveConfigs[WaveNumber - 1].WaveStartDelay;
	WaveSpawnIndex = 0;
	CurrentWaveMonsters.Empty();

	OnWaveStarted.Broadcast(CurrentWave);

	UE_LOG(LogTemp, Log, TEXT("Wave %d started"), CurrentWave);
}

void AHarmoniaDynamicSpawnManager::StopWaveMode()
{
	bWaveActive = false;
	CurrentWave = 0;

	// Clean up wave monsters
	CurrentWaveMonsters.Empty();

	UE_LOG(LogTemp, Log, TEXT("Wave mode stopped"));
}

void AHarmoniaDynamicSpawnManager::UpdateWaveSpawning(float DeltaTime)
{
	if (!bWaveActive || CurrentWave < 1 || CurrentWave > WaveConfigs.Num())
	{
		return;
	}

	const FHarmoniaWaveConfig& WaveConfig = WaveConfigs[CurrentWave - 1];

	WaveSpawnTimer -= DeltaTime;

	if (WaveSpawnTimer <= 0.0f)
	{
		// Spawn next monster
		SpawnWaveMonsters(WaveConfig);

		// Reset timer
		WaveSpawnTimer = WaveConfig.SpawnInterval;
	}

	// Check if wave is complete
	if (IsWaveComplete())
	{
		CompleteWave();
	}
}

void AHarmoniaDynamicSpawnManager::SpawnWaveMonsters(const FHarmoniaWaveConfig& WaveConfig)
{
	// Simple implementation - would need to be expanded with actual spawning logic
	// This is a placeholder that shows the structure

	int32 TotalMonstersToSpawn = WaveConfig.MonsterTypes.Num() * WaveConfig.MonstersPerType;

	if (WaveSpawnIndex >= TotalMonstersToSpawn)
	{
		return; // All monsters spawned
	}

	// Use first spawner for wave spawning
	if (ManagedSpawners.Num() > 0 && ManagedSpawners[0])
	{
		// Trigger spawner
		// ManagedSpawners[0]->SpawnMonster();
	}

	WaveSpawnIndex++;

	UE_LOG(LogTemp, Log, TEXT("Wave %d: Spawned monster %d/%d"), CurrentWave, WaveSpawnIndex, TotalMonstersToSpawn);
}

bool AHarmoniaDynamicSpawnManager::IsWaveComplete() const
{
	if (!bWaveActive)
	{
		return false;
	}

	// Check if all wave monsters are spawned and defeated
	if (CurrentWave < 1 || CurrentWave > WaveConfigs.Num())
	{
		return false;
	}

	const FHarmoniaWaveConfig& WaveConfig = WaveConfigs[CurrentWave - 1];
	int32 TotalMonstersToSpawn = WaveConfig.MonsterTypes.Num() * WaveConfig.MonstersPerType;

	// All monsters spawned?
	if (WaveSpawnIndex < TotalMonstersToSpawn)
	{
		return false;
	}

	// All monsters defeated?
	int32 AliveCount = 0;
	for (AActor* Monster : CurrentWaveMonsters)
	{
		if (IsValid(Monster))
		{
			AliveCount++;
		}
	}

	return AliveCount == 0;
}

void AHarmoniaDynamicSpawnManager::CompleteWave()
{
	float CompletionTime = GetWorld()->GetTimeSeconds() - WaveStartTime;

	OnWaveCompleted.Broadcast(CurrentWave, CompletionTime);

	UE_LOG(LogTemp, Log, TEXT("Wave %d completed in %.1f seconds"), CurrentWave, CompletionTime);

	// Check if there are more waves
	if (CurrentWave < WaveConfigs.Num())
	{
		// Start next wave
		StartWave(CurrentWave + 1);
	}
	else
	{
		// All waves completed
		bWaveActive = false;
		OnAllWavesCompleted.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("All waves completed!"));
	}
}

float AHarmoniaDynamicSpawnManager::GetCurrentSpawnRateMultiplier() const
{
	return CurrentSpawnRateMultiplier;
}

float AHarmoniaDynamicSpawnManager::GetCurrentMonsterCountMultiplier() const
{
	return CurrentMonsterCountMultiplier;
}

void AHarmoniaDynamicSpawnManager::RefreshSpawnModifiers()
{
	ApplySpawnModifiers();
}
