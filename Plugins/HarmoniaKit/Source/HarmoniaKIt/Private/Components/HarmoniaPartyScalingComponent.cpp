// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaPartyScalingComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UHarmoniaPartyScalingComponent::UHarmoniaPartyScalingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Default 0.5 second tick

	bEnablePartyScaling = true;
	DetectionRadius = 3000.0f;
	UpdateInterval = 1.0f;
	PlayerSeparationDistance = 500.0f;
	bShowDebug = false;
}

void UHarmoniaPartyScalingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());

	if (!OwnerMonster)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaPartyScalingComponent: Owner is not a HarmoniaMonsterBase!"));
		return;
	}

	// Initialize default configs if empty
	if (ScalingConfigs.Num() == 0)
	{
		InitializeDefaultScalingConfigs();
	}

	// Initial detection
	ForceUpdatePartySize();
}

void UHarmoniaPartyScalingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DetectedPartyMembers.Empty();
	OwnerMonster = nullptr;
}

void UHarmoniaPartyScalingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnablePartyScaling || !OwnerMonster)
	{
		return;
	}

	UpdateTimer += DeltaTime;

	if (UpdateTimer >= UpdateInterval)
	{
		UpdateTimer = 0.0f;
		DetectPartyMembers();
	}

	if (bShowDebug)
	{
		DrawDebugInfo();
	}
}

void UHarmoniaPartyScalingComponent::DetectPartyMembers()
{
	if (!OwnerMonster)
	{
		return;
	}

	DetectedPartyMembers.Empty();

	// Get all player controllers in range
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::class, FoundActors);

	TArray<APawn*> NearbyPlayers;
	FVector MonsterLocation = OwnerMonster->GetActorLocation();

	// Find all pawns within detection radius
	for (AActor* Actor : FoundActors)
	{
		APawn* Pawn = Cast<APawn>(Actor);
		if (!Pawn || !Pawn->IsPlayerControlled())
		{
			continue;
		}

		float Distance = FVector::Dist(MonsterLocation, Pawn->GetActorLocation());
		if (Distance <= DetectionRadius)
		{
			NearbyPlayers.Add(Pawn);
		}
	}

	// Count players considering separation distance
	// This prevents counting the same "group" multiple times
	TArray<APawn*> CountedPlayers;

	for (APawn* Player : NearbyPlayers)
	{
		bool bAlreadyCounted = false;

		// Check if this player is too close to any already counted player
		for (APawn* CountedPlayer : CountedPlayers)
		{
			float Distance = FVector::Dist(Player->GetActorLocation(), CountedPlayer->GetActorLocation());
			if (Distance < PlayerSeparationDistance)
			{
				bAlreadyCounted = true;
				break;
			}
		}

		if (!bAlreadyCounted)
		{
			CountedPlayers.Add(Player);
			DetectedPartyMembers.Add(Player);
		}
	}

	int32 NewPartySize = FMath::Max(1, DetectedPartyMembers.Num());

	// Apply scaling if party size changed
	if (NewPartySize != CurrentPartySize)
	{
		PreviousPartySize = CurrentPartySize;
		CurrentPartySize = NewPartySize;

		OnPartySizeChanged.Broadcast(PreviousPartySize, CurrentPartySize);
		ApplyScaling(CurrentPartySize);
	}
}

