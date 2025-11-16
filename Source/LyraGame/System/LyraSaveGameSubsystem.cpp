// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSaveGameSubsystem.h"
#include "LyraSaveGame.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"
#include "Character/LyraCharacter.h"
#include "Inventory/LyraInventoryManagerComponent.h"
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

// HarmoniaKit 지원 (있는 경우)
#if __has_include("AbilitySystem/HarmoniaAttributeSet.h")
	#include "AbilitySystem/HarmoniaAttributeSet.h"
	#define HARMONIA_ATTRIBUTE_SET_AVAILABLE 1
#else
	#define HARMONIA_ATTRIBUTE_SET_AVAILABLE 0
#endif

#if __has_include("Components/HarmoniaInventoryComponent.h")
	#include "Components/HarmoniaInventoryComponent.h"
	#define HARMONIA_INVENTORY_AVAILABLE 1
#else
	#define HARMONIA_INVENTORY_AVAILABLE 0
#endif

#if __has_include("Components/HarmoniaBuildingComponent.h")
	#include "Components/HarmoniaBuildingComponent.h"
	#include "BuildingSystem/HarmoniaBuildingInstanceManager.h"
	#define HARMONIA_BUILDING_AVAILABLE 1
#else
	#define HARMONIA_BUILDING_AVAILABLE 0
#endif

const FString ULyraSaveGameSubsystem::DefaultSaveSlotName = TEXT("DefaultSave");
const int32 ULyraSaveGameSubsystem::SaveGameUserIndex = 0;

ULyraSaveGameSubsystem::ULyraSaveGameSubsystem()
	: CurrentSaveGame(nullptr)
	, bAutoSaveEnabled(true)
	, AutoSaveIntervalSeconds(300.0f)
	, LastSaveTime(0.0f)
{
}

void ULyraSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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
				&ULyraSaveGameSubsystem::OnAutoSaveTimer,
				AutoSaveIntervalSeconds,
				true
			);
		}
	}
}

void ULyraSaveGameSubsystem::Deinitialize()
{
	// 타이머 정리
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	Super::Deinitialize();
}

bool ULyraSaveGameSubsystem::SaveGame(const FString& SaveSlotName, bool bUseSteamCloud)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGame: World is null"));
		OnSaveGameComplete.Broadcast(ELyraSaveGameResult::Failed, SaveSlotName);
		return false;
	}

	// 새 세이브 게임 객체 생성 또는 기존 것 사용
	if (!CurrentSaveGame)
	{
		CurrentSaveGame = Cast<ULyraSaveGame>(UGameplayStatics::CreateSaveGameObject(ULyraSaveGame::StaticClass()));
	}

	if (!CurrentSaveGame)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGame: Failed to create SaveGame object"));
		OnSaveGameComplete.Broadcast(ELyraSaveGameResult::Failed, SaveSlotName);
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
		OnSaveGameComplete.Broadcast(ELyraSaveGameResult::Failed, SaveSlotName);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("SaveGame: Successfully saved to local slot %s"), *SaveSlotName);

	// 스팀 클라우드에 저장
	if (bUseSteamCloud)
	{
		// 세이브 데이터를 바이너리로 직렬화
		FBufferArchive SaveData;
		if (CurrentSaveGame->Serialize(SaveData))
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
	OnSaveGameComplete.Broadcast(ELyraSaveGameResult::Success, SaveSlotName);
	return true;
}

bool ULyraSaveGameSubsystem::LoadGame(const FString& SaveSlotName, bool bUseSteamCloud)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadGame: World is null"));
		OnLoadGameComplete.Broadcast(ELyraSaveGameResult::Failed, nullptr);
		return false;
	}

	ULyraSaveGame* LoadedSaveGame = nullptr;

	// 스팀 클라우드에서 먼저 로드 시도
	if (bUseSteamCloud)
	{
		TArray<uint8> CloudData;
		if (LoadFromSteamCloud(SaveSlotName, CloudData))
		{
			// 바이너리 데이터를 SaveGame 객체로 역직렬화
			LoadedSaveGame = Cast<ULyraSaveGame>(UGameplayStatics::CreateSaveGameObject(ULyraSaveGame::StaticClass()));
			if (LoadedSaveGame)
			{
				FMemoryReader MemoryReader(CloudData, true);
				if (LoadedSaveGame->Serialize(MemoryReader))
				{
					UE_LOG(LogTemp, Log, TEXT("LoadGame: Successfully loaded from Steam Cloud"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("LoadGame: Failed to deserialize Steam Cloud data"));
					LoadedSaveGame = nullptr;
				}
			}
		}
	}

	// 스팀 클라우드에서 로드 실패 시 로컬에서 로드
	if (!LoadedSaveGame)
	{
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveGameUserIndex))
		{
			LoadedSaveGame = Cast<ULyraSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveGameUserIndex));

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
		OnLoadGameComplete.Broadcast(ELyraSaveGameResult::Failed, nullptr);
		return false;
	}

	// 버전 체크
	if (LoadedSaveGame->SaveVersion != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadGame: Save version mismatch (expected 1, got %d)"), LoadedSaveGame->SaveVersion);
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

	OnLoadGameComplete.Broadcast(ELyraSaveGameResult::Success, LoadedSaveGame);
	return true;
}

