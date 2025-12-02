// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaSaveGameSubsystem.h"
#include "System/HarmoniaSaveGame.h"
#include "System/HarmoniaCheckpointSubsystem.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"
#include "Character/LyraCharacter.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/Attributes/LyraCombatSet.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Managers/HarmoniaBuildingInstanceManager.h"
#include "EngineUtils.h"
#include "Misc/CRC.h"

// HarmoniaKit Ïª¥Ìè¨?åÌä∏
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Components/HarmoniaBuildingComponent.h"
#include "Managers/HarmoniaBuildingInstanceManager.h"
#include "HarmoniaWorldGeneratorSubsystem.h"
#include "System/HarmoniaTimeWeatherManager.h"

const FString UHarmoniaSaveGameSubsystem::DefaultSaveSlotName = TEXT("DefaultSave");
const int32 UHarmoniaSaveGameSubsystem::SaveGameUserIndex = 0;

UHarmoniaSaveGameSubsystem::UHarmoniaSaveGameSubsystem()
	: CurrentSaveGame(nullptr)
	, bAutoSaveEnabled(true)
	, AutoSaveIntervalSeconds(300.0f)
	, LastSaveTime(0.0f)
{
}

void UHarmoniaSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ?êÎèô ?Ä???Ä?¥Î®∏ ?úÏûë
	if (bAutoSaveEnabled && AutoSaveIntervalSeconds > 0.0f)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				AutoSaveTimerHandle,
				this,
				&UHarmoniaSaveGameSubsystem::OnAutoSaveTimer,
				AutoSaveIntervalSeconds,
				true
			);
		}
	}
}

void UHarmoniaSaveGameSubsystem::Deinitialize()
{
	// ?Ä?¥Î®∏ ?ïÎ¶¨
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	Super::Deinitialize();
}

bool UHarmoniaSaveGameSubsystem::SaveGame(const FString& SaveSlotName, bool bUseSteamCloud)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGame: World is null"));
		OnSaveGameComplete.Broadcast(EHarmoniaSaveGameResult::Failed, SaveSlotName);
		return false;
	}

	// ???∏Ïù¥Î∏?Í≤åÏûÑ Í∞ùÏ≤¥ ?ùÏÑ± ?êÎäî Í∏∞Ï°¥ Í≤??¨Ïö©
	if (!CurrentSaveGame)
	{
		CurrentSaveGame = Cast<UHarmoniaSaveGame>(UGameplayStatics::CreateSaveGameObject(UHarmoniaSaveGame::StaticClass()));
	}

	if (!CurrentSaveGame)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGame: Failed to create SaveGame object"));
		OnSaveGameComplete.Broadcast(EHarmoniaSaveGameResult::Failed, SaveSlotName);
		return false;
	}

	// ?∏Ïù¥Î∏?Í≤åÏûÑ Î©îÌ??∞Ïù¥???§Ï†ï
	CurrentSaveGame->SaveSlotName = SaveSlotName;
	CurrentSaveGame->LastSaveTimestamp = FDateTime::Now();

	// Î™®Îì† ?åÎ†à?¥Ïñ¥ ?∞Ïù¥???Ä??
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->PlayerState)
		{
			SavePlayerData(PC, CurrentSaveGame);

			// Ï≤?Î≤àÏß∏ ?åÎ†à?¥Ïñ¥Î•??úÎ≤Ñ ?åÏú†Ï£ºÎ°ú ?§Ï†ï
			if (IsServerOwner(PC))
			{
				CurrentSaveGame->ServerOwnerSteamID = GetSteamIDForPlayer(PC);
			}
		}
	}

	// ?îÎìú ?∞Ïù¥???Ä??(ÎπåÎî© ??
	SaveWorldData(CurrentSaveGame);

	// Î°úÏª¨???Ä??
	bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, SaveGameUserIndex);

	if (!bSaveSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGame: Failed to save to local slot %s"), *SaveSlotName);
		OnSaveGameComplete.Broadcast(EHarmoniaSaveGameResult::Failed, SaveSlotName);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("SaveGame: Successfully saved to local slot %s"), *SaveSlotName);

	// ?§Ì? ?¥Îùº?∞Îìú???Ä??
	if (bUseSteamCloud)
	{
		// ?∏Ïù¥Î∏??∞Ïù¥?∞Î? Î∞îÏù¥?àÎ¶¨Î°?ÏßÅÎ†¨??
		FBufferArchive SaveData;
		CurrentSaveGame->Serialize(SaveData);
		if (SaveData.Num() > 0)
		{
			TArray<uint8> BinaryData;
			BinaryData.Append(SaveData.GetData(), SaveData.Num());

			// [SECURITY] Calculate checksum before encryption
			uint32 Checksum = CalculateChecksum(BinaryData);

			// [SECURITY] Encrypt save data
			TArray<uint8> EncryptedData;
			EncryptSaveData(BinaryData, EncryptedData);

			// [SECURITY] Prepend checksum to encrypted data
			// Format: [4 bytes checksum][encrypted save data]
			TArray<uint8> FinalData;
			FinalData.SetNum(4 + EncryptedData.Num());
			FMemory::Memcpy(FinalData.GetData(), &Checksum, 4);
			FMemory::Memcpy(FinalData.GetData() + 4, EncryptedData.GetData(), EncryptedData.Num());

			if (SaveToSteamCloud(SaveSlotName, FinalData))
			{
				UE_LOG(LogTemp, Log, TEXT("SaveGame: Successfully saved to Steam Cloud (encrypted, %d bytes)"), FinalData.Num());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SaveGame: Failed to save to Steam Cloud, but local save succeeded"));
			}
		}
	}

	LastSaveTime = World->GetTimeSeconds();
	OnSaveGameComplete.Broadcast(EHarmoniaSaveGameResult::Success, SaveSlotName);
	return true;
}

