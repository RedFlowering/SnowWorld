// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaTerritoryDisputeComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Components/HarmoniaSquadComponent.h"
#include "Components/HarmoniaThreatComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AI/HarmoniaMonsterAIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

UHarmoniaTerritoryDisputeComponent::UHarmoniaTerritoryDisputeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.25f;

	bEnableTerritoryDispute = true;
	DetectionRadius = 2000.0f;
	ScanInterval = 2.0f;
	SignificantAdvantageThreshold = 2.0f;
	SlightAdvantageThreshold = 1.3f;
	PursuitChance = 0.7f;
	MaxPursuitDistance = 5000.0f;
	PlayerInterruptFleeTime = 3.0f;
	MinHealthToEngage = 0.3f;
	bCanInitiateDispute = true;
	bCanDefendTerritory = true;
	bShowDebug = false;

	DisputeState = EHarmoniaTerritoryDisputeState::None;
	CurrentOpponent = nullptr;
	ScanTimer = 0.0f;
	FleeTimer = 0.0f;

	// Enable replication for multiplayer
	SetIsReplicatedByDefault(true);
}

void UHarmoniaTerritoryDisputeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaTerritoryDisputeComponent, DisputeState);
	DOREPLIFETIME(UHarmoniaTerritoryDisputeComponent, CurrentOpponent);
}

void UHarmoniaTerritoryDisputeComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());
	if (!OwnerMonster)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaTerritoryDisputeComponent: Owner is not a HarmoniaMonsterBase!"));
		return;
	}

	ScanTimer = FMath::FRandRange(0.0f, ScanInterval);
}

void UHarmoniaTerritoryDisputeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	CurrentOpponent = nullptr;
	DetectedOpponents.Empty();
	OwnerMonster = nullptr;
}

void UHarmoniaTerritoryDisputeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableTerritoryDispute || !OwnerMonster)
	{
		return;
	}

	// Update scan timer
	ScanTimer += DeltaTime;
	if (ScanTimer >= ScanInterval)
	{
		ScanTimer = 0.0f;
		ScanForOpponents();
	}

	// Update dispute state
	UpdateDisputeState(DeltaTime);

	// Check for player interruption
	if (DisputeState == EHarmoniaTerritoryDisputeState::Challenging ||
		DisputeState == EHarmoniaTerritoryDisputeState::Defending)
	{
		CheckPlayerInterruption();
	}

	if (bShowDebug)
	{
		DrawDebugInfo();
	}
}

void UHarmoniaTerritoryDisputeComponent::ScanForOpponents()
{
	if (!OwnerMonster || DisputeState != EHarmoniaTerritoryDisputeState::None)
	{
		return;
	}

	// Don't engage if health is too low
	UAbilitySystemComponent* ASC = OwnerMonster->GetAbilitySystemComponent();
	if (ASC)
	{
		const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
		if (AttributeSet)
		{
			float HealthPercent = AttributeSet->GetHealth() / FMath::Max(1.0f, AttributeSet->GetMaxHealth());
			if (HealthPercent < MinHealthToEngage)
			{
				return;
			}
		}
	}

	DetectedOpponents.Empty();

	// Find all nearby monsters
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHarmoniaMonsterBase::StaticClass(), FoundActors);

	FVector MyLocation = OwnerMonster->GetActorLocation();
	EHarmoniaMonsterFaction MyFaction = OwnerMonster->GetFaction();

	for (AActor* Actor : FoundActors)
	{
		if (Actor == OwnerMonster)
		{
			continue;
		}

		AHarmoniaMonsterBase* OtherMonster = Cast<AHarmoniaMonsterBase>(Actor);
		if (!OtherMonster)
		{
			continue;
		}

		// Check distance
		float Distance = FVector::Dist(MyLocation, OtherMonster->GetActorLocation());
		if (Distance > DetectionRadius)
		{
			continue;
		}

		// Check faction - only fight different factions
		EHarmoniaMonsterFaction OtherFaction = OtherMonster->GetFaction();
		if (OtherFaction == MyFaction || OtherFaction == EHarmoniaMonsterFaction::Neutral)
		{
			continue;
		}

		DetectedOpponents.Add(OtherMonster);

		// Consider engaging this opponent
		if (bCanInitiateDispute && ShouldEngageOpponent(OtherMonster))
		{
			StartDispute(OtherMonster);
			break;
		}
	}
}

