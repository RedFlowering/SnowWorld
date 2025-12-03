// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLoadManagerEditorModule.cpp
 * @brief Harmonia Load Manager Editor module implementation
 * 
 * This module provides editor-time functionality for the Harmonia Load Manager,
 * including automatic macro regeneration when Registry assets are saved.
 */

#include "HarmoniaLoadManagerEditorModule.h"
#include "Modules/ModuleManager.h"
#include "HarmoniaRegistryAsset.h"
#include "Macro/HarmoniaMacroGenerator.h"
#include "UObject/ObjectSaveContext.h"

IMPLEMENT_MODULE(FHarmoniaLoadManagerEditor, HarmoniaLoadManagerEditor);

void FHarmoniaLoadManagerEditor::StartupModule()
{
	// Register callback to handle object pre-save events
	// This allows us to regenerate macros when Registry assets are modified
	FCoreUObjectDelegates::OnObjectPreSave.AddRaw(this, &FHarmoniaLoadManagerEditor::HandleObjectPreSave);
}

void FHarmoniaLoadManagerEditor::ShutdownModule()
{
	// Unregister all callbacks to prevent dangling references
	FCoreUObjectDelegates::OnObjectPreSave.RemoveAll(this);
}

void FHarmoniaLoadManagerEditor::HandleObjectPreSave(UObject* Object, FObjectPreSaveContext Context)
{
	if (Object != nullptr)
	{
		// Check if the saved object is a Harmonia Registry asset
		// If so, validate and regenerate the associated macros
		if (const UHarmoniaRegistryAsset* Registry = Cast<UHarmoniaRegistryAsset>(Object))
		{
			FHarmoniaMacroGenerator::ValidateAndGenerate(Registry);

			UE_LOG(LogTemp, Log, TEXT("[Harmonia] Macro regenerated from RegistryAsset save! (IsCooking=%d)"),
				Context.IsCooking());
		}
	}
}