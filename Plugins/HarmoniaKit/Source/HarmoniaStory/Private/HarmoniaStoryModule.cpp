// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaStoryModule.cpp
 * @brief HarmoniaStory 모듈 구현
 */

#include "HarmoniaStoryModule.h"
#include "HarmoniaStoryLog.h"

#define LOCTEXT_NAMESPACE "FHarmoniaStoryModule"

//=============================================================================
// Module Lifecycle
//=============================================================================

void FHarmoniaStoryModule::StartupModule()
{
    UE_LOG(LogHarmoniaStory, Log, TEXT("[Harmonia] Story module started"));
}

void FHarmoniaStoryModule::ShutdownModule()
{
    UE_LOG(LogHarmoniaStory, Log, TEXT("[Harmonia] Story module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHarmoniaStoryModule, HarmoniaStory)