bool UHarmoniaTerritoryDisputeComponent::ShouldEngageOpponent(AHarmoniaMonsterBase* Opponent)
{
	if (!Opponent || !OwnerMonster)
	{
		return false;
	}

	// Evaluate combat advantage
	EHarmoniaCombatAdvantage Advantage = EvaluateCombatAdvantage(Opponent);

	// Don't engage if at significant disadvantage
	if (Advantage == EHarmoniaCombatAdvantage::SignificantDisadvantage)
	{
		return false;
	}

	// Always engage if significant advantage
	if (Advantage == EHarmoniaCombatAdvantage::SignificantAdvantage)
	{
		return true;
	}

	// Random chance for even or slight advantage/disadvantage
	float EngageChance = 0.5f;
	if (Advantage == EHarmoniaCombatAdvantage::SlightAdvantage)
	{
		EngageChance = 0.7f;
	}
	else if (Advantage == EHarmoniaCombatAdvantage::SlightDisadvantage)
	{
		EngageChance = 0.3f;
	}

	return FMath::FRand() < EngageChance;
}

EHarmoniaCombatAdvantage UHarmoniaTerritoryDisputeComponent::EvaluateCombatAdvantage(AActor* Opponent)
{
	if (!Opponent || !OwnerMonster)
	{
		return EHarmoniaCombatAdvantage::Even;
	}

	float MyCombatPower = CalculateCombatPower(OwnerMonster);
	float OpponentCombatPower = CalculateCombatPower(Opponent);

	if (OpponentCombatPower <= 0.0f)
	{
		return EHarmoniaCombatAdvantage::SignificantAdvantage;
	}

	float PowerRatio = MyCombatPower / OpponentCombatPower;

	if (PowerRatio >= SignificantAdvantageThreshold)
	{
		return EHarmoniaCombatAdvantage::SignificantAdvantage;
	}
	else if (PowerRatio >= SlightAdvantageThreshold)
	{
		return EHarmoniaCombatAdvantage::SlightAdvantage;
	}
	else if (PowerRatio <= (1.0f / SignificantAdvantageThreshold))
	{
		return EHarmoniaCombatAdvantage::SignificantDisadvantage;
	}
	else if (PowerRatio <= (1.0f / SlightAdvantageThreshold))
	{
		return EHarmoniaCombatAdvantage::SlightDisadvantage;
	}

	return EHarmoniaCombatAdvantage::Even;
}

float UHarmoniaTerritoryDisputeComponent::CalculateCombatPower(AActor* Actor)
{
	if (!Actor)
	{
		return 0.0f;
	}

	float CombatPower = 100.0f; // Base power

	// If it's a player, estimate based on being a player
	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		// Players are generally stronger - base 200 power
		CombatPower = 200.0f;
		return CombatPower;
	}

	// If it's a monster, calculate from stats
	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(Actor);
	if (Monster)
	{
		UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
		if (ASC)
		{
			const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
			if (AttributeSet)
			{
				float Health = AttributeSet->GetHealth();
				float MaxHealth = AttributeSet->GetMaxHealth();
				float AttackPower = AttributeSet->GetAttackPower();
				float Defense = AttributeSet->GetDefense();

				// Health percentage matters
				float HealthPercent = Health / FMath::Max(1.0f, MaxHealth);

				// Combat power = (Attack + Defense) * HealthPercent
				CombatPower = (AttackPower + Defense * 0.5f) * HealthPercent;
			}
		}

		// Squad size multiplier
		UHarmoniaSquadComponent* SquadComp = Monster->FindComponentByClass<UHarmoniaSquadComponent>();
		if (SquadComp)
		{
			int32 SquadSize = SquadComp->SquadMembers.Num();
			CombatPower *= (1.0f + (SquadSize - 1) * 0.3f); // +30% per additional squad member
		}
	}

	return FMath::Max(1.0f, CombatPower);
}

