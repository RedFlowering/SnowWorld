// Copyright 2025 Snow Game Studio.

#include "Player/HarmoniaPlayerState.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaPlayerState)

AHarmoniaPlayerState::AHarmoniaPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("HealthSet"))
						   .DoNotCreateDefaultSubobject(TEXT("CombatSet")))
{
	// Create HarmoniaAttributeSet
	// This will be automatically detected and registered by the AbilitySystemComponent
	HarmoniaSet = CreateDefaultSubobject<UHarmoniaAttributeSet>(TEXT("HarmoniaSet"));
}
