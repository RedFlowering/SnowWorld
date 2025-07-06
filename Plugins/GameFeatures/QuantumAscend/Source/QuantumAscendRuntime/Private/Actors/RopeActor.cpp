// Copyright 2025 Snow Game Studio.

#include "Actors/RopeActor.h"
#include "CableComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

ARopeActor::ARopeActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Cable Component
    Rope = CreateDefaultSubobject<UCableComponent>(TEXT("Rope"));
    Rope->SetupAttachment(RootComponent);
    Rope->bAttachStart = true;
    Rope->bAttachEnd = true;
    Rope->CableLength = 1000.0f;  // Default length
    Rope->NumSegments = 10;     // Number of segments
    Rope->SolverIterations = 10;
    Rope->CableWidth = 2.0f;
    Rope->EndLocation = FVector::ZeroVector;  // Dynamic update later
}

void ARopeActor::BeginPlay()
{
    Super::BeginPlay();
}

void ARopeActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (OwnerStart && OwnerEnd)
    {
        float NewCableLength = (OwnerStart->GetComponentLocation() - OwnerEnd->GetComponentLocation()).Length();

        Rope->CableLength = NewCableLength - 400.0f;
    }
}

void ARopeActor::UpdateCable(UPrimitiveComponent* StartComponent, FName StartBoneName, UPrimitiveComponent* EndComponent, FName EndBoneName)
{
    if (Rope && StartComponent && EndComponent)
    {
        OwnerStart = StartComponent;
        OwnerEnd = EndComponent;

        Rope->SetAttachEndTo(EndComponent->GetOwner(), EndBoneName, EndBoneName);
    }
}

void ARopeActor::DetachRope()
{
    if (Rope)
    {
        // 끝점 분리
        Rope->SetAttachEndTo(nullptr, FName(), FName());
    }
}