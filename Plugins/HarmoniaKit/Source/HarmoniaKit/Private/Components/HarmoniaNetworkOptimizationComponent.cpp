// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaNetworkOptimizationComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

UHarmoniaNetworkOptimizationComponent::UHarmoniaNetworkOptimizationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Low frequency tick
	SetIsReplicatedByDefault(false); // This component runs server-side
}

void UHarmoniaNetworkOptimizationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only run on server/authority
	if (!GetOwner()->HasAuthority())
	{
		SetComponentTickEnabled(false);
		return;
	}

	// Store original values
	OriginalNetUpdateFrequency = GetOwner()->GetNetUpdateFrequency();
	OriginalDormancy = GetOwner()->NetDormancy;

	// Apply custom net cull distance if configured
	if (Config.bUseCustomRelevancy)
	{
		GetOwner()->SetNetCullDistanceSquared(Config.NetCullDistanceSquared);
	}

	// Initial calculation
	RecalculateLevel();
}

void UHarmoniaNetworkOptimizationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Restore original values
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		GetOwner()->SetNetUpdateFrequency(OriginalNetUpdateFrequency);
		GetOwner()->SetNetDormancy(OriginalDormancy);
	}

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaNetworkOptimizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only run on server
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Recalculation timer
	TimeSinceRecalculation += DeltaTime;
	if (TimeSinceRecalculation >= Config.RecalculationInterval)
	{
		TimeSinceRecalculation = 0.0f;
		RecalculateLevel();
	}

	// Update dormancy timer
	if (CurrentLevel == EHarmoniaNetOptLevel::Dormant && Config.bEnableDormancy && !bDormancyActive)
	{
		DormantEligibleTime += DeltaTime;
		if (DormantEligibleTime >= Config.DormancyDelay)
		{
			HandleDormancyTransition(true);
		}
	}
}

bool UHarmoniaNetworkOptimizationComponent::IsDormant() const
{
	return bDormancyActive;
}

float UHarmoniaNetworkOptimizationComponent::GetCurrentNetUpdateFrequency() const
{
	return GetNetUpdateFrequencyForLevel(CurrentLevel);
}

void UHarmoniaNetworkOptimizationComponent::ForceLevel(EHarmoniaNetOptLevel Level)
{
	bLevelForced = true;
	ForcedLevel = Level;

	EHarmoniaNetOptLevel OldLevel = CurrentLevel;
	CurrentLevel = Level;

	if (OldLevel != CurrentLevel)
	{
		ApplyLevelSettings();
		OnLevelChanged.Broadcast(OldLevel, CurrentLevel);
	}
}

void UHarmoniaNetworkOptimizationComponent::ClearForcedLevel()
{
	bLevelForced = false;
	RecalculateLevel();
}

void UHarmoniaNetworkOptimizationComponent::SetInCombat(bool bInCombat)
{
	bIsInCombat = bInCombat;

	if (bInCombat)
	{
		// Wake from dormancy if needed
		if (bDormancyActive)
		{
			HandleDormancyTransition(false);
		}

		// Recalculate with combat boost
		RecalculateLevel();
	}
}

void UHarmoniaNetworkOptimizationComponent::ForceNetUpdate()
{
	if (GetOwner())
	{
		GetOwner()->ForceNetUpdate();
	}
}

void UHarmoniaNetworkOptimizationComponent::FlushNetDormancy()
{
	if (GetOwner())
	{
		GetOwner()->FlushNetDormancy();
	}
}

void UHarmoniaNetworkOptimizationComponent::RecalculateLevel()
{
	if (bLevelForced)
	{
		return;
	}

	// Calculate distance
	CachedDistanceToNearestPlayer = CalculateDistanceToNearestPlayer();

	// Convert to level
	EHarmoniaNetOptLevel NewLevel = DistanceToLevel(CachedDistanceToNearestPlayer);

	// Apply combat boost
	if (bIsInCombat && Config.bUseCombatBoost)
	{
		if (NewLevel > Config.CombatMinLevel)
		{
			NewLevel = Config.CombatMinLevel;
		}
	}

	// Check for change
	if (NewLevel != CurrentLevel)
	{
		EHarmoniaNetOptLevel OldLevel = CurrentLevel;
		CurrentLevel = NewLevel;

		// Reset dormancy timer if no longer dormant-eligible
		if (NewLevel != EHarmoniaNetOptLevel::Dormant)
		{
			DormantEligibleTime = 0.0f;
			if (bDormancyActive)
			{
				HandleDormancyTransition(false);
			}
		}

		ApplyLevelSettings();
		OnLevelChanged.Broadcast(OldLevel, CurrentLevel);
	}
}