bool UHarmoniaSaveGameSubsystem::LoadGame(const FString& SaveSlotName, bool bUseSteamCloud)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadGame: World is null"));
		OnLoadGameComplete.Broadcast(EHarmoniaSaveGameResult::Failed, nullptr);
		return false;
	}

	UHarmoniaSaveGame* LoadedSaveGame = nullptr;

	// ?§Ì? ?¥Îùº?∞Îìú?êÏÑú Î®ºÏ? Î°úÎìú ?úÎèÑ
	if (bUseSteamCloud)
	{
		TArray<uint8> CloudData;
		if (LoadFromSteamCloud(SaveSlotName, CloudData))
		{
			// [SECURITY] Verify data has checksum (minimum 4 bytes)
			if (CloudData.Num() < 4)
			{
				UE_LOG(LogTemp, Error, TEXT("[SECURITY] LoadGame: Invalid save data from Steam Cloud (too small)"));
			}
			else
			{
				// [SECURITY] Extract checksum from encrypted data
				// Format: [4 bytes checksum][encrypted save data]
				uint32 StoredChecksum = 0;
				FMemory::Memcpy(&StoredChecksum, CloudData.GetData(), 4);

				// [SECURITY] Extract encrypted data
				TArray<uint8> EncryptedData;
				EncryptedData.SetNum(CloudData.Num() - 4);
				FMemory::Memcpy(EncryptedData.GetData(), CloudData.GetData() + 4, EncryptedData.Num());

				// [SECURITY] Decrypt save data
				TArray<uint8> DecryptedData;
				DecryptSaveData(EncryptedData, DecryptedData);

				// [SECURITY] Verify checksum
				if (!VerifyChecksum(DecryptedData, StoredChecksum))
				{
					UE_LOG(LogTemp, Error, TEXT("[ANTI-CHEAT] LoadGame: Save file integrity check failed! Data may be corrupted or tampered."));
					UE_LOG(LogTemp, Error, TEXT("[SECURITY] Refusing to load potentially compromised save data."));
				}
				else
				{
					// Checksum verified - safe to deserialize
					LoadedSaveGame = Cast<UHarmoniaSaveGame>(UGameplayStatics::CreateSaveGameObject(UHarmoniaSaveGame::StaticClass()));
					if (LoadedSaveGame)
					{
						FMemoryReader MemoryReader(DecryptedData, true);
						LoadedSaveGame->Serialize(MemoryReader);
						UE_LOG(LogTemp, Log, TEXT("LoadGame: Successfully loaded from Steam Cloud (decrypted and verified)"));
					}
				}
			}
		}
	}

	// ?§Ì? ?¥Îùº?∞Îìú?êÏÑú Î°úÎìú ?§Ìå® ??Î°úÏª¨?êÏÑú Î°úÎìú
	if (!LoadedSaveGame)
	{
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveGameUserIndex))
		{
			LoadedSaveGame = Cast<UHarmoniaSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveGameUserIndex));

			if (LoadedSaveGame)
			{
				UE_LOG(LogTemp, Log, TEXT("LoadGame: Successfully loaded from local slot %s"), *SaveSlotName);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("LoadGame: Failed to load from local slot %s"), *SaveSlotName);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadGame: Save file does not exist: %s"), *SaveSlotName);
		}
	}

	if (!LoadedSaveGame)
	{
		OnLoadGameComplete.Broadcast(EHarmoniaSaveGameResult::Failed, nullptr);
		return false;
	}

	// Î≤ÑÏ†Ñ Ï≤¥ÌÅ¨
	if (LoadedSaveGame->SaveVersion != 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadGame: Save version mismatch (expected 2, got %d). Some data may not load correctly."), LoadedSaveGame->SaveVersion);

		// Version 1 saves are still compatible, but will be missing new fields
		if (LoadedSaveGame->SaveVersion < 1)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadGame: Save version too old (version %d). Cannot load."), LoadedSaveGame->SaveVersion);
			OnLoadGameComplete.Broadcast(EHarmoniaSaveGameResult::Failed, nullptr);
			return false;
		}
	}

	CurrentSaveGame = LoadedSaveGame;

	// Î™®Îì† ?åÎ†à?¥Ïñ¥ ?∞Ïù¥??Î°úÎìú
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->PlayerState)
		{
			LoadPlayerData(PC, LoadedSaveGame);
		}
	}

	// ?îÎìú ?∞Ïù¥??Î°úÎìú
	LoadWorldData(LoadedSaveGame);

	OnLoadGameComplete.Broadcast(EHarmoniaSaveGameResult::Success, LoadedSaveGame);
	return true;
}

bool UHarmoniaSaveGameSubsystem::DeleteSaveGame(const FString& SaveSlotName, bool bDeleteFromSteamCloud)
{
	bool bSuccess = UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveGameUserIndex);

	if (bDeleteFromSteamCloud)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineUserCloudPtr UserCloud = OnlineSub->GetUserCloudInterface();
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (UserCloud.IsValid() && Identity.IsValid())
			{
				FUniqueNetIdPtr UniqueId = Identity->GetUniquePlayerId(0);
				if (UniqueId.IsValid())
				{
					// ?§Ì? ?¥Îùº?∞Îìú?êÏÑú ?åÏùº ??†ú
					FString CloudFileName = SaveSlotName + TEXT(".sav");
					UserCloud->DeleteUserFile(*UniqueId, CloudFileName, true, true);
				}
			}
		}
	}

	if (bSuccess)
	{
		if (CurrentSaveGame && CurrentSaveGame->SaveSlotName == SaveSlotName)
		{
			CurrentSaveGame = nullptr;
		}
	}

	return bSuccess;
}

bool UHarmoniaSaveGameSubsystem::DoesSaveGameExist(const FString& SaveSlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveGameUserIndex);
}