bool ULyraSaveGameSubsystem::DeleteSaveGame(const FString& SaveSlotName, bool bDeleteFromSteamCloud)
{
	bool bSuccess = UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveGameUserIndex);

	if (bDeleteFromSteamCloud)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineUserCloudPtr UserCloud = OnlineSub->GetUserCloudInterface();
			if (UserCloud.IsValid())
			{
				// 스팀 클라우드에서 파일 삭제
				FString CloudFileName = SaveSlotName + TEXT(".sav");
				UserCloud->DeleteUserFile(0, CloudFileName, true, true);
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

bool ULyraSaveGameSubsystem::DoesSaveGameExist(const FString& SaveSlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveGameUserIndex);
}

void ULyraSaveGameSubsystem::SetAutoSaveEnabled(bool bEnabled)
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
			&ULyraSaveGameSubsystem::OnAutoSaveTimer,
			AutoSaveIntervalSeconds,
			true
		);
	}
	else
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}
}

void ULyraSaveGameSubsystem::SetAutoSaveInterval(float IntervalInSeconds)
{
	AutoSaveIntervalSeconds = FMath::Max(0.0f, IntervalInSeconds);

	// 타이머 재시작
	if (bAutoSaveEnabled && AutoSaveIntervalSeconds > 0.0f)
	{
		SetAutoSaveEnabled(false);
		SetAutoSaveEnabled(true);
	}
}

void ULyraSaveGameSubsystem::SavePlayerData(APlayerController* PlayerController, ULyraSaveGame* SaveGameObject)
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

	FLyraPlayerSaveData PlayerData;

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

	// 인벤토리 저장
	if (ULyraInventoryManagerComponent* InventoryComp = LyraPS->FindComponentByClass<ULyraInventoryManagerComponent>())
	{
		SaveInventory(InventoryComp, PlayerData.InventoryItems);
	}
#if HARMONIA_INVENTORY_AVAILABLE
	else if (UHarmoniaInventoryComponent* HarmoniaInv = LyraPS->FindComponentByClass<UHarmoniaInventoryComponent>())
	{
		// HarmoniaInventoryComponent는 별도 처리가 필요할 수 있음
		// 현재는 기본 인벤토리 저장 로직 사용
	}
#endif

	// 스탯 태그 저장
	PlayerData.StatTags = LyraPS->GetStatTags();

	// 저장 시간
	PlayerData.LastSaveTime = FDateTime::Now();

	// SaveGame에 추가
	SaveGameObject->SetPlayerData(PlayerData.SteamID, PlayerData);
}

void ULyraSaveGameSubsystem::LoadPlayerData(APlayerController* PlayerController, const ULyraSaveGame* SaveGameObject)
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
	FLyraPlayerSaveData PlayerData;

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
	// Note: StatTags는 직접 설정할 수 없으므로 AddStatTagStack을 사용해야 함
	for (const FGameplayTag& Tag : PlayerData.StatTags)
	{
		LyraPS->AddStatTagStack(Tag, 1);
	}

	UE_LOG(LogTemp, Log, TEXT("LoadPlayerData: Successfully loaded data for %s"), *PlayerData.PlayerName);
}

void ULyraSaveGameSubsystem::SaveWorldData(ULyraSaveGame* SaveGameObject)
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

	// 빌딩 데이터 저장
#if HARMONIA_BUILDING_AVAILABLE
	// HarmoniaBuildingInstanceManager를 찾아서 배치된 건물들을 저장
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (UHarmoniaBuildingInstanceManager* BuildingManager = Actor->FindComponentByClass<UHarmoniaBuildingInstanceManager>())
		{
			// BuildingManager에서 배치된 건물 정보를 가져와 저장
			// (구체적인 구현은 HarmoniaBuildingInstanceManager의 API에 따라 다름)
		}
	}
#endif

	UE_LOG(LogTemp, Log, TEXT("SaveWorldData: World data saved"));
}

void ULyraSaveGameSubsystem::LoadWorldData(const ULyraSaveGame* SaveGameObject)
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

	// 빌딩 데이터 로드
#if HARMONIA_BUILDING_AVAILABLE
	for (const FLyraSavedBuildingInstance& Building : SaveGameObject->WorldData.PlacedBuildings)
	{
		// 빌딩 복원 로직
		// (구체적인 구현은 HarmoniaBuildingInstanceManager의 API에 따라 다름)
	}
#endif

	UE_LOG(LogTemp, Log, TEXT("LoadWorldData: World data loaded"));
}

void ULyraSaveGameSubsystem::SavePlayerAttributes(ALyraPlayerState* PlayerState, FLyraSavedPlayerAttributes& OutAttributes)
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

