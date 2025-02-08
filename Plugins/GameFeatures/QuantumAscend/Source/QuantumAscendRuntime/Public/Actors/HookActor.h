// Copyright 2025 RedFlowering.

#pragma once

#include "HookActor.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;

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

	/** Hook Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HookMesh = nullptr;

	/** Movement Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	/** Handle collision events */
	UFUNCTION()
	void OnHookOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};