// Copyright 2025 RedFlowering.

#pragma once

#include "GameFramework/Actor.h"
#include "RopeActor.generated.h"

class UCableComponent;

UCLASS()
class QUANTUMASCENDRUNTIME_API ARopeActor : public AActor
{
    GENERATED_BODY()

public:
    ARopeActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    /** Cable Component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCableComponent> Rope = nullptr;

    /** Attach Point for the Hook */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> AttachPoint = nullptr;

    /** Updates the cable's endpoint */
    void UpdateCableEndpoint(AActor* HookActor);
};