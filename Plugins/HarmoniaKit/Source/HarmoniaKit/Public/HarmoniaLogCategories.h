// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Standardized log categories for HarmoniaKit plugin
 *
 * Usage:
 *   UE_LOG(LogHarmoniaKit, Warning, TEXT("Your message here"));
 *   UE_LOG(LogHarmoniaCombat, Error, TEXT("Combat error: %s"), *ErrorMessage);
 *
 * Log Verbosity Levels:
 *   - Fatal: Critical errors that crash the application
 *   - Error: Errors that should be fixed
 *   - Warning: Warnings about potential issues
 *   - Display: Important information (default)
 *   - Log: General logging information
 *   - Verbose: Detailed logging for debugging
 *   - VeryVerbose: Extremely detailed logging
 */

// Core plugin category
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaKit, Log, All);

// Combat system categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaCombat, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaWeapon, Log, All);

// AI system categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaSquad, Log, All);

// Life Content system categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaLifeContent, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaFishing, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaGathering, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaCooking, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaFarming, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaMusic, Log, All);

// Quest and progression categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaQuest, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaProgression, Log, All);

// Inventory and item categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaInventory, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaItem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaCrafting, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaEnhancement, Log, All);

// Map and navigation categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaMap, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaWaypoint, Log, All);

// Dungeon and world generation categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaDungeon, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaWorldGen, Log, All);

// Monster and spawn categories
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaMonster, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaSpawn, Log, All);

// Save system category
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaSave, Log, All);

// UI category
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaUI, Log, All);

// Animation category
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaAnimation, Log, All);

// Ability system category
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaAbility, Log, All);

// Network/Replication category
DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaNetwork, Log, All);
