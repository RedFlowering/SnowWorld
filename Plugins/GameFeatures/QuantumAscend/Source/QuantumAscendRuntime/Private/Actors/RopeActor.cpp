// Copyright 2025 RedFlowering.

#include "Actors/RopeActor.h"
#include "CableComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

ARopeActor::ARopeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root Component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

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

    RopePhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("RopePhysicsConstraint"));
    RopePhysicsConstraint->SetupAttachment(RootComponent);
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

        // ���� ���� ���� (���� ���������� ����)
        RopePhysicsConstraint->SetConstrainedComponents(StartComponent, StartBoneName, EndComponent, EndBoneName);

        // ���� ���� ���� ����
        RopePhysicsConstraint->SetLinearPositionDrive(true, true, true);
        RopePhysicsConstraint->SetLinearDriveParams(500.0f, 10.0f, 0.0f); // ����, ���� ����

        RopePhysicsConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 45.0f);
        RopePhysicsConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 45.0f);
    }
}

void ARopeActor::DetachRope()
{
    if (Rope)
    {
        // ���� �и�
        Rope->SetAttachEndTo(nullptr, FName(), FName());

        // ���� ���� ����
        RopePhysicsConstraint->BreakConstraint();
    }
}