void UHarmoniaPartyScalingComponent::ApplyScaling(int32 PartySize)
{
	if (!OwnerMonster)
	{
		return;
	}

	FHarmoniaPartyScalingConfig ScalingConfig = GetScalingForPartySize(PartySize);

	UAbilitySystemComponent* ASC = OwnerMonster->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Apply health scaling
	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (AttributeSet)
	{
		float BaseMaxHealth = AttributeSet->GetMaxHealth();
		float CurrentHealthPercent = 1.0f;

		if (bScalingApplied && BaseMaxHealth > 0)
		{
			// Preserve health percentage when rescaling
			CurrentHealthPercent = AttributeSet->GetHealth() / BaseMaxHealth;
		}

		// Calculate new max health
		// If this is first scaling, use current max health as base
		// If rescaling, reverse previous multiplier and apply new one
		float NewMaxHealth = BaseMaxHealth;
		if (!bScalingApplied)
		{
			NewMaxHealth = BaseMaxHealth * ScalingConfig.HealthMultiplier;
		}
		else
		{
			// Get previous scaling
			FHarmoniaPartyScalingConfig PrevScaling = GetScalingForPartySize(PreviousPartySize);
			if (PrevScaling.HealthMultiplier > 0)
			{
				// Remove previous scaling and apply new one
				NewMaxHealth = (BaseMaxHealth / PrevScaling.HealthMultiplier) * ScalingConfig.HealthMultiplier;
			}
		}

		// Set new max health
		ASC->SetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute(), NewMaxHealth);

		// Restore health percentage
		float NewHealth = NewMaxHealth * CurrentHealthPercent;
		ASC->SetNumericAttributeBase(AttributeSet->GetHealthAttribute(), NewHealth);

		// Apply damage scaling
		float CurrentAttackPower = AttributeSet->GetAttackPower();
		float NewAttackPower = CurrentAttackPower;

		if (!bScalingApplied)
		{
			NewAttackPower = CurrentAttackPower * ScalingConfig.DamageMultiplier;
		}
		else
		{
			FHarmoniaPartyScalingConfig PrevScaling = GetScalingForPartySize(PreviousPartySize);
			if (PrevScaling.DamageMultiplier > 0)
			{
				NewAttackPower = (CurrentAttackPower / PrevScaling.DamageMultiplier) * ScalingConfig.DamageMultiplier;
			}
		}

		ASC->SetNumericAttributeBase(AttributeSet->GetAttackPowerAttribute(), NewAttackPower);
	}

	bScalingApplied = true;

	// Broadcast scaling applied event
	OnScalingApplied.Broadcast(PartySize, ScalingConfig.HealthMultiplier, ScalingConfig.DamageMultiplier);

	UE_LOG(LogTemp, Log, TEXT("Monster %s scaled for party size %d (Health: %.2fx, Damage: %.2fx)"),
		*OwnerMonster->GetName(), PartySize, ScalingConfig.HealthMultiplier, ScalingConfig.DamageMultiplier);
}

FHarmoniaPartyScalingConfig UHarmoniaPartyScalingComponent::GetScalingForPartySize(int32 PartySize) const
{
	// Find exact match
	for (const FHarmoniaPartyScalingConfig& Config : ScalingConfigs)
	{
		if (Config.PartySize == PartySize)
		{
			return Config;
		}
	}

	// Find closest lower tier
	FHarmoniaPartyScalingConfig BestConfig;
	int32 BestDifference = MAX_int32;

	for (const FHarmoniaPartyScalingConfig& Config : ScalingConfigs)
	{
		if (Config.PartySize <= PartySize)
		{
			int32 Difference = PartySize - Config.PartySize;
			if (Difference < BestDifference)
			{
				BestDifference = Difference;
				BestConfig = Config;
			}
		}
	}

	// If no lower tier found, use highest available
	if (BestDifference == MAX_int32 && ScalingConfigs.Num() > 0)
	{
		int32 HighestPartySize = 0;
		for (const FHarmoniaPartyScalingConfig& Config : ScalingConfigs)
		{
			if (Config.PartySize > HighestPartySize)
			{
				HighestPartySize = Config.PartySize;
				BestConfig = Config;
			}
		}
	}

	return BestConfig;
}

