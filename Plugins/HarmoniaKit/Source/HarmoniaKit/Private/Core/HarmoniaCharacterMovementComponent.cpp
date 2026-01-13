// Copyright 2025 Snow Game Studio.

#include "Core/HarmoniaCharacterMovementComponent.h"
#include "Core/HarmoniaCharacter.h"
#include "HarmoniaLogCategories.h"
#include "GameFramework/PhysicsVolume.h"
#include "Net/UnrealNetwork.h"
#include "AlsCharacterMovementComponent.h"

UHarmoniaCharacterMovementComponent::UHarmoniaCharacterMovementComponent()
	: Super()
{

}

void UHarmoniaCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AHarmoniaCharacter>(CharacterOwner);
}

void UHarmoniaCharacterMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UHarmoniaCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UHarmoniaCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}
}

void UHarmoniaCharacterMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode /*= 0*/)
{
	Super::SetMovementMode(NewMovementMode, NewCustomMode);

	HarmoniaMovementMode = static_cast<EHarmoniaMovementMode>(NewMovementMode);
	HarmoniaCustomMovementMode = static_cast<EHarmoniaCustomMovementMode>(NewCustomMode);
}

EHarmoniaMovementMode UHarmoniaCharacterMovementComponent::GetMovementMode()
{
	return HarmoniaMovementMode;
}

EHarmoniaCustomMovementMode UHarmoniaCharacterMovementComponent::GetCustomMovementMode()
{
	return HarmoniaCustomMovementMode;
}

void UHarmoniaCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UHarmoniaCharacterMovementComponent::MoveSmooth(const FVector& InVelocity, float DeltaTime, FStepDownResult* StepDownResult /*= nullptr*/)
{
	Super::MoveSmooth(InVelocity, DeltaTime, StepDownResult);
}

void UHarmoniaCharacterMovementComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	// Ignore the parent implementation of this function and provide our own, because the parent
	// implementation has no effect when we ignore rotation changes in AAlsCharacter::FaceRotation().

	const auto& BasedMovement{ CharacterOwner->GetBasedMovement() };

	FVector MovementBaseLocation;
	FQuat MovementBaseRotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName,
		MovementBaseLocation, MovementBaseRotation);

	if (!OldBaseQuat.Equals(MovementBaseRotation, UE_SMALL_NUMBER))
	{
		const auto DeltaRotation{ (MovementBaseRotation * OldBaseQuat.Inverse()).Rotator() };
		auto NewControlRotation{ CharacterOwner->Controller->GetControlRotation() };

		NewControlRotation.Pitch += DeltaRotation.Pitch;
		NewControlRotation.Yaw += DeltaRotation.Yaw;
		NewControlRotation.Normalize();

		CharacterOwner->Controller->SetControlRotation(NewControlRotation);
	}
}

void UHarmoniaCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);
}

void UHarmoniaCharacterMovementComponent::PhysNavWalking(const float DeltaTime, int32 IterationsCount)
{
	Super::PhysNavWalking(DeltaTime, IterationsCount);
}

void UHarmoniaCharacterMovementComponent::PhysCustom(const float DeltaTime, int32 IterationsCount)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		Super::PhysCustom(DeltaTime, IterationsCount);
		return;
	}

	switch (static_cast<EHarmoniaCustomMovementMode>(CustomMovementMode))
	{
	case EHarmoniaCustomMovementMode::MOVE_Leaping:
		PhysLeaping(DeltaTime);
		break;
	case EHarmoniaCustomMovementMode::MOVE_Knockback:
		PhysKnockback(DeltaTime);
		break;
	case EHarmoniaCustomMovementMode::MOVE_None:
	default:
		Super::PhysCustom(DeltaTime, IterationsCount);
		break;
	}
}

void UHarmoniaCharacterMovementComponent::StartLeaping(FVector TargetLocation, float Angle, float Duration)
{
	if (!CharacterOwner || Duration <= 0.0f)
	{
		return;
	}

	LeapStartLocation = CharacterOwner->GetActorLocation();
	LeapTargetLocation = TargetLocation;
	LeapDuration = Duration;
	LeapElapsedTime = 0.0f;

	// Calculate arc height based on angle and distance
	float Distance = FVector::Dist2D(LeapStartLocation, LeapTargetLocation);
	float RadAngle = FMath::DegreesToRadians(Angle);
	LeapArcHeight = (Distance * 0.5f) * FMath::Tan(RadAngle);
	LeapArcHeight = FMath::Max(LeapArcHeight, 100.0f); // Minimum arc height

	SetMovementMode(MOVE_Custom, static_cast<uint8>(EHarmoniaCustomMovementMode::MOVE_Leaping));

	UE_LOG(LogHarmoniaCombat, Log, TEXT("StartLeaping: Target=%s, Duration=%.2f, ArcHeight=%.2f"), 
		*LeapTargetLocation.ToString(), LeapDuration, LeapArcHeight);
}

