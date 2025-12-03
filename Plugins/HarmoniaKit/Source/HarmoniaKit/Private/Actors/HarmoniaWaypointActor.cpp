// Copyright RedFlowering. All Rights Reserved.

#include "Actors/HarmoniaWaypointActor.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

AHarmoniaWaypointActor::AHarmoniaWaypointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create detection sphere
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(Root);
	DetectionSphere->SetSphereRadius(500.0f);
	DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaWaypointActor::OnDetectionSphereBeginOverlap);

	// Create billboard for editor visualization
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(Root);

	bDiscovered = false;
}

void AHarmoniaWaypointActor::BeginPlay()
{
	Super::BeginPlay();

	// Set discovery radius
	if (DetectionSphere)
	{
		DetectionSphere->SetSphereRadius(WaypointData.DiscoveryRadius);
	}

	// Check if discovered by default
	if (WaypointData.bDiscoveredByDefault)
	{
		bDiscovered = true;
	}
}

void AHarmoniaWaypointActor::DiscoverWaypoint()
{
	if (bDiscovered)
	{
		return;
	}

	bDiscovered = true;

	// Broadcast discovery event
	// This would typically notify the game's waypoint manager system
}

bool AHarmoniaWaypointActor::CanTravelTo() const
{
	if (!bDiscovered)
	{
		return false;
	}

	// Add additional checks here (quest requirements, level requirements, etc.)

	return true;
}

void AHarmoniaWaypointActor::OnDetectionSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Check if it's a player character
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	// Discover waypoint if not already discovered
	if (!bDiscovered)
	{
		DiscoverWaypoint();

		// Send discovery event to character
		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor);
		if (ASI)
		{
			UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
			if (ASC)
			{
				FGameplayEventData EventData;
				EventData.OptionalObject = this;
				ASC->HandleGameplayEvent(
					HarmoniaGameplayTags::GameplayEvent_Waypoint_Discovered,
					&EventData
				);
			}
		}
	}
}