void UHarmoniaSaveGameSubsystem::SetAutoSaveEnabled(bool bEnabled)
{
	bAutoSaveEnabled = bEnabled;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bEnabled && AutoSaveIntervalSeconds > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			this,
			&UHarmoniaSaveGameSubsystem::OnAutoSaveTimer,
			AutoSaveIntervalSeconds,
			true
		);
	}
	else
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}
}

void UHarmoniaSaveGameSubsystem::SetAutoSaveInterval(float IntervalInSeconds)
{
	AutoSaveIntervalSeconds = FMath::Max(0.0f, IntervalInSeconds);

	// ?Ä?¥Î®∏ ?¨Ïãú??
	if (bAutoSaveEnabled && AutoSaveIntervalSeconds > 0.0f)
	{
		SetAutoSaveEnabled(false);
		SetAutoSaveEnabled(true);
	}
}

void UHarmoniaSaveGameSubsystem::SavePlayerData(APlayerController* PlayerController, UHarmoniaSaveGame* SaveGameObject)
{
	if (!PlayerController || !SaveGameObject)
	{
		return;
	}

	ALyraPlayerState* LyraPS = PlayerController->GetPlayerState<ALyraPlayerState>();
	if (!LyraPS)
	{
		return;
	}

	FHarmoniaPlayerSaveData PlayerData;

	// ?§Ì? ID ?Ä??
	PlayerData.SteamID = GetSteamIDForPlayer(PlayerController);
	PlayerData.PlayerName = LyraPS->GetPlayerName();

	// ?åÎ†à?¥Ïñ¥ ?ÑÏπò Î∞??åÏ†Ñ ?Ä??
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		PlayerData.PlayerLocation = Pawn->GetActorLocation();
		PlayerData.PlayerRotation = Pawn->GetActorRotation();
	}

	// ?åÎ†à?¥Ïñ¥ ?çÏÑ± ?Ä??
	SavePlayerAttributes(LyraPS, PlayerData.Attributes);

	// ?∏Î≤§?†Î¶¨ ?Ä??- Î®ºÏ? Lyra ?∏Î≤§?†Î¶¨ ?úÎèÑ
	if (ULyraInventoryManagerComponent* InventoryComp = LyraPS->FindComponentByClass<ULyraInventoryManagerComponent>())
	{
		SaveInventory(InventoryComp, PlayerData.InventoryItems);
	}
	// HarmoniaInventoryComponent??Î≥ÑÎèÑ Ï≤òÎ¶¨ ?ÑÏöî ??Ï∂îÍ?

	// ?§ÌÉØ ?úÍ∑∏ ?Ä??
	// Note: StatTags are saved in the SaveGame structure directly
	// PlayerData.StatTags is populated during the save process

	// ÎßàÏ?Îß?Ï≤¥ÌÅ¨?¨Ïù∏???Ä??
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		if (UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>())
		{
			PlayerData.LastCheckpointID = CheckpointSubsystem->GetPlayerLastCheckpoint(PlayerController);
		}
	}

	// ?Ä???úÍ∞Ñ
	PlayerData.LastSaveTime = FDateTime::Now();

	// SaveGame??Ï∂îÍ?
	SaveGameObject->SetPlayerData(PlayerData.SteamID, PlayerData);
}