void UHarmoniaCharacterMovementComponent::StopLeaping()
{
	if (IsLeaping())
	{
		LeapElapsedTime = 0.0f;
		LeapDuration = 0.0f;
		LeapArcHeight = 0.0f;
		
		UE_LOG(LogHarmoniaCombat, Log, TEXT("StopLeaping"));
	}
}

bool UHarmoniaCharacterMovementComponent::IsLeaping() const
{
	return MovementMode == MOVE_Custom && 
		   CustomMovementMode == static_cast<uint8>(EHarmoniaCustomMovementMode::MOVE_Leaping);
}

void UHarmoniaCharacterMovementComponent::PhysLeaping(float DeltaTime)
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		StopLeaping();
		SetMovementMode(MOVE_Falling);
		return;
	}

	LeapElapsedTime += DeltaTime;

	// Timeout check - use leap duration
	if (LeapElapsedTime >= LeapDuration)
	{
		StopLeaping();
		SetMovementMode(MOVE_Falling);
		return;
	}

	float Alpha = FMath::Clamp(LeapElapsedTime / LeapDuration, 0.0f, 1.0f);

	// Calculate desired position on parabolic arc
	FVector DesiredLocation = FMath::Lerp(LeapStartLocation, LeapTargetLocation, Alpha);
	DesiredLocation.Z += LeapArcHeight * FMath::Sin(Alpha * PI);

	// Calculate movement delta
	FVector Delta = DesiredLocation - UpdatedComponent->GetComponentLocation();

	// SafeMove with sliding (obstacle deflection)
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	// If hit, slide along surface (continue trying, don't stop)
	if (Hit.bBlockingHit)
	{
		SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}

	// Update velocity for animation
	Velocity = Delta / DeltaTime;
}

// ============================================================================
// Knockback Movement
// ============================================================================

void UHarmoniaCharacterMovementComponent::StartKnockback(FVector Direction, float Speed, float Duration)
{
	if (!CharacterOwner || Duration <= 0.0f || Speed <= 0.0f)
	{
		return;
	}

	KnockbackDirection = Direction.GetSafeNormal();
	KnockbackSpeed = Speed;
	KnockbackDuration = Duration;
	KnockbackElapsedTime = 0.0f;

	SetMovementMode(MOVE_Custom, static_cast<uint8>(EHarmoniaCustomMovementMode::MOVE_Knockback));

	UE_LOG(LogHarmoniaCombat, Log, TEXT("StartKnockback: Direction=%s, Speed=%.2f, Duration=%.2f"), 
		*KnockbackDirection.ToString(), KnockbackSpeed, KnockbackDuration);
}

void UHarmoniaCharacterMovementComponent::StopKnockback()
{
	if (IsKnockback())
	{
		KnockbackElapsedTime = 0.0f;
		KnockbackDuration = 0.0f;
		KnockbackSpeed = 0.0f;
		KnockbackDirection = FVector::ZeroVector;

		// Transition back to walking
		SetMovementMode(MOVE_Walking);
		
		UE_LOG(LogHarmoniaCombat, Log, TEXT("StopKnockback"));
	}
}

bool UHarmoniaCharacterMovementComponent::IsKnockback() const
{
	return MovementMode == MOVE_Custom && 
		   CustomMovementMode == static_cast<uint8>(EHarmoniaCustomMovementMode::MOVE_Knockback);
}

void UHarmoniaCharacterMovementComponent::PhysKnockback(float DeltaTime)
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		StopKnockback();
		return;
	}

	KnockbackElapsedTime += DeltaTime;

	// Duration check
	if (KnockbackElapsedTime >= KnockbackDuration)
	{
		StopKnockback();
		return;
	}

	// Calculate knockback velocity with deceleration (easing out)
	float Alpha = KnockbackElapsedTime / KnockbackDuration;
	float SpeedMultiplier = 1.0f - (Alpha * Alpha); // Quadratic ease-out
	FVector KnockbackVelocity = KnockbackDirection * KnockbackSpeed * SpeedMultiplier;

	// Keep knockback horizontal only (no gravity during knockback)
	KnockbackVelocity.Z = 0.0f;

	// Use SafeMove for collision detection
	FVector Delta = KnockbackVelocity * DeltaTime;
	
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	// Handle collision - slide along surface
	if (Hit.bBlockingHit)
	{
		SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}

	// Update velocity for animation system
	Velocity = KnockbackVelocity;
}

