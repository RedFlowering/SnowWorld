// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaModSystemModule.cpp
 * @brief HarmoniaModSystem 모듈 구현
 */

#include "HarmoniaModSystemModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FHarmoniaModSystem, HarmoniaModSystem);

//=============================================================================
// Module Lifecycle
//=============================================================================

void FHarmoniaModSystem::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] ModSystem module started"));
}

void FHarmoniaModSystem::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] ModSystem module shutdown"));
}
