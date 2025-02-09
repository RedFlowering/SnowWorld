// Copyright 2025 RedFlowering.

#pragma once

#include "HookActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHookArrivedDelegate);

UCLASS()
class QUANTUMASCENDRUNTIME_API AHookActor : public AActor
{
	GENERATED_BODY()

public:
	AHookActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void MoveToTarget(FVector TargetLocation);

protected:
	void HookActorToTarget();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HookActor")
	TObjectPtr<USphereComponent> Collision = nullptr;

	// Hook Mesh 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HookActor")
	TObjectPtr<UStaticMeshComponent> AnchorMesh = nullptr;

	// Movement Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HookActor")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "HookActor")
    FOnHookArrivedDelegate OnHookArrived;

protected:
	FVector TargetLocation = FVector::ZeroVector;

	FVector LastDirection = FVector::ZeroVector;

	bool bMoveStart = false;
};