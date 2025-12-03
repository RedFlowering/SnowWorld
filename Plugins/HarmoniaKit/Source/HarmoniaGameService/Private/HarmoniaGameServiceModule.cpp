// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaGameServiceModule.cpp
 * @brief Module implementation for HarmoniaGameService
 */

#include "HarmoniaGameServiceModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FHarmoniaGameService"

IMPLEMENT_MODULE(FHarmoniaGameService, HarmoniaGameService);

void FHarmoniaGameService::StartupModule()
{
	// Log module startup
	// The actual service initialization happens in UHarmoniaGameService::Initialize()
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] GameService module has started."));
}

void FHarmoniaGameService::ShutdownModule()
{
	// Log module shutdown
	// Service cleanup happens in UHarmoniaGameService::Deinitialize()
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] GameService module has shut down."));
}

#undef LOCTEXT_NAMESPACE
