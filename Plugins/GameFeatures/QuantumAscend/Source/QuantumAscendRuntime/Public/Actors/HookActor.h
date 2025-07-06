// Copyright 2025 Snow Game Studio.

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
	void ArrivalsToTarget();
	
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

	UPROPERTY(EditAnywhere, Category = "HookActor|Trace")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECollisionChannel::ECC_Pawn;

	UPROPERTY(EditAnywhere, Category = "HookActor|Trace|Debug")
	bool UseDebugMode = false;

	UPROPERTY(EditAnywhere, Category = "HookActor|Trace|Debug")
	float DebugTraceLifeTime = 0.5f;

protected:
	FVector TargetLocation = FVector::ZeroVector;

	FVector LastDirection = FVector::ZeroVector;

	bool bMoveStart = false;
};