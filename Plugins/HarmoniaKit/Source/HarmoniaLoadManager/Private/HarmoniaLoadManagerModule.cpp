// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLoadManagerModule.cpp
 * @brief HarmoniaLoadManager 모듈 구현
 */

#include "HarmoniaLoadManagerModule.h"
#include "Macro/HarmoniaMacroGenerator.h"

#define LOCTEXT_NAMESPACE "FHarmoniaLoadManagerModule"

//=============================================================================
// Module Lifecycle
//=============================================================================

void FHarmoniaLoadManagerModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] LoadManager module started"));
}

void FHarmoniaLoadManagerModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] LoadManager module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHarmoniaLoadManagerModule, HarmoniaLoadManager)