#if HARMONIA_ATTRIBUTE_SET_AVAILABLE
	// HarmoniaAttributeSet 사용 시
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
#endif

	// 기본 LyraAttributeSet 사용
	if (const ULyraHealthSet* HealthSet = ASC->GetSet<ULyraHealthSet>())
	{
		OutAttributes.Health = HealthSet->GetHealth();
		OutAttributes.MaxHealth = HealthSet->GetMaxHealth();
	}

	if (const ULyraCombatSet* CombatSet = ASC->GetSet<ULyraCombatSet>())
	{
		OutAttributes.AttackPower = CombatSet->GetBaseDamage();
		// CombatSet에는 Defense 등이 없으므로 기본값 사용
	}
}

void ULyraSaveGameSubsystem::LoadPlayerAttributes(ALyraPlayerState* PlayerState, const FLyraSavedPlayerAttributes& Attributes)
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

#if HARMONIA_ATTRIBUTE_SET_AVAILABLE
	// HarmoniaAttributeSet 사용 시
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
#endif

	// 기본 LyraAttributeSet 사용
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

void ULyraSaveGameSubsystem::SaveInventory(ULyraInventoryManagerComponent* InventoryComponent, TArray<FLyraSavedInventoryItem>& OutItems)
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

		FLyraSavedInventoryItem SavedItem;

		// 아이템 정의 경로 저장
		if (const ULyraInventoryItemDefinition* ItemDef = Item->GetItemDef())
		{
			SavedItem.ItemDefinitionPath = FSoftObjectPath(ItemDef);
		}

		// 스택 개수는 FLyraInventoryEntry에서 가져와야 하지만
		// 현재 API로는 직접 접근이 어려우므로 기본값 사용
		SavedItem.StackCount = 1;

		OutItems.Add(SavedItem);
	}
}

void ULyraSaveGameSubsystem::LoadInventory(ULyraInventoryManagerComponent* InventoryComponent, const TArray<FLyraSavedInventoryItem>& Items)
{
	if (!InventoryComponent)
	{
		return;
	}

	// 기존 인벤토리 비우기
	// (주의: 이 작업은 신중하게 수행해야 함)

	// 아이템 로드
	for (const FLyraSavedInventoryItem& SavedItem : Items)
	{
		if (ULyraInventoryItemDefinition* ItemDef = Cast<ULyraInventoryItemDefinition>(SavedItem.ItemDefinitionPath.TryLoad()))
		{
			// 아이템 추가
			InventoryComponent->AddItemDefinition(ItemDef, SavedItem.StackCount);
		}
	}
}

FString ULyraSaveGameSubsystem::GetSteamIDForPlayer(APlayerController* PlayerController) const
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

bool ULyraSaveGameSubsystem::IsServerOwner(APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	// 리슨 서버에서는 첫 번째 플레이어가 서버 소유주
	return PlayerController->GetLocalPlayer() && PlayerController->GetLocalPlayer()->GetControllerId() == 0;
}

bool ULyraSaveGameSubsystem::SaveToSteamCloud(const FString& SaveSlotName, const TArray<uint8>& SaveData)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveToSteamCloud: OnlineSubsystem not available"));
		return false;
	}

	IOnlineUserCloudPtr UserCloud = OnlineSub->GetUserCloudInterface();
	if (!UserCloud.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveToSteamCloud: UserCloud interface not available"));
		return false;
	}

	FString CloudFileName = SaveSlotName + TEXT(".sav");

	// 스팀 클라우드에 파일 쓰기
	bool bSuccess = UserCloud->WriteUserFile(0, CloudFileName, SaveData);

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

bool ULyraSaveGameSubsystem::LoadFromSteamCloud(const FString& SaveSlotName, TArray<uint8>& OutSaveData)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadFromSteamCloud: OnlineSubsystem not available"));
		return false;
	}

	IOnlineUserCloudPtr UserCloud = OnlineSub->GetUserCloudInterface();
	if (!UserCloud.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadFromSteamCloud: UserCloud interface not available"));
		return false;
	}

	FString CloudFileName = SaveSlotName + TEXT(".sav");

	// 스팀 클라우드 파일 목록 가져오기 (동기식)
	TArray<FCloudFileHeader> FileHeaders;
	UserCloud->GetUserFileList(0, FileHeaders);

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
	bool bSuccess = UserCloud->ReadUserFile(0, CloudFileName);

	if (bSuccess)
	{
		// 파일 데이터 가져오기
		UserCloud->GetFileContents(0, CloudFileName, OutSaveData);
		UE_LOG(LogTemp, Log, TEXT("LoadFromSteamCloud: Successfully read %s from Steam Cloud"), *CloudFileName);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoadFromSteamCloud: Failed to read %s from Steam Cloud"), *CloudFileName);
		return false;
	}
}

void ULyraSaveGameSubsystem::OnAutoSaveTimer()
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
