// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaEditor.cpp
 * @brief Main Harmonia Editor module implementation
 */

#include "HarmoniaEditor.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FHarmoniaEditorModule"

void FHarmoniaEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] Editor module started"));
	
	// Initialize editor menu extensions for Harmonia tools
	RegisterMenuExtensions();
}

void FHarmoniaEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] Editor module shutdown"));
	
	// Clean up menu extensions before module unload
	UnregisterMenuExtensions();
}

void FHarmoniaEditorModule::RegisterMenuExtensions()
{
	// TODO: Register editor menu extensions
	// - World Generator tool menu
	// - Map Capture utilities
	// - Terrain editing tools
}

void FHarmoniaEditorModule::UnregisterMenuExtensions()
{
	// TODO: Unregister all menu extensions
	// Clean up any registered commands and menu items
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHarmoniaEditorModule, HarmoniaEditor)
