// Copyright 2025 Snow Game Studio.

#include "Animation/HarmoniaMonsterAnimationInterface.h"

// Default implementations
// Animation Blueprint classes should override these functions

bool IHarmoniaMonsterAnimationInterface::PlayMonsterAnimation_Implementation(EHarmoniaMonsterAnimationType AnimationType, float PlayRate)
{
	return false;
}

bool IHarmoniaMonsterAnimationInterface::PlayCustomMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	return false;
}

void IHarmoniaMonsterAnimationInterface::StopMonsterAnimation_Implementation(float BlendOutTime)
{
	// Override in implementation
}

EHarmoniaMonsterAnimationType IHarmoniaMonsterAnimationInterface::GetCurrentAnimationType_Implementation() const
{
	return EHarmoniaMonsterAnimationType::Idle;
}

void IHarmoniaMonsterAnimationInterface::UpdateMovementSpeed_Implementation(float Speed)
{
	// Override in implementation
}

void IHarmoniaMonsterAnimationInterface::UpdateMovementDirection_Implementation(float Direction)
{
	// Override in implementation
}

void IHarmoniaMonsterAnimationInterface::UpdateIsInAir_Implementation(bool bInAir)
{
	// Override in implementation
}

void IHarmoniaMonsterAnimationInterface::UpdateIsInCombat_Implementation(bool bInCombat)
{
	// Override in implementation
}

void IHarmoniaMonsterAnimationInterface::UpdateMonsterState_Implementation(EHarmoniaMonsterState State)
{
	// Override in implementation
}

void IHarmoniaMonsterAnimationInterface::UpdateFlightState_Implementation(bool bIsFlying, float FlightSpeed)
{
	// Override in implementation
}

void IHarmoniaMonsterAnimationInterface::TriggerLimbAttack_Implementation(int32 LimbIndex)
{
	// Override in implementation
}

void IHarmoniaMonsterAnimationInterface::TriggerAbilityAnimation_Implementation(FName AbilityName)
{
	// Override in implementation
}
