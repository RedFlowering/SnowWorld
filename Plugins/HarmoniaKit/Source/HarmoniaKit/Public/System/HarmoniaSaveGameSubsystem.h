// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HarmoniaSaveGame.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "HarmoniaSaveGameSubsystem.generated.h"

class APlayerController;
class ALyraPlayerState;
class ULyraInventoryManagerComponent;

/** ?¸ì´ë¸?ë¡œë“œ ê²°ê³¼ */
UENUM(BlueprintType)
enum class EHarmoniaSaveGameResult : uint8
{
	Success,
	Failed,
	InProgress,
	NotSupported
};

/** ?¸ì´ë¸?ë¡œë“œ ?„ë£Œ ?¸ë¦¬ê²Œì´??*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaSaveGameComplete, EHarmoniaSaveGameResult, Result, const FString&, SaveSlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaLoadGameComplete, EHarmoniaSaveGameResult, Result, UHarmoniaSaveGame*, SaveGameObject);

/**
 * ê²Œì„ ?¸ì´ë¸?ë¡œë“œ ?œìŠ¤??
 *
 * ê¸°ëŠ¥:
 * - ë¡œì»¬ PC???€??(?œë²„ ?Œìœ ì£?
 * - ?¤í? ?´ë¼?°ë“œ ?¸ì´ë¸?ì§€??
 * - ë©€?°í”Œ?ˆì´?? ê°??Œë ˆ?´ì–´???¤í? IDë³??°ì´???€??
 *
 * ?¬ìš© ë°©ë²•:
 * 1. SaveGame() - ?„ì¬ ê²Œì„ ?íƒœ ?€??
 * 2. LoadGame() - ê²Œì„ ?íƒœ ë¡œë“œ
 * 3. AutoSave ?œì„±?????ë™ ?€??
 */
