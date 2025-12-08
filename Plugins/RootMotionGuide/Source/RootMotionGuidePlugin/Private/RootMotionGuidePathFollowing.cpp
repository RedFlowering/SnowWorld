// Copyright 2017 Lee Ju Sik

#include "RootMotionGuidePathFollowing.h"
#include "GameFramework/Character.h"
#include "RootMotionMovement.h"


// Sets default values for this component's properties
URootMotionGuidePathFollowing::URootMotionGuidePathFollowing()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	SetComponentTickEnabled(false);
}

void URootMotionGuidePathFollowing::StartPathFollowing()
{
	SetComponentTickEnabled(true);
}


// Called when the game starts
void URootMotionGuidePathFollowing::BeginPlay()
{
	Super::BeginPlay();

	// ...
	Character = Cast<ACharacter>(GetOwner());

	if (Character)
	{
		RootMotionMovement = Character->FindComponentByClass<URootMotionMovement>();
	}
}


// Called every frame
void URootMotionGuidePathFollowing::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if (IsValid(RootMotionMovement) == false)
	{
		return;
	}

	if (RootMotionMovement->IsPlayingRootMotion() == false)
	{
		SetComponentTickEnabled(false);
	}


	Character->AddMovementInput(MovementInput);
}

