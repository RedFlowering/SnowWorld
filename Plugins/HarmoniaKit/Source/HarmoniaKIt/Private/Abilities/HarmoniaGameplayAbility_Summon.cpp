// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_Summon.h"
#include "AbilitySystemComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"

UHarmoniaGameplayAbility_Summon::UHarmoniaGameplayAbility_Summon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	MaxActiveSummons = 5;
}

void UHarmoniaGameplayAbility_Summon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cleanup dead summons
	CleanupDeadSummons();

	// Check if can summon more
	if (ActiveSummons.Num() >= MaxActiveSummons)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot summon - max active summons reached (%d)"), MaxActiveSummons);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Play summon animation if available
	if (SummonMontage)
	{
		ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (Character && Character->GetMesh())
		{
			Character->PlayAnimMontage(SummonMontage);
		}
	}

	// Perform summon after short delay (for animation)
	FTimerHandle SummonTimer;
	GetWorld()->GetTimerManager().SetTimer(SummonTimer, this, &UHarmoniaGameplayAbility_Summon::PerformSummon, 0.5f, false);

	// End ability after summon
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_Summon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Summon::PerformSummon()
{
	if (SummonConfigs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No summon configurations set!"));
		return;
	}

	// Select random config or use all configs
	int32 ConfigIndex = FMath::RandRange(0, SummonConfigs.Num() - 1);
	const FHarmoniaSummonConfig& Config = SummonConfigs[ConfigIndex];

	SummonMonster(Config);

	UE_LOG(LogTemp, Log, TEXT("Summoning %d monsters"), Config.SummonCount);
}

void UHarmoniaGameplayAbility_Summon::SummonMonster(const FHarmoniaSummonConfig& Config)
{
	AActor* Summoner = GetAvatarActorFromActorInfo();
	if (!Summoner)
	{
		return;
	}

	AHarmoniaMonsterBase* SummonerMonster = Cast<AHarmoniaMonsterBase>(Summoner);

	// Load monster data
	UHarmoniaMonsterData* MonsterData = Config.MonsterDataToSummon.LoadSynchronous();
	if (!MonsterData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load monster data for summon!"));
		return;
	}

	// Spawn monsters
	int32 NumToSpawn = FMath::Min(Config.SummonCount, MaxActiveSummons - ActiveSummons.Num());

	for (int32 i = 0; i < NumToSpawn; ++i)
	{
		FVector SpawnLocation = GetSummonSpawnLocation(Config.SpawnRadius);
		FRotator SpawnRotation = Summoner->GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Summoner;
		SpawnParams.Instigator = Cast<APawn>(Summoner);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Spawn the monster (assuming we have a monster class to spawn)
		// Note: This assumes HarmoniaMonsterBase or a similar class can be spawned directly
		// In reality, you might need to get the monster class from MonsterData
		AHarmoniaMonsterBase* SummonedMonster = GetWorld()->SpawnActor<AHarmoniaMonsterBase>(
			AHarmoniaMonsterBase::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (SummonedMonster)
		{
			// Initialize with monster data
			SummonedMonster->InitializeWithMonsterData(MonsterData);

			// Inherit level if configured
			if (Config.bInheritSummonerLevel && SummonerMonster)
			{
				SummonedMonster->SetMonsterLevel(SummonerMonster->GetMonsterLevel());
			}

			// Add to active summons
			ActiveSummons.Add(SummonedMonster);

			// Bind death event
			SummonedMonster->OnMonsterDeath.AddDynamic(this, &UHarmoniaGameplayAbility_Summon::OnSummonedMonsterDied);

			// Set lifetime if temporary
			if (Config.SummonedLifetime > 0.0f)
			{
				FTimerHandle LifetimeTimer;
				GetWorld()->GetTimerManager().SetTimer(LifetimeTimer, [SummonedMonster]()
				{
					if (SummonedMonster && !SummonedMonster->IsPendingKill())
					{
						SummonedMonster->Destroy();
					}
				}, Config.SummonedLifetime, false);
			}

			// Spawn visual effect
			if (SummonEffect)
			{
				FActorSpawnParameters EffectSpawnParams;
				EffectSpawnParams.Owner = Summoner;

				AActor* Effect = GetWorld()->SpawnActor<AActor>(SummonEffect, SpawnLocation, SpawnRotation, EffectSpawnParams);
				if (Effect)
				{
					// Auto-destroy effect after a short time
					Effect->SetLifeSpan(2.0f);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Summoned monster at %s"), *SpawnLocation.ToString());
		}
	}
}

void UHarmoniaGameplayAbility_Summon::CleanupDeadSummons()
{
	ActiveSummons.RemoveAll([](AHarmoniaMonsterBase* Summon)
	{
		return !IsValid(Summon) || Summon->IsPendingKill() || Summon->GetMonsterState() == EHarmoniaMonsterState::Dead;
	});
}

void UHarmoniaGameplayAbility_Summon::OnSummonedMonsterDied(AHarmoniaMonsterBase* DeadMonster)
{
	if (DeadMonster)
	{
		ActiveSummons.Remove(DeadMonster);
		UE_LOG(LogTemp, Log, TEXT("Summoned monster died, active summons: %d"), ActiveSummons.Num());
	}
}

void UHarmoniaGameplayAbility_Summon::DestroyAllSummons()
{
	for (AHarmoniaMonsterBase* Summon : ActiveSummons)
	{
		if (IsValid(Summon) && !Summon->IsPendingKill())
		{
			Summon->Destroy();
		}
	}

	ActiveSummons.Empty();
}

FVector UHarmoniaGameplayAbility_Summon::GetSummonSpawnLocation(float Radius) const
{
	AActor* Summoner = GetAvatarActorFromActorInfo();
	if (!Summoner)
	{
		return FVector::ZeroVector;
	}

	FVector SummonerLocation = Summoner->GetActorLocation();

	// Random location in radius
	float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
	float Distance = FMath::FRandRange(Radius * 0.5f, Radius);

	FVector Offset(
		FMath::Cos(Angle) * Distance,
		FMath::Sin(Angle) * Distance,
		0.0f
	);

	FVector TargetLocation = SummonerLocation + Offset;

	// Try to find navigable point
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->GetRandomReachablePointInRadius(TargetLocation, 200.0f, NavLocation))
		{
			return NavLocation.Location;
		}
	}

	return TargetLocation;
}
