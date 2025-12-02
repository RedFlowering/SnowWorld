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
	LogCheat(FString::Printf(TEXT("Attempting to set health to %.0f."), NewHealth));
	LogCheat(TEXT("This function needs to be implemented according to project's AttributeSet."));
	
	// Implementation example:
	// UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
	// if (ASC && ASC->HasAttributeSetForAttribute(UYourAttributeSet::GetHealthAttribute()))
	// {
	// 	   ASC->SetNumericAttributeBase(UYourAttributeSet::GetHealthAttribute(), NewHealth);
	// }
}

void UHarmoniaCheatManager::HarmoniaSetMaxHealth(float NewMaxHealth)
{
	LogCheat(FString::Printf(TEXT("최�? 체력??%.0f�??�정?�려�??�도?�습?�다."), NewMaxHealth));
	LogCheat(TEXT("??기능?� ?�로?�트??AttributeSet??맞게 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaHealFull()
{
	LogCheat(TEXT("체력???�전???�복?�려�??�도?�습?�다."));
	LogCheat(TEXT("??기능?� ?�로?�트??AttributeSet??맞게 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaSetMana(float NewMana)
{
	LogCheat(FString::Printf(TEXT("마나�?%.0f�??�정?�려�??�도?�습?�다."), NewMana));
	LogCheat(TEXT("??기능?� ?�로?�트??AttributeSet??맞게 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaSetMaxMana(float NewMaxMana)
{
	LogCheat(FString::Printf(TEXT("최�? 마나�?%.0f�??�정?�려�??�도?�습?�다."), NewMaxMana));
	LogCheat(TEXT("??기능?� ?�로?�트??AttributeSet??맞게 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaSetStamina(float NewStamina)
{
	LogCheat(FString::Printf(TEXT("?�태미나�?%.0f�??�정?�려�??�도?�습?�다."), NewStamina));
	LogCheat(TEXT("??기능?� ?�로?�트??AttributeSet??맞게 구현?�필?�합?�다."));
}

void UHarmoniaCheatManager::HarmoniaSetMaxStamina(float NewMaxStamina)
{
	LogCheat(FString::Printf(TEXT("최�? ?�태미나�?%.0f�??�정?�려�??�도?�습?�다."), NewMaxStamina));
	LogCheat(TEXT("??기능?� ?�로?�트??AttributeSet??맞게 구현???�요?�니??"));
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
				LogCheat(FString::Printf(TEXT("%d 골드�?지급했?�니??"), Amount));
			}
			else
			{
				CurrencyManager->RemoveCurrency(EHarmoniaCurrencyType::Gold, -Amount);
				LogCheat(FString::Printf(TEXT("%d 골드�??�거?�습?�다."), -Amount));
			}
		}
		else
		{
			LogCheat(TEXT("Currency Manager Component�?찾을 ???�습?�다."));
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
			LogCheat(FString::Printf(TEXT("골드�?%d�??�정?�습?�다."), Amount));
		}
		else
		{
			LogCheat(TEXT("Currency Manager Component�?찾을 ???�습?�다."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaGiveCurrency(const FString& CurrencyName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("?�화 '%s' %d개�? 지급하?�고 ?�도?�습?�다."), *CurrencyName, Amount));
	LogCheat(TEXT("??기능?� ?�로?�트???�정 ?�화 ?�?�에 맞게 구현???�요?�니??"));
}

// ==================== Items ====================

void UHarmoniaCheatManager::HarmoniaGiveItem(const FString& ItemName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("?�이??'%s' %d개�? 지급하?�고 ?�도?�습?�다."), *ItemName, Amount));
	LogCheat(TEXT("??기능?� ?�벤?�리 ?�스?�과 ?�동?�여 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaGiveAllItems()
{
	LogCheat(TEXT("모든 ?�이?�을 지급하?�고 ?�도?�습?�다."));
	LogCheat(TEXT("??기능?� Data Asset??모든 ?�이?�을 ?�회?�여 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaClearInventory()
{
	LogCheat(TEXT("?�벤?�리�??�리?�했?�니??"));
	LogCheat(TEXT("??기능?� ?�벤?�리 ?�스?�과 ?�동?�여 구현???�요?�니??"));
}

// ==================== Level & Experience ====================

void UHarmoniaCheatManager::HarmoniaSetLevel(int32 NewLevel)
{
	LogCheat(FString::Printf(TEXT("?�벨??%d�??�정?�습?�다."), NewLevel));
	LogCheat(TEXT("??기능?� ?�벨�??�스?�과 ?�동?�여 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaGiveXP(int32 Amount)
{
	LogCheat(FString::Printf(TEXT("%d 경험치�? 지급했?�니??"), Amount));
	LogCheat(TEXT("??기능?� 경험�??�스?�과 ?�동?�여 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaLevelUp()
{
	LogCheat(TEXT("?�벨?�을 ?�도?�습?�다."));
	LogCheat(TEXT("??기능?� ?�벨�??�스?�과 ?�동?�여 구현???�요?�니??"));
}

// ==================== Combat ====================

void UHarmoniaCheatManager::HarmoniaToggleInvincible()
{
	bInvincible = !bInvincible;
	LogCheat(FString::Printf(TEXT("무적 모드: %s"), bInvincible ? TEXT("켜짐") : TEXT("꺼짐")));
	
	// 무적 ?�태�??�용?�려�??��?지 처리 로직?�서 bInvincible ?�래그�? ?�인?�야 ?�니??
}

void UHarmoniaCheatManager::HarmoniaToggleGodMode()
{
	bGodMode = !bGodMode;
	LogCheat(FString::Printf(TEXT("??모드: %s"), bGodMode ? TEXT("켜짐") : TEXT("꺼짐")));
	
	if (bGodMode)
	{
		bInvincible = true;
		// ??모드?�서??체력, 마나, ?�태미나�?지?�적?�로 채워줘야 ?�니??
	}
	else
	{
		bInvincible = false;
	}
}

void UHarmoniaCheatManager::HarmoniaToggleOneHitKill()
{
	bOneHitKill = !bOneHitKill;
	LogCheat(FString::Printf(TEXT("?�샷 ?�킬 모드: %s"), bOneHitKill ? TEXT("켜짐") : TEXT("꺼짐")));
}

void UHarmoniaCheatManager::HarmoniaSetDamageMultiplier(float Multiplier)
{
	DamageMultiplier = FMath::Max(0.0f, Multiplier);
	LogCheat(FString::Printf(TEXT("?��?지 배수: %.1fx"), DamageMultiplier));
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
			// 기본 ?�도�??�?�하�?배수�??�용
			const float BaseSpeed = 600.0f; // ?�로?�트??맞게 조정
			MovementComp->MaxWalkSpeed = BaseSpeed * SpeedMultiplier;
			LogCheat(FString::Printf(TEXT("?�동 ?�도: %.1fx (%.0f)"), SpeedMultiplier, MovementComp->MaxWalkSpeed));
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
				LogCheat(TEXT("?�클�?모드: 켜짐"));
			}
			else
			{
				MovementComp->SetMovementMode(MOVE_Walking);
				PlayerChar->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
				LogCheat(TEXT("?�클�?모드: 꺼짐"));
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
				LogCheat(TEXT("비행 모드: 켜짐"));
			}
			else
			{
				MovementComp->SetMovementMode(MOVE_Walking);
				LogCheat(TEXT("비행 모드: 꺼짐"));
			}
		}
	}
}

void UHarmoniaCheatManager::HarmoniaTeleportToMarker(const FString& MarkerName)
{
	LogCheat(FString::Printf(TEXT("마커 '%s'�??�레?�트�??�도?�습?�다."), *MarkerName));
	LogCheat(TEXT("??기능?� ?�드??마커/?�이?�인???�스?�과 ?�동?�여 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaTeleport(float X, float Y, float Z)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		FVector NewLocation(X, Y, Z);
		PlayerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		LogCheat(FString::Printf(TEXT("좌표 (%.0f, %.0f, %.0f)�??�레?�트?�습?�다."), X, Y, Z));
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
			// ?�간 ?�정?� 블루?�린?�에??처리?�도�?delegate ?�출
			Hour = FMath::Clamp(Hour, 0, 23);
			Minute = FMath::Clamp(Minute, 0, 59);
			
			EHarmoniaTimeOfDay TimeOfDay = UHarmoniaTimeWeatherManager::HourToTimeOfDay(Hour);
			TimeWeatherManager->BroadcastTimeChange(TimeWeatherManager->GetCurrentTimeOfDay(), TimeOfDay, Hour, Minute);
			
			LogCheat(FString::Printf(TEXT("?�간??%02d:%02d�??�정?�습?�다."), Hour, Minute));
		}
		else
		{
			LogCheat(TEXT("Time Weather Manager�?찾을 ???�습?�다."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaSetTimeScale(float Multiplier)
{
	TimeScale = FMath::Max(0.0f, Multiplier);
	
	if (UWorld* World = GetWorld())
	{
		// ?�건 World???�간 ?�름??변경하??것이 ?�니?? 게임 ???�간 ?�스?�의 배속?�니??
		LogCheat(FString::Printf(TEXT("?�간 배속: %.1fx"), TimeScale));
		LogCheat(TEXT("?�제 ?�간 ?�름 변경�? ?�???�스?�에??TimeScale 값을 참조?�여 구현?�야 ?�니??"));
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
			LogCheat(FString::Printf(TEXT("?�씨�?'%s'�??�정?�습?�다."), *WeatherType));
		}
		else
		{
			LogCheat(TEXT("Time Weather Manager�?찾을 ???�습?�다."));
		}
	}
}

// ==================== Quest & Achievement ====================

void UHarmoniaCheatManager::HarmoniaCompleteQuest(const FString& QuestName)
{
	LogCheat(FString::Printf(TEXT("?�스??'%s'�??�료?�습?�다."), *QuestName));
	LogCheat(TEXT("??기능?� ?�스???�스?�과 ?�동?�여 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaCompleteAllQuests()
{
	LogCheat(TEXT("모든 ?�스?��? ?�료?�습?�다."));
	LogCheat(TEXT("??기능?� ?�스???�스?�과 ?�동?�여 구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaUnlockAchievement(const FString& AchievementName)
{
	LogCheat(FString::Printf(TEXT("?�적 '%s'�??�제?�습?�다."), *AchievementName));
	LogCheat(TEXT("??기능?� ?�적 ?�스?�과 ?�동?�여 구현???�요?�니??"));
}

// ==================== Debug ====================

void UHarmoniaCheatManager::HarmoniaToggleDebugInfo()
{
	bShowDebugInfo = !bShowDebugInfo;
	LogCheat(FString::Printf(TEXT("?�버�??�보 ?�시: %s"), bShowDebugInfo ? TEXT("켜짐") : TEXT("꺼짐")));
}

void UHarmoniaCheatManager::HarmoniaKillAllEnemies()
{
	LogCheat(TEXT("모든 ?�을 ?�거?�습?�다."));
	LogCheat(TEXT("??기능?� ???�터?�을 찾아???�거?�도�?구현???�요?�니??"));
}

void UHarmoniaCheatManager::HarmoniaSpawnEnemy(const FString& EnemyName, int32 Count)
{
	LogCheat(FString::Printf(TEXT("??'%s'�?%d마리 ?�폰?�습?�다."), *EnemyName, Count));
	LogCheat(TEXT("??기능?� ???�폰 ?�스?�과 ?�동?�여 구현???�요?�니??"));
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
	
	// ?�도 리셋
	HarmoniaSetSpeed(1.0f);
	
	LogCheat(TEXT("모든 치트�?리셋?�습?�다."));
}

void UHarmoniaCheatManager::HarmoniaHelp()
{
	LogCheat(TEXT("========== HarmoniaKit 치트 명령??목록 =========="));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 체력/마나/?�태미나 =="));
	LogCheat(TEXT("HarmoniaSetHealth <value> - Set health"));
	LogCheat(TEXT("HarmoniaSetMaxHealth <value> - Set max health"));
	LogCheat(TEXT("HarmoniaHealFull - Fully restore health"));
	LogCheat(TEXT("HarmoniaSetMana <value> - Set mana"));
	LogCheat(TEXT("HarmoniaSetStamina <value> - Set stamina"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Currency =="));
	LogCheat(TEXT("HarmoniaGiveGold <amount> - Give gold"));
	LogCheat(TEXT("HarmoniaSetGold <amount> - Set gold"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Items =="));
	LogCheat(TEXT("HarmoniaGiveItem <name> [quantity] - Give item"));
	LogCheat(TEXT("HarmoniaGiveAllItems - Give all items"));
	LogCheat(TEXT("HarmoniaClearInventory - Clear inventory"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Level/XP =="));
	LogCheat(TEXT("HarmoniaSetLevel <level> - Set level"));
	LogCheat(TEXT("HarmoniaGiveXP <amount> - Give XP"));
	LogCheat(TEXT("HarmoniaLevelUp - Level up"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Combat =="));
	LogCheat(TEXT("HarmoniaToggleInvincible - Toggle invincibility"));
	LogCheat(TEXT("HarmoniaToggleGodMode - Toggle god mode"));
	LogCheat(TEXT("HarmoniaToggleOneHitKill - Toggle one hit kill"));
	LogCheat(TEXT("HarmoniaSetDamageMultiplier <multiplier> - Set damage multiplier"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Movement =="));
	LogCheat(TEXT("HarmoniaSetSpeed <multiplier> - Set speed"));
	LogCheat(TEXT("HarmoniaToggleNoClip - Toggle no clip mode"));
	LogCheat(TEXT("HarmoniaToggleFly - Toggle fly mode"));
	LogCheat(TEXT("HarmoniaTeleport <X> <Y> <Z> - Teleport to coordinates"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Time/Weather =="));
	LogCheat(TEXT("HarmoniaSetTime <hour> <minute> - Set time"));
	LogCheat(TEXT("HarmoniaSetTimeScale <multiplier> - Set time scale"));
	LogCheat(TEXT("HarmoniaSetWeather <type> - Set weather (Clear/Rain/Snow/Storm)"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Misc =="));
	LogCheat(TEXT("HarmoniaToggleDebugInfo - Toggle debug info"));
	LogCheat(TEXT("HarmoniaResetCheats - Reset all cheats"));
	LogCheat(TEXT("HarmoniaHelp - Show this help"));
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