void UHarmoniaTerritoryDisputeComponent::StartDispute(AActor* Target)
{
	if (!Target || !OwnerMonster)
	{
		return;
	}

	CurrentOpponent = Target;
	DisputeState = EHarmoniaTerritoryDisputeState::Challenging;
	DisputeStartLocation = OwnerMonster->GetActorLocation();

	AHarmoniaMonsterBase* OpponentMonster = Cast<AHarmoniaMonsterBase>(Target);
	if (OpponentMonster)
	{
		OnDisputeStarted.Broadcast(OwnerMonster, OpponentMonster);

		// Notify opponent to defend
		UHarmoniaTerritoryDisputeComponent* OpponentDispute = OpponentMonster->FindComponentByClass<UHarmoniaTerritoryDisputeComponent>();
		if (OpponentDispute && OpponentDispute->bCanDefendTerritory)
		{
			OpponentDispute->DisputeState = EHarmoniaTerritoryDisputeState::Defending;
			OpponentDispute->CurrentOpponent = OwnerMonster;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s started territory dispute with %s"),
		*OwnerMonster->GetName(), *Target->GetName());
}

void UHarmoniaTerritoryDisputeComponent::EndDispute(bool bVictorious)
{
	if (DisputeState == EHarmoniaTerritoryDisputeState::None)
	{
		return;
	}

	AActor* Opponent = CurrentOpponent;
	AHarmoniaMonsterBase* OpponentMonster = Cast<AHarmoniaMonsterBase>(Opponent);

	if (bVictorious && OpponentMonster)
	{
		OnDisputeEnded.Broadcast(OwnerMonster, OpponentMonster);
	}
	else if (!bVictorious && OpponentMonster)
	{
		OnDisputeEnded.Broadcast(OpponentMonster, OwnerMonster);
	}

	DisputeState = EHarmoniaTerritoryDisputeState::None;
	CurrentOpponent = nullptr;

	UE_LOG(LogTemp, Log, TEXT("%s ended territory dispute (%s)"),
		*OwnerMonster->GetName(), bVictorious ? TEXT("Victory") : TEXT("Defeat"));
}

void UHarmoniaTerritoryDisputeComponent::FleeFromOpponent(AActor* Opponent)
{
	if (!Opponent || !OwnerMonster)
	{
		return;
	}

	CurrentOpponent = Opponent;
	DisputeState = EHarmoniaTerritoryDisputeState::Fleeing;

	EHarmoniaCombatAdvantage Advantage = EvaluateCombatAdvantage(Opponent);
	OnFleeingFromOpponent.Broadcast(Opponent, Advantage);

	// Set monster state to retreating
	OwnerMonster->SetMonsterState(EHarmoniaMonsterState::Retreating);

	UE_LOG(LogTemp, Log, TEXT("%s fleeing from %s"),
		*OwnerMonster->GetName(), *Opponent->GetName());
}

void UHarmoniaTerritoryDisputeComponent::PursueOpponent(AActor* Opponent)
{
	if (!Opponent || !OwnerMonster)
	{
		return;
	}

	// Check if too far to pursue
	float Distance = FVector::Dist(OwnerMonster->GetActorLocation(), Opponent->GetActorLocation());
	if (Distance > MaxPursuitDistance)
	{
		EndDispute(true);
		return;
	}

	CurrentOpponent = Opponent;
	DisputeState = EHarmoniaTerritoryDisputeState::Pursuing;

	EHarmoniaCombatAdvantage Advantage = EvaluateCombatAdvantage(Opponent);
	OnPursuingOpponent.Broadcast(Opponent, Advantage);

	UE_LOG(LogTemp, Log, TEXT("%s pursuing %s"),
		*OwnerMonster->GetName(), *Opponent->GetName());
}

void UHarmoniaTerritoryDisputeComponent::HandlePlayerInterruption(AActor* Player)
{
	if (!Player || !OwnerMonster)
	{
		return;
	}

	// Player interrupted dispute - shorten flee time
	DisputeState = EHarmoniaTerritoryDisputeState::PlayerInterrupted;
	FleeTimer = PlayerInterruptFleeTime;

	OnPlayerInterruptedDispute.Broadcast(Player);

	// Flee from player
	CurrentOpponent = Player;

	UE_LOG(LogTemp, Log, TEXT("%s interrupted by player - fleeing"),
		*OwnerMonster->GetName());
}

bool UHarmoniaTerritoryDisputeComponent::ShouldFleeFromOpponent(AActor* Opponent)
{
	EHarmoniaCombatAdvantage Advantage = EvaluateCombatAdvantage(Opponent);

	// Always flee from significant disadvantage
	if (Advantage == EHarmoniaCombatAdvantage::SignificantDisadvantage)
	{
		return true;
	}

	// Sometimes flee from slight disadvantage
	if (Advantage == EHarmoniaCombatAdvantage::SlightDisadvantage)
	{
		return FMath::FRand() < 0.5f;
	}

	return false;
}

bool UHarmoniaTerritoryDisputeComponent::ShouldPursueOpponent(AActor* Opponent)
{
	EHarmoniaCombatAdvantage Advantage = EvaluateCombatAdvantage(Opponent);

	// Check distance
	float Distance = FVector::Dist(OwnerMonster->GetActorLocation(), Opponent->GetActorLocation());
	if (Distance > MaxPursuitDistance)
	{
		return false;
	}

	// Pursue if significant advantage
	if (Advantage == EHarmoniaCombatAdvantage::SignificantAdvantage)
	{
		return FMath::FRand() < PursuitChance;
	}

	// Sometimes pursue if slight advantage
	if (Advantage == EHarmoniaCombatAdvantage::SlightAdvantage)
	{
		return FMath::FRand() < (PursuitChance * 0.5f);
	}

	return false;
}

FVector UHarmoniaTerritoryDisputeComponent::GetFleeLocation(AActor* Opponent)
{
	if (!Opponent || !OwnerMonster)
	{
		return OwnerMonster->GetActorLocation();
	}

	FVector MyLocation = OwnerMonster->GetActorLocation();
	FVector OpponentLocation = Opponent->GetActorLocation();

	// Flee in opposite direction
	FVector FleeDirection = (MyLocation - OpponentLocation).GetSafeNormal();
	FVector FleeLocation = MyLocation + FleeDirection * 2000.0f; // 20 meters away

	// Try to find navigable location
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->GetRandomReachablePointInRadius(FleeLocation, 500.0f, NavLocation))
		{
			return NavLocation.Location;
		}
	}

	return FleeLocation;
}

