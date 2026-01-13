// Copyright 2025 Snow Game Studio.

#pragma once 

#include "Character/LyraCharacterMovementComponent.h"
#include "HarmoniaCharacterMovementComponent.generated.h"

class AHarmoniaCharacter;

UENUM()
enum class EHarmoniaMovementMode : uint8
{
	MOVE_None		UMETA(DisplayName = "None"),
	MOVE_Walking	UMETA(DisplayName = "Walking"),
	MOVE_NavWalking	UMETA(DisplayName = "Navmesh Walking"),
	MOVE_Falling	UMETA(DisplayName = "Falling"),
	MOVE_Swimming	UMETA(DisplayName = "Swimming"),
	MOVE_Flying		UMETA(DisplayName = "Flying"),
	MOVE_Custom		UMETA(DisplayName = "Custom"),
	MOVE_MAX		UMETA(Hidden),
};

UENUM()
enum class EHarmoniaCustomMovementMode : uint8
{
	MOVE_None				UMETA(DisplayName = "None"),
	MOVE_Leaping			UMETA(DisplayName = "Leaping"),		// 도약 중 (포물선 이동)
	MOVE_Knockback			UMETA(DisplayName = "Knockback"),	// 넉백 중 (직선 이동)
};

UCLASS(Config = Game)
class HARMONIAKIT_API UHarmoniaCharacterMovementComponent : public ULyraCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UHarmoniaCharacterMovementComponent();

public:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;

	EHarmoniaMovementMode GetMovementMode();

	EHarmoniaCustomMovementMode GetCustomMovementMode();

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void MoveSmooth(const FVector& InVelocity, float DeltaTime, FStepDownResult* StepDownResult = nullptr);

protected:
	virtual void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation) override;

	virtual void PhysicsRotation(float DeltaTime);

	virtual void PhysNavWalking(float DeltaTime, int32 IterationsCount) override;

	virtual void PhysCustom(float DeltaTime, int32 IterationsCount) override;

	// ============================================================================
	// Leaping Movement
	// ============================================================================

public:
	/** Start leaping towards target location */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Movement|Leap")
	void StartLeaping(FVector TargetLocation, float Angle, float Duration);

	/** Stop leaping and transition to falling */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Movement|Leap")
	void StopLeaping();

	/** Check if currently leaping */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Movement|Leap")
	bool IsLeaping() const;

protected:
	/** Physics update for leaping movement */
	virtual void PhysLeaping(float DeltaTime);

	// ============================================================================
	// Knockback Movement
	// ============================================================================

public:
	/** Start knockback in specified direction */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Movement|Knockback")
	void StartKnockback(FVector Direction, float Speed, float Duration);

	/** Stop knockback and transition to walking */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Movement|Knockback")
	void StopKnockback();

	/** Check if currently in knockback */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Movement|Knockback")
	bool IsKnockback() const;

protected:
	/** Physics update for knockback movement */
	virtual void PhysKnockback(float DeltaTime);

protected:
	UPROPERTY()
	TObjectPtr<AHarmoniaCharacter> OwnerCharacter = nullptr;
	EHarmoniaMovementMode HarmoniaMovementMode = EHarmoniaMovementMode::MOVE_Walking;
	EHarmoniaCustomMovementMode HarmoniaCustomMovementMode = EHarmoniaCustomMovementMode::MOVE_None;

	// Leap state
	FVector LeapStartLocation;
	FVector LeapTargetLocation;
	float LeapArcHeight = 0.0f;
	float LeapDuration = 0.0f;
	float LeapElapsedTime = 0.0f;

	// Knockback state
	FVector KnockbackDirection;
	float KnockbackSpeed = 0.0f;
	float KnockbackDuration = 0.0f;
	float KnockbackElapsedTime = 0.0f;
};
