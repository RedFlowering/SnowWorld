// Copyright 2024 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnTargetingComponent.generated.h"

class AActor;
class APawn;
class ABaseCharacter;

/**
 * ULockOnTargetingComponent
 *
 * Component that handles lock-on targeting system for souls-like combat.
 * Allows the player to lock onto enemies and maintain focus during combat.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULockOnTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULockOnTargetingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Toggle lock-on targeting */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void ToggleLockOn();

	/** Enable lock-on to the nearest target */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void EnableLockOn();

	/** Disable lock-on */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void DisableLockOn();

	/** Switch to the next target to the right */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void SwitchTargetRight();

	/** Switch to the next target to the left */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void SwitchTargetLeft();

	/** Check if currently locked on to a target */
	UFUNCTION(BlueprintPure, Category = "Lock-On")
	bool IsLockedOn() const { return bIsLockedOn && CurrentTarget.IsValid(); }

	/** Get the current locked target */
	UFUNCTION(BlueprintPure, Category = "Lock-On")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

protected:
	virtual void BeginPlay() override;

	/** Find the nearest valid target within range */
	AActor* FindNearestTarget() const;

	/** Find all valid targets within range */
	TArray<AActor*> FindValidTargets() const;

	/** Check if an actor is a valid target */
	bool IsValidTarget(AActor* Target) const;

	/** Update camera rotation to face locked target */
	void UpdateCameraToTarget(float DeltaTime);

	/** Check if target is still valid and within range */
	void ValidateCurrentTarget();

	/** Switch to a different target based on direction */
	void SwitchTarget(bool bSwitchRight);

	/** Get the angle between the camera forward and target direction */
	float GetAngleToTarget(AActor* Target) const;

protected:
	/** Maximum distance for lock-on targeting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Settings")
	float MaxLockOnDistance = 1500.0f;

	/** Maximum angle from camera forward for initial lock-on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Settings")
	float MaxLockOnAngle = 60.0f;

	/** Distance at which lock-on is automatically broken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Settings")
	float LockOnBreakDistance = 2000.0f;

	/** Speed at which camera rotates to face target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Settings")
	float CameraRotationSpeed = 10.0f;

	/** Height offset for lock-on focus point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Settings")
	float TargetHeightOffset = 100.0f;

	/** Tag that identifies targetable actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Settings")
	FName TargetableTag = "Enemy";

private:
	/** Currently locked target */
	TWeakObjectPtr<AActor> CurrentTarget;

	/** Whether lock-on is currently active */
	bool bIsLockedOn = false;

	/** Owner character reference */
	TWeakObjectPtr<ABaseCharacter> OwnerCharacter;
};
