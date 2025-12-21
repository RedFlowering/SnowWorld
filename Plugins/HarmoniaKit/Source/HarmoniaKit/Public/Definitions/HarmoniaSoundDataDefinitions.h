// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundCue.h"
#include "HarmoniaSoundDataDefinitions.generated.h"

/**
 * Sound Data - DataTable row for sound assets
 * Simplified tag-to-asset mapping. All playback settings should be configured in SoundCue.
 *
 * Usage:
 * - Create DataTable with this struct
 * - Set SoundTag as row name
 * - Configure SoundCue with desired playback settings
 * - Use tag to query and play sounds via HarmoniaSoundCacheSubsystem
 *
 * Example Tags:
 * - Sound.SFX.Hit.Metal
 * - Sound.SFX.Footstep.Stone
 * - Sound.Ambient.Forest.Bed.Day
 * - Sound.Ambient.Forest.Life
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSoundData : public FTableRowBase
{
	GENERATED_BODY()

	// ============================================================================
	// Identification
	// ============================================================================

	/** Sound identifier tag (also used as row name). Only shows Sound.* tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Categories = "Sound"))
	FGameplayTag SoundTag;

	/** Display name for editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FText DisplayName;

	// ============================================================================
	// Sound Asset
	// ============================================================================

	/** Sound cue to play (configure volume, pitch, attenuation, looping in the cue) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundCue> SoundCue;

	// ============================================================================
	// Priority
	// ============================================================================

	/** Priority for concurrent sound management. Higher = more important. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0", ClampMax = "100"))
	int32 Priority = 5;

	/** Whether this sound should loop. When true with Random nodes, each loop picks a new random sound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool bShouldLoop = false;

	// ============================================================================
	// Fade Settings
	// ============================================================================

	/** Fade in duration in seconds (0 = no fade) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0.0"))
	float FadeInDuration = 0.0f;

	/** Fade out duration in seconds (0 = no fade) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0.0"))
	float FadeOutDuration = 0.0f;
};

/**
 * Sound Category - For organizing sounds in editor
 */
UENUM(BlueprintType)
enum class EHarmoniaSoundCategory : uint8
{
	SFX UMETA(DisplayName = "Sound Effects"),
	Music UMETA(DisplayName = "Music"),
	Voice UMETA(DisplayName = "Voice/Dialogue"),
	Ambience UMETA(DisplayName = "Ambience"),
	UI UMETA(DisplayName = "UI Sounds"),
	Footsteps UMETA(DisplayName = "Footsteps"),
	Weapons UMETA(DisplayName = "Weapon Sounds"),
	Impact UMETA(DisplayName = "Impact Sounds"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Sound Playback Context - Optional runtime overrides
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSoundPlaybackContext
{
	GENERATED_BODY()

	/** Volume multiplier (applied on top of SoundCue settings) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** Pitch multiplier (applied on top of SoundCue settings) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PitchMultiplier = 1.0f;

	/** Fade in duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float FadeInDuration = 0.0f;

	/** Fade out duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float FadeOutDuration = 0.0f;
};
