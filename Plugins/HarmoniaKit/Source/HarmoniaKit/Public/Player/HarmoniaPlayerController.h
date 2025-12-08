// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerController.h"
#include "HarmoniaPlayerController.generated.h"

/**
 * Harmonia Player Controller
 * Extends LyraPlayerController with HarmoniaKit-specific functionality
 * 
 * Key Features:
 * - Uses UHarmoniaCheatManager for cheat commands
 * 
 * Usage:
 * - Set this as the PlayerControllerClass in your GameMode
 * - Or create a Blueprint subclass of this
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaPlayerController : public ALyraPlayerController
{
	GENERATED_BODY()

public:
	AHarmoniaPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
