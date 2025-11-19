// Copyright 2025 Snow Game Studio.

#pragma once

#include "GameFramework/Actor.h"
#include "HarmoniaInteractionSystemDefinitions.generated.h"

UENUM(BlueprintType)
enum class EHarmoniaInteractionType : uint8
{
    None UMETA(DisplayName="None"),
    Pickup UMETA(DisplayName="Pickup"),
    Gather UMETA(DisplayName="Gather"),
    Chop UMETA(DisplayName="Chop"),
    Open UMETA(DisplayName="Open"),
    Custom UMETA(DisplayName="Custom")
};

USTRUCT(BlueprintType)
struct FHarmoniaInteractionContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> Interactor = nullptr; // Player, AI, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> Interactable = nullptr; // Door, chest, NPC, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHarmoniaInteractionType InteractionType = EHarmoniaInteractionType::None;
};

USTRUCT(BlueprintType)
struct FHarmoniaInteractionResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;
};

// ============================================================================
// Sense-Based Interaction System Definitions
// ============================================================================

/**
 * Sense Interaction Trigger Type
 * Defines how the interaction is triggered through the sense system
 */
UENUM(BlueprintType)
enum class ESenseInteractionTriggerType : uint8
{
    /** Triggered when entering proximity range */
    OnProximityEnter UMETA(DisplayName="On Proximity Enter"),

    /** Triggered while in proximity range */
    OnProximityStay UMETA(DisplayName="On Proximity Stay"),

    /** Triggered when exiting proximity range */
    OnProximityExit UMETA(DisplayName="On Proximity Exit"),

    /** Triggered when first seen */
    OnSeen UMETA(DisplayName="On Seen"),

    /** Triggered while being seen */
    WhileSeen UMETA(DisplayName="While Seen"),

    /** Triggered when lost from sight */
    OnLostSight UMETA(DisplayName="On Lost Sight"),

    /** Triggered when sound is heard */
    OnHeard UMETA(DisplayName="On Heard"),

    /** Triggered manually with input */
    Manual UMETA(DisplayName="Manual (Input Required)"),

    /** Automatic trigger when sensed */
    Automatic UMETA(DisplayName="Automatic")
};

/**
 * Sense Interaction Mode
 * Defines the interaction behavior mode
 */
UENUM(BlueprintType)
enum class ESenseInteractionMode : uint8
{
    /** Single interaction per trigger */
    Single UMETA(DisplayName="Single"),

    /** Continuous interaction while condition is met */
    Continuous UMETA(DisplayName="Continuous"),

    /** Can be repeated after cooldown */
    Repeatable UMETA(DisplayName="Repeatable"),

    /** One-time interaction only */
    OneTime UMETA(DisplayName="One Time")
};

/**
 * Sense Interaction Configuration
 * Settings for sense-based interactions
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FSenseInteractionConfig
{
    GENERATED_BODY()

    /** Interaction type for this sense interaction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EHarmoniaInteractionType InteractionType = EHarmoniaInteractionType::Custom;

    /** How the interaction is triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ESenseInteractionTriggerType TriggerType = ESenseInteractionTriggerType::Manual;

    /** Interaction behavior mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ESenseInteractionMode InteractionMode = ESenseInteractionMode::Single;

    /** Sensor tag to use for this interaction (e.g., "Proximity", "Vision", "Hearing") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FName SensorTag = NAME_None;

    /** Minimum score required to trigger interaction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float MinimumSenseScore = 0.5f;

    /** Interaction range for proximity-based interactions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "TriggerType == ESenseInteractionTriggerType::OnProximityEnter || TriggerType == ESenseInteractionTriggerType::OnProximityStay"))
    float InteractionRange = 200.0f;

    /** Cooldown time between interactions (for Repeatable mode) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "InteractionMode == ESenseInteractionMode::Repeatable"))
    float CooldownTime = 1.0f;

    /** Whether interaction requires line of sight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresLineOfSight = false;

    /** Whether to show interaction prompt */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bShowInteractionPrompt = true;

    /** Interaction prompt text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FText InteractionPrompt = FText::FromString("Interact");

    /** Whether this interaction is currently enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bEnabled = true;
};

/**
 * Sense Interaction Data
 * Runtime data for sense-based interactions
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FSenseInteractionData
{
    GENERATED_BODY()

    /** The configuration for this interaction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSenseInteractionConfig Config;

    /** Last interaction time (for cooldown) */
    UPROPERTY(BlueprintReadOnly)
    float LastInteractionTime = -1.0f;

    /** Number of times this interaction has been triggered */
    UPROPERTY(BlueprintReadOnly)
    int32 InteractionCount = 0;

    /** Whether this interaction has been completed (for OneTime mode) */
    UPROPERTY(BlueprintReadOnly)
    bool bCompleted = false;

    /** Currently sensed stimulus for this interaction */
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> CurrentSensedActor = nullptr;

    /** Current sense score */
    UPROPERTY(BlueprintReadOnly)
    float CurrentSenseScore = 0.0f;

    /** Whether the target is currently sensed */
    UPROPERTY(BlueprintReadOnly)
    bool bCurrentlySensed = false;

    /** Check if interaction is available */
    bool IsAvailable(float CurrentTime) const
    {
        if (!Config.bEnabled || bCompleted)
            return false;

        if (Config.InteractionMode == ESenseInteractionMode::OneTime && InteractionCount > 0)
            return false;

        if (Config.InteractionMode == ESenseInteractionMode::Repeatable)
        {
            if (LastInteractionTime >= 0.0f && (CurrentTime - LastInteractionTime) < Config.CooldownTime)
                return false;
        }

        return true;
    }

    /** Reset interaction data */
    void Reset()
    {
        LastInteractionTime = -1.0f;
        InteractionCount = 0;
        bCompleted = false;
        CurrentSensedActor = nullptr;
        CurrentSenseScore = 0.0f;
        bCurrentlySensed = false;
    }
};