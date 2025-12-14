// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerState.h"
#include "HarmoniaPlayerState.generated.h"

class UHarmoniaAttributeSet;

/**
 * AHarmoniaPlayerState
 *
 * Extended player state for Soul-like RPG gameplay.
 * Inherits from ALyraPlayerState to maintain compatibility with Lyra systems (LyraPlayerController, etc).
 * Uses SetDefaultSubobjectClass to replace LyraHealthSet with HarmoniaAttributeSet.
 */
UCLASS(Config = Game)
class HARMONIAKIT_API AHarmoniaPlayerState : public ALyraPlayerState
{
	GENERATED_BODY()

public:
	AHarmoniaPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Get the Harmonia attribute set (dynamically retrieved from ASC) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|PlayerState")
	const UHarmoniaAttributeSet* GetHarmoniaAttributeSet() const;
};

