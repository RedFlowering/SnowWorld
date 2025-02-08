// Copyright 2025 RedFlowering.

#include "Actors/RopeActor.h"
#include "CableComponent.h"

ARopeActor::ARopeActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root Component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Cable Component
    Rope = CreateDefaultSubobject<UCableComponent>(TEXT("Rope"));
    Rope->SetupAttachment(RootComponent);

    // Attach Point
    AttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttachPoint"));
    AttachPoint->SetupAttachment(RootComponent);

    // Cable Default Settings
    Rope->CableLength = 500.0f;  // Default length
    Rope->NumSegments = 10;     // Number of segments
    Rope->EndLocation = FVector(0, 0, 0);  // Dynamic update later
}

void ARopeActor::BeginPlay()
{
    Super::BeginPlay();
}

void ARopeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ARopeActor::UpdateCableEndpoint(AActor* HookActor)
{
    if (Rope && HookActor)
    {
        Rope->SetAttachEndTo(HookActor, FName(), FName());
    }
}
