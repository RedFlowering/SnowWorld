// Copyright 2025 Snow Game Studio.

#include "GameModes/HarmoniaGameMode.h"
#include "Player/HarmoniaPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameMode)

AHarmoniaGameMode::AHarmoniaGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Use HarmoniaPlayerState which includes HarmoniaAttributeSet
	// This enables proper IncomingDamage processing and extended combat attributes
	PlayerStateClass = AHarmoniaPlayerState::StaticClass();
}
