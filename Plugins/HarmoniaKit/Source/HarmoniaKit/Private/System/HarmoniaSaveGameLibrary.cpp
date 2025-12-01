// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaSaveGameLibrary.h"
#include "System/HarmoniaSaveGameSubsystem.h"
#include "System/HarmoniaSaveGame.h"
#include "Core/HarmoniaCoreBFL.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

bool UHarmoniaSaveGameLibrary::SaveGame(const UObject* WorldContextObject, const FString& SaveSlotName, bool bUseSteamCloud)
{
	HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(UHarmoniaSaveGameSubsystem, SaveSystem, WorldContextObject, false);
	return SaveSystem->SaveGame(SaveSlotName, bUseSteamCloud);
}

bool UHarmoniaSaveGameLibrary::LoadGame(const UObject* WorldContextObject, const FString& SaveSlotName, bool bUseSteamCloud)
{
	HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(UHarmoniaSaveGameSubsystem, SaveSystem, WorldContextObject, false);
	return SaveSystem->LoadGame(SaveSlotName, bUseSteamCloud);
}

bool UHarmoniaSaveGameLibrary::DeleteSaveGame(const UObject* WorldContextObject, const FString& SaveSlotName, bool bDeleteFromSteamCloud)
{
	HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(UHarmoniaSaveGameSubsystem, SaveSystem, WorldContextObject, false);
	return SaveSystem->DeleteSaveGame(SaveSlotName, bDeleteFromSteamCloud);
}

bool UHarmoniaSaveGameLibrary::DoesSaveGameExist(const UObject* WorldContextObject, const FString& SaveSlotName)
{
	HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(UHarmoniaSaveGameSubsystem, SaveSystem, WorldContextObject, false);
	return SaveSystem->DoesSaveGameExist(SaveSlotName);
}

UHarmoniaSaveGame* UHarmoniaSaveGameLibrary::GetCurrentSaveGame(const UObject* WorldContextObject)
{
	HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(UHarmoniaSaveGameSubsystem, SaveSystem, WorldContextObject, nullptr);
	return SaveSystem->GetCurrentSaveGame();
}

UHarmoniaSaveGameSubsystem* UHarmoniaSaveGameLibrary::GetSaveGameSubsystem(const UObject* WorldContextObject)
{
	return UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaSaveGameSubsystem>(WorldContextObject);
}

void UHarmoniaSaveGameLibrary::SetAutoSaveEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	UHarmoniaSaveGameSubsystem* SaveSystem = GetSaveGameSubsystem(WorldContextObject);
	if (SaveSystem)
	{
		SaveSystem->SetAutoSaveEnabled(bEnabled);
	}
}

void UHarmoniaSaveGameLibrary::SetAutoSaveInterval(const UObject* WorldContextObject, float IntervalInSeconds)
{
	UHarmoniaSaveGameSubsystem* SaveSystem = GetSaveGameSubsystem(WorldContextObject);
	if (SaveSystem)
	{
		SaveSystem->SetAutoSaveInterval(IntervalInSeconds);
	}
}

FString UHarmoniaSaveGameLibrary::GetPlayerSteamID(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return FString();
	}

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
		if (Identity.IsValid())
		{
			int32 LocalUserNum = PlayerController->GetLocalPlayer() ? PlayerController->GetLocalPlayer()->GetControllerId() : 0;
			FUniqueNetIdPtr UniqueId = Identity->GetUniquePlayerId(LocalUserNum);

			if (UniqueId.IsValid())
			{
				return UniqueId->ToString();
			}
		}
	}

	// 온라인 서브시스템을 사용할 수 없는 경우 PlayerState의 PlayerId 사용
	if (APlayerState* PS = PlayerController->PlayerState)
	{
		return FString::Printf(TEXT("Player_%d"), PS->GetPlayerId());
	}

	return TEXT("Unknown");
}

bool UHarmoniaSaveGameLibrary::IsServerOwner(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return false;
	}

	// 리슨 서버에서는 첫 번째 플레이어가 서버 소유주
	return PlayerController->GetLocalPlayer() && PlayerController->GetLocalPlayer()->GetControllerId() == 0;
}
