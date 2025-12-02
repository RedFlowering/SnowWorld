// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaSaveGameSubsystem.h"
#include "HarmoniaSaveGameLibrary.generated.h"

/**
 * ë¸”ë£¨?„ë¦°?¸ì—???¸ì´ë¸?ë¡œë“œ ?œìŠ¤?œì„ ?½ê²Œ ?¬ìš©?????ˆë„ë¡??˜ëŠ” ?¨ìˆ˜ ?¼ì´ë¸ŒëŸ¬ë¦?
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaSaveGameLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * ê²Œì„ ?€??
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @param SaveSlotName ?€???¬ë¡¯ ?´ë¦„
	 * @param bUseSteamCloud ?¤í? ?´ë¼?°ë“œ ?¬ìš© ?¬ë?
	 * @return ?€???±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool SaveGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * ê²Œì„ ë¡œë“œ
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @param SaveSlotName ë¡œë“œ???¬ë¡¯ ?´ë¦„
	 * @param bUseSteamCloud ?¤í? ?´ë¼?°ë“œ?ì„œ ë¡œë“œ ?¬ë?
	 * @return ë¡œë“œ ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool LoadGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * ?¸ì´ë¸??Œì¼ ?? œ
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @param SaveSlotName ?? œ???¬ë¡¯ ?´ë¦„
	 * @param bDeleteFromSteamCloud ?¤í? ?´ë¼?°ë“œ?ì„œ???? œ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool DeleteSaveGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bDeleteFromSteamCloud = true);

	/**
	 * ?¸ì´ë¸??Œì¼ ì¡´ì¬ ?¬ë? ?•ì¸
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @param SaveSlotName ?•ì¸???¬ë¡¯ ?´ë¦„
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool DoesSaveGameExist(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"));

	/**
	 * ?„ì¬ ë¡œë“œ???¸ì´ë¸?ê²Œì„ ë°˜í™˜
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaSaveGame* GetCurrentSaveGame(const UObject* WorldContextObject);

	/**
	 * ?¸ì´ë¸?ê²Œì„ ?œë¸Œ?œìŠ¤??ê°€?¸ì˜¤ê¸?
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaSaveGameSubsystem* GetSaveGameSubsystem(const UObject* WorldContextObject);

	/**
	 * ?ë™ ?€???œì„±??ë¹„í™œ?±í™”
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @param bEnabled ?œì„±???¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoSaveEnabled(const UObject* WorldContextObject, bool bEnabled);

	/**
	 * ?ë™ ?€??ê°„ê²© ?¤ì •
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @param IntervalInSeconds ê°„ê²© (ì´??¨ìœ„)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoSaveInterval(const UObject* WorldContextObject, float IntervalInSeconds);

	/**
	 * ?Œë ˆ?´ì–´???¤í? ID ê°€?¸ì˜¤ê¸?
	 * @param PlayerController ?Œë ˆ?´ì–´ ì»¨íŠ¸ë¡¤ëŸ¬
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame")
	static FString GetPlayerSteamID(APlayerController* PlayerController);

	/**
	 * ?„ì¬ ?Œë ˆ?´ì–´ê°€ ?œë²„ ?Œìœ ì£¼ì¸ì§€ ?•ì¸
	 * @param PlayerController ?Œë ˆ?´ì–´ ì»¨íŠ¸ë¡¤ëŸ¬
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame")
	static bool IsServerOwner(APlayerController* PlayerController);
};
