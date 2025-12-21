// Copyright 2025 Snow Game Studio.

#include "Core/HarmoniaCharacterMovementComponent.h"
#include "Core/HarmoniaCharacter.h"
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

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Found Owner Harmonia Character!"));
	}
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

	UE_LOG(LogTemp, Log, TEXT("Movement Mode Changed: NewMode = %d, CustomMode = %d"), MovementMode, CustomMovementMode);
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

	UE_LOG(LogTemp, Log, TEXT("StartLeaping: Target=%s, Duration=%.2f, ArcHeight=%.2f"), 
		*LeapTargetLocation.ToString(), LeapDuration, LeapArcHeight);
}

void UHarmoniaCharacterMovementComponent::StopLeaping()
{
	if (IsLeaping())
	{
		LeapElapsedTime = 0.0f;
		LeapDuration = 0.0f;
		LeapArcHeight = 0.0f;
		
		UE_LOG(LogTemp, Log, TEXT("StopLeaping"));
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
