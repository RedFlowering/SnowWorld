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

// HarmoniaKit 컴포넌트
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

	// 자동 저장 타이머 시작
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
	// 타이머 정리
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

	// 새 세이브 게임 객체 생성 또는 기존 것 사용
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

	// 세이브 게임 메타데이터 설정
	CurrentSaveGame->SaveSlotName = SaveSlotName;
	CurrentSaveGame->LastSaveTimestamp = FDateTime::Now();

	// 모든 플레이어 데이터 저장
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->PlayerState)
		{
			SavePlayerData(PC, CurrentSaveGame);

			// 첫 번째 플레이어를 서버 소유주로 설정
			if (IsServerOwner(PC))
			{
				CurrentSaveGame->ServerOwnerSteamID = GetSteamIDForPlayer(PC);
			}
		}
	}

	// 월드 데이터 저장 (빌딩 등)
	SaveWorldData(CurrentSaveGame);

	// 로컬에 저장
	bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, SaveGameUserIndex);

	if (!bSaveSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGame: Failed to save to local slot %s"), *SaveSlotName);
		OnSaveGameComplete.Broadcast(EHarmoniaSaveGameResult::Failed, SaveSlotName);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("SaveGame: Successfully saved to local slot %s"), *SaveSlotName);

	// 스팀 클라우드에 저장
	if (bUseSteamCloud)
	{
		// 세이브 데이터를 바이너리로 직렬화
		FBufferArchive SaveData;
		CurrentSaveGame->Serialize(SaveData);
		if (SaveData.Num() > 0)
		{
			TArray<uint8> BinaryData;
			BinaryData.Append(SaveData.GetData(), SaveData.Num());

			if (SaveToSteamCloud(SaveSlotName, BinaryData))
			{
				UE_LOG(LogTemp, Log, TEXT("SaveGame: Successfully saved to Steam Cloud"));
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

	// 스팀 클라우드에서 먼저 로드 시도
	if (bUseSteamCloud)
	{
		TArray<uint8> CloudData;
		if (LoadFromSteamCloud(SaveSlotName, CloudData))
		{
			// 바이너리 데이터를 SaveGame 객체로 역직렬화
			LoadedSaveGame = Cast<UHarmoniaSaveGame>(UGameplayStatics::CreateSaveGameObject(UHarmoniaSaveGame::StaticClass()));
			if (LoadedSaveGame)
			{
				FMemoryReader MemoryReader(CloudData, true);
				LoadedSaveGame->Serialize(MemoryReader);
				UE_LOG(LogTemp, Log, TEXT("LoadGame: Successfully loaded from Steam Cloud"));
			}
		}
	}

	// 스팀 클라우드에서 로드 실패 시 로컬에서 로드
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

	// 버전 체크
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

	// 모든 플레이어 데이터 로드
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->PlayerState)
		{
			LoadPlayerData(PC, LoadedSaveGame);
		}
	}

	// 월드 데이터 로드
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
					// 스팀 클라우드에서 파일 삭제
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

	// 타이머 재시작
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

	// 스팀 ID 저장
	PlayerData.SteamID = GetSteamIDForPlayer(PlayerController);
	PlayerData.PlayerName = LyraPS->GetPlayerName();

	// 플레이어 위치 및 회전 저장
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		PlayerData.PlayerLocation = Pawn->GetActorLocation();
		PlayerData.PlayerRotation = Pawn->GetActorRotation();
	}

	// 플레이어 속성 저장
	SavePlayerAttributes(LyraPS, PlayerData.Attributes);

	// 인벤토리 저장 - 먼저 Lyra 인벤토리 시도
	if (ULyraInventoryManagerComponent* InventoryComp = LyraPS->FindComponentByClass<ULyraInventoryManagerComponent>())
	{
		SaveInventory(InventoryComp, PlayerData.InventoryItems);
	}
	// HarmoniaInventoryComponent는 별도 처리 필요 시 추가

	// 스탯 태그 저장
	// Note: StatTags are saved in the SaveGame structure directly
	// PlayerData.StatTags is populated during the save process

	// 마지막 체크포인트 저장
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		if (UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>())
		{
			PlayerData.LastCheckpointID = CheckpointSubsystem->GetPlayerLastCheckpoint(PlayerController);
		}
	}

	// 저장 시간
	PlayerData.LastSaveTime = FDateTime::Now();

	// SaveGame에 추가
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

	// 스팀 ID로 플레이어 데이터 찾기
	FString SteamID = GetSteamIDForPlayer(PlayerController);
	FHarmoniaPlayerSaveData PlayerData;

	if (!SaveGameObject->GetPlayerData(SteamID, PlayerData))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadPlayerData: No save data found for Steam ID %s"), *SteamID);
		return;
	}

	// 플레이어 위치 및 회전 로드
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		Pawn->SetActorLocation(PlayerData.PlayerLocation);
		Pawn->SetActorRotation(PlayerData.PlayerRotation);
	}

	// 플레이어 속성 로드
	LoadPlayerAttributes(LyraPS, PlayerData.Attributes);

	// 인벤토리 로드
	if (ULyraInventoryManagerComponent* InventoryComp = LyraPS->FindComponentByClass<ULyraInventoryManagerComponent>())
	{
		LoadInventory(InventoryComp, PlayerData.InventoryItems);
	}

	// 스탯 태그 로드
	for (const FGameplayTag& Tag : PlayerData.StatTags)
	{
		LyraPS->AddStatTagStack(Tag, 1);
	}

	// 마지막 체크포인트 로드
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

	// ===== 빌딩 데이터 저장 =====
	if (UHarmoniaBuildingInstanceManager* BuildingManager = World->GetSubsystem<UHarmoniaBuildingInstanceManager>())
	{
		// BuildingManager에서 배치된 건물 정보를 가져와 저장
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

	// ===== 월드 생성 정보 저장 =====
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		if (UHarmoniaWorldGeneratorSubsystem* WorldGenSubsystem = GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>())
		{
			// 시즌 정보 저장
			WorldData.CurrentSeason = static_cast<uint8>(WorldGenSubsystem->GetCurrentSeason());
			WorldData.SeasonProgress = WorldGenSubsystem->GetSeasonProgress();

			// 날씨 정보 저장
			WorldData.CurrentWeatherType = static_cast<uint8>(WorldGenSubsystem->GetCurrentWeather());

			// 시간 정보 저장
			WorldData.CurrentGameHour = WorldGenSubsystem->GetCurrentGameTime();
			WorldData.CurrentGameDay = 0; // WorldGenSubsystem에서 Day 정보는 별도로 없음

			UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved world generator data (Season: %d, Weather: %d, Time: %.2f)"),
				WorldData.CurrentSeason, WorldData.CurrentWeatherType, WorldData.CurrentGameHour);
		}
	}

	// ===== 시간/날씨 시스템 저장 (TimeWeatherManager) =====
	if (UHarmoniaTimeWeatherManager* TimeWeatherManager = World->GetSubsystem<UHarmoniaTimeWeatherManager>())
	{
		// 현재 날씨 정보
		WorldData.CurrentWeatherType = static_cast<uint8>(TimeWeatherManager->GetCurrentWeather());
		WorldData.WeatherIntensity = TimeWeatherManager->GetCurrentWeatherIntensity();

		// 현재 시간 정보 (시/분)
		WorldData.CurrentGameHour = static_cast<float>(TimeWeatherManager->GetCurrentHour()) +
									(TimeWeatherManager->GetCurrentMinute() / 60.0f);
		WorldData.CurrentTimeOfDay = static_cast<uint8>(TimeWeatherManager->GetCurrentTimeOfDay());

		UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved TimeWeatherManager data (Weather: %d, Intensity: %.2f, Hour: %.2f)"),
			WorldData.CurrentWeatherType, WorldData.WeatherIntensity, WorldData.CurrentGameHour);
	}

	// ===== 리소스 노드 상태 저장 =====
	// NOTE: HarmoniaResourceManager가 아직 구현되지 않았습니다.
	// TODO: HarmoniaResourceManager 구현 후 아래 코드를 활성화하세요.
	// if (UHarmoniaResourceManager* ResourceManager = World->GetSubsystem<UHarmoniaResourceManager>())
	// {
	//     ResourceManager->SaveResourceNodeStates(WorldData.ResourceNodeStates);
	//     UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d resource nodes"), WorldData.ResourceNodeStates.Num());
	// }

	// ===== POI 진행 상태 저장 =====
	// NOTE: HarmoniaPOIManager가 아직 구현되지 않았습니다.
	// TODO: HarmoniaPOIManager 구현 후 아래 코드를 활성화하세요.
	// if (UHarmoniaPOIManager* POIManager = World->GetSubsystem<UHarmoniaPOIManager>())
	// {
	//     POIManager->SavePOIStates(WorldData.POIStates);
	//     UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d POI states"), WorldData.POIStates.Num());
	// }

	// ===== 수동 생성 월드 데이터 저장 =====
	// NOTE: Manual world generation 기능이 아직 구현되지 않았습니다.
	// TODO: 수동 생성 월드 지원 구현 후 아래 코드를 활성화하세요.
	// if (!WorldData.bIsAutomaticallyGenerated)
	// {
	//     // Save all manually placed world objects (trees, rocks, etc.)
	//     if (GameInstance && WorldGenSubsystem)
	//     {
	//         WorldGenSubsystem->SaveManualWorldObjects(WorldData.ManualObjectLocations);
	//         UE_LOG(LogTemp, Log, TEXT("SaveWorldData: Saved %d manual world objects"), WorldData.ManualObjectLocations.Num());
	//     }
	// }

	// ===== 체크포인트 데이터 저장 =====
	UGameInstance* GameInstance = GetGameInstance();
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

	// ===== 빌딩 데이터 로드 =====
	if (UHarmoniaBuildingInstanceManager* BuildingManager = World->GetSubsystem<UHarmoniaBuildingInstanceManager>())
	{
		// NOTE: BuildingManager의 PlaceBuilding 함수는 FBuildingPartData를 요구하지만,
		// 우리는 PartID만 저장하고 있습니다. 따라서 BuildingDataTable에서
		// PartID로 FBuildingPartData를 조회해야 합니다.
		// TODO: HarmoniaBuildingInstanceManager에 PartID를 받는 PlaceBuilding 오버로드 추가
		//       또는 데이터 테이블 접근 API 추가
		//
		// Example implementation:
		// for (const FHarmoniaSavedBuildingInstance& Building : WorldData.PlacedBuildings)
		// {
		//     // PartID로 FBuildingPartData 조회
		//     FBuildingPartData* PartData = BuildingManager->GetBuildingPartData(Building.PartID);
		//     if (PartData)
		//     {
		//         FGuid RestoredGuid = BuildingManager->PlaceBuilding(
		//             *PartData,
		//             Building.Location,
		//             Building.Rotation,
		//             nullptr // Owner는 별도로 복원 필요
		//         );
		//     }
		// }

		UE_LOG(LogTemp, Warning, TEXT("LoadWorldData: Building restoration not implemented - needs API enhancement"));
	}

	// ===== 월드 생성 정보 로드 (WorldGeneratorSubsystem) =====
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		if (UHarmoniaWorldGeneratorSubsystem* WorldGenSubsystem = GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>())
		{
			// 시즌 정보 복원
			WorldGenSubsystem->SetCurrentSeason(static_cast<ESeasonType>(WorldData.CurrentSeason), false);

			// 날씨 정보 복원
			WorldGenSubsystem->ChangeWeather(static_cast<EWeatherType>(WorldData.CurrentWeatherType), 0.0f);

			// 시간 정보 복원
			WorldGenSubsystem->SetCurrentGameTime(WorldData.CurrentGameHour);

			UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored world generator data (Season: %d, Weather: %d, Time: %.2f)"),
				WorldData.CurrentSeason, WorldData.CurrentWeatherType, WorldData.CurrentGameHour);

			// NOTE: Manual world generation 기능은 아직 구현되지 않았습니다.
			// TODO: 수동 생성 월드 지원 구현 후 아래 코드를 활성화하세요.
			// if (!WorldData.bIsAutomaticallyGenerated && WorldData.ManualObjectLocations.Num() > 0)
			// {
			//     WorldGenSubsystem->LoadManualWorldObjects(WorldData.ManualObjectLocations);
			//     UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored %d manual world objects"), WorldData.ManualObjectLocations.Num());
			// }
		}
	}

	// ===== 시간/날씨 시스템 로드 (TimeWeatherManager) =====
	// TimeWeatherManager는 delegate 시스템만 제공하므로, WorldGeneratorSubsystem에서 이미 처리됨
	// 필요 시 추가 정보 복원
	if (UHarmoniaTimeWeatherManager* TimeWeatherManager = World->GetSubsystem<UHarmoniaTimeWeatherManager>())
	{
		// TimeWeatherManager의 현재 상태를 저장된 값으로 브로드캐스트
		// (실제 상태 변경은 WorldGeneratorSubsystem에서 처리)
		UE_LOG(LogTemp, Log, TEXT("LoadWorldData: TimeWeatherManager is ready for state synchronization"));
	}

	// ===== 리소스 노드 상태 로드 =====
	// NOTE: HarmoniaResourceManager가 아직 구현되지 않았습니다.
	// TODO: HarmoniaResourceManager 구현 후 아래 코드를 활성화하세요.
	// if (UHarmoniaResourceManager* ResourceManager = World->GetSubsystem<UHarmoniaResourceManager>())
	// {
	//     ResourceManager->LoadResourceNodeStates(WorldData.ResourceNodeStates);
	//     UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored %d resource nodes"), WorldData.ResourceNodeStates.Num());
	// }

	// ===== POI 진행 상태 로드 =====
	// NOTE: HarmoniaPOIManager가 아직 구현되지 않았습니다.
	// TODO: HarmoniaPOIManager 구현 후 아래 코드를 활성화하세요.
	// if (UHarmoniaPOIManager* POIManager = World->GetSubsystem<UHarmoniaPOIManager>())
	// {
	//     POIManager->LoadPOIStates(WorldData.POIStates);
	//     UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Restored %d POI states"), WorldData.POIStates.Num());
	// }

	// ===== 변경된 바이옴 데이터 로드 =====
	// NOTE: Biome modification 기능이 아직 구현되지 않았습니다.
	// TODO: Biome modification 지원 구현 후 아래 코드를 활성화하세요.
	// if (GameInstance && WorldData.ModifiedBiomeIndices.Num() > 0)
	// {
	//     if (UHarmoniaWorldGeneratorSubsystem* WorldGenSubsystem = GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>())
	//     {
	//         WorldGenSubsystem->ApplyBiomeModifications(WorldData.ModifiedBiomeIndices, WorldData.ModifiedBiomeTypes);
	//         UE_LOG(LogTemp, Log, TEXT("LoadWorldData: Applied %d biome modifications"), WorldData.ModifiedBiomeIndices.Num());
	//     }
	// }

	// ===== 체크포인트 데이터 로드 =====
	UGameInstance* GameInstance = GetGameInstance();
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

	// HarmoniaAttributeSet 사용
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

	// 기본 LyraAttributeSet 사용 (폴백)
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

	// HarmoniaAttributeSet 사용
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

	// 기본 LyraAttributeSet 사용 (폴백)
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

		// 아이템 정의 경로 저장
		TSubclassOf<ULyraInventoryItemDefinition> ItemDef = Item->GetItemDef();
		if (ItemDef)
		{
			SavedItem.ItemDefinitionPath = FSoftObjectPath(ItemDef.Get());
		}

		// 스택 개수는 FLyraInventoryEntry에서 가져와야 하지만
		// 현재 API로는 직접 접근이 어려우므로 기본값 사용
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

	// 아이템 로드
	for (const FHarmoniaSavedInventoryItem& SavedItem : Items)
	{
		if (UClass* ItemDefClass = Cast<UClass>(SavedItem.ItemDefinitionPath.TryLoad()))
		{
			// 아이템 추가
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

	// 온라인 서브시스템에서 UniqueNetId 가져오기
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

bool UHarmoniaSaveGameSubsystem::IsServerOwner(APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	// 리슨 서버에서는 첫 번째 플레이어가 서버 소유주
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

	// 스팀 클라우드에 파일 쓰기
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

	// 스팀 클라우드 파일 목록 가져오기 (동기식)
	TArray<FCloudFileHeader> FileHeaders;
	UserCloud->GetUserFileList(*UniqueId, FileHeaders);

	// 파일 존재 여부 확인
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

	// 파일 읽기
	bool bSuccess = UserCloud->ReadUserFile(*UniqueId, CloudFileName);

	if (bSuccess)
	{
		// 파일 데이터 가져오기
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

	// 게임이 진행 중일 때만 자동 저장
	if (World->GetAuthGameMode())
	{
		UE_LOG(LogTemp, Log, TEXT("AutoSave: Saving game..."));
		SaveGame(DefaultSaveSlotName, true);
	}
}
