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

		UWorld* World = GetWorld();
		bool IsHit = false;

		if (World)
		{
			const FCollisionShape Shape = FCollisionShape::MakeCapsule(Collision->GetScaledSphereRadius(), Collision->GetScaledSphereRadius());
			
			FCollisionQueryParams TraceParams(FName(TEXT("SweepMultiTrace")), true, this);
			TraceParams.bTraceComplex = true;
			TraceParams.bFindInitialOverlaps = true;
			TraceParams.bReturnPhysicalMaterial = false;
			TraceParams.AddIgnoredActor(this);
			TraceParams.AddIgnoredActor(GetOwner());

			IsHit = World->OverlapBlockingTestByChannel(GetActorLocation(), FQuat(GetActorRotation()), CollisionChannel, Shape, TraceParams);

#if WITH_EDITOR
			if (UseDebugMode)
			{
				DrawDebugSphere(World, GetActorLocation(), Collision->GetScaledSphereRadius(), 1, IsHit ? FColor::Green : FColor::Red, false, DebugTraceLifeTime, 0.0f, 1.0f);
			}
#endif
		}

		float PassedByPoint = FVector::DotProduct(PointDirNormal, LastDirection.GetSafeNormal());
		LastDirection = PointDir;

		const float LandingExtent = 10.f;

		if (PassedByPoint < 0.0f || PointDir.IsNearlyZero(LandingExtent) || IsHit)
		{
			ArrivalsToTarget();
		}
	}
}

void AHookActor::MoveToTarget(FVector TargetPosition)
{
	TargetLocation = TargetPosition;
	bMoveStart = true;
}

void AHookActor::ArrivalsToTarget()
{
	// If ProjectileMovement is active, handle stopping and deactivating it
	if (ProjectileMovement && ProjectileMovement->IsActive())
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	// SetActorLocation(TargetLocation);

	LastDirection = FVector::ZeroVector;
	TargetLocation = FVector::ZeroVector;

	// Events will be delivered if the Ability that spawned this Actor has pre-bound them.
	OnHookArrived.Broadcast();
}