void UHarmoniaTerritoryDisputeComponent::UpdateDisputeState(float DeltaTime)
{
	if (DisputeState == EHarmoniaTerritoryDisputeState::None)
	{
		return;
	}

	if (!CurrentOpponent || !OwnerMonster)
	{
		EndDispute(false);
		return;
	}

	// Check if opponent is still valid
	if (!IsValid(CurrentOpponent))
	{
		EndDispute(true);
		return;
	}

	// Handle player interrupted flee timer
	if (DisputeState == EHarmoniaTerritoryDisputeState::PlayerInterrupted)
	{
		FleeTimer -= DeltaTime;
		if (FleeTimer <= 0.0f)
		{
			// Try to make it 1v1 with player
			DisputeState = EHarmoniaTerritoryDisputeState::Challenging;
			OwnerMonster->SetMonsterState(EHarmoniaMonsterState::Combat);
		}
		return;
	}

	// Handle ongoing dispute/pursuit/flee
	float Distance = FVector::Dist(OwnerMonster->GetActorLocation(), CurrentOpponent->GetActorLocation());

	if (DisputeState == EHarmoniaTerritoryDisputeState::Fleeing)
	{
		// Stop fleeing if far enough
		if (Distance > DetectionRadius * 1.5f)
		{
			EndDispute(false);
		}
	}
	else if (DisputeState == EHarmoniaTerritoryDisputeState::Pursuing)
	{
		// Stop pursuing if too far
		if (Distance > MaxPursuitDistance)
		{
			EndDispute(true);
		}
	}
}

void UHarmoniaTerritoryDisputeComponent::CheckPlayerInterruption()
{
	if (!OwnerMonster || !CurrentOpponent)
	{
		return;
	}

	// Find nearby players
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

	FVector DisputeCenter = (OwnerMonster->GetActorLocation() + CurrentOpponent->GetActorLocation()) * 0.5f;

	for (AActor* Actor : FoundActors)
	{
		APawn* Pawn = Cast<APawn>(Actor);
		if (!Pawn || !Pawn->IsPlayerControlled())
		{
			continue;
		}

		float Distance = FVector::Dist(DisputeCenter, Pawn->GetActorLocation());
		if (Distance < DetectionRadius * 0.5f) // Player within half detection radius
		{
			HandlePlayerInterruption(Pawn);
			break;
		}
	}
}

void UHarmoniaTerritoryDisputeComponent::DrawDebugInfo()
{
	if (!OwnerMonster)
	{
		return;
	}

	FVector MyLocation = OwnerMonster->GetActorLocation();

	// Draw detection radius
	DrawDebugSphere(GetWorld(), MyLocation, DetectionRadius, 32, FColor::Orange, false, -1.0f, 0, 2.0f);

	// Draw line to current opponent
	if (CurrentOpponent)
	{
		FColor LineColor = FColor::Red;
		if (DisputeState == EHarmoniaTerritoryDisputeState::Pursuing)
		{
			LineColor = FColor::Yellow;
		}
		else if (DisputeState == EHarmoniaTerritoryDisputeState::Fleeing)
		{
			LineColor = FColor::Blue;
		}

		DrawDebugLine(GetWorld(), MyLocation, CurrentOpponent->GetActorLocation(), LineColor, false, -1.0f, 0, 3.0f);
	}

	// Draw state text
	FString StateText = FString::Printf(TEXT("Dispute: %s"),
		DisputeState == EHarmoniaTerritoryDisputeState::None ? TEXT("None") :
		DisputeState == EHarmoniaTerritoryDisputeState::Challenging ? TEXT("Challenging") :
		DisputeState == EHarmoniaTerritoryDisputeState::Defending ? TEXT("Defending") :
		DisputeState == EHarmoniaTerritoryDisputeState::Fleeing ? TEXT("Fleeing") :
		DisputeState == EHarmoniaTerritoryDisputeState::Pursuing ? TEXT("Pursuing") :
		TEXT("Player Interrupted"));

	DrawDebugString(GetWorld(), MyLocation + FVector(0, 0, 250), StateText, nullptr, FColor::White, 0.0f, true);
}
