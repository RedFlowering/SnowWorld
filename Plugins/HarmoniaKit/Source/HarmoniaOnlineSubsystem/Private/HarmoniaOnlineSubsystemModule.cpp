// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaOnlineSubsystemModule.cpp
 * @brief HarmoniaOnlineSubsystem 모듈 구현
 */

#include "HarmoniaOnlineSubsystemModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FHarmoniaOnlineSubsystem, HarmoniaOnlineSubsystem);

//=============================================================================
// Module Lifecycle
//=============================================================================

void FHarmoniaOnlineSubsystem::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] OnlineSubsystem module started"));
}

void FHarmoniaOnlineSubsystem::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] OnlineSubsystem module shutdown"));
}
