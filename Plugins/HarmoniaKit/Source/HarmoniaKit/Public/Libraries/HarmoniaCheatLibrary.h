// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaCheatLibrary.generated.h"

/**
 * HarmoniaCheatLibrary
 * 
 * ë¸”ë£¨?„ë¦°?¸ì—??ì¹˜íŠ¸ ë§¤ë‹ˆ?€???½ê²Œ ?‘ê·¼?????ˆë„ë¡??˜ëŠ” ?¨ìˆ˜ ?¼ì´ë¸ŒëŸ¬ë¦¬ì…?ˆë‹¤.
 * 
 * ? ï¸ ì£¼ì˜: ???¼ì´ë¸ŒëŸ¬ë¦¬ëŠ” Shipping ë¹Œë“œ?ì„œ ì»´íŒŒ?¼ë˜ì§€ë§?ì¹˜íŠ¸ ë§¤ë‹ˆ?€ê°€ ?†ìœ¼ë¯€ë¡?
 *          ëª¨ë“  ?¨ìˆ˜ê°€ ?¤ì§ˆ?ìœ¼ë¡??™ì‘?˜ì? ?ŠìŠµ?ˆë‹¤.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCheatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * ?„ì¬ ?Œë ˆ?´ì–´??ì¹˜íŠ¸ ë§¤ë‹ˆ?€ë¥?ê°€?¸ì˜µ?ˆë‹¤.
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @return ì¹˜íŠ¸ ë§¤ë‹ˆ?€ (?†ìœ¼ë©?nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static class UHarmoniaCheatManager* GetHarmoniaCheatManager(const UObject* WorldContextObject);

	/**
	 * ì¹˜íŠ¸ ë§¤ë‹ˆ?€ê°€ ?œì„±?”ë˜???ˆëŠ”ì§€ ?•ì¸?©ë‹ˆ??
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @return ì¹˜íŠ¸ ë§¤ë‹ˆ?€ ?¬ìš© ê°€???¬ë?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatManagerAvailable(const UObject* WorldContextObject);

	/**
	 * ì¹˜íŠ¸ê°€ ?„ì¬ ?œì„±?”ë˜???ˆëŠ”ì§€ ?•ì¸ (Development ë¹Œë“œ??-cheat ì»¤ë§¨?œë¼???µì…˜ ?„ìš”)
	 * @param WorldContextObject ?”ë“œ ì»¨í…?¤íŠ¸
	 * @return ì¹˜íŠ¸ ?œì„±???¬ë?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatsEnabled(const UObject* WorldContextObject);

	// ==================== Quick Cheat Functions ====================

	/**
	 * ë¹ ë¥¸ ì¹˜íŠ¸: ?Œë ˆ?´ì–´ë¥??„ì „???Œë³µ?©ë‹ˆ??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickHeal(const UObject* WorldContextObject);

	/**
	 * ë¹ ë¥¸ ì¹˜íŠ¸: ë¬´ì  ëª¨ë“œë¥?? ê??©ë‹ˆ??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleInvincible(const UObject* WorldContextObject);

	/**
	 * ë¹ ë¥¸ ì¹˜íŠ¸: ê³¨ë“œ 10000??ì§€ê¸‰í•©?ˆë‹¤.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickGiveGold(const UObject* WorldContextObject, int32 Amount = 10000);

	/**
	 * ë¹ ë¥¸ ì¹˜íŠ¸: ë¹„í–‰ ëª¨ë“œë¥?? ê??©ë‹ˆ??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleFly(const UObject* WorldContextObject);

	/**
	 * ë¹ ë¥¸ ì¹˜íŠ¸: ?´ë™ ?ë„ë¥?2ë°°ë¡œ ?¤ì •?©ë‹ˆ??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickDoubleSpeed(const UObject* WorldContextObject);

	/**
	 * ë¹ ë¥¸ ì¹˜íŠ¸: ì¹˜íŠ¸ë¥?ëª¨ë‘ ë¦¬ì…‹?©ë‹ˆ??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickResetCheats(const UObject* WorldContextObject);
};
