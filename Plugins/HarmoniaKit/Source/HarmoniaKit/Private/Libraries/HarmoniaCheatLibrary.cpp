// Copyright Snow Game Studio. All Rights Reserved.

#include "Libraries/HarmoniaCheatLibrary.h"
#include "System/HarmoniaCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UHarmoniaCheatManager* UHarmoniaCheatLibrary::GetHarmoniaCheatManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	return Cast<UHarmoniaCheatManager>(PC->CheatManager);
}

bool UHarmoniaCheatLibrary::IsCheatManagerAvailable(const UObject* WorldContextObject)
{
	return GetHarmoniaCheatManager(WorldContextObject) != nullptr;
}

bool UHarmoniaCheatLibrary::IsCheatsEnabled(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}

	// CheatManager exists means cheats are enabled
	return PC->CheatManager != nullptr;
}

// ==================== Quick Cheat Functions ====================

void UHarmoniaCheatLibrary::QuickHeal(const UObject* WorldContextObject)
{
	if (UHarmoniaCheatManager* CheatManager = GetHarmoniaCheatManager(WorldContextObject))
	{
		CheatManager->HarmoniaHealFull();
	}
}

void UHarmoniaCheatLibrary::QuickToggleInvincible(const UObject* WorldContextObject)
{
	if (UHarmoniaCheatManager* CheatManager = GetHarmoniaCheatManager(WorldContextObject))
	{
		CheatManager->HarmoniaToggleInvincible();
	}
}

void UHarmoniaCheatLibrary::QuickGiveGold(const UObject* WorldContextObject, int32 Amount)
{
	if (UHarmoniaCheatManager* CheatManager = GetHarmoniaCheatManager(WorldContextObject))
	{
		CheatManager->HarmoniaGiveGold(Amount);
	}
}

void UHarmoniaCheatLibrary::QuickToggleFly(const UObject* WorldContextObject)
{
	if (UHarmoniaCheatManager* CheatManager = GetHarmoniaCheatManager(WorldContextObject))
	{
		CheatManager->HarmoniaToggleFly();
	}
}

void UHarmoniaCheatLibrary::QuickDoubleSpeed(const UObject* WorldContextObject)
{
	if (UHarmoniaCheatManager* CheatManager = GetHarmoniaCheatManager(WorldContextObject))
	{
		CheatManager->HarmoniaSetSpeed(2.0f);
	}
}

void UHarmoniaCheatLibrary::QuickResetCheats(const UObject* WorldContextObject)
{
	if (UHarmoniaCheatManager* CheatManager = GetHarmoniaCheatManager(WorldContextObject))
	{
		CheatManager->HarmoniaResetCheats();
	}
}
