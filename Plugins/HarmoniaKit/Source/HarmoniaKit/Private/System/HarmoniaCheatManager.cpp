// Copyright Snow Game Studio. All Rights Reserved.

#include "System/HarmoniaCheatManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/HarmoniaCurrencyManagerComponent.h"
#include "System/HarmoniaTimeWeatherManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UHarmoniaCheatManager::UHarmoniaCheatManager()
{
	bInvincible = false;
	bGodMode = false;
	bOneHitKill = false;
	bNoClip = false;
	bShowDebugInfo = false;
	SpeedMultiplier = 1.0f;
	DamageMultiplier = 1.0f;
	TimeScale = 1.0f;
}

// ==================== Health, Mana, Stamina ====================

void UHarmoniaCheatManager::HarmoniaSetHealth(float NewHealth)
{
	LogCheat(FString::Printf(TEXT("ì²´ë ¥??%.0fë¡??¤ì •?˜ë ¤ê³??œë„?ˆìŠµ?ˆë‹¤."), NewHealth));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸??AttributeSet??ë§ê²Œ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
	
	// ?„ë¡œ?íŠ¸ë³?êµ¬í˜„ ?ˆì‹œ:
	// UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
	// if (ASC && ASC->HasAttributeSetForAttribute(UYourAttributeSet::GetHealthAttribute()))
	// {
	// 	   ASC->SetNumericAttributeBase(UYourAttributeSet::GetHealthAttribute(), NewHealth);
	// }
}

