// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaGameServiceModule.h
 * @brief Main module interface for HarmoniaGameService
 * 
 * This module provides platform-agnostic game service integration,
 * abstracting away platform-specific APIs for achievements, cloud saves,
 * leaderboards, statistics, and cross-play functionality.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * @class FHarmoniaGameService
 * @brief Module class for HarmoniaGameService
 * 
 * Handles module lifecycle and initialization of platform service connections.
 * The actual game service functionality is provided by UHarmoniaGameService subsystem.
 * 
 * @see UHarmoniaGameService
 */
class FHarmoniaGameService : public FDefaultModuleImpl
{
public:
	/**
	 * Called when the module is loaded into memory.
	 * Initializes platform service connections and logging.
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is unloaded from memory.
	 * Cleans up platform service connections.
	 */
	virtual void ShutdownModule() override;
};
