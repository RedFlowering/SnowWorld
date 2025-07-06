// Copyright 2025 Snow Game Studio.

#pragma once

#include "GameFramework/Actor.h"
#include "RopeActor.generated.h"

class UCableComponent;
class UPhysicsConstraintComponent;

UCLASS()
class QUANTUMASCENDRUNTIME_API ARopeActor : public AActor
{
    GENERATED_BODY()

public:
    ARopeActor();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
    // Cable Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RopeActor")
    TObjectPtr<UCableComponent> Rope = nullptr;

    TObjectPtr<UPrimitiveComponent> OwnerStart = nullptr;

    TObjectPtr<UPrimitiveComponent> OwnerEnd = nullptr;

    // Updates the cable's endpoint
    void UpdateCable(UPrimitiveComponent* StartComponent, FName StartBoneName, UPrimitiveComponent* EndComponent, FName EndBoneName);

    void DetachRope();
};