void UHarmoniaPartyScalingComponent::InitializeDefaultScalingConfigs()
{
	ScalingConfigs.Empty();

	// Solo player (1)
	FHarmoniaPartyScalingConfig Solo;
	Solo.PartySize = 1;
	Solo.HealthMultiplier = 1.0f;
	Solo.DamageMultiplier = 1.0f;
	Solo.ExperienceMultiplier = 1.0f;
	Solo.LootMultiplier = 1.0f;
	ScalingConfigs.Add(Solo);

	// Duo (2)
	FHarmoniaPartyScalingConfig Duo;
	Duo.PartySize = 2;
	Duo.HealthMultiplier = 1.5f;
	Duo.DamageMultiplier = 1.2f;
	Duo.ExperienceMultiplier = 1.3f;
	Duo.LootMultiplier = 1.5f;
	ScalingConfigs.Add(Duo);

	// Small party (3)
	FHarmoniaPartyScalingConfig SmallParty;
	SmallParty.PartySize = 3;
	SmallParty.HealthMultiplier = 2.0f;
	SmallParty.DamageMultiplier = 1.4f;
	SmallParty.ExperienceMultiplier = 1.5f;
	SmallParty.LootMultiplier = 2.0f;
	ScalingConfigs.Add(SmallParty);

	// Medium party (4)
	FHarmoniaPartyScalingConfig MediumParty;
	MediumParty.PartySize = 4;
	MediumParty.HealthMultiplier = 2.5f;
	MediumParty.DamageMultiplier = 1.6f;
	MediumParty.ExperienceMultiplier = 1.7f;
	MediumParty.LootMultiplier = 2.5f;
	ScalingConfigs.Add(MediumParty);

	// Large party (5+)
	FHarmoniaPartyScalingConfig LargeParty;
	LargeParty.PartySize = 5;
	LargeParty.HealthMultiplier = 3.0f;
	LargeParty.DamageMultiplier = 1.8f;
	LargeParty.ExperienceMultiplier = 2.0f;
	LargeParty.LootMultiplier = 3.0f;
	ScalingConfigs.Add(LargeParty);
}

FHarmoniaPartyScalingConfig UHarmoniaPartyScalingComponent::GetCurrentScaling() const
{
	return GetScalingForPartySize(CurrentPartySize);
}

void UHarmoniaPartyScalingComponent::ForceUpdatePartySize()
{
	DetectPartyMembers();
}

void UHarmoniaPartyScalingComponent::SetPartySize(int32 NewSize)
{
	NewSize = FMath::Max(1, NewSize);

	if (NewSize != CurrentPartySize)
	{
		PreviousPartySize = CurrentPartySize;
		CurrentPartySize = NewSize;

		OnPartySizeChanged.Broadcast(PreviousPartySize, CurrentPartySize);
		ApplyScaling(CurrentPartySize);
	}
}

void UHarmoniaPartyScalingComponent::DrawDebugInfo() const
{
	if (!OwnerMonster)
	{
		return;
	}

	FVector MonsterLocation = OwnerMonster->GetActorLocation();

	// Draw detection radius
	DrawDebugSphere(GetWorld(), MonsterLocation, DetectionRadius, 32, FColor::Yellow, false, -1.0f, 0, 2.0f);

	// Draw lines to detected party members
	for (AActor* Member : DetectedPartyMembers)
	{
		if (Member)
		{
			DrawDebugLine(GetWorld(), MonsterLocation, Member->GetActorLocation(), FColor::Green, false, -1.0f, 0, 2.0f);
			DrawDebugSphere(GetWorld(), Member->GetActorLocation(), 100.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f);
		}
	}

	// Draw text info
	FString DebugText = FString::Printf(TEXT("Party Size: %d\nHealth Mult: %.2fx\nDamage Mult: %.2fx"),
		CurrentPartySize,
		GetCurrentScaling().HealthMultiplier,
		GetCurrentScaling().DamageMultiplier);

	DrawDebugString(GetWorld(), MonsterLocation + FVector(0, 0, 200), DebugText, nullptr, FColor::White, 0.0f, true);
}