void UHarmoniaSaveGameSubsystem::LoadPlayerData(APlayerController* PlayerController, const UHarmoniaSaveGame* SaveGameObject)
{
	if (!PlayerController || !SaveGameObject)
	{
		return;
	}

	ALyraPlayerState* LyraPS = PlayerController->GetPlayerState<ALyraPlayerState>();
	if (!LyraPS)
	{
		return;
	}

	// ?§Ì? IDÎ°??åÎ†à?¥Ïñ¥ ?∞Ïù¥??Ï∞æÍ∏∞
	FString SteamID = GetSteamIDForPlayer(PlayerController);
	FHarmoniaPlayerSaveData PlayerData;

	if (!SaveGameObject->GetPlayerData(SteamID, PlayerData))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadPlayerData: No save data found for Steam ID %s"), *SteamID);
		return;
	}

	// ?åÎ†à?¥Ïñ¥ ?ÑÏπò Î∞??åÏ†Ñ Î°úÎìú
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		Pawn->SetActorLocation(PlayerData.PlayerLocation);
		Pawn->SetActorRotation(PlayerData.PlayerRotation);
	}

	// ?åÎ†à?¥Ïñ¥ ?çÏÑ± Î°úÎìú
	LoadPlayerAttributes(LyraPS, PlayerData.Attributes);

	// ?∏Î≤§?†Î¶¨ Î°úÎìú
	if (ULyraInventoryManagerComponent* InventoryComp = LyraPS->FindComponentByClass<ULyraInventoryManagerComponent>())
	{
		LoadInventory(InventoryComp, PlayerData.InventoryItems);
	}

	// ?§ÌÉØ ?úÍ∑∏ Î°úÎìú
	for (const FGameplayTag& Tag : PlayerData.StatTags)
	{
		LyraPS->AddStatTagStack(Tag, 1);
	}

	// ÎßàÏ?Îß?Ï≤¥ÌÅ¨?¨Ïù∏??Î°úÎìú
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && !PlayerData.LastCheckpointID.IsNone())
	{
		if (UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>())
		{
			CheckpointSubsystem->SetPlayerLastCheckpoint(PlayerController, PlayerData.LastCheckpointID);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LoadPlayerData: Successfully loaded data for %s"), *PlayerData.PlayerName);
}

void UHarmoniaSaveGameSubsystem::SaveWorldData(UHarmoniaSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FHarmoniaWorldSaveData& WorldData = SaveGameObject->WorldData;

	// ===== ÎπåÎî© ?∞Ïù¥???Ä??=====
	if (UHarmoniaBuildingInstanceManager* BuildingManager = World->GetSubsystem<UHarmoniaBuildingInstanceManager>())
	{
		// BuildingManager?êÏÑú Î∞∞Ïπò??Í±¥Î¨º ?ïÎ≥¥Î•?Í∞Ä?∏Ï? ?Ä??
		TArray<FBuildingInstanceMetadata> AllBuildings;
		BuildingManager->GetAllBuildingMetadata(AllBuildings);

		WorldData.PlacedBuildings.Empty(AllBuildings.Num());
		for (const FBuildingInstanceMetadata& Metadata : AllBuildings)
		{
			FHarmoniaSavedBuildingInstance SavedBuilding;
			SavedBuilding.PartID = Metadata.PartID;
			SavedBuilding.Location = Metadata.Location;
			SavedBuilding.Rotation = Metadata.Rotation;
			SavedBuilding.Scale = FVector::OneVector; // Default scale

			WorldData.PlacedBuildings.Add(SavedBuilding);
		}

		UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d buildings"), WorldData.PlacedBuildings.Num());
	}

	// ===== ?îÎìú ?ùÏÑ± ?ïÎ≥¥ ?Ä??=====
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		if (UHarmoniaWorldGeneratorSubsystem* WorldGenSubsystem = GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>())
		{
			// ?úÏ¶å ?ïÎ≥¥ ?Ä??
			WorldData.CurrentSeason = static_cast<uint8>(WorldGenSubsystem->GetCurrentSeason());
			WorldData.SeasonProgress = WorldGenSubsystem->GetSeasonProgress();

			// ?†Ïî® ?ïÎ≥¥ ?Ä??
			WorldData.CurrentWeatherType = static_cast<uint8>(WorldGenSubsystem->GetCurrentWeather());

			// ?úÍ∞Ñ ?ïÎ≥¥ ?Ä??
			WorldData.CurrentGameHour = WorldGenSubsystem->GetCurrentGameTime();
			WorldData.CurrentGameDay = 0; // WorldGenSubsystem?êÏÑú Day ?ïÎ≥¥??Î≥ÑÎèÑÎ°??ÜÏùå

			UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved world generator data (Season: %d, Weather: %d, Time: %.2f)"),
				WorldData.CurrentSeason, WorldData.CurrentWeatherType, WorldData.CurrentGameHour);
		}
	}

	// ===== ?úÍ∞Ñ/?†Ïî® ?úÏä§???Ä??(TimeWeatherManager) =====
	if (UHarmoniaTimeWeatherManager* TimeWeatherManager = World->GetSubsystem<UHarmoniaTimeWeatherManager>())
	{
		// ?ÑÏû¨ ?†Ïî® ?ïÎ≥¥
		WorldData.CurrentWeatherType = static_cast<uint8>(TimeWeatherManager->GetCurrentWeather());
		WorldData.WeatherIntensity = TimeWeatherManager->GetCurrentWeatherIntensity();

		// ?ÑÏû¨ ?úÍ∞Ñ ?ïÎ≥¥ (??Î∂?
		WorldData.CurrentGameHour = static_cast<float>(TimeWeatherManager->GetCurrentHour()) +
									(TimeWeatherManager->GetCurrentMinute() / 60.0f);
		WorldData.CurrentTimeOfDay = static_cast<uint8>(TimeWeatherManager->GetCurrentTimeOfDay());

		UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved TimeWeatherManager data (Weather: %d, Intensity: %.2f, Hour: %.2f)"),
			WorldData.CurrentWeatherType, WorldData.WeatherIntensity, WorldData.CurrentGameHour);
	}

	// ===== Î¶¨ÏÜå???∏Îìú ?ÅÌÉú ?Ä??=====
	// NOTE: HarmoniaResourceManagerÍ∞Ä ?ÑÏßÅ Íµ¨ÌòÑ?òÏ? ?äÏïò?µÎãà??
	// TODO: HarmoniaResourceManager Íµ¨ÌòÑ ???ÑÎûò ÏΩîÎìúÎ•??úÏÑ±?îÌïò?∏Ïöî.
	// if (UHarmoniaResourceManager* ResourceManager = World->GetSubsystem<UHarmoniaResourceManager>())
	// {
	//     ResourceManager->SaveResourceNodeStates(WorldData.ResourceNodeStates);
	//     UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d resource nodes"), WorldData.ResourceNodeStates.Num());
	// }

	// ===== POI ÏßÑÌñâ ?ÅÌÉú ?Ä??=====
	// NOTE: HarmoniaPOIManagerÍ∞Ä ?ÑÏßÅ Íµ¨ÌòÑ?òÏ? ?äÏïò?µÎãà??
	// TODO: HarmoniaPOIManager Íµ¨ÌòÑ ???ÑÎûò ÏΩîÎìúÎ•??úÏÑ±?îÌïò?∏Ïöî.
	// if (UHarmoniaPOIManager* POIManager = World->GetSubsystem<UHarmoniaPOIManager>())
	// {
	//     POIManager->SavePOIStates(WorldData.POIStates);
	//     UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d POI states"), WorldData.POIStates.Num());
	// }

	// ===== ?òÎèô ?ùÏÑ± ?îÎìú ?∞Ïù¥???Ä??=====
	// NOTE: Manual world generation Í∏∞Îä•???ÑÏßÅ Íµ¨ÌòÑ?òÏ? ?äÏïò?µÎãà??
	// TODO: ?òÎèô ?ùÏÑ± ?îÎìú ÏßÄ??Íµ¨ÌòÑ ???ÑÎûò ÏΩîÎìúÎ•??úÏÑ±?îÌïò?∏Ïöî.
	// if (!WorldData.bIsAutomaticallyGenerated)
	// {
	//     // Save all manually placed world objects (trees, rocks, etc.)
	//     if (GameInstance && WorldGenSubsystem)
	//     {
	//         WorldGenSubsystem->SaveManualWorldObjects(WorldData.ManualObjectLocations);
	//         UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d manual world objects"), WorldData.ManualObjectLocations.Num());
	//     }
	// }

	// ===== Ï≤¥ÌÅ¨?¨Ïù∏???∞Ïù¥???Ä??=====
	if (GameInstance)
	{
		if (UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>())
		{
			WorldData.CheckpointStates = CheckpointSubsystem->GetCheckpointDataForSave();
			UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d checkpoint states"), WorldData.CheckpointStates.Num());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SaveWorldData: World data saved (Seed: %d, Season: %d, Weather: %d, Hour: %.1f, Day: %d)"),
		WorldData.WorldSeed,
		WorldData.CurrentSeason,
		WorldData.CurrentWeatherType,
		WorldData.CurrentGameHour,
		WorldData.CurrentGameDay);
}

