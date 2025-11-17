// Copyright 2025 Snow Game Studio.

#include "HarmoniaEditor.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FHarmoniaEditorModule"

void FHarmoniaEditorModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("HarmoniaEditor module started"));
    RegisterMenuExtensions();
}

void FHarmoniaEditorModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("HarmoniaEditor module shutdown"));
    UnregisterMenuExtensions();
}

void FHarmoniaEditorModule::RegisterMenuExtensions()
{
    // Register editor menu extensions here if needed
}

void FHarmoniaEditorModule::UnregisterMenuExtensions()
{
    // Unregister menu extensions
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHarmoniaEditorModule, HarmoniaEditor)
