// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "HarmoniaCheatManager.generated.h"

/**
 * HarmoniaCheatManager
 * 
 * ê°œë°œ???¸í•˜ê²??˜ê¸° ?„í•œ ì¹˜íŠ¸ ë§¤ë‹ˆ?€?…ë‹ˆ??
 * ì½˜ì†” ëª…ë ¹?´ë? ?µí•´ ?¤ì–‘??ê°œë°œ ê¸°ëŠ¥???œê³µ?©ë‹ˆ??
 * 
 * ? ï¸ ì£¼ì˜: Exec ?¨ìˆ˜??Shipping ë¹Œë“œ?ì„œ ?ë™?¼ë¡œ ë¹„í™œ?±í™”?©ë‹ˆ??
 *          Unreal Engine??CheatManager??Development ë¹Œë“œ?€ Editor?ì„œë§??‘ë™?©ë‹ˆ??
 * 
 * ?¬ìš©ë²?
 * - ê²Œì„ ?¤í–‰ ??~ ?¤ë? ?ŒëŸ¬ ì½˜ì†”???´ê³  ëª…ë ¹?´ë? ?…ë ¥?©ë‹ˆ??
 * - ?? Harmonia.GiveGold 10000
 */
UCLASS(Blueprintable, BlueprintType)
class HARMONIAKIT_API UHarmoniaCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UHarmoniaCheatManager();

	// ==================== Health, Mana, Stamina ====================
	
	/**
	 * ?Œë ˆ?´ì–´??ì²´ë ¥???¤ì •?©ë‹ˆ??
	 * @param NewHealth ?ˆë¡œ??ì²´ë ¥ ê°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
	void HarmoniaSetHealth(float NewHealth);

	/**
	 * ?Œë ˆ?´ì–´??ìµœë? ì²´ë ¥???¤ì •?©ë‹ˆ??
	 * @param NewMaxHealth ?ˆë¡œ??ìµœë? ì²´ë ¥ ê°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
	void HarmoniaSetMaxHealth(float NewMaxHealth);

	/**
	 * ?Œë ˆ?´ì–´??ì²´ë ¥???„ì „???Œë³µ?©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
	void HarmoniaHealFull();

	/**
	 * ?Œë ˆ?´ì–´??ë§ˆë‚˜ë¥??¤ì •?©ë‹ˆ??
	 * @param NewMana ?ˆë¡œ??ë§ˆë‚˜ ê°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
	void HarmoniaSetMana(float NewMana);

	/**
	 * ?Œë ˆ?´ì–´??ìµœë? ë§ˆë‚˜ë¥??¤ì •?©ë‹ˆ??
	 * @param NewMaxMana ?ˆë¡œ??ìµœë? ë§ˆë‚˜ ê°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
	void HarmoniaSetMaxMana(float NewMaxMana);

	/**
	 * ?Œë ˆ?´ì–´???¤íƒœë¯¸ë‚˜ë¥??¤ì •?©ë‹ˆ??
	 * @param NewStamina ?ˆë¡œ???¤íƒœë¯¸ë‚˜ ê°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
	void HarmoniaSetStamina(float NewStamina);

	/**
	 * ?Œë ˆ?´ì–´??ìµœë? ?¤íƒœë¯¸ë‚˜ë¥??¤ì •?©ë‹ˆ??
	 * @param NewMaxStamina ?ˆë¡œ??ìµœë? ?¤íƒœë¯¸ë‚˜ ê°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
	void HarmoniaSetMaxStamina(float NewMaxStamina);

	// ==================== Currency ====================

	/**
	 * ?Œë ˆ?´ì–´?ê²Œ ê³¨ë“œë¥?ì§€ê¸‰í•©?ˆë‹¤.
	 * @param Amount ì§€ê¸‰í•  ê³¨ë“œ ??(?Œìˆ˜ ê°€??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Currency")
	void HarmoniaGiveGold(int32 Amount);

	/**
	 * ?Œë ˆ?´ì–´??ê³¨ë“œë¥??¤ì •?©ë‹ˆ??
	 * @param Amount ?¤ì •??ê³¨ë“œ ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Currency")
	void HarmoniaSetGold(int32 Amount);

	/**
	 * ?Œë ˆ?´ì–´?ê²Œ ?¹ì • ?µí™”ë¥?ì§€ê¸‰í•©?ˆë‹¤.
	 * @param CurrencyName ?µí™” ?´ë¦„
	 * @param Amount ì§€ê¸‰í•  ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Currency")
	void HarmoniaGiveCurrency(const FString& CurrencyName, int32 Amount);

	// ==================== Items ====================

	/**
	 * ?Œë ˆ?´ì–´?ê²Œ ?„ì´?œì„ ì§€ê¸‰í•©?ˆë‹¤.
	 * @param ItemName ?„ì´???´ë¦„ ?ëŠ” ID
	 * @param Amount ì§€ê¸‰í•  ?˜ëŸ‰
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Items")
	void HarmoniaGiveItem(const FString& ItemName, int32 Amount = 1);

	/**
	 * ?Œë ˆ?´ì–´?ê²Œ ëª¨ë“  ?„ì´?œì„ ì§€ê¸‰í•©?ˆë‹¤.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Items")
	void HarmoniaGiveAllItems();

	/**
	 * ?Œë ˆ?´ì–´???¸ë²¤? ë¦¬ë¥??´ë¦¬?´í•©?ˆë‹¤.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Items")
	void HarmoniaClearInventory();

	// ==================== Level & Experience ====================

	/**
	 * ?Œë ˆ?´ì–´???ˆë²¨???¤ì •?©ë‹ˆ??
	 * @param NewLevel ?ˆë¡œ???ˆë²¨
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Level")
	void HarmoniaSetLevel(int32 NewLevel);

	/**
	 * ?Œë ˆ?´ì–´?ê²Œ ê²½í—˜ì¹˜ë? ì§€ê¸‰í•©?ˆë‹¤.
	 * @param Amount ì§€ê¸‰í•  ê²½í—˜ì¹?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Level")
	void HarmoniaGiveXP(int32 Amount);

	/**
	 * ?Œë ˆ?´ì–´ë¥?ì¦‰ì‹œ ?ˆë²¨???œí‚µ?ˆë‹¤.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Level")
	void HarmoniaLevelUp();

	// ==================== Combat ====================

	/**
	 * ë¬´ì  ëª¨ë“œë¥?? ê??©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
	void HarmoniaToggleInvincible();

	/**
	 * ??ëª¨ë“œë¥?? ê??©ë‹ˆ?? (ë¬´ì  + ë¬´í•œ ?ì›)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
	void HarmoniaToggleGodMode();

	/**
	 * ?ìƒ· ?í‚¬ ëª¨ë“œë¥?? ê??©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
	void HarmoniaToggleOneHitKill();

	/**
	 * ?Œë ˆ?´ì–´??ê³µê²©?¥ì„ ë°°ìˆ˜ë¡??¤ì •?©ë‹ˆ??
	 * @param Multiplier ?°ë?ì§€ ë°°ìˆ˜ (1.0 = ê¸°ë³¸, 2.0 = 2ë°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
	void HarmoniaSetDamageMultiplier(float Multiplier);

	// ==================== Movement ====================

	/**
	 * ?Œë ˆ?´ì–´???´ë™ ?ë„ë¥?ë°°ìˆ˜ë¡??¤ì •?©ë‹ˆ??
	 * @param Multiplier ?ë„ ë°°ìˆ˜ (1.0 = ê¸°ë³¸, 2.0 = 2ë°?
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
	void HarmoniaSetSpeed(float Multiplier);

	/**
	 * ?¸í´ë¦?ëª¨ë“œë¥?? ê??©ë‹ˆ?? (ë²??µê³¼)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
	void HarmoniaToggleNoClip();

	/**
	 * ?Œë¼??ëª¨ë“œë¥?? ê??©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
	void HarmoniaToggleFly();

	/**
	 * ë§ˆì»¤ ?„ì¹˜ë¡??”ë ˆ?¬íŠ¸?©ë‹ˆ??
	 * @param MarkerName ë§ˆì»¤ ?´ë¦„
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
	void HarmoniaTeleportToMarker(const FString& MarkerName);

	/**
	 * ì¢Œí‘œë¡??”ë ˆ?¬íŠ¸?©ë‹ˆ??
	 * @param X X ì¢Œí‘œ
	 * @param Y Y ì¢Œí‘œ
	 * @param Z Z ì¢Œí‘œ
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
	void HarmoniaTeleport(float X, float Y, float Z);

	// ==================== Time & Weather ====================

	/**
	 * ê²Œì„ ???œê°„???¤ì •?©ë‹ˆ??
	 * @param Hour ?œê°„ (0-23)
	 * @param Minute ë¶?(0-59)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|World")
	void HarmoniaSetTime(int32 Hour, int32 Minute);

	/**
	 * ?œê°„ ?ë„ë¥??¤ì •?©ë‹ˆ??
	 * @param Multiplier ?œê°„ ë°°ìˆ˜ (1.0 = ê¸°ë³¸, 0 = ?•ì?)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|World")
	void HarmoniaSetTimeScale(float Multiplier);

	/**
	 * ? ì”¨ë¥??¤ì •?©ë‹ˆ??
	 * @param WeatherType ? ì”¨ ?€??(Clear, Rain, Snow, Storm ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|World")
	void HarmoniaSetWeather(const FString& WeatherType);

	// ==================== Quest & Achievement ====================

	/**
	 * ?˜ìŠ¤?¸ë? ?„ë£Œ?©ë‹ˆ??
	 * @param QuestName ?˜ìŠ¤???´ë¦„ ?ëŠ” ID
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Quest")
	void HarmoniaCompleteQuest(const FString& QuestName);

	/**
	 * ëª¨ë“  ?˜ìŠ¤?¸ë? ?„ë£Œ?©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Quest")
	void HarmoniaCompleteAllQuests();

	/**
	 * ?…ì ???´ì œ?©ë‹ˆ??
	 * @param AchievementName ?…ì  ?´ë¦„ ?ëŠ” ID
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Achievement")
	void HarmoniaUnlockAchievement(const FString& AchievementName);

	// ==================== Debug ====================

	/**
	 * ?”ë²„ê·??•ë³´ë¥??”ë©´???œì‹œ?©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
	void HarmoniaToggleDebugInfo();

	/**
	 * ëª¨ë“  ?ì„ ?œê±°?©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
	void HarmoniaKillAllEnemies();

	/**
	 * ì£¼ë????ì„ ?¤í°?©ë‹ˆ??
	 * @param EnemyName ???´ë¦„ ?ëŠ” ID
	 * @param Count ?¤í°????
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
	void HarmoniaSpawnEnemy(const FString& EnemyName, int32 Count = 1);

	/**
	 * ëª¨ë“  ì¹˜íŠ¸ë¥?ë¦¬ì…‹?©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
	void HarmoniaResetCheats();

	/**
	 * ?¬ìš© ê°€?¥í•œ ì¹˜íŠ¸ ëª…ë ¹??ëª©ë¡??ì¶œë ¥?©ë‹ˆ??
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
	void HarmoniaHelp();

protected:
	// ì¹˜íŠ¸ ?íƒœ ?Œë˜ê·?
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	bool bInvincible;

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	bool bGodMode;

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	bool bOneHitKill;

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	bool bNoClip;

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	bool bShowDebugInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	float SpeedMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	float DamageMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Cheat")
	float TimeScale;

	// ?¬í¼ ?¨ìˆ˜??
	class ACharacter* GetPlayerCharacter() const;
	class UAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
	void LogCheat(const FString& Message) const;
};
