// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameMode.h"
#include "HarmoniaGameMode.generated.h"

/**
 * AHarmoniaGameMode
 *
 * Extended GameMode that uses HarmoniaPlayerState for combat system.
 * This GameMode should be used (or inherited from) when using the Harmonia combat system,
 * to ensure HarmoniaAttributeSet is properly initialized for all players.
 *
 * Key Features:
 * - Uses HarmoniaPlayerState which includes HarmoniaAttributeSet
 * - Enables proper IncomingDamage processing for networked damage
 * - Supports extended attributes (Stamina, Mana, etc.)
 *
 * Setup:
 * 1. Set this as Default Game Mode in Project Settings
 * 2. Or create a Blueprint that inherits from this class
 * 3. Or set World Settings -> Game Mode Override
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaGameMode : public ALyraGameMode
{
	GENERATED_BODY()

public:
	AHarmoniaGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
