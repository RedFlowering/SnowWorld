// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundAttenuation.h"
#include "HarmoniaSoundDataDefinitions.generated.h"

/**
 * Sound Data - DataTable row for sound assets
 * Stores sound cues and waves with gameplay tag-based lookup
 *
 * Usage:
 * - Create DataTable with this struct
 * - Set SoundTag as row name or in the struct
 * - Use tag to query and play sounds
 *
 * Example Tags:
 * - Sound.SFX.Hit.Metal
 * - Sound.SFX.Hit.Flesh
 * - Sound.SFX.Footstep.Stone
 * - Sound.SFX.Weapon.Sword.Swing
 * - Sound.Music.Combat.Intense
 * - Sound.Voice.Player.Grunt
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSoundData : public FTableRowBase
{
	GENERATED_BODY()

	// ============================================================================
	// Identification
	// ============================================================================

	/** Sound identifier tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FGameplayTag SoundTag;

	/** Display name for editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FText DisplayName;

	/** Description of this sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FText Description;

	// ============================================================================
	// Sound Assets
	// ============================================================================

	/** Sound cue (preferred for variations and randomization) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Asset")
	TSoftObjectPtr<USoundCue> SoundCue;

	/** Sound wave (for simple sounds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Asset")
	TSoftObjectPtr<USoundWave> SoundWave;

	/** Sound base (generic, can be Cue or Wave) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Asset")
	TSoftObjectPtr<USoundBase> SoundBase;

	// ============================================================================
	// Playback Settings
	// ============================================================================

	/** Volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Playback", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** Pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Playback", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PitchMultiplier = 1.0f;

	/** Random volume range (applied on top of VolumeMultiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Playback")
	FVector2D VolumeRange = FVector2D(1.0f, 1.0f);

	/** Random pitch range (applied on top of PitchMultiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Playback")
	FVector2D PitchRange = FVector2D(1.0f, 1.0f);

	/** Start time offset (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Playback", meta = (ClampMin = "0.0"))
	float StartTime = 0.0f;

	// ============================================================================
	// Spatial Settings
	// ============================================================================

	/** Whether this is a 2D sound (UI, music) or 3D spatial sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Spatial")
	bool bIs2D = false;

	/** Sound attenuation settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Spatial")
	TSoftObjectPtr<USoundAttenuation> AttenuationSettings;

	/** Override attenuation distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Spatial")
	bool bOverrideAttenuation = false;

	/** Custom attenuation distance (if overriding) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Spatial", meta = (EditCondition = "bOverrideAttenuation", ClampMin = "0.0"))
	float AttenuationDistance = 1000.0f;

	// ============================================================================
	// Concurrency
	// ============================================================================

	/** Maximum concurrent instances of this sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Concurrency", meta = (ClampMin = "1", ClampMax = "20"))
	int32 MaxConcurrentInstances = 1;

	/** What to do when max instances reached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Concurrency")
	bool bStopOldestInstance = true;

	/** Minimum time between plays (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Concurrency", meta = (ClampMin = "0.0"))
	float MinTimeBetweenPlays = 0.0f;

	// ============================================================================
	// Sound Class
	// ============================================================================

	/** Sound class (SFX, Music, Voice, UI, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Class")
	TSoftObjectPtr<USoundClass> SoundClass;

	/** Priority (higher = less likely to be culled) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Class", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Priority = 0.5f;

	// ============================================================================
	// Metadata
	// ============================================================================

	/** Sound duration (for reference, calculated at load) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sound|Info")
	float Duration = 0.0f;

	/** Related gameplay tags (for queries) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Gameplay")
	FGameplayTagContainer GameplayTags;

	/** Whether this sound should auto-destroy after playing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Gameplay")
	bool bAutoDestroy = true;

	/** Subtitle text (if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Gameplay")
	FText SubtitleText;
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
 * Sound Playback Context - Additional context for playing sounds
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSoundPlaybackContext
{
	GENERATED_BODY()

	/** Override volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeOverride = 1.0f;

	/** Whether to use volume override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverrideVolume = false;

	/** Override pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchOverride = 1.0f;

	/** Whether to use pitch override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverridePitch = false;

	/** Fade in duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeInDuration = 0.0f;

	/** Fade out duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOutDuration = 0.0f;

	/** Stop other sounds with the same tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStopOthersWithSameTag = false;

	/** Additional gameplay tags to add during playback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer AdditionalTags;
};
