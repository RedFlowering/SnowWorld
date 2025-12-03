// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLoadManagerEditorModule.h
 * @brief Harmonia Load Manager Editor module header
 * 
 * Editor-only module that provides automatic macro regeneration
 * when Harmonia Registry assets are saved in the editor.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * @class FHarmoniaLoadManagerEditor
 * @brief Editor module for Harmonia Load Manager
 * 
 * This module hooks into the editor's save system to automatically
 * regenerate macros whenever a HarmoniaRegistryAsset is saved.
 * This ensures that macro definitions stay in sync with registry changes.
 */
class FHarmoniaLoadManagerEditor : public FDefaultModuleImpl
{
public:
	/**
	 * Called when the module is loaded into memory.
	 * Registers the pre-save callback for Registry assets.
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is unloaded from memory.
	 * Unregisters all callbacks to prevent memory issues.
	 */
	virtual void ShutdownModule() override;

	/**
	 * Callback handler for object pre-save events.
	 * Checks if the object is a HarmoniaRegistryAsset and regenerates macros if needed.
	 * @param Object - The object being saved
	 * @param Context - Save context containing cooking and other flags
	 */
	void HandleObjectPreSave(UObject* Object, FObjectPreSaveContext Context);
};