void UHarmoniaNetworkOptimizationComponent::ApplyLevelSettings()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Apply NetUpdateFrequency
	float NewFrequency = GetNetUpdateFrequencyForLevel(CurrentLevel);
	Owner->SetNetUpdateFrequency(NewFrequency);

	// Log in development
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Verbose, TEXT("[HarmoniaNetOpt] %s: Level=%s, NetUpdateFreq=%.1f, Distance=%.0f"),
		*Owner->GetName(),
		*UEnum::GetValueAsString(CurrentLevel),
		NewFrequency,
		CachedDistanceToNearestPlayer);
#endif
}

float UHarmoniaNetworkOptimizationComponent::CalculateDistanceToNearestPlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	float NearestDistSq = FLT_MAX;

	// Check all player controllers
	for (TActorIterator<APlayerController> It(World); It; ++It)
	{
		APlayerController* PC = *It;
		if (PC && PC->GetPawn())
		{
			float DistSq = FVector::DistSquared(OwnerLocation, PC->GetPawn()->GetActorLocation());
			NearestDistSq = FMath::Min(NearestDistSq, DistSq);
		}
	}

	return (NearestDistSq < FLT_MAX) ? FMath::Sqrt(NearestDistSq) : 0.0f;
}

EHarmoniaNetOptLevel UHarmoniaNetworkOptimizationComponent::DistanceToLevel(float Distance) const
{
	if (Distance <= Config.CriticalDistance)
	{
		return EHarmoniaNetOptLevel::Critical;
	}
	else if (Distance <= Config.HighDistance)
	{
		return EHarmoniaNetOptLevel::High;
	}
	else if (Distance <= Config.MediumDistance)
	{
		return EHarmoniaNetOptLevel::Medium;
	}
	else if (Distance <= Config.LowDistance)
	{
		return EHarmoniaNetOptLevel::Low;
	}
	else if (Distance <= Config.MinimalDistance)
	{
		return EHarmoniaNetOptLevel::Minimal;
	}
	else
	{
		return EHarmoniaNetOptLevel::Dormant;
	}
}

float UHarmoniaNetworkOptimizationComponent::GetNetUpdateFrequencyForLevel(EHarmoniaNetOptLevel Level) const
{
	switch (Level)
	{
		case EHarmoniaNetOptLevel::Critical:
			return Config.CriticalNetUpdateFrequency;
		case EHarmoniaNetOptLevel::High:
			return Config.HighNetUpdateFrequency;
		case EHarmoniaNetOptLevel::Medium:
			return Config.MediumNetUpdateFrequency;
		case EHarmoniaNetOptLevel::Low:
			return Config.LowNetUpdateFrequency;
		case EHarmoniaNetOptLevel::Minimal:
		case EHarmoniaNetOptLevel::Dormant:
			return Config.MinimalNetUpdateFrequency;
		default:
			return OriginalNetUpdateFrequency;
	}
}

void UHarmoniaNetworkOptimizationComponent::HandleDormancyTransition(bool bShouldBeDormant)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (bShouldBeDormant && !bDormancyActive)
	{
		// Enter dormancy
		bDormancyActive = true;
		Owner->SetNetDormancy(Config.DormancyMode);

#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Verbose, TEXT("[HarmoniaNetOpt] %s: Entering dormancy"),
			*Owner->GetName());
#endif
	}
	else if (!bShouldBeDormant && bDormancyActive)
	{
		// Exit dormancy
		bDormancyActive = false;
		DormantEligibleTime = 0.0f;
		Owner->SetNetDormancy(DORM_Awake);
		Owner->ForceNetUpdate();

#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Verbose, TEXT("[HarmoniaNetOpt] %s: Exiting dormancy"),
			*Owner->GetName());
#endif
	}
}
