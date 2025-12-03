// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "HarmoniaAnimationDataDefinitions.generated.h"

/**
 * Animation Data - DataTable row for animation sequences and montages
 * Stores animation assets with gameplay tag-based lookup
 *
 * Usage:
 * - Create DataTable with this struct
 * - Set AnimationTag as row name or in the struct
 * - Use tag to query and play animations
 *
 * Example Tags:
 * - Anim.Player.Attack.Sword.Light
 * - Anim.Player.Attack.Sword.Heavy
 * - Anim.Player.Dodge.Forward
 * - Anim.Monster.Goblin.Attack.Melee
 * - Anim.Monster.Goblin.Death
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAnimationData : public FTableRowBase
{
	GENERATED_BODY()

	// ============================================================================
	// Identification
	// ============================================================================

	/** Animation identifier tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FGameplayTag AnimationTag;

	/** Display name for editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FText DisplayName;

	/** Description of this animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FText Description;

	// ============================================================================
	// Animation Assets
	// ============================================================================

	/** Animation sequence (for simple playback) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Asset")
	TSoftObjectPtr<UAnimSequence> AnimSequence;

	/** Animation montage (preferred for gameplay animations) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Asset")
	TSoftObjectPtr<UAnimMontage> AnimMontage;

	/** Blend space (for directional animations) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Asset")
	TSoftObjectPtr<UBlendSpace> BlendSpace;

	// ============================================================================
	// Playback Settings
	// ============================================================================

	/** Play rate multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Playback", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float PlayRate = 1.0f;

	/** Starting position (normalized 0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Playback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StartPosition = 0.0f;

	/** Whether to loop the animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Playback")
	bool bLooping = false;

	// ============================================================================
	// Montage Settings
	// ============================================================================

	/** Montage slot name (DefaultSlot, UpperBody, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Montage")
	FName MontageSlot = FName("DefaultSlot");

	/** Blend in time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Montage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlendInTime = 0.25f;

	/** Blend out time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Montage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlendOutTime = 0.25f;

	/** Blend out trigger time (negative = from end) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Montage")
	float BlendOutTriggerTime = -1.0f;

	/** Whether to auto-blend out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Montage")
	bool bAutoBlendOut = true;

	// ============================================================================
	// Root Motion
	// ============================================================================

	/** Whether to enable root motion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|RootMotion")
	bool bEnableRootMotion = false;

	/** Root motion scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|RootMotion", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float RootMotionScale = 1.0f;

	// ============================================================================
	// Metadata
	// ============================================================================

	/** Animation duration (for reference, calculated at load) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|Info")
	float Duration = 0.0f;

	/** Whether this animation can be interrupted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Gameplay")
	bool bCanBeInterrupted = true;

	/** Priority level (higher = less likely to be interrupted) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Gameplay", meta = (ClampMin = "0", ClampMax = "10"))
	int32 Priority = 5;

	/** Related gameplay tags (for queries) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Gameplay")
	FGameplayTagContainer GameplayTags;
};

/**
 * Animation Category - For organizing animations in editor
 */
UENUM(BlueprintType)
enum class EHarmoniaAnimationCategory : uint8
{
	Attack UMETA(DisplayName = "Attack Animations"),
	Dodge UMETA(DisplayName = "Dodge/Evade Animations"),
	HitReaction UMETA(DisplayName = "Hit Reaction Animations"),
	Death UMETA(DisplayName = "Death Animations"),
	Idle UMETA(DisplayName = "Idle Animations"),
	Locomotion UMETA(DisplayName = "Locomotion Animations"),
	Ability UMETA(DisplayName = "Ability Animations"),
	Emote UMETA(DisplayName = "Emote Animations"),
	Custom UMETA(DisplayName = "Custom Animations")
};

/**
 * Animation Playback Context - Additional context for playing animations
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAnimationPlaybackContext
{
	GENERATED_BODY()

	/** Override play rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRateOverride = 1.0f;

	/** Whether to use play rate override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverridePlayRate = false;

	/** Override blend in time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendInTimeOverride = 0.25f;

	/** Whether to use blend in time override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverrideBlendInTime = false;

	/** Stop all other montages in the same slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStopAllMontages = false;

	/** Additional gameplay tags to add during playback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer AdditionalTags;
};
