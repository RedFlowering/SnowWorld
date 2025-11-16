// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSaveGameLibrary.h"
#include "LyraSaveGameSubsystem.h"
#include "LyraSaveGame.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

bool ULyraSaveGameLibrary::SaveGame(const UObject* WorldContextObject, const FString& SaveSlotName, bool bUseSteamCloud)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	ULyraSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<ULyraSaveGameSubsystem>();
	if (!SaveSystem)
	{
		return false;
	}

	return SaveSystem->SaveGame(SaveSlotName, bUseSteamCloud);
}

bool ULyraSaveGameLibrary::LoadGame(const UObject* WorldContextObject, const FString& SaveSlotName, bool bUseSteamCloud)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	ULyraSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<ULyraSaveGameSubsystem>();
	if (!SaveSystem)
	{
		return false;
	}

	return SaveSystem->LoadGame(SaveSlotName, bUseSteamCloud);
}

bool ULyraSaveGameLibrary::DeleteSaveGame(const UObject* WorldContextObject, const FString& SaveSlotName, bool bDeleteFromSteamCloud)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	ULyraSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<ULyraSaveGameSubsystem>();
	if (!SaveSystem)
	{
		return false;
	}

	return SaveSystem->DeleteSaveGame(SaveSlotName, bDeleteFromSteamCloud);
}

bool ULyraSaveGameLibrary::DoesSaveGameExist(const UObject* WorldContextObject, const FString& SaveSlotName)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	ULyraSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<ULyraSaveGameSubsystem>();
	if (!SaveSystem)
	{
		return false;
	}

	return SaveSystem->DoesSaveGameExist(SaveSlotName);
}

ULyraSaveGame* ULyraSaveGameLibrary::GetCurrentSaveGame(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	ULyraSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<ULyraSaveGameSubsystem>();
	if (!SaveSystem)
	{
		return nullptr;
	}

	return SaveSystem->GetCurrentSaveGame();
}

ULyraSaveGameSubsystem* ULyraSaveGameLibrary::GetSaveGameSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<ULyraSaveGameSubsystem>();
}

void ULyraSaveGameLibrary::SetAutoSaveEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	ULyraSaveGameSubsystem* SaveSystem = GetSaveGameSubsystem(WorldContextObject);
	if (SaveSystem)
	{
		SaveSystem->SetAutoSaveEnabled(bEnabled);
	}
}

void ULyraSaveGameLibrary::SetAutoSaveInterval(const UObject* WorldContextObject, float IntervalInSeconds)
{
	ULyraSaveGameSubsystem* SaveSystem = GetSaveGameSubsystem(WorldContextObject);
	if (SaveSystem)
	{
		SaveSystem->SetAutoSaveInterval(IntervalInSeconds);
	}
}

FString ULyraSaveGameLibrary::GetPlayerSteamID(APlayerController* PlayerController)
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

bool ULyraSaveGameLibrary::IsServerOwner(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return false;
	}

	// 리슨 서버에서는 첫 번째 플레이어가 서버 소유주
	return PlayerController->GetLocalPlayer() && PlayerController->GetLocalPlayer()->GetControllerId() == 0;
}
