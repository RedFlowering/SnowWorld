// Copyright 2025 Snow Game Studio.

#include "HarmoniaStoryModule.h"
#include "HarmoniaStoryLog.h"

#define LOCTEXT_NAMESPACE "FHarmoniaStoryModule"

void FHarmoniaStoryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    UE_LOG(LogHarmoniaStory, Log, TEXT("HarmoniaStory Module Started"));
}

void FHarmoniaStoryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHarmoniaStoryModule, HarmoniaStory)