void UHarmoniaSaveGameSubsystem::LoadWorldData(const UHarmoniaSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FHarmoniaWorldSaveData& WorldData = SaveGameObject->WorldData;

	// ===== ÎπåÎî© ?∞Ïù¥??Î°úÎìú =====
	if (UHarmoniaBuildingInstanceManager* BuildingManager = World->GetSubsystem<UHarmoniaBuildingInstanceManager>())
	{
		// BuildingDataTable Ï∞æÍ∏∞
		UDataTable* BuildingDataTable = nullptr;

		// ?ÑÎ°ú?ùÌä∏ ?§Ï†ï?¥ÎÇò Í≥†Ï†ï Í≤ΩÎ°ú?êÏÑú ?∞Ïù¥???åÏù¥Î∏?Î°úÎìú
		// NOTE: ?§Ï†ú Í≤ΩÎ°ú???ÑÎ°ú?ùÌä∏ Íµ¨Ï°∞??ÎßûÍ≤å ?òÏ†ï ?ÑÏöî
		FSoftObjectPath DataTablePath(TEXT("/Game/Data/DT_BuildingParts.DT_BuildingParts"));
		BuildingDataTable = Cast<UDataTable>(DataTablePath.TryLoad());

		if (!BuildingDataTable)
		{
			// ?§Î•∏ Í∞Ä?•Ìïú Í≤ΩÎ°ú ?úÎèÑ
			DataTablePath = FSoftObjectPath(TEXT("/HarmoniaKit/Data/DT_BuildingParts.DT_BuildingParts"));
			BuildingDataTable = Cast<UDataTable>(DataTablePath.TryLoad());
		}

		if (BuildingDataTable)
		{
			int32 RestoredCount = 0;
			for (const FHarmoniaSavedBuildingInstance& Building : WorldData.PlacedBuildings)
			{
				// PartIDÎ°?FHarmoniaBuildingPartData Ï°∞Ìöå
				FHarmoniaBuildingPartData* PartData = BuildingDataTable->FindRow<FHarmoniaBuildingPartData>(Building.PartID, TEXT("LoadWorldData"));
				if (PartData)
				{
					FGuid RestoredGuid = BuildingManager->PlaceBuilding(
						*PartData,
						Building.Location,
						Building.Rotation,
						nullptr // Owner??Î≥ÑÎèÑÎ°?Î≥µÏõê ?ÑÏöî
					);

					if (RestoredGuid.IsValid())
					{
						RestoredCount++;
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("LoadWorldData: Failed to find building part data for PartID: %s"), *Building.PartID.ToString());
				}
			}

			UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored %d/%d buildings"), RestoredCount, WorldData.PlacedBuildings.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadWorldData: Building data table not found. Buildings cannot be restored."));
			UE_LOG(LogTemp, Warning, TEXT("LoadWorldData: Please ensure BuildingDataTable is set up at one of the expected paths."));
		}
	}

	// ===== ?îÎìú ?ùÏÑ± ?ïÎ≥¥ Î°úÎìú (WorldGeneratorSubsystem) =====
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		if (UHarmoniaWorldGeneratorSubsystem* WorldGenSubsystem = GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>())
		{
			// ?úÏ¶å ?ïÎ≥¥ Î≥µÏõê
			WorldGenSubsystem->SetCurrentSeason(static_cast<ESeasonType>(WorldData.CurrentSeason), false);

			// ?†Ïî® ?ïÎ≥¥ Î≥µÏõê
			WorldGenSubsystem->ChangeWeather(static_cast<EWeatherType>(WorldData.CurrentWeatherType), 0.0f);

			// ?úÍ∞Ñ ?ïÎ≥¥ Î≥µÏõê
			WorldGenSubsystem->SetCurrentGameTime(WorldData.CurrentGameHour);

			UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored world generator data (Season: %d, Weather: %d, Time: %.2f)"),
				WorldData.CurrentSeason, WorldData.CurrentWeatherType, WorldData.CurrentGameHour);

			// NOTE: Manual world generation Í∏∞Îä•?Ä ?ÑÏßÅ Íµ¨ÌòÑ?òÏ? ?äÏïò?µÎãà??
			// TODO: ?òÎèô ?ùÏÑ± ?îÎìú ÏßÄ??Íµ¨ÌòÑ ???ÑÎûò ÏΩîÎìúÎ•??úÏÑ±?îÌïò?∏Ïöî.
			// if (!WorldData.bIsAutomaticallyGenerated && WorldData.ManualObjectLocations.Num() > 0)
			// {
			//     WorldGenSubsystem->LoadManualWorldObjects(WorldData.ManualObjectLocations);
			//     UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored %d manual world objects"), WorldData.ManualObjectLocations.Num());
			// }
		}
	}

	// ===== ?úÍ∞Ñ/?†Ïî® ?úÏä§??Î°úÎìú (TimeWeatherManager) =====
	// TimeWeatherManager??delegate ?úÏä§?úÎßå ?úÍ≥µ?òÎ?Î°? WorldGeneratorSubsystem?êÏÑú ?¥Î? Ï≤òÎ¶¨??
	// ?ÑÏöî ??Ï∂îÍ? ?ïÎ≥¥ Î≥µÏõê
	if (UHarmoniaTimeWeatherManager* TimeWeatherManager = World->GetSubsystem<UHarmoniaTimeWeatherManager>())
	{
		// TimeWeatherManager???ÑÏû¨ ?ÅÌÉúÎ•??Ä?•Îêú Í∞íÏúºÎ°?Î∏åÎ°ú?úÏ∫ê?§Ìä∏
		// (?§Ï†ú ?ÅÌÉú Î≥ÄÍ≤ΩÏ? WorldGeneratorSubsystem?êÏÑú Ï≤òÎ¶¨)
		UE_LOG(LogTemp, Log, TEXT("LoadWorldData: TimeWeatherManager is ready for state synchronization"));
	}

	// ===== Î¶¨ÏÜå???∏Îìú ?ÅÌÉú Î°úÎìú =====
	// NOTE: HarmoniaResourceManagerÍ∞Ä ?ÑÏßÅ Íµ¨ÌòÑ?òÏ? ?äÏïò?µÎãà??
	// TODO: HarmoniaResourceManager Íµ¨ÌòÑ ???ÑÎûò ÏΩîÎìúÎ•??úÏÑ±?îÌïò?∏Ïöî.
	// if (UHarmoniaResourceManager* ResourceManager = World->GetSubsystem<UHarmoniaResourceManager>())
	// {
	//     ResourceManager->LoadResourceNodeStates(WorldData.ResourceNodeStates);
	//     UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored %d resource nodes"), WorldData.ResourceNodeStates.Num());
	// }

	// ===== POI ÏßÑÌñâ ?ÅÌÉú Î°úÎìú =====
	// NOTE: HarmoniaPOIManagerÍ∞Ä ?ÑÏßÅ Íµ¨ÌòÑ?òÏ? ?äÏïò?µÎãà??
	// TODO: HarmoniaPOIManager Íµ¨ÌòÑ ???ÑÎûò ÏΩîÎìúÎ•??úÏÑ±?îÌïò?∏Ïöî.
	// if (UHarmoniaPOIManager* POIManager = World->GetSubsystem<UHarmoniaPOIManager>())
	// {
	//     POIManager->LoadPOIStates(WorldData.POIStates);
	//     UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored %d POI states"), WorldData.POIStates.Num());
	// }

	// ===== Î≥ÄÍ≤ΩÎêú Î∞îÏù¥???∞Ïù¥??Î°úÎìú =====
	// NOTE: Biome modification Í∏∞Îä•???ÑÏßÅ Íµ¨ÌòÑ?òÏ? ?äÏïò?µÎãà??
	// TODO: Biome modification ÏßÄ??Íµ¨ÌòÑ ???ÑÎûò ÏΩîÎìúÎ•??úÏÑ±?îÌïò?∏Ïöî.
	// if (GameInstance && WorldData.ModifiedBiomeIndices.Num() > 0)
	// {
	//     if (UHarmoniaWorldGeneratorSubsystem* WorldGenSubsystem = GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>())
	//     {
	//         WorldGenSubsystem->ApplyBiomeModifications(WorldData.ModifiedBiomeIndices, WorldData.ModifiedBiomeTypes);
	//         UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Applied %d biome modifications"), WorldData.ModifiedBiomeIndices.Num());
	//     }
	// }

	// ===== Ï≤¥ÌÅ¨?¨Ïù∏???∞Ïù¥??Î°úÎìú =====
	if (GameInstance)
	{
		if (UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>())
		{
			CheckpointSubsystem->ApplyCheckpointDataFromLoad(WorldData.CheckpointStates);
			UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Loaded %d checkpoint states"), WorldData.CheckpointStates.Num());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LoadWorldData: World data loaded (Seed: %d, Season: %d, Weather: %d, Hour: %.1f, Day: %d, Buildings: %d)"),
		WorldData.WorldSeed,
		WorldData.CurrentSeason,
		WorldData.CurrentWeatherType,
		WorldData.CurrentGameHour,
		WorldData.CurrentGameDay,
		WorldData.PlacedBuildings.Num());
}

