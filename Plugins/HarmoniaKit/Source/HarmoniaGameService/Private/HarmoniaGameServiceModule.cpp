// Copyright 2025 Snow Game Studio.

#include "HarmoniaGameServiceModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FHarmoniaGameService"

IMPLEMENT_MODULE(FHarmoniaGameService, HarmoniaGameService);

void FHarmoniaGameService::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaGameService module has started."));
}

void FHarmoniaGameService::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaGameService module has shut down."));
}

#undef LOCTEXT_NAMESPACE
