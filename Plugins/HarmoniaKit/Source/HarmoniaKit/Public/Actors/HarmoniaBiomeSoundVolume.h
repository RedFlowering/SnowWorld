// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "GameplayTagContainer.h"
#include "HarmoniaBiomeSoundVolume.generated.h"

class APawn;

/**
 * Biome Sound Volume
 * Plays ambient sounds when player enters using HarmoniaSoundCacheSubsystem.
 * Priority is configured per-sound in the Sound DataTable.
 *
 * Usage:
 * 1. Place volume in world
 * 2. Add SoundTags matching DT_Sound rows
 * 3. Configure fade settings
 */
UCLASS(HideCategories=(Collision, Cooking, HLOD, Physics, Networking, Input, Actor))
class HARMONIAKIT_API AHarmoniaBiomeSoundVolume : public AVolume
{
	GENERATED_BODY()

public:
	AHarmoniaBiomeSoundVolume();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	// ============================================================================
	// Sound Settings
	// ============================================================================

	/** Sound tags to play (must exist in Sound DataTable). Only shows Sound.* tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sound", meta = (Categories = "Sound"))
	FGameplayTagContainer SoundTags;

	/** Volume multiplier (applied to all sounds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sound", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** Fade out duration when exiting volume (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sound", meta = (ClampMin = "0.0"))
	float FadeOutDuration = 2.0f;

protected:
	/** Start playing all sounds via SoundCacheSubsystem */
	UFUNCTION(BlueprintCallable, Category = "Biome Sound")
	void StartSounds();

	/** Stop all currently playing sounds */
	UFUNCTION(BlueprintCallable, Category = "Biome Sound")
	void StopSounds();

	/** Check if the actor is a player pawn */
	bool IsPlayerPawn(AActor* Actor) const;

	/** Sound tags currently active for this volume */
	TArray<FGameplayTag> ActiveSoundTags;
};
