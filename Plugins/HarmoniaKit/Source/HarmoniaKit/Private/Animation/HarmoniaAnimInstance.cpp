// Copyright 2025 Snow Game Studio.

#include "Animation/HarmoniaAnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaAnimInstance)

UHarmoniaAnimInstance::UHarmoniaAnimInstance()
{
}

void UHarmoniaAnimInstance::SetDodgeDirection_Implementation(float DirectionX, float DirectionY)
{
	DodgeDirectionX = FMath::Clamp(DirectionX, -1.0f, 1.0f);
	DodgeDirectionY = FMath::Clamp(DirectionY, -1.0f, 1.0f);
	bIsDodging = true;
}

void UHarmoniaAnimInstance::ClearDodgeDirection_Implementation()
{
	DodgeDirectionX = 0.0f;
	DodgeDirectionY = 0.0f;
	bIsDodging = false;
}