UCLASS(config=Game)
class HARMONIAKIT_API UHarmoniaSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaSaveGameSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End of USubsystem interface

	/**
	 * ê²Œì„ ?€??
	 * @param SaveSlotName ?€???¬ë¡¯ ?´ë¦„
	 * @param bUseSteamCloud ?¤í? ?´ë¼?°ë“œ ?¬ìš© ?¬ë?
	 * @return ?€???±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool SaveGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * ê²Œì„ ë¡œë“œ
	 * @param SaveSlotName ë¡œë“œ???¬ë¡¯ ?´ë¦„
	 * @param bUseSteamCloud ?¤í? ?´ë¼?°ë“œ?ì„œ ë¡œë“œ ?¬ë?
	 * @return ë¡œë“œ ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool LoadGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * ?¸ì´ë¸??Œì¼ ?? œ
	 * @param SaveSlotName ?? œ???¬ë¡¯ ?´ë¦„
	 * @param bDeleteFromSteamCloud ?¤í? ?´ë¼?°ë“œ?ì„œ???? œ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool DeleteSaveGame(const FString& SaveSlotName = TEXT("DefaultSave"), bool bDeleteFromSteamCloud = true);

	/**
	 * ?¸ì´ë¸??Œì¼ ì¡´ì¬ ?¬ë? ?•ì¸
	 * @param SaveSlotName ?•ì¸???¬ë¡¯ ?´ë¦„
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	bool DoesSaveGameExist(const FString& SaveSlotName = TEXT("DefaultSave")) const;

	/**
	 * ?„ì¬ ë¡œë“œ???¸ì´ë¸?ê²Œì„ ë°˜í™˜
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame")
	UHarmoniaSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }

	/**
	 * ?ë™ ?€???œì„±??ë¹„í™œ?±í™”
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	void SetAutoSaveEnabled(bool bEnabled);

	/**
	 * ?ë™ ?€??ê°„ê²© ?¤ì • (ì´??¨ìœ„)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame")
	void SetAutoSaveInterval(float IntervalInSeconds);

	/** ?¸ì´ë¸??„ë£Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|SaveGame")
	FOnHarmoniaSaveGameComplete OnSaveGameComplete;

	/** ë¡œë“œ ?„ë£Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|SaveGame")
	FOnHarmoniaLoadGameComplete OnLoadGameComplete;

protected:
	/** ?Œë ˆ?´ì–´ ?°ì´?°ë? SaveGame???€??*/
	void SavePlayerData(APlayerController* PlayerController, UHarmoniaSaveGame* SaveGameObject);

	/** SaveGame?ì„œ ?Œë ˆ?´ì–´ ?°ì´??ë¡œë“œ */
	void LoadPlayerData(APlayerController* PlayerController, const UHarmoniaSaveGame* SaveGameObject);

	/** ?”ë“œ ?°ì´???€??(ë¹Œë”© ?? */
	void SaveWorldData(UHarmoniaSaveGame* SaveGameObject);

	/** ?”ë“œ ?°ì´??ë¡œë“œ */
	void LoadWorldData(const UHarmoniaSaveGame* SaveGameObject);

	/** ?Œë ˆ?´ì–´ ?ì„± ?€??*/
	void SavePlayerAttributes(ALyraPlayerState* PlayerState, FHarmoniaSavedPlayerAttributes& OutAttributes);

	/** ?Œë ˆ?´ì–´ ?ì„± ë¡œë“œ */
	void LoadPlayerAttributes(ALyraPlayerState* PlayerState, const FHarmoniaSavedPlayerAttributes& Attributes);

	/** ?¸ë²¤? ë¦¬ ?€??*/
	void SaveInventory(ULyraInventoryManagerComponent* InventoryComponent, TArray<FHarmoniaSavedInventoryItem>& OutItems);

	/** ?¸ë²¤? ë¦¬ ë¡œë“œ */
	void LoadInventory(ULyraInventoryManagerComponent* InventoryComponent, const TArray<FHarmoniaSavedInventoryItem>& Items);

	/** ?¤í? ID ê°€?¸ì˜¤ê¸?*/
	FString GetSteamIDForPlayer(APlayerController* PlayerController) const;

	/** ë¡œì»¬ ?Œë ˆ?´ì–´ê°€ ?œë²„ ?Œìœ ì£¼ì¸ì§€ ?•ì¸ */
	bool IsServerOwner(APlayerController* PlayerController) const;

	/** ?¤í? ?´ë¼?°ë“œ???€??*/
	bool SaveToSteamCloud(const FString& SaveSlotName, const TArray<uint8>& SaveData);

	/** ?¤í? ?´ë¼?°ë“œ?ì„œ ë¡œë“œ */
	bool LoadFromSteamCloud(const FString& SaveSlotName, TArray<uint8>& OutSaveData);

	/** ?ë™ ?€???€?´ë¨¸ */
	void OnAutoSaveTimer();

	// [SECURITY] Save file encryption and integrity
	/** Encrypt save data using XOR cipher */
	void EncryptSaveData(const TArray<uint8>& InData, TArray<uint8>& OutEncryptedData) const;

	/** Decrypt save data using XOR cipher */
	void DecryptSaveData(const TArray<uint8>& InEncryptedData, TArray<uint8>& OutData) const;

	/** Calculate CRC32 checksum for save data integrity */
	uint32 CalculateChecksum(const TArray<uint8>& Data) const;

	/** Verify save data integrity using CRC32 checksum */
	bool VerifyChecksum(const TArray<uint8>& Data, uint32 ExpectedChecksum) const;

private:
	/** ?„ì¬ ë¡œë“œ???¸ì´ë¸?ê²Œì„ */
	UPROPERTY(Transient)
	TObjectPtr<UHarmoniaSaveGame> CurrentSaveGame;

	/** ?ë™ ?€???œì„±???¬ë? */
	UPROPERTY(Config)
	bool bAutoSaveEnabled = true;

	/** ?ë™ ?€??ê°„ê²© (ì´? */
	UPROPERTY(Config)
	float AutoSaveIntervalSeconds = 300.0f; // 5ë¶?

	/** ?ë™ ?€???€?´ë¨¸ ?¸ë“¤ */
	FTimerHandle AutoSaveTimerHandle;

	/** ë§ˆì?ë§??€???œê°„ */
	float LastSaveTime = 0.0f;

	/** ê¸°ë³¸ ?€???¬ë¡¯ ?´ë¦„ */
	static const FString DefaultSaveSlotName;

	/** ?¸ì´ë¸??Œì¼ ?¬ìš©???¸ë±??*/
	static const int32 SaveGameUserIndex;
};