void UHarmoniaSaveGameSubsystem::SavePlayerAttributes(ALyraPlayerState* PlayerState, FHarmoniaSavedPlayerAttributes& OutAttributes)
{
	if (!PlayerState)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = PlayerState->GetLyraAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// HarmoniaAttributeSet ?¨Ïö©
	if (const UHarmoniaAttributeSet* HarmoniaAttrs = ASC->GetSet<UHarmoniaAttributeSet>())
	{
		OutAttributes.Health = HarmoniaAttrs->GetHealth();
		OutAttributes.MaxHealth = HarmoniaAttrs->GetMaxHealth();
		OutAttributes.Stamina = HarmoniaAttrs->GetStamina();
		OutAttributes.MaxStamina = HarmoniaAttrs->GetMaxStamina();
		OutAttributes.AttackPower = HarmoniaAttrs->GetAttackPower();
		OutAttributes.Defense = HarmoniaAttrs->GetDefense();
		OutAttributes.CriticalChance = HarmoniaAttrs->GetCriticalChance();
		OutAttributes.CriticalDamage = HarmoniaAttrs->GetCriticalDamage();
		OutAttributes.MovementSpeed = HarmoniaAttrs->GetMovementSpeed();
		OutAttributes.AttackSpeed = HarmoniaAttrs->GetAttackSpeed();
		return;
	}

	// Í∏∞Î≥∏ LyraAttributeSet ?¨Ïö© (?¥Î∞±)
	if (const ULyraHealthSet* HealthSet = ASC->GetSet<ULyraHealthSet>())
	{
		OutAttributes.Health = HealthSet->GetHealth();
		OutAttributes.MaxHealth = HealthSet->GetMaxHealth();
	}

	if (const ULyraCombatSet* CombatSet = ASC->GetSet<ULyraCombatSet>())
	{
		OutAttributes.AttackPower = CombatSet->GetBaseDamage();
	}
}

