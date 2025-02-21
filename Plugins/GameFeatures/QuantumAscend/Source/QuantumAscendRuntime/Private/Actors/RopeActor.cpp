// Copyright 2025 RedFlowering.

#include "Actors/RopeActor.h"
#include "CableComponent.h"
#include "Actors/HookActor.h"

ARopeActor::ARopeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root Component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Cable Component
    Rope = CreateDefaultSubobject<UCableComponent>(TEXT("Rope"));
    Rope->CableLength = 500.0f;  // Default length
    Rope->NumSegments = 10;     // Number of segments
    Rope->EndLocation = FVector::ZeroVector;  // Dynamic update later
    Rope->SetupAttachment(RootComponent);

    // Attach Point
    AttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttachPoint"));
    AttachPoint->SetupAttachment(RootComponent);
}

void ARopeActor::BeginPlay()
{
    Super::BeginPlay();
}

void ARopeActor::UpdateCableEndpoint(AHookActor* HookActor)
{
    if (Rope && HookActor)
    {
        Hook = Cast<AHookActor>(HookActor);
        Rope->SetAttachEndTo(Hook, FName(), FName());
    }
}
