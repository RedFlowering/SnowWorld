// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaCoordinatedAttackComponent.h"
#include "Core/HarmoniaCoreBFL.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Kismet/GameplayStatics.h"

UHarmoniaCoordinatedAttackComponent::UHarmoniaCoordinatedAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	bEnableCoordination = true;
	AllyDetectionRadius = 1000.0f;
	CoordinationCooldown = 10.0f;
}

void UHarmoniaCoordinatedAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());
}

void UHarmoniaCoordinatedAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ActiveRoles.Empty();
}

void UHarmoniaCoordinatedAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableCoordination || !OwnerMonster)
	{
		return;
	}

	CooldownTimer -= DeltaTime;
}

bool UHarmoniaCoordinatedAttackComponent::InitiateCoordinatedAttack(AActor* Target)
{
	// Only run on server
	HARMONIA_REQUIRE_SERVER_RETURN(this, false);

	if (!Target || CooldownTimer > 0.0f)
	{
		return false;
	}

	TArray<AHarmoniaMonsterBase*> Allies = GetNearbyAllies();
	if (Allies.Num() < 2)
	{
		return false;
	}

	// Find suitable pattern
	for (const FHarmoniaCoordinatedAttackPattern& Pattern : AttackPatterns)
	{
		if (Allies.Num() >= Pattern.RequiredParticipants)
		{
			OnAttackStarted.Broadcast(Pattern.PatternName, Allies.Num());
			CooldownTimer = CoordinationCooldown;
			return true;
		}
	}

	return false;
}

void UHarmoniaCoordinatedAttackComponent::AssignRole(AHarmoniaMonsterBase* Monster, EHarmoniaCoordinatedRole Role)
{
	// Only run on server
	HARMONIA_REQUIRE_SERVER(this);

	if (Monster)
	{
		ActiveRoles.Add(Monster, Role);
	}
}

TArray<AHarmoniaMonsterBase*> UHarmoniaCoordinatedAttackComponent::GetNearbyAllies() const
{
	TArray<AHarmoniaMonsterBase*> Allies;

	if (!OwnerMonster)
	{
		return Allies;
	}

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
		if (OtherMonster && OtherMonster->GetFaction() == MyFaction)
		{
			float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
			if (Distance <= AllyDetectionRadius)
			{
				Allies.Add(OtherMonster);
			}
		}
	}

	return Allies;
}
