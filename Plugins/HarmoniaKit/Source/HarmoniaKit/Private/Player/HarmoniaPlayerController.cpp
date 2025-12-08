// Copyright 2025 Snow Game Studio.

#include "Player/HarmoniaPlayerController.h"
#include "System/HarmoniaCheatManager.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaPlayerController)

AHarmoniaPlayerController::AHarmoniaPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if !UE_BUILD_SHIPPING
	CheatClass = UHarmoniaCheatManager::StaticClass();
#endif
}