void UHarmoniaCheatManager::HarmoniaSetMaxHealth(float NewMaxHealth)
{
	LogCheat(FString::Printf(TEXT("ìµœë? ì²´ë ¥??%.0fë¡??¤ì •?˜ë ¤ê³??œë„?ˆìŠµ?ˆë‹¤."), NewMaxHealth));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸??AttributeSet??ë§ê²Œ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaHealFull()
{
	LogCheat(TEXT("ì²´ë ¥???„ì „???Œë³µ?˜ë ¤ê³??œë„?ˆìŠµ?ˆë‹¤."));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸??AttributeSet??ë§ê²Œ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaSetMana(float NewMana)
{
	LogCheat(FString::Printf(TEXT("ë§ˆë‚˜ë¥?%.0fë¡??¤ì •?˜ë ¤ê³??œë„?ˆìŠµ?ˆë‹¤."), NewMana));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸??AttributeSet??ë§ê²Œ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaSetMaxMana(float NewMaxMana)
{
	LogCheat(FString::Printf(TEXT("ìµœë? ë§ˆë‚˜ë¥?%.0fë¡??¤ì •?˜ë ¤ê³??œë„?ˆìŠµ?ˆë‹¤."), NewMaxMana));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸??AttributeSet??ë§ê²Œ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaSetStamina(float NewStamina)
{
	LogCheat(FString::Printf(TEXT("?¤íƒœë¯¸ë‚˜ë¥?%.0fë¡??¤ì •?˜ë ¤ê³??œë„?ˆìŠµ?ˆë‹¤."), NewStamina));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸??AttributeSet??ë§ê²Œ êµ¬í˜„?´í•„?”í•©?ˆë‹¤."));
}

void UHarmoniaCheatManager::HarmoniaSetMaxStamina(float NewMaxStamina)
{
	LogCheat(FString::Printf(TEXT("ìµœë? ?¤íƒœë¯¸ë‚˜ë¥?%.0fë¡??¤ì •?˜ë ¤ê³??œë„?ˆìŠµ?ˆë‹¤."), NewMaxStamina));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸??AttributeSet??ë§ê²Œ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

// ==================== Currency ====================

void UHarmoniaCheatManager::HarmoniaGiveGold(int32 Amount)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UHarmoniaCurrencyManagerComponent* CurrencyManager = PlayerChar->FindComponentByClass<UHarmoniaCurrencyManagerComponent>();
		if (CurrencyManager)
		{
			if (Amount >= 0)
			{
				CurrencyManager->AddCurrency(EHarmoniaCurrencyType::Gold, Amount);
				LogCheat(FString::Printf(TEXT("%d ê³¨ë“œë¥?ì§€ê¸‰í–ˆ?µë‹ˆ??"), Amount));
			}
			else
			{
				CurrencyManager->RemoveCurrency(EHarmoniaCurrencyType::Gold, -Amount);
				LogCheat(FString::Printf(TEXT("%d ê³¨ë“œë¥??œê±°?ˆìŠµ?ˆë‹¤."), -Amount));
			}
		}
		else
		{
			LogCheat(TEXT("Currency Manager Componentë¥?ì°¾ì„ ???†ìŠµ?ˆë‹¤."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaSetGold(int32 Amount)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UHarmoniaCurrencyManagerComponent* CurrencyManager = PlayerChar->FindComponentByClass<UHarmoniaCurrencyManagerComponent>();
		if (CurrencyManager)
		{
			CurrencyManager->SetCurrency(EHarmoniaCurrencyType::Gold, Amount);
			LogCheat(FString::Printf(TEXT("ê³¨ë“œë¥?%dë¡??¤ì •?ˆìŠµ?ˆë‹¤."), Amount));
		}
		else
		{
			LogCheat(TEXT("Currency Manager Componentë¥?ì°¾ì„ ???†ìŠµ?ˆë‹¤."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaGiveCurrency(const FString& CurrencyName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("?µí™” '%s' %dê°œë? ì§€ê¸‰í•˜?¤ê³  ?œë„?ˆìŠµ?ˆë‹¤."), *CurrencyName, Amount));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?„ë¡œ?íŠ¸???¹ì • ?µí™” ?€?…ì— ë§ê²Œ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

// ==================== Items ====================

void UHarmoniaCheatManager::HarmoniaGiveItem(const FString& ItemName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("?„ì´??'%s' %dê°œë? ì§€ê¸‰í•˜?¤ê³  ?œë„?ˆìŠµ?ˆë‹¤."), *ItemName, Amount));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?¸ë²¤? ë¦¬ ?œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaGiveAllItems()
{
	LogCheat(TEXT("ëª¨ë“  ?„ì´?œì„ ì§€ê¸‰í•˜?¤ê³  ?œë„?ˆìŠµ?ˆë‹¤."));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ Data Asset??ëª¨ë“  ?„ì´?œì„ ?œíšŒ?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaClearInventory()
{
	LogCheat(TEXT("?¸ë²¤? ë¦¬ë¥??´ë¦¬?´í–ˆ?µë‹ˆ??"));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?¸ë²¤? ë¦¬ ?œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

// ==================== Level & Experience ====================

void UHarmoniaCheatManager::HarmoniaSetLevel(int32 NewLevel)
{
	LogCheat(FString::Printf(TEXT("?ˆë²¨??%dë¡??¤ì •?ˆìŠµ?ˆë‹¤."), NewLevel));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?ˆë²¨ë§??œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaGiveXP(int32 Amount)
{
	LogCheat(FString::Printf(TEXT("%d ê²½í—˜ì¹˜ë? ì§€ê¸‰í–ˆ?µë‹ˆ??"), Amount));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ê²½í—˜ì¹??œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaLevelUp()
{
	LogCheat(TEXT("?ˆë²¨?…ì„ ?œë„?ˆìŠµ?ˆë‹¤."));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?ˆë²¨ë§??œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

// ==================== Combat ====================

void UHarmoniaCheatManager::HarmoniaToggleInvincible()
{
	bInvincible = !bInvincible;
	LogCheat(FString::Printf(TEXT("ë¬´ì  ëª¨ë“œ: %s"), bInvincible ? TEXT("ì¼œì§") : TEXT("êº¼ì§")));
	
	// ë¬´ì  ?íƒœë¥??ìš©?˜ë ¤ë©??°ë?ì§€ ì²˜ë¦¬ ë¡œì§?ì„œ bInvincible ?Œë˜ê·¸ë? ?•ì¸?´ì•¼ ?©ë‹ˆ??
}

void UHarmoniaCheatManager::HarmoniaToggleGodMode()
{
	bGodMode = !bGodMode;
	LogCheat(FString::Printf(TEXT("??ëª¨ë“œ: %s"), bGodMode ? TEXT("ì¼œì§") : TEXT("êº¼ì§")));
	
	if (bGodMode)
	{
		bInvincible = true;
		// ??ëª¨ë“œ?ì„œ??ì²´ë ¥, ë§ˆë‚˜, ?¤íƒœë¯¸ë‚˜ë¥?ì§€?ì ?¼ë¡œ ì±„ì›Œì¤˜ì•¼ ?©ë‹ˆ??
	}
	else
	{
		bInvincible = false;
	}
}

void UHarmoniaCheatManager::HarmoniaToggleOneHitKill()
{
	bOneHitKill = !bOneHitKill;
	LogCheat(FString::Printf(TEXT("?ìƒ· ?í‚¬ ëª¨ë“œ: %s"), bOneHitKill ? TEXT("ì¼œì§") : TEXT("êº¼ì§")));
}

void UHarmoniaCheatManager::HarmoniaSetDamageMultiplier(float Multiplier)
{
	DamageMultiplier = FMath::Max(0.0f, Multiplier);
	LogCheat(FString::Printf(TEXT("?°ë?ì§€ ë°°ìˆ˜: %.1fx"), DamageMultiplier));
}

// ==================== Movement ====================

void UHarmoniaCheatManager::HarmoniaSetSpeed(float Multiplier)
{
	SpeedMultiplier = FMath::Max(0.1f, Multiplier);
	
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement();
		if (MovementComp)
		{
			// ê¸°ë³¸ ?ë„ë¥??€?¥í•˜ê³?ë°°ìˆ˜ë¥??ìš©
			const float BaseSpeed = 600.0f; // ?„ë¡œ?íŠ¸??ë§ê²Œ ì¡°ì •
			MovementComp->MaxWalkSpeed = BaseSpeed * SpeedMultiplier;
			LogCheat(FString::Printf(TEXT("?´ë™ ?ë„: %.1fx (%.0f)"), SpeedMultiplier, MovementComp->MaxWalkSpeed));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaToggleNoClip()
{
	bNoClip = !bNoClip;
	
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement();
		if (MovementComp)
		{
			if (bNoClip)
			{
				MovementComp->SetMovementMode(MOVE_Flying);
				PlayerChar->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
				LogCheat(TEXT("?¸í´ë¦?ëª¨ë“œ: ì¼œì§"));
			}
			else
			{
				MovementComp->SetMovementMode(MOVE_Walking);
				PlayerChar->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
				LogCheat(TEXT("?¸í´ë¦?ëª¨ë“œ: êº¼ì§"));
			}
		}
	}
}

void UHarmoniaCheatManager::HarmoniaToggleFly()
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement();
		if (MovementComp)
		{
			if (MovementComp->MovementMode == MOVE_Walking)
			{
				MovementComp->SetMovementMode(MOVE_Flying);
				LogCheat(TEXT("ë¹„í–‰ ëª¨ë“œ: ì¼œì§"));
			}
			else
			{
				MovementComp->SetMovementMode(MOVE_Walking);
				LogCheat(TEXT("ë¹„í–‰ ëª¨ë“œ: êº¼ì§"));
			}
		}
	}
}

void UHarmoniaCheatManager::HarmoniaTeleportToMarker(const FString& MarkerName)
{
	LogCheat(FString::Printf(TEXT("ë§ˆì»¤ '%s'ë¡??”ë ˆ?¬íŠ¸ë¥??œë„?ˆìŠµ?ˆë‹¤."), *MarkerName));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?”ë“œ??ë§ˆì»¤/?¨ì´?¬ì¸???œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaTeleport(float X, float Y, float Z)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		FVector NewLocation(X, Y, Z);
		PlayerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		LogCheat(FString::Printf(TEXT("ì¢Œí‘œ (%.0f, %.0f, %.0f)ë¡??”ë ˆ?¬íŠ¸?ˆìŠµ?ˆë‹¤."), X, Y, Z));
	}
}

// ==================== Time & Weather ====================

void UHarmoniaCheatManager::HarmoniaSetTime(int32 Hour, int32 Minute)
{
	if (UWorld* World = GetWorld())
	{
		UHarmoniaTimeWeatherManager* TimeWeatherManager = World->GetSubsystem<UHarmoniaTimeWeatherManager>();
		if (TimeWeatherManager)
		{
			// ?œê°„ ?¤ì •?€ ë¸”ë£¨?„ë¦°?¸ì—??ì²˜ë¦¬?˜ë„ë¡?delegate ?¸ì¶œ
			Hour = FMath::Clamp(Hour, 0, 23);
			Minute = FMath::Clamp(Minute, 0, 59);
			
			EHarmoniaTimeOfDay TimeOfDay = UHarmoniaTimeWeatherManager::HourToTimeOfDay(Hour);
			TimeWeatherManager->BroadcastTimeChange(TimeWeatherManager->GetCurrentTimeOfDay(), TimeOfDay, Hour, Minute);
			
			LogCheat(FString::Printf(TEXT("?œê°„??%02d:%02dë¡??¤ì •?ˆìŠµ?ˆë‹¤."), Hour, Minute));
		}
		else
		{
			LogCheat(TEXT("Time Weather Managerë¥?ì°¾ì„ ???†ìŠµ?ˆë‹¤."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaSetTimeScale(float Multiplier)
{
	TimeScale = FMath::Max(0.0f, Multiplier);
	
	if (UWorld* World = GetWorld())
	{
		// ?´ê±´ World???œê°„ ?ë¦„??ë³€ê²½í•˜??ê²ƒì´ ?„ë‹ˆ?? ê²Œì„ ???œê°„ ?œìŠ¤?œì˜ ë°°ì†?…ë‹ˆ??
		LogCheat(FString::Printf(TEXT("?œê°„ ë°°ì†: %.1fx"), TimeScale));
		LogCheat(TEXT("?¤ì œ ?œê°„ ?ë¦„ ë³€ê²½ì? ?€???œìŠ¤?œì—??TimeScale ê°’ì„ ì°¸ì¡°?˜ì—¬ êµ¬í˜„?´ì•¼ ?©ë‹ˆ??"));
	}
}

void UHarmoniaCheatManager::HarmoniaSetWeather(const FString& WeatherType)
{
	if (UWorld* World = GetWorld())
	{
		UHarmoniaTimeWeatherManager* TimeWeatherManager = World->GetSubsystem<UHarmoniaTimeWeatherManager>();
		if (TimeWeatherManager)
		{
			EHarmoniaWeatherType NewWeather = EHarmoniaWeatherType::Clear;
			
			if (WeatherType.Equals(TEXT("Clear"), ESearchCase::IgnoreCase))
			{
				NewWeather = EHarmoniaWeatherType::Clear;
			}
			else if (WeatherType.Equals(TEXT("Cloudy"), ESearchCase::IgnoreCase))
			{
				NewWeather = EHarmoniaWeatherType::Cloudy;
			}
			else if (WeatherType.Equals(TEXT("Rain"), ESearchCase::IgnoreCase) || WeatherType.Equals(TEXT("Rainy"), ESearchCase::IgnoreCase))
			{
				NewWeather = EHarmoniaWeatherType::Rainy;
			}
			else if (WeatherType.Equals(TEXT("Storm"), ESearchCase::IgnoreCase) || WeatherType.Equals(TEXT("Stormy"), ESearchCase::IgnoreCase))
			{
				NewWeather = EHarmoniaWeatherType::Stormy;
			}
			else if (WeatherType.Equals(TEXT("Fog"), ESearchCase::IgnoreCase) || WeatherType.Equals(TEXT("Foggy"), ESearchCase::IgnoreCase))
			{
				NewWeather = EHarmoniaWeatherType::Foggy;
			}
			else if (WeatherType.Equals(TEXT("Snow"), ESearchCase::IgnoreCase) || WeatherType.Equals(TEXT("Snowy"), ESearchCase::IgnoreCase))
			{
				NewWeather = EHarmoniaWeatherType::Snowy;
			}
			else if (WeatherType.Equals(TEXT("Blizzard"), ESearchCase::IgnoreCase))
			{
				NewWeather = EHarmoniaWeatherType::Blizzard;
			}
			
			TimeWeatherManager->BroadcastWeatherChange(TimeWeatherManager->GetCurrentWeather(), NewWeather, 1.0f, 5.0f);
			LogCheat(FString::Printf(TEXT("? ì”¨ë¥?'%s'ë¡??¤ì •?ˆìŠµ?ˆë‹¤."), *WeatherType));
		}
		else
		{
			LogCheat(TEXT("Time Weather Managerë¥?ì°¾ì„ ???†ìŠµ?ˆë‹¤."));
		}
	}
}

// ==================== Quest & Achievement ====================

void UHarmoniaCheatManager::HarmoniaCompleteQuest(const FString& QuestName)
{
	LogCheat(FString::Printf(TEXT("?˜ìŠ¤??'%s'ë¥??„ë£Œ?ˆìŠµ?ˆë‹¤."), *QuestName));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?˜ìŠ¤???œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaCompleteAllQuests()
{
	LogCheat(TEXT("ëª¨ë“  ?˜ìŠ¤?¸ë? ?„ë£Œ?ˆìŠµ?ˆë‹¤."));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?˜ìŠ¤???œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaUnlockAchievement(const FString& AchievementName)
{
	LogCheat(FString::Printf(TEXT("?…ì  '%s'ë¥??´ì œ?ˆìŠµ?ˆë‹¤."), *AchievementName));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ?…ì  ?œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

// ==================== Debug ====================

void UHarmoniaCheatManager::HarmoniaToggleDebugInfo()
{
	bShowDebugInfo = !bShowDebugInfo;
	LogCheat(FString::Printf(TEXT("?”ë²„ê·??•ë³´ ?œì‹œ: %s"), bShowDebugInfo ? TEXT("ì¼œì§") : TEXT("êº¼ì§")));
}

void UHarmoniaCheatManager::HarmoniaKillAllEnemies()
{
	LogCheat(TEXT("ëª¨ë“  ?ì„ ?œê±°?ˆìŠµ?ˆë‹¤."));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ???¡í„°?¤ì„ ì°¾ì•„???œê±°?˜ë„ë¡?êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaSpawnEnemy(const FString& EnemyName, int32 Count)
{
	LogCheat(FString::Printf(TEXT("??'%s'ë¥?%dë§ˆë¦¬ ?¤í°?ˆìŠµ?ˆë‹¤."), *EnemyName, Count));
	LogCheat(TEXT("??ê¸°ëŠ¥?€ ???¤í° ?œìŠ¤?œê³¼ ?°ë™?˜ì—¬ êµ¬í˜„???„ìš”?©ë‹ˆ??"));
}

void UHarmoniaCheatManager::HarmoniaResetCheats()
{
	bInvincible = false;
	bGodMode = false;
	bOneHitKill = false;
	bNoClip = false;
	bShowDebugInfo = false;
	SpeedMultiplier = 1.0f;
	DamageMultiplier = 1.0f;
	TimeScale = 1.0f;
	
	// ?ë„ ë¦¬ì…‹
	HarmoniaSetSpeed(1.0f);
	
	LogCheat(TEXT("ëª¨ë“  ì¹˜íŠ¸ë¥?ë¦¬ì…‹?ˆìŠµ?ˆë‹¤."));
}

void UHarmoniaCheatManager::HarmoniaHelp()
{
	LogCheat(TEXT("========== HarmoniaKit ì¹˜íŠ¸ ëª…ë ¹??ëª©ë¡ =========="));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ì²´ë ¥/ë§ˆë‚˜/?¤íƒœë¯¸ë‚˜ =="));
	LogCheat(TEXT("HarmoniaSetHealth <ê°? - ì²´ë ¥ ?¤ì •"));
	LogCheat(TEXT("HarmoniaSetMaxHealth <ê°? - ìµœë? ì²´ë ¥ ?¤ì •"));
	LogCheat(TEXT("HarmoniaHealFull - ì²´ë ¥ ?„ì „ ?Œë³µ"));
	LogCheat(TEXT("HarmoniaSetMana <ê°? - ë§ˆë‚˜ ?¤ì •"));
	LogCheat(TEXT("HarmoniaSetStamina <ê°? - ?¤íƒœë¯¸ë‚˜ ?¤ì •"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ?¬í™” =="));
	LogCheat(TEXT("HarmoniaGiveGold <?? - ê³¨ë“œ ì§€ê¸?));
	LogCheat(TEXT("HarmoniaSetGold <?? - ê³¨ë“œ ?¤ì •"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ?„ì´??=="));
	LogCheat(TEXT("HarmoniaGiveItem <?´ë¦„> [?˜ëŸ‰] - ?„ì´??ì§€ê¸?));
	LogCheat(TEXT("HarmoniaGiveAllItems - ëª¨ë“  ?„ì´??ì§€ê¸?));
	LogCheat(TEXT("HarmoniaClearInventory - ?¸ë²¤? ë¦¬ ?´ë¦¬??));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ?ˆë²¨/ê²½í—˜ì¹?=="));
	LogCheat(TEXT("HarmoniaSetLevel <?ˆë²¨> - ?ˆë²¨ ?¤ì •"));
	LogCheat(TEXT("HarmoniaGiveXP <?? - ê²½í—˜ì¹?ì§€ê¸?));
	LogCheat(TEXT("HarmoniaLevelUp - ?ˆë²¨??));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ?„íˆ¬ =="));
	LogCheat(TEXT("HarmoniaToggleInvincible - ë¬´ì  ëª¨ë“œ ? ê?"));
	LogCheat(TEXT("HarmoniaToggleGodMode - ??ëª¨ë“œ ? ê?"));
	LogCheat(TEXT("HarmoniaToggleOneHitKill - ?í‚¬ ëª¨ë“œ ? ê?"));
	LogCheat(TEXT("HarmoniaSetDamageMultiplier <ë°°ìˆ˜> - ?°ë?ì§€ ë°°ìˆ˜ ?¤ì •"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ?´ë™ =="));
	LogCheat(TEXT("HarmoniaSetSpeed <ë°°ìˆ˜> - ?´ë™ ?ë„ ?¤ì •"));
	LogCheat(TEXT("HarmoniaToggleNoClip - ?¸í´ë¦?ëª¨ë“œ ? ê?"));
	LogCheat(TEXT("HarmoniaToggleFly - ë¹„í–‰ ëª¨ë“œ ? ê?"));
	LogCheat(TEXT("HarmoniaTeleport <X> <Y> <Z> - ì¢Œí‘œë¡??”ë ˆ?¬íŠ¸"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ?œê°„/? ì”¨ =="));
	LogCheat(TEXT("HarmoniaSetTime <?? <ë¶? - ?œê°„ ?¤ì •"));
	LogCheat(TEXT("HarmoniaSetTimeScale <ë°°ìˆ˜> - ?œê°„ ?ë¦„ ?ë„"));
	LogCheat(TEXT("HarmoniaSetWeather <?€?? - ? ì”¨ ?¤ì • (Clear/Rain/Snow/Storm ??"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== ê¸°í? =="));
	LogCheat(TEXT("HarmoniaToggleDebugInfo - ?”ë²„ê·??•ë³´ ?œì‹œ"));
	LogCheat(TEXT("HarmoniaResetCheats - ëª¨ë“  ì¹˜íŠ¸ ë¦¬ì…‹"));
	LogCheat(TEXT("HarmoniaHelp - ???„ì?ë§??œì‹œ"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("================================================"));
}

// ==================== Helper Functions ====================

ACharacter* UHarmoniaCheatManager::GetPlayerCharacter() const
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return Cast<ACharacter>(PC->GetPawn());
		}
	}
	return nullptr;
}

UAbilitySystemComponent* UHarmoniaCheatManager::GetPlayerAbilitySystemComponent() const
{
	if (ACharacter* PlayerChar = GetPlayerCharacter())
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerChar);
	}
	return nullptr;
}

void UHarmoniaCheatManager::LogCheat(const FString& Message) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("[Harmonia Cheat] %s"), *Message));
	}
	UE_LOG(LogTemp, Log, TEXT("[Harmonia Cheat] %s"), *Message);
}
