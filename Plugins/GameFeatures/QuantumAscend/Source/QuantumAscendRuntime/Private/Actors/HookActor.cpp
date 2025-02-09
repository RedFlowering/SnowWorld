// Copyright 2025 RedFlowering.

#include "Actors/HookActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/RFCharacter.h"

AHookActor::AHookActor()
{
    PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(5.0f);
	Collision->BodyInstance.SetCollisionProfileName("Projectile");
	Collision->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	Collision->CanCharacterStepUpOn = ECB_No;
	RootComponent = Collision;

	AnchorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnchorMesh"));
	AnchorMesh->PrimaryComponentTick.bStartWithTickEnabled = 0;
	AnchorMesh->PrimaryComponentTick.bAllowTickOnDedicatedServer = 0;
	AnchorMesh->SetEnableGravity(true);
	AnchorMesh->bApplyImpulseOnDamage = 0;
	AnchorMesh->bReplicatePhysicsToAutonomousProxy = 0;
	AnchorMesh->SetGenerateOverlapEvents(false);
	AnchorMesh->SetCollisionProfileName(FName("NoCollision"));
    AnchorMesh->SetupAttachment(GetRootComponent());

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 2000.0f;
    ProjectileMovement->MaxSpeed = 2000.0f;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
}

void AHookActor::BeginPlay()
{
    Super::BeginPlay();
}

void AHookActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	if (bMoveStart)
	{
		FVector PointDir = TargetLocation - GetActorLocation();
		FVector PointDirNormal = PointDir.GetSafeNormal();

		if (LastDirection.IsNearlyZero())
		{
			LastDirection = PointDir;
		}

		float PassedByPoint = FVector::DotProduct(PointDirNormal, LastDirection.GetSafeNormal());
		LastDirection = PointDir;

		const float LandingExtent = 10.f;

		if (PassedByPoint < 0.0f || PointDir.IsNearlyZero(LandingExtent))
		{
			HookActorToTarget();
		}
	}
}

void AHookActor::MoveToTarget(FVector TargetPosition)
{
	TargetLocation = TargetPosition;
	bMoveStart = true;
}

void AHookActor::HookActorToTarget()
{
	// If ProjectileMovement is active, handle stopping and deactivating it
	if (ProjectileMovement && ProjectileMovement->IsActive())
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	SetActorLocation(TargetLocation);

	LastDirection = FVector::ZeroVector;
	TargetLocation = FVector::ZeroVector;

	// Events will be delivered if the Ability that spawned this Actor has pre-bound them.
	OnHookArrived.Broadcast();
}