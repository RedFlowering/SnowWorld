// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "HarmoniaCheatManager.generated.h"

/**
 * HarmoniaCheatManager
 * 
 * 개발과 테스트를 위한 치트 매니저입니다.
 * 콘솔 명령어를 통해 다양한 개발 기능을 제공합니다.
 * 
 *  주의: Exec 함수는 Shipping 빌드에서 자동으로 비활성화됩니다.
 *          Unreal Engine의 CheatManager는 Development 빌드와 Editor에서만 작동합니다.
 * 
 * 사용법:
 * - 게임 실행 후 ~ 키를 눌러 콘솔을 열고 명령어를 입력합니다.
 * - 예: Harmonia.GiveGold 10000
 */
UCLASS(Blueprintable, BlueprintType)
class HARMONIAKIT_API UHarmoniaCheatManager : public UCheatManager
{
GENERATED_BODY()

public:
UHarmoniaCheatManager();

// ==================== Health, Mana, Stamina ====================
// Note: UFUNCTION(Exec) functions are automatically disabled in Shipping builds by UCheatManager

/**
 * 플레이어의 체력을 설정합니다.
 * @param NewHealth 새로운 체력 값
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
void HarmoniaSetHealth(float NewHealth);

/**
 * 플레이어의 최대 체력을 설정합니다.
 * @param NewMaxHealth 새로운 최대 체력 값
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
void HarmoniaSetMaxHealth(float NewMaxHealth);

/**
 * 플레이어의 체력을 완전히 회복합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
void HarmoniaHealFull();

/**
 * 플레이어의 마나를 설정합니다.
 * @param NewMana 새로운 마나 값
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
void HarmoniaSetMana(float NewMana);

/**
 * 플레이어의 최대 마나를 설정합니다.
 * @param NewMaxMana 새로운 최대 마나 값
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
void HarmoniaSetMaxMana(float NewMaxMana);

/**
 * 플레이어의 스태미나를 설정합니다.
 * @param NewStamina 새로운 스태미나 값
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
void HarmoniaSetStamina(float NewStamina);

/**
 * 플레이어의 최대 스태미나를 설정합니다.
 * @param NewMaxStamina 새로운 최대 스태미나 값
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Stats")
void HarmoniaSetMaxStamina(float NewMaxStamina);

// ==================== Currency ====================

/**
 * 플레이어에게 골드를 지급합니다.
 * @param Amount 지급할 골드 양 (음수 가능)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Currency")
void HarmoniaGiveGold(int32 Amount);

/**
 * 플레이어의 골드를 설정합니다.
 * @param Amount 설정할 골드 양
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Currency")
void HarmoniaSetGold(int32 Amount);

/**
 * 플레이어에게 특정 화폐를 지급합니다.
 * @param CurrencyName 화폐 이름
 * @param Amount 지급할 양
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Currency")
void HarmoniaGiveCurrency(const FString& CurrencyName, int32 Amount);

// ==================== Items ====================

/**
 * 플레이어에게 아이템을 지급합니다.
 * @param ItemName 아이템 이름 또는 ID
 * @param Amount 지급할 수량
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Items")
void HarmoniaGiveItem(const FString& ItemName, int32 Amount = 1);

/**
 * 플레이어에게 모든 아이템을 지급합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Items")
void HarmoniaGiveAllItems();

/**
 * 플레이어의 인벤토리를 비웁니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Items")
void HarmoniaClearInventory();

// ==================== Equipment ====================

/**
 * 장비를 장착합니다.
 * @param EquipmentId 장비 ID (예: 10000000)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Equipment")
void HarmoniaEquipItem(const FString& EquipmentId);

/**
 * 특정 슬롯의 장비를 해제합니다.
 * @param SlotName 슬롯 이름 (Head, Chest, Legs, Feet, Hands, MainHand, OffHand, Accessory1, Accessory2, Back)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Equipment")
void HarmoniaUnequipSlot(const FString& SlotName);

/**
 * 모든 장비를 해제합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Equipment")
void HarmoniaUnequipAll();

// ==================== Level & Experience ====================

/**
 * 플레이어의 레벨을 설정합니다.
 * @param NewLevel 새로운 레벨
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Level")
void HarmoniaSetLevel(int32 NewLevel);

/**
 * 플레이어에게 경험치를 지급합니다.
 * @param Amount 지급할 경험치
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Level")
void HarmoniaGiveXP(int32 Amount);

/**
 * 플레이어를 즉시 레벨업 시킵니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Level")
void HarmoniaLevelUp();

// ==================== Combat ====================

/**
 * 무적 모드를 토글합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
void HarmoniaToggleInvincible();

/**
 * 갓 모드를 토글합니다. (무적 + 무한 자원)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
void HarmoniaToggleGodMode();

/**
 * 원샷 킬 모드를 토글합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
void HarmoniaToggleOneHitKill();

/**
 * 플레이어의 공격력을 배수로 설정합니다.
 * @param Multiplier 데미지 배수 (1.0 = 기본, 2.0 = 2배)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Combat")
void HarmoniaSetDamageMultiplier(float Multiplier);

// ==================== Movement ====================

/**
 * 플레이어의 이동 속도를 배수로 설정합니다.
 * @param Multiplier 속도 배수 (1.0 = 기본, 2.0 = 2배)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
void HarmoniaSetSpeed(float Multiplier);

/**
 * 노클립 모드를 토글합니다. (벽 통과)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
void HarmoniaToggleNoClip();

/**
 * 플라이 모드를 토글합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
void HarmoniaToggleFly();

/**
 * 마커 위치로 텔레포트합니다.
 * @param MarkerName 마커 이름
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
void HarmoniaTeleportToMarker(const FString& MarkerName);

/**
 * 좌표로 텔레포트합니다.
 * @param X X 좌표
 * @param Y Y 좌표
 * @param Z Z 좌표
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Movement")
void HarmoniaTeleport(float X, float Y, float Z);

// ==================== Time & Weather ====================

/**
 * 게임 내 시간을 설정합니다.
 * @param Hour 시간 (0-23)
 * @param Minute 분 (0-59)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|World")
void HarmoniaSetTime(int32 Hour, int32 Minute);

/**
 * 시간 속도를 설정합니다.
 * @param Multiplier 시간 배수 (1.0 = 기본, 0 = 정지)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|World")
void HarmoniaSetTimeScale(float Multiplier);

/**
 * 날씨를 설정합니다.
 * @param WeatherType 날씨 타입 (Clear, Rain, Snow, Storm 등)
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|World")
void HarmoniaSetWeather(const FString& WeatherType);

// ==================== Quest & Achievement ====================

/**
 * 퀘스트를 완료합니다.
 * @param QuestName 퀘스트 이름 또는 ID
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Quest")
void HarmoniaCompleteQuest(const FString& QuestName);

/**
 * 모든 퀘스트를 완료합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Quest")
void HarmoniaCompleteAllQuests();

/**
 * 업적을 해제합니다.
 * @param AchievementName 업적 이름 또는 ID
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Achievement")
void HarmoniaUnlockAchievement(const FString& AchievementName);

// ==================== Debug ====================

/**
 * 디버그 정보를 화면에 표시합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaToggleDebugInfo();

/**
 * 스태미나 정보를 화면에 표시합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaDebugStamina();

/**
 * 전투 관련 정보(상태, 장비 등)를 화면에 표시합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaDebugCombat();

/**
 * 활성화된 모든 게임플레이 태그를 화면에 표시합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaDebugTags();

/**
 * 모든 디버그 정보를 화면에 표시합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaDebugAll();

/**
 * 모든 적을 제거합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaKillAllEnemies();

/**
 * 주변에 적을 스폰합니다.
 * @param EnemyName 적 이름 또는 ID
 * @param Count 스폰할 수
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaSpawnEnemy(const FString& EnemyName, int32 Count = 1);

/**
 * 모든 치트를 리셋합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaResetCheats();

/**
 * 사용 가능한 치트 명령어 목록을 출력합니다.
 */
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Debug")
void HarmoniaHelp();

// 헬퍼 함수들
class ACharacter* GetPlayerCharacter() const;
class UAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
void LogCheat(const FString& Message) const;

protected:
// 치트 상태 플래그 (UPROPERTY는 #if 블록 안에 있을 수 없음)
// Shipping 빌드에서도 변수 자체는 존재하지만 초기화/사용 코드는 제외됨
bool bInvincible = false;
bool bGodMode = false;
bool bOneHitKill = false;
bool bNoClip = false;
bool bShowDebugInfo = false;
float SpeedMultiplier = 1.0f;
float DamageMultiplier = 1.0f;
float TimeScale = 1.0f;
};