void UHarmoniaSaveGameSubsystem::LoadPlayerAttributes(ALyraPlayerState* PlayerState, const FHarmoniaSavedPlayerAttributes& Attributes)
{
	if (!PlayerState)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = PlayerState->GetLyraAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// HarmoniaAttributeSet ?¨Ïö©
	if (const UHarmoniaAttributeSet* HarmoniaAttrs = ASC->GetSet<UHarmoniaAttributeSet>())
	{
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetHealthAttribute(), Attributes.Health);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetMaxHealthAttribute(), Attributes.MaxHealth);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetStaminaAttribute(), Attributes.Stamina);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetMaxStaminaAttribute(), Attributes.MaxStamina);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetAttackPowerAttribute(), Attributes.AttackPower);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetDefenseAttribute(), Attributes.Defense);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetCriticalChanceAttribute(), Attributes.CriticalChance);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetCriticalDamageAttribute(), Attributes.CriticalDamage);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetMovementSpeedAttribute(), Attributes.MovementSpeed);
		ASC->SetNumericAttributeBase(HarmoniaAttrs->GetAttackSpeedAttribute(), Attributes.AttackSpeed);
		return;
	}

	// Í∏∞Î≥∏ LyraAttributeSet ?¨Ïö© (?¥Î∞±)
	if (const ULyraHealthSet* HealthSet = ASC->GetSet<ULyraHealthSet>())
	{
		ASC->SetNumericAttributeBase(HealthSet->GetHealthAttribute(), Attributes.Health);
		ASC->SetNumericAttributeBase(HealthSet->GetMaxHealthAttribute(), Attributes.MaxHealth);
	}

	if (const ULyraCombatSet* CombatSet = ASC->GetSet<ULyraCombatSet>())
	{
		ASC->SetNumericAttributeBase(CombatSet->GetBaseDamageAttribute(), Attributes.AttackPower);
	}
}

void UHarmoniaSaveGameSubsystem::SaveInventory(ULyraInventoryManagerComponent* InventoryComponent, TArray<FHarmoniaSavedInventoryItem>& OutItems)
{
	if (!InventoryComponent)
	{
		return;
	}

	OutItems.Empty();

	TArray<ULyraInventoryItemInstance*> AllItems = InventoryComponent->GetAllItems();
	for (ULyraInventoryItemInstance* Item : AllItems)
	{
		if (!Item)
		{
			continue;
		}

		FHarmoniaSavedInventoryItem SavedItem;

		// ?ÑÏù¥???ïÏùò Í≤ΩÎ°ú ?Ä??
		TSubclassOf<ULyraInventoryItemDefinition> ItemDef = Item->GetItemDef();
		if (ItemDef)
		{
			SavedItem.ItemDefinitionPath = FSoftObjectPath(ItemDef.Get());
		}

		// ?§ÌÉù Í∞úÏàò??FLyraInventoryEntry?êÏÑú Í∞Ä?∏Ï????òÏ?Îß?
		// ?ÑÏû¨ APIÎ°úÎäî ÏßÅÏ†ë ?ëÍ∑º???¥Î†§?∞Î?Î°?Í∏∞Î≥∏Í∞??¨Ïö©
		SavedItem.StackCount = 1;

		OutItems.Add(SavedItem);
	}
}

void UHarmoniaSaveGameSubsystem::LoadInventory(ULyraInventoryManagerComponent* InventoryComponent, const TArray<FHarmoniaSavedInventoryItem>& Items)
{
	if (!InventoryComponent)
	{
		return;
	}

	// ?ÑÏù¥??Î°úÎìú
	for (const FHarmoniaSavedInventoryItem& SavedItem : Items)
	{
		if (UClass* ItemDefClass = Cast<UClass>(SavedItem.ItemDefinitionPath.TryLoad()))
		{
			// ?ÑÏù¥??Ï∂îÍ?
			InventoryComponent->AddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition>(ItemDefClass), SavedItem.StackCount);
		}
	}
}

FString UHarmoniaSaveGameSubsystem::GetSteamIDForPlayer(APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return FString();
	}

	// ?®Îùº???úÎ∏å?úÏä§?úÏóê??UniqueNetId Í∞Ä?∏Ïò§Í∏?
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

	// ?®Îùº???úÎ∏å?úÏä§?úÏùÑ ?¨Ïö©?????ÜÎäî Í≤ΩÏö∞ PlayerState??PlayerId ?¨Ïö©
	if (APlayerState* PS = PlayerController->PlayerState)
	{
		return FString::Printf(TEXT("Player_%d"), PS->GetPlayerId());
	}

	return TEXT("Unknown");
}

bool UHarmoniaSaveGameSubsystem::IsServerOwner(APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	// Î¶¨Ïä® ?úÎ≤Ñ?êÏÑú??Ï≤?Î≤àÏß∏ ?åÎ†à?¥Ïñ¥Í∞Ä ?úÎ≤Ñ ?åÏú†Ï£?
	return PlayerController->GetLocalPlayer() && PlayerController->GetLocalPlayer()->GetControllerId() == 0;
}

bool UHarmoniaSaveGameSubsystem::SaveToSteamCloud(const FString& SaveSlotName, const TArray<uint8>& SaveData)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveToSteamCloud: OnlineSubsystem not available"));
		return false;
	}

	IOnlineUserCloudPtr UserCloud = OnlineSub->GetUserCloudInterface();
	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	if (!UserCloud.IsValid() || !Identity.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveToSteamCloud: UserCloud or Identity interface not available"));
		return false;
	}

	FUniqueNetIdPtr UniqueId = Identity->GetUniquePlayerId(0);
	if (!UniqueId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveToSteamCloud: Failed to get unique player ID"));
		return false;
	}

	FString CloudFileName = SaveSlotName + TEXT(".sav");

	// ?§Ì? ?¥Îùº?∞Îìú???åÏùº ?∞Í∏∞
	TArray<uint8> MutableSaveData = SaveData;
	bool bSuccess = UserCloud->WriteUserFile(*UniqueId, CloudFileName, MutableSaveData);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("SaveToSteamCloud: Successfully wrote %s to Steam Cloud"), *CloudFileName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SaveToSteamCloud: Failed to write %s to Steam Cloud"), *CloudFileName);
	}

	return bSuccess;
}

