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
	LogCheat(FString::Printf(TEXT("체력을 %.0f로 설정하려고 시도했습니다."), NewHealth));
	LogCheat(TEXT("이 기능은 프로젝트의 AttributeSet에 맞게 구현이 필요합니다."));
	
	// 프로젝트별 구현 예시:
	// UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
	// if (ASC && ASC->HasAttributeSetForAttribute(UYourAttributeSet::GetHealthAttribute()))
	// {
	// 	   ASC->SetNumericAttributeBase(UYourAttributeSet::GetHealthAttribute(), NewHealth);
	// }
}

void UHarmoniaCheatManager::HarmoniaSetMaxHealth(float NewMaxHealth)
{
	LogCheat(FString::Printf(TEXT("최대 체력을 %.0f로 설정하려고 시도했습니다."), NewMaxHealth));
	LogCheat(TEXT("이 기능은 프로젝트의 AttributeSet에 맞게 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaHealFull()
{
	LogCheat(TEXT("체력을 완전히 회복하려고 시도했습니다."));
	LogCheat(TEXT("이 기능은 프로젝트의 AttributeSet에 맞게 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaSetMana(float NewMana)
{
	LogCheat(FString::Printf(TEXT("마나를 %.0f로 설정하려고 시도했습니다."), NewMana));
	LogCheat(TEXT("이 기능은 프로젝트의 AttributeSet에 맞게 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaSetMaxMana(float NewMaxMana)
{
	LogCheat(FString::Printf(TEXT("최대 마나를 %.0f로 설정하려고 시도했습니다."), NewMaxMana));
	LogCheat(TEXT("이 기능은 프로젝트의 AttributeSet에 맞게 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaSetStamina(float NewStamina)
{
	LogCheat(FString::Printf(TEXT("스태미나를 %.0f로 설정하려고 시도했습니다."), NewStamina));
	LogCheat(TEXT("이 기능은 프로젝트의 AttributeSet에 맞게 구현이필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaSetMaxStamina(float NewMaxStamina)
{
	LogCheat(FString::Printf(TEXT("최대 스태미나를 %.0f로 설정하려고 시도했습니다."), NewMaxStamina));
	LogCheat(TEXT("이 기능은 프로젝트의 AttributeSet에 맞게 구현이 필요합니다."));
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
				LogCheat(FString::Printf(TEXT("%d 골드를 지급했습니다."), Amount));
			}
			else
			{
				CurrencyManager->RemoveCurrency(EHarmoniaCurrencyType::Gold, -Amount);
				LogCheat(FString::Printf(TEXT("%d 골드를 제거했습니다."), -Amount));
			}
		}
		else
		{
			LogCheat(TEXT("Currency Manager Component를 찾을 수 없습니다."));
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
			LogCheat(FString::Printf(TEXT("골드를 %d로 설정했습니다."), Amount));
		}
		else
		{
			LogCheat(TEXT("Currency Manager Component를 찾을 수 없습니다."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaGiveCurrency(const FString& CurrencyName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("통화 '%s' %d개를 지급하려고 시도했습니다."), *CurrencyName, Amount));
	LogCheat(TEXT("이 기능은 프로젝트의 특정 통화 타입에 맞게 구현이 필요합니다."));
}

// ==================== Items ====================

void UHarmoniaCheatManager::HarmoniaGiveItem(const FString& ItemName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("아이템 '%s' %d개를 지급하려고 시도했습니다."), *ItemName, Amount));
	LogCheat(TEXT("이 기능은 인벤토리 시스템과 연동하여 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaGiveAllItems()
{
	LogCheat(TEXT("모든 아이템을 지급하려고 시도했습니다."));
	LogCheat(TEXT("이 기능은 Data Asset의 모든 아이템을 순회하여 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaClearInventory()
{
	LogCheat(TEXT("인벤토리를 클리어했습니다."));
	LogCheat(TEXT("이 기능은 인벤토리 시스템과 연동하여 구현이 필요합니다."));
}

// ==================== Level & Experience ====================

void UHarmoniaCheatManager::HarmoniaSetLevel(int32 NewLevel)
{
	LogCheat(FString::Printf(TEXT("레벨을 %d로 설정했습니다."), NewLevel));
	LogCheat(TEXT("이 기능은 레벨링 시스템과 연동하여 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaGiveXP(int32 Amount)
{
	LogCheat(FString::Printf(TEXT("%d 경험치를 지급했습니다."), Amount));
	LogCheat(TEXT("이 기능은 경험치 시스템과 연동하여 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaLevelUp()
{
	LogCheat(TEXT("레벨업을 시도했습니다."));
	LogCheat(TEXT("이 기능은 레벨링 시스템과 연동하여 구현이 필요합니다."));
}

// ==================== Combat ====================

void UHarmoniaCheatManager::HarmoniaToggleInvincible()
{
	bInvincible = !bInvincible;
	LogCheat(FString::Printf(TEXT("무적 모드: %s"), bInvincible ? TEXT("켜짐") : TEXT("꺼짐")));
	
	// 무적 상태를 적용하려면 데미지 처리 로직에서 bInvincible 플래그를 확인해야 합니다.
}

void UHarmoniaCheatManager::HarmoniaToggleGodMode()
{
	bGodMode = !bGodMode;
	LogCheat(FString::Printf(TEXT("신 모드: %s"), bGodMode ? TEXT("켜짐") : TEXT("꺼짐")));
	
	if (bGodMode)
	{
		bInvincible = true;
		// 신 모드에서는 체력, 마나, 스태미나를 지속적으로 채워줘야 합니다.
	}
	else
	{
		bInvincible = false;
	}
}

void UHarmoniaCheatManager::HarmoniaToggleOneHitKill()
{
	bOneHitKill = !bOneHitKill;
	LogCheat(FString::Printf(TEXT("원샷 원킬 모드: %s"), bOneHitKill ? TEXT("켜짐") : TEXT("꺼짐")));
}

void UHarmoniaCheatManager::HarmoniaSetDamageMultiplier(float Multiplier)
{
	DamageMultiplier = FMath::Max(0.0f, Multiplier);
	LogCheat(FString::Printf(TEXT("데미지 배수: %.1fx"), DamageMultiplier));
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
			// 기본 속도를 저장하고 배수를 적용
			const float BaseSpeed = 600.0f; // 프로젝트에 맞게 조정
			MovementComp->MaxWalkSpeed = BaseSpeed * SpeedMultiplier;
			LogCheat(FString::Printf(TEXT("이동 속도: %.1fx (%.0f)"), SpeedMultiplier, MovementComp->MaxWalkSpeed));
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
				LogCheat(TEXT("노클립 모드: 켜짐"));
			}
			else
			{
				MovementComp->SetMovementMode(MOVE_Walking);
				PlayerChar->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
				LogCheat(TEXT("노클립 모드: 꺼짐"));
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
	LogCheat(FString::Printf(TEXT("마커 '%s'로 텔레포트를 시도했습니다."), *MarkerName));
	LogCheat(TEXT("이 기능은 월드의 마커/웨이포인트 시스템과 연동하여 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaTeleport(float X, float Y, float Z)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		FVector NewLocation(X, Y, Z);
		PlayerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		LogCheat(FString::Printf(TEXT("좌표 (%.0f, %.0f, %.0f)로 텔레포트했습니다."), X, Y, Z));
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
			// 시간 설정은 블루프린트에서 처리하도록 delegate 호출
			Hour = FMath::Clamp(Hour, 0, 23);
			Minute = FMath::Clamp(Minute, 0, 59);
			
			EHarmoniaTimeOfDay TimeOfDay = UHarmoniaTimeWeatherManager::HourToTimeOfDay(Hour);
			TimeWeatherManager->BroadcastTimeChange(TimeWeatherManager->GetCurrentTimeOfDay(), TimeOfDay, Hour, Minute);
			
			LogCheat(FString::Printf(TEXT("시간을 %02d:%02d로 설정했습니다."), Hour, Minute));
		}
		else
		{
			LogCheat(TEXT("Time Weather Manager를 찾을 수 없습니다."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaSetTimeScale(float Multiplier)
{
	TimeScale = FMath::Max(0.0f, Multiplier);
	
	if (UWorld* World = GetWorld())
	{
		// 이건 World의 시간 흐름을 변경하는 것이 아니라, 게임 내 시간 시스템의 배속입니다.
		LogCheat(FString::Printf(TEXT("시간 배속: %.1fx"), TimeScale));
		LogCheat(TEXT("실제 시간 흐름 변경은 타임 시스템에서 TimeScale 값을 참조하여 구현해야 합니다."));
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
			LogCheat(FString::Printf(TEXT("날씨를 '%s'로 설정했습니다."), *WeatherType));
		}
		else
		{
			LogCheat(TEXT("Time Weather Manager를 찾을 수 없습니다."));
		}
	}
}

// ==================== Quest & Achievement ====================

void UHarmoniaCheatManager::HarmoniaCompleteQuest(const FString& QuestName)
{
	LogCheat(FString::Printf(TEXT("퀘스트 '%s'를 완료했습니다."), *QuestName));
	LogCheat(TEXT("이 기능은 퀘스트 시스템과 연동하여 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaCompleteAllQuests()
{
	LogCheat(TEXT("모든 퀘스트를 완료했습니다."));
	LogCheat(TEXT("이 기능은 퀘스트 시스템과 연동하여 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaUnlockAchievement(const FString& AchievementName)
{
	LogCheat(FString::Printf(TEXT("업적 '%s'를 해제했습니다."), *AchievementName));
	LogCheat(TEXT("이 기능은 업적 시스템과 연동하여 구현이 필요합니다."));
}

// ==================== Debug ====================

void UHarmoniaCheatManager::HarmoniaToggleDebugInfo()
{
	bShowDebugInfo = !bShowDebugInfo;
	LogCheat(FString::Printf(TEXT("디버그 정보 표시: %s"), bShowDebugInfo ? TEXT("켜짐") : TEXT("꺼짐")));
}

void UHarmoniaCheatManager::HarmoniaKillAllEnemies()
{
	LogCheat(TEXT("모든 적을 제거했습니다."));
	LogCheat(TEXT("이 기능은 적 액터들을 찾아서 제거하도록 구현이 필요합니다."));
}

void UHarmoniaCheatManager::HarmoniaSpawnEnemy(const FString& EnemyName, int32 Count)
{
	LogCheat(FString::Printf(TEXT("적 '%s'를 %d마리 스폰했습니다."), *EnemyName, Count));
	LogCheat(TEXT("이 기능은 적 스폰 시스템과 연동하여 구현이 필요합니다."));
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
	
	// 속도 리셋
	HarmoniaSetSpeed(1.0f);
	
	LogCheat(TEXT("모든 치트를 리셋했습니다."));
}

void UHarmoniaCheatManager::HarmoniaHelp()
{
	LogCheat(TEXT("========== HarmoniaKit 치트 명령어 목록 =========="));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 체력/마나/스태미나 =="));
	LogCheat(TEXT("HarmoniaSetHealth <값> - 체력 설정"));
	LogCheat(TEXT("HarmoniaSetMaxHealth <값> - 최대 체력 설정"));
	LogCheat(TEXT("HarmoniaHealFull - 체력 완전 회복"));
	LogCheat(TEXT("HarmoniaSetMana <값> - 마나 설정"));
	LogCheat(TEXT("HarmoniaSetStamina <값> - 스태미나 설정"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 재화 =="));
	LogCheat(TEXT("HarmoniaGiveGold <양> - 골드 지급"));
	LogCheat(TEXT("HarmoniaSetGold <양> - 골드 설정"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 아이템 =="));
	LogCheat(TEXT("HarmoniaGiveItem <이름> [수량] - 아이템 지급"));
	LogCheat(TEXT("HarmoniaGiveAllItems - 모든 아이템 지급"));
	LogCheat(TEXT("HarmoniaClearInventory - 인벤토리 클리어"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 레벨/경험치 =="));
	LogCheat(TEXT("HarmoniaSetLevel <레벨> - 레벨 설정"));
	LogCheat(TEXT("HarmoniaGiveXP <양> - 경험치 지급"));
	LogCheat(TEXT("HarmoniaLevelUp - 레벨업"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 전투 =="));
	LogCheat(TEXT("HarmoniaToggleInvincible - 무적 모드 토글"));
	LogCheat(TEXT("HarmoniaToggleGodMode - 신 모드 토글"));
	LogCheat(TEXT("HarmoniaToggleOneHitKill - 원킬 모드 토글"));
	LogCheat(TEXT("HarmoniaSetDamageMultiplier <배수> - 데미지 배수 설정"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 이동 =="));
	LogCheat(TEXT("HarmoniaSetSpeed <배수> - 이동 속도 설정"));
	LogCheat(TEXT("HarmoniaToggleNoClip - 노클립 모드 토글"));
	LogCheat(TEXT("HarmoniaToggleFly - 비행 모드 토글"));
	LogCheat(TEXT("HarmoniaTeleport <X> <Y> <Z> - 좌표로 텔레포트"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 시간/날씨 =="));
	LogCheat(TEXT("HarmoniaSetTime <시> <분> - 시간 설정"));
	LogCheat(TEXT("HarmoniaSetTimeScale <배수> - 시간 흐름 속도"));
	LogCheat(TEXT("HarmoniaSetWeather <타입> - 날씨 설정 (Clear/Rain/Snow/Storm 등)"));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== 기타 =="));
	LogCheat(TEXT("HarmoniaToggleDebugInfo - 디버그 정보 표시"));
	LogCheat(TEXT("HarmoniaResetCheats - 모든 치트 리셋"));
	LogCheat(TEXT("HarmoniaHelp - 이 도움말 표시"));
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
