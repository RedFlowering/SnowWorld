// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLocalizationSystemModule.cpp
 * @brief HarmoniaLocalizationSystem 모듈 구현
 */

#include "HarmoniaLocalizationSystemModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FHarmoniaLocalizationSystem, HarmoniaLocalizationSystem);

//=============================================================================
// Module Lifecycle
//=============================================================================

void FHarmoniaLocalizationSystem::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] LocalizationSystem module started"));
}

void FHarmoniaLocalizationSystem::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] LocalizationSystem module shutdown"));
}