bool UHarmoniaSaveGameSubsystem::LoadFromSteamCloud(const FString& SaveSlotName, TArray<uint8>& OutSaveData)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadFromSteamCloud: OnlineSubsystem not available"));
		return false;
	}

	IOnlineUserCloudPtr UserCloud = OnlineSub->GetUserCloudInterface();
	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	if (!UserCloud.IsValid() || !Identity.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadFromSteamCloud: UserCloud or Identity interface not available"));
		return false;
	}

	FUniqueNetIdPtr UniqueId = Identity->GetUniquePlayerId(0);
	if (!UniqueId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadFromSteamCloud: Failed to get unique player ID"));
		return false;
	}

	FString CloudFileName = SaveSlotName + TEXT(".sav");

	// ?§Ì? ?¥Îùº?∞Îìú ?åÏùº Î™©Î°ù Í∞Ä?∏Ïò§Í∏?(?ôÍ∏∞??
	TArray<FCloudFileHeader> FileHeaders;
	UserCloud->GetUserFileList(*UniqueId, FileHeaders);

	// ?åÏùº Ï°¥Ïû¨ ?¨Î? ?ïÏù∏
	bool bFileExists = false;
	for (const FCloudFileHeader& Header : FileHeaders)
	{
		if (Header.FileName == CloudFileName)
		{
			bFileExists = true;
			break;
		}
	}

	if (!bFileExists)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadFromSteamCloud: File %s not found in Steam Cloud"), *CloudFileName);
		return false;
	}

	// ?åÏùº ?ΩÍ∏∞
	bool bSuccess = UserCloud->ReadUserFile(*UniqueId, CloudFileName);

	if (bSuccess)
	{
		// ?åÏùº ?∞Ïù¥??Í∞Ä?∏Ïò§Í∏?
		UserCloud->GetFileContents(*UniqueId, CloudFileName, OutSaveData);
		UE_LOG(LogTemp, Log, TEXT("LoadFromSteamCloud: Successfully read %s from Steam Cloud"), *CloudFileName);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoadFromSteamCloud: Failed to read %s from Steam Cloud"), *CloudFileName);
		return false;
	}
}

void UHarmoniaSaveGameSubsystem::OnAutoSaveTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Í≤åÏûÑ??ÏßÑÌñâ Ï§ëÏùº ?åÎßå ?êÎèô ?Ä??
	if (World->GetAuthGameMode())
	{
		UE_LOG(LogTemp, Log, TEXT("AutoSave: Saving game..."));
		SaveGame(DefaultSaveSlotName, true);
	}
}

// =============================================================================
// [SECURITY] Save File Encryption and Integrity Validation
// =============================================================================

void UHarmoniaSaveGameSubsystem::EncryptSaveData(const TArray<uint8>& InData, TArray<uint8>& OutEncryptedData) const
{
	// [SECURITY] Simple XOR cipher for save file encryption
	// NOTE: For production, consider using a stronger encryption algorithm (AES-256)
	// This provides basic protection against casual save file editing

	// Encryption key - In production, this should be obfuscated or stored securely
	// For multiplayer games, server should validate save data independently
	static const uint8 EncryptionKey[] = {
		0x48, 0x61, 0x72, 0x6D, 0x6F, 0x6E, 0x69, 0x61,  // "Harmonia"
		0x53, 0x61, 0x76, 0x65, 0x47, 0x61, 0x6D, 0x65   // "SaveGame"
	};
	static const int32 KeyLength = sizeof(EncryptionKey);

	OutEncryptedData.SetNum(InData.Num());

	for (int32 i = 0; i < InData.Num(); ++i)
	{
		OutEncryptedData[i] = InData[i] ^ EncryptionKey[i % KeyLength];
	}

	UE_LOG(LogTemp, Log, TEXT("[SECURITY] Save data encrypted: %d bytes"), OutEncryptedData.Num());
}

void UHarmoniaSaveGameSubsystem::DecryptSaveData(const TArray<uint8>& InEncryptedData, TArray<uint8>& OutData) const
{
	// XOR cipher is symmetric, so decryption is the same as encryption
	EncryptSaveData(InEncryptedData, OutData);
	UE_LOG(LogTemp, Log, TEXT("[SECURITY] Save data decrypted: %d bytes"), OutData.Num());
}

uint32 UHarmoniaSaveGameSubsystem::CalculateChecksum(const TArray<uint8>& Data) const
{
	// [SECURITY] Calculate CRC32 checksum for data integrity validation
	uint32 Checksum = FCrc::MemCrc32(Data.GetData(), Data.Num());
	UE_LOG(LogTemp, Verbose, TEXT("[SECURITY] Calculated checksum: 0x%08X for %d bytes"), Checksum, Data.Num());
	return Checksum;
}

bool UHarmoniaSaveGameSubsystem::VerifyChecksum(const TArray<uint8>& Data, uint32 ExpectedChecksum) const
{
	uint32 ActualChecksum = CalculateChecksum(Data);
	bool bValid = (ActualChecksum == ExpectedChecksum);

	if (bValid)
	{
		UE_LOG(LogTemp, Log, TEXT("[SECURITY] Checksum verification passed: 0x%08X"), ActualChecksum);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SECURITY] Checksum verification FAILED! Expected: 0x%08X, Actual: 0x%08X"),
			ExpectedChecksum, ActualChecksum);
		UE_LOG(LogTemp, Error, TEXT("[ANTI-CHEAT] Save file may have been tampered with!"));
	}

	return bValid;
}
