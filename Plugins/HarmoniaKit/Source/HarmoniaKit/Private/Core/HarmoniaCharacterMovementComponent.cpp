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
	case EHarmoniaCustomMovementMode::MOVE_None:
	default:
		Super::PhysCustom(DeltaTime, IterationsCount);
		break;
	}
}
