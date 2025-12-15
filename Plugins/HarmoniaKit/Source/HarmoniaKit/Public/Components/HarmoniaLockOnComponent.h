// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HarmoniaLockOnComponent.generated.h"

class AActor;
class APawn;
class ACharacter;
class USenseStimulusComponent;

/**
 * Lock-On Target Changed Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLockOnTargetChangedDelegate, AActor*, OldTarget, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnStateChangedDelegate, bool, bIsLockedOn);

/**
 * UHarmoniaLockOnComponent
 *
 * Souls-like lock-on targeting component for Harmonia Kit.
 * Allows players to lock onto enemies and maintain camera focus during combat.
 *
 * Features:
 * - Automatic target acquisition based on camera view
 * - Target switching (left/right)
 * - Auto camera rotation towards locked target
 * - Distance-based automatic unlock
 * - Integration with Sense System for target validation
 * - Configurable targeting parameters
 * - Network replication ready
 *
 * Usage:
 * - Add this component to your character
 * - Call ToggleLockOn() from input
 * - Call SwitchTargetLeft/Right() for target switching
 * - Component handles camera rotation automatically
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaLockOnComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Lock-On Control
	// ============================================================================

	/**
	 * Toggle lock-on on/off
	 * Finds nearest valid target if not currently locked on
	 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ToggleLockOn();

	/**
	 * Enable lock-on to the nearest valid target
	 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void EnableLockOn();

	/**
	 * Disable lock-on
	 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void DisableLockOn();

	/**
	 * Switch to the next target to the right
	 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SwitchTargetRight();

	/**
	 * Switch to the next target to the left
	 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SwitchTargetLeft();

	/**
	 * Manually set lock-on target
	 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SetLockOnTarget(AActor* NewTarget);

	// ============================================================================
	// State Queries
	// ============================================================================

	/**
	 * Check if currently locked on to a target
	 */
	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLockedOn() const { return bIsLockedOn && CurrentTarget.IsValid(); }

	/**
	 * Get the current locked target
	 */
	UFUNCTION(BlueprintPure, Category = "LockOn")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	/**
	 * Get distance to current target
	 */
	UFUNCTION(BlueprintPure, Category = "LockOn")
	float GetDistanceToTarget() const;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Maximum distance for lock-on targeting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	float MaxLockOnDistance = 1500.0f;

	/** Maximum angle from camera forward for initial lock-on (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	float MaxLockOnAngle = 60.0f;

	/** Distance at which lock-on is automatically broken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	float LockOnBreakDistance = 2000.0f;

	/** Speed at which camera rotates to face target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	float CameraRotationSpeed = 10.0f;

	/** Height offset for lock-on focus point (relative to target actor location) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	float TargetHeightOffset = 100.0f;

	/** Socket name on target mesh for precise lock-on (if empty, uses actor location) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	FName TargetSocketName = NAME_None;

	/** Gameplay tag that identifies targetable actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	FGameplayTag TargetableTag;

	/** Whether to automatically break lock when target dies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	bool bBreakLockOnTargetDeath = true;

	/** Whether to smooth camera rotation or snap instantly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Settings")
	bool bSmoothCameraRotation = true;

	// ============================================================================
	// Delegates
	// ============================================================================

	/** Called when lock-on target changes */
	UPROPERTY(BlueprintAssignable, Category = "LockOn|Events")
	FOnLockOnTargetChangedDelegate OnLockOnTargetChanged;

	/** Called when lock-on state changes (on/off) */
	UPROPERTY(BlueprintAssignable, Category = "LockOn|Events")
	FOnLockOnStateChangedDelegate OnLockOnStateChanged;

protected:
	virtual void BeginPlay() override;

	// ============================================================================
	// Target Selection
	// ============================================================================

	/**
	 * Find the nearest valid target within range and angle
	 */
	AActor* FindNearestTarget() const;

	/**
	 * Find all valid targets within range
	 */
	TArray<AActor*> FindValidTargets() const;

	/**
	 * Check if an actor is a valid lock-on target
	 */
	bool IsValidTarget(AActor* Target) const;

	/**
	 * Get lock-on point on target (socket location if specified, otherwise actor location + offset)
	 */
	FVector GetTargetLockOnPoint(AActor* Target) const;

	// ============================================================================
	// Camera Control
	// ============================================================================

	/**
	 * Update camera rotation to face locked target
	 */
	void UpdateCameraToTarget(float DeltaTime);

	// ============================================================================
	// Validation
	// ============================================================================

	/**
	 * Check if current target is still valid and within range
	 */
	void ValidateCurrentTarget();

	/**
	 * Switch to a different target based on direction
	 */
	void SwitchTarget(bool bSwitchRight);

	/**
	 * Get the angle between the camera forward and target direction
	 */
	float GetAngleToTarget(AActor* Target) const;

	/**
	 * Get screen position of a target (-1 to 1 range, 0 is center)
	 */
	FVector2D GetTargetScreenPosition(AActor* Target) const;

private:
	/** Currently locked target */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentTarget;

	/** Whether lock-on is currently active */
	UPROPERTY(Transient)
	bool bIsLockedOn = false;

	/** Owner character reference (cached) */
	UPROPERTY(Transient)
	TWeakObjectPtr<ACharacter> OwnerCharacter;
};
