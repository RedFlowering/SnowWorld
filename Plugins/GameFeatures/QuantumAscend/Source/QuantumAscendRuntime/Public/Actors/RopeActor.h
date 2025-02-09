// Copyright 2025 RedFlowering.

#pragma once

#include "GameFramework/Actor.h"
#include "RopeActor.generated.h"

class UCableComponent;
class AHookActor;

UCLASS()
class QUANTUMASCENDRUNTIME_API ARopeActor : public AActor
{
    GENERATED_BODY()

public:
    ARopeActor();

protected:
    virtual void BeginPlay() override;

public:
    // Cable Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RopeActor")
    TObjectPtr<UCableComponent> Rope = nullptr;

    // Attach Point for the Hook
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RopeActor")
    TObjectPtr<USceneComponent> AttachPoint = nullptr;

    // Updates the cable's endpoint
    void UpdateCableEndpoint(AActor* HookActor);
};