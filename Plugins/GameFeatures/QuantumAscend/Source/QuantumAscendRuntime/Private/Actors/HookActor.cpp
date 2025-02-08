// Copyright 2025 RedFlowering.

#include "Actors/HookActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHookActor::AHookActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Hook Mesh
    HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HookMesh"));
    RootComponent = HookMesh;
    HookMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // Projectile Movement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 2000.0f;
    ProjectileMovement->MaxSpeed = 2000.0f;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    // Overlap 이벤트 바인딩
    HookMesh->OnComponentBeginOverlap.AddDynamic(this, &AHookActor::OnHookOverlap);
}

void AHookActor::BeginPlay()
{
    Super::BeginPlay();
}

void AHookActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AHookActor::OnHookOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        // ProjectileMovement 비활성화
        if (ProjectileMovement)
        {
            ProjectileMovement->StopMovementImmediately();
        }

        // Hook Actor를 충돌 대상에 Attach
        AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);

        UE_LOG(LogTemp, Log, TEXT("Hook attached to: %s"), *OtherActor->GetName());
    }
}