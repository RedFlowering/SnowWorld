// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaWorldGeneratorModule.cpp
 * @brief HarmoniaWorldGenerator 모듈 구현
 */

#include "HarmoniaWorldGeneratorModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FHarmoniaWorldGenerator, HarmoniaWorldGenerator);

//=============================================================================
// Module Lifecycle
//=============================================================================

void FHarmoniaWorldGenerator::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("[Harmonia] WorldGenerator module started"));
}

void FHarmoniaWorldGenerator::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("[Harmonia] WorldGenerator module shutdown"));
}
