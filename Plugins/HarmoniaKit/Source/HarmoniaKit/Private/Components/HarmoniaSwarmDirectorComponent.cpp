// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaSwarmDirectorComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UHarmoniaSwarmDirectorComponent::UHarmoniaSwarmDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	bIsSwarmLeader = false;
	bAutoElectLeader = true;
	SwarmRadius = 1500.0f;
	CommandInterval = 1.0f;
	CurrentFormation = EHarmoniaSwarmFormation::Swarm;

	// Enable replication for multiplayer
	SetIsReplicatedByDefault(true);
}

void UHarmoniaSwarmDirectorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaSwarmDirectorComponent, CurrentFormation);
	DOREPLIFETIME(UHarmoniaSwarmDirectorComponent, SwarmMembers);
}

void UHarmoniaSwarmDirectorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());

	if (bAutoElectLeader && !bIsSwarmLeader)
	{
		// Check if there's already a leader nearby
		UpdateSwarmMembers();
		bool bHasLeader = false;

		for (AHarmoniaMonsterBase* Member : SwarmMembers)
		{
			UHarmoniaSwarmDirectorComponent* DirectorComp = Member->FindComponentByClass<UHarmoniaSwarmDirectorComponent>();
			if (DirectorComp && DirectorComp->bIsSwarmLeader)
			{
				bHasLeader = true;
				break;
			}
		}

		if (!bHasLeader && SwarmMembers.Num() > 0)
		{
			bIsSwarmLeader = true;
			UE_LOG(LogTemp, Log, TEXT("%s elected as swarm leader"), *GetOwner()->GetName());
		}
	}
}

void UHarmoniaSwarmDirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (bIsSwarmLeader)
	{
		OnLeaderDied.Broadcast(OwnerMonster);
	}

	SwarmMembers.Empty();
}

void UHarmoniaSwarmDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerMonster)
	{
		return;
	}

	UpdateTimer += DeltaTime;
	if (UpdateTimer >= 2.0f)
	{
		UpdateTimer = 0.0f;
		UpdateSwarmMembers();
	}

	if (bIsSwarmLeader)
	{
		CommandTimer += DeltaTime;
		if (CommandTimer >= CommandInterval)
		{
			CommandTimer = 0.0f;
			ExecuteCommand(DeltaTime);
		}
	}
}

void UHarmoniaSwarmDirectorComponent::IssueCommand(EHarmoniaSwarmCommand Command, AActor* Target)
{
	if (!bIsSwarmLeader)
	{
		return;
	}

	CurrentCommand = Command;
	CommandTarget = Target;

	OnCommandIssued.Broadcast(Command, Target);

	UE_LOG(LogTemp, Log, TEXT("Swarm command issued: %d"), (int32)Command);
}

void UHarmoniaSwarmDirectorComponent::SetFormation(EHarmoniaSwarmFormation Formation)
{
	CurrentFormation = Formation;
}

FVector UHarmoniaSwarmDirectorComponent::GetFormationPosition(AHarmoniaMonsterBase* Member, AActor* Target) const
{
	if (!OwnerMonster || !Member || SwarmMembers.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	int32 MemberIndex = SwarmMembers.IndexOfByKey(Member);
	if (MemberIndex == INDEX_NONE)
	{
		return Member->GetActorLocation();
	}

	FVector LeaderLocation = OwnerMonster->GetActorLocation();
	FVector TargetLocation = Target ? Target->GetActorLocation() : LeaderLocation + FVector(1000, 0, 0);

	switch (CurrentFormation)
	{
	case EHarmoniaSwarmFormation::Circle:
		{
			float Angle = (MemberIndex / (float)SwarmMembers.Num()) * 2.0f * PI;
			float Radius = 500.0f;
			return TargetLocation + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
		}

	case EHarmoniaSwarmFormation::Line:
		{
			FVector Direction = (TargetLocation - LeaderLocation).GetSafeNormal();
			FVector Right = FVector::CrossProduct(Direction, FVector::UpVector);
			return LeaderLocation + Right * (MemberIndex - SwarmMembers.Num() / 2.0f) * 200.0f;
		}

	default:
		return LeaderLocation + FVector(FMath::RandRange(-300.0f, 300.0f), FMath::RandRange(-300.0f, 300.0f), 0.0f);
	}
}

void UHarmoniaSwarmDirectorComponent::ElectNewLeader()
{
	UpdateSwarmMembers();

	if (SwarmMembers.Num() > 0)
	{
		// Elect strongest member as leader
		AHarmoniaMonsterBase* NewLeader = SwarmMembers[0];
		UHarmoniaSwarmDirectorComponent* NewDirector = NewLeader->FindComponentByClass<UHarmoniaSwarmDirectorComponent>();
		if (NewDirector)
		{
			NewDirector->bIsSwarmLeader = true;
			UE_LOG(LogTemp, Log, TEXT("%s elected as new swarm leader"), *NewLeader->GetName());
		}
	}
}

void UHarmoniaSwarmDirectorComponent::UpdateSwarmMembers()
{
	SwarmMembers.Empty();

	if (!OwnerMonster)
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHarmoniaMonsterBase::StaticClass(), FoundActors);

	FVector MyLocation = OwnerMonster->GetActorLocation();
	EHarmoniaMonsterFaction MyFaction = OwnerMonster->GetFaction();

	for (AActor* Actor : FoundActors)
	{
		if (Actor == GetOwner())
		{
			continue;
		}

		AHarmoniaMonsterBase* OtherMonster = Cast<AHarmoniaMonsterBase>(Actor);
		if (OtherMonster && OtherMonster->GetFaction() == MyFaction)
		{
			float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
			if (Distance <= SwarmRadius)
			{
				SwarmMembers.Add(OtherMonster);
			}
		}
	}
}

void UHarmoniaSwarmDirectorComponent::ExecuteCommand(float DeltaTime)
{
	// Command execution would be implemented here
	// For now, just placeholder
}

void UHarmoniaSwarmDirectorComponent::OnLeaderDeath(AHarmoniaMonsterBase* DeadLeader)
{
	if (DeadLeader == OwnerMonster)
	{
		OnLeaderDied.Broadcast(DeadLeader);
		ElectNewLeader();
	}
}
