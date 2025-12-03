// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaEditor.h
 * @brief Main Harmonia Editor module header
 * 
 * This module provides editor-only tools and utilities for the Harmonia system,
 * including world generation visualization, map capture, and terrain editing tools.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * @class FHarmoniaEditorModule
 * @brief Main editor module for Harmonia tools
 * 
 * Provides the core editor functionality for Harmonia, including:
 * - Editor menu extensions
 * - World generator editor subsystem
 * - Map capture utilities
 * - Terrain visualization tools
 */
class FHarmoniaEditorModule : public IModuleInterface
{
public:
	/**
	 * Called when the module starts up.
	 * Initializes editor tools and registers menu extensions.
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module shuts down.
	 * Cleans up resources and unregisters menu extensions.
	 */
	virtual void ShutdownModule() override;

private:
	/**
	 * Registers Harmonia menu items in the editor toolbar.
	 * Adds entries for world generation, map capture, and other tools.
	 */
	void RegisterMenuExtensions();

	/**
	 * Unregisters all menu extensions added by this module.
	 */
	void UnregisterMenuExtensions();
};
