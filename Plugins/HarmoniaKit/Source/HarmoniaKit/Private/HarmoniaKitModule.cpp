// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaKitModule.cpp
 * @brief HarmoniaKit 플러그인 코어 모듈 구현
 */

#include "HarmoniaKitModule.h"
#include "HarmoniaKit.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FHarmoniaKit, HarmoniaKit);

//=============================================================================
// Module Lifecycle
//=============================================================================

void FHarmoniaKit::StartupModule()
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("[Harmonia] HarmoniaKit module started"));
}

void FHarmoniaKit::ShutdownModule()
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("[Harmonia] HarmoniaKit module shutdown"));
}
