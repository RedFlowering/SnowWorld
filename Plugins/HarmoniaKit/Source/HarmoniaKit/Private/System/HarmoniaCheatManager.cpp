// Copyright Snow Game Studio. All Rights Reserved.

#include "System/HarmoniaCheatManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/HarmoniaCurrencyManagerComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "System/HarmoniaTimeWeatherManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UHarmoniaCheatManager::UHarmoniaCheatManager()
{
#if !UE_BUILD_SHIPPING
	bInvincible = false;
	bGodMode = false;
	bOneHitKill = false;
	bNoClip = false;
	bShowDebugInfo = false;
	SpeedMultiplier = 1.0f;
	DamageMultiplier = 1.0f;
	TimeScale = 1.0f;
#endif
}

#if !UE_BUILD_SHIPPING
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
	LogCheat(FString::Printf(TEXT("Attempting to set max health to %.0f."), NewMaxHealth));
	LogCheat(TEXT("This feature requires implementation with project's AttributeSet."));
}

void UHarmoniaCheatManager::HarmoniaHealFull()
{
	LogCheat(TEXT("Attempting to fully restore health."));
	LogCheat(TEXT("This feature requires implementation with project's AttributeSet."));
}

void UHarmoniaCheatManager::HarmoniaSetMana(float NewMana)
{
	LogCheat(FString::Printf(TEXT("Attempting to set mana to %.0f."), NewMana));
	LogCheat(TEXT("This feature requires implementation with project's AttributeSet."));
}

void UHarmoniaCheatManager::HarmoniaSetMaxMana(float NewMaxMana)
{
	LogCheat(FString::Printf(TEXT("Attempting to set max mana to %.0f."), NewMaxMana));
	LogCheat(TEXT("This feature requires implementation with project's AttributeSet."));
}

void UHarmoniaCheatManager::HarmoniaSetStamina(float NewStamina)
{
	LogCheat(FString::Printf(TEXT("Attempting to set stamina to %.0f."), NewStamina));
	LogCheat(TEXT("This feature requires implementation with project's AttributeSet."));
}

void UHarmoniaCheatManager::HarmoniaSetMaxStamina(float NewMaxStamina)
{
	LogCheat(FString::Printf(TEXT("Attempting to set max stamina to %.0f."), NewMaxStamina));
	LogCheat(TEXT("This feature requires implementation with project's AttributeSet."));
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
				LogCheat(FString::Printf(TEXT("Granted %d gold."), Amount));
			}
			else
			{
				CurrencyManager->RemoveCurrency(EHarmoniaCurrencyType::Gold, -Amount);
				LogCheat(FString::Printf(TEXT("Removed %d gold."), -Amount));
			}
		}
		else
		{
			LogCheat(TEXT("Currency Manager Component not found."));
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
			LogCheat(FString::Printf(TEXT("Set gold to %d."), Amount));
		}
		else
		{
			LogCheat(TEXT("Currency Manager Component not found."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaGiveCurrency(const FString& CurrencyName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("Attempting to grant %d of currency '%s'."), Amount, *CurrencyName));
	LogCheat(TEXT("This feature requires implementation with project's specific currency type."));
}

// ==================== Items ====================

void UHarmoniaCheatManager::HarmoniaGiveItem(const FString& ItemName, int32 Amount)
{
	LogCheat(FString::Printf(TEXT("Attempting to grant %d of item '%s'."), Amount, *ItemName));
	LogCheat(TEXT("This feature requires implementation with inventory system."));
}

void UHarmoniaCheatManager::HarmoniaGiveAllItems()
{
	LogCheat(TEXT("Attempting to grant all items."));
	LogCheat(TEXT("This feature requires implementation by iterating all items from Data Asset."));
}

void UHarmoniaCheatManager::HarmoniaClearInventory()
{
	LogCheat(TEXT("Inventory cleared."));
	LogCheat(TEXT("This feature requires implementation with inventory system."));
}

// ==================== Equipment ====================

void UHarmoniaCheatManager::HarmoniaEquipItem(const FString& EquipmentId)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UHarmoniaEquipmentComponent* EquipmentComp = PlayerChar->FindComponentByClass<UHarmoniaEquipmentComponent>();
		if (EquipmentComp)
		{
			FHarmoniaID Id;
			Id.Id = FName(*EquipmentId);
			
			if (EquipmentComp->EquipItem(Id))
			{
				LogCheat(FString::Printf(TEXT("Equipped item '%s'."), *EquipmentId));
			}
			else
			{
				LogCheat(FString::Printf(TEXT("Failed to equip item '%s'. Check if ID exists in DataTable."), *EquipmentId));
			}
		}
		else
		{
			LogCheat(TEXT("Equipment Component not found on player."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaUnequipSlot(const FString& SlotName)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UHarmoniaEquipmentComponent* EquipmentComp = PlayerChar->FindComponentByClass<UHarmoniaEquipmentComponent>();
		if (EquipmentComp)
		{
			EEquipmentSlot Slot = EEquipmentSlot::None;
			
			if (SlotName.Equals(TEXT("Head"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Head;
			else if (SlotName.Equals(TEXT("Chest"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Chest;
			else if (SlotName.Equals(TEXT("Legs"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Legs;
			else if (SlotName.Equals(TEXT("Feet"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Feet;
			else if (SlotName.Equals(TEXT("Hands"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Hands;
			else if (SlotName.Equals(TEXT("MainHand"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::MainHand;
			else if (SlotName.Equals(TEXT("OffHand"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::OffHand;
			else if (SlotName.Equals(TEXT("Accessory1"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Accessory1;
			else if (SlotName.Equals(TEXT("Accessory2"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Accessory2;
			else if (SlotName.Equals(TEXT("Back"), ESearchCase::IgnoreCase)) Slot = EEquipmentSlot::Back;
			else
			{
				LogCheat(FString::Printf(TEXT("Unknown slot '%s'. Valid: Head, Chest, Legs, Feet, Hands, MainHand, OffHand, Accessory1, Accessory2, Back"), *SlotName));
				return;
			}
			
			if (EquipmentComp->UnequipItem(Slot))
			{
				LogCheat(FString::Printf(TEXT("Unequipped slot '%s'."), *SlotName));
			}
			else
			{
				LogCheat(FString::Printf(TEXT("Slot '%s' is already empty."), *SlotName));
			}
		}
		else
		{
			LogCheat(TEXT("Equipment Component not found on player."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaUnequipAll()
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		UHarmoniaEquipmentComponent* EquipmentComp = PlayerChar->FindComponentByClass<UHarmoniaEquipmentComponent>();
		if (EquipmentComp)
		{
			EquipmentComp->UnequipAll();
			LogCheat(TEXT("Unequipped all items."));
		}
		else
		{
			LogCheat(TEXT("Equipment Component not found on player."));
		}
	}
}

// ==================== Level & Experience ====================

void UHarmoniaCheatManager::HarmoniaSetLevel(int32 NewLevel)
{
	LogCheat(FString::Printf(TEXT("Set level to %d."), NewLevel));
	LogCheat(TEXT("This feature requires implementation with level system."));
}

void UHarmoniaCheatManager::HarmoniaGiveXP(int32 Amount)
{
	LogCheat(FString::Printf(TEXT("Granted %d experience."), Amount));
	LogCheat(TEXT("This feature requires implementation with experience system."));
}

void UHarmoniaCheatManager::HarmoniaLevelUp()
{
	LogCheat(TEXT("Attempting to level up."));
	LogCheat(TEXT("This feature requires implementation with level system."));
}

// ==================== Combat ====================

void UHarmoniaCheatManager::HarmoniaToggleInvincible()
{
	bInvincible = !bInvincible;
	LogCheat(FString::Printf(TEXT("Invincible mode: %s"), bInvincible ? TEXT("ON") : TEXT("OFF")));
	
	// To apply invincibility, damage processing logic should check bInvincible flag
}

void UHarmoniaCheatManager::HarmoniaToggleGodMode()
{
	bGodMode = !bGodMode;
	LogCheat(FString::Printf(TEXT("God mode: %s"), bGodMode ? TEXT("ON") : TEXT("OFF")));
	
	if (bGodMode)
	{
		bInvincible = true;
		// In God mode, health, mana, and stamina should be continuously refilled
	}
	else
	{
		bInvincible = false;
	}
}

void UHarmoniaCheatManager::HarmoniaToggleOneHitKill()
{
	bOneHitKill = !bOneHitKill;
	LogCheat(FString::Printf(TEXT("One-hit kill mode: %s"), bOneHitKill ? TEXT("ON") : TEXT("OFF")));
}

void UHarmoniaCheatManager::HarmoniaSetDamageMultiplier(float Multiplier)
{
	DamageMultiplier = FMath::Max(0.0f, Multiplier);
	LogCheat(FString::Printf(TEXT("Damage multiplier: %.1fx"), DamageMultiplier));
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
			// Store base speed and apply multiplier
			const float BaseSpeed = 600.0f; // Adjust to match project
			MovementComp->MaxWalkSpeed = BaseSpeed * SpeedMultiplier;
			LogCheat(FString::Printf(TEXT("Movement speed: %.1fx (%.0f)"), SpeedMultiplier, MovementComp->MaxWalkSpeed));
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
				LogCheat(TEXT("NoClip mode: ON"));
			}
			else
			{
				MovementComp->SetMovementMode(MOVE_Walking);
				PlayerChar->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
				LogCheat(TEXT("NoClip mode: OFF"));
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
				LogCheat(TEXT("Fly mode: ON"));
			}
			else
			{
				MovementComp->SetMovementMode(MOVE_Walking);
				LogCheat(TEXT("Fly mode: OFF"));
			}
		}
	}
}

void UHarmoniaCheatManager::HarmoniaTeleportToMarker(const FString& MarkerName)
{
	LogCheat(FString::Printf(TEXT("Attempting to teleport to marker '%s'."), *MarkerName));
	LogCheat(TEXT("This feature requires implementation with world marker/waypoint system."));
}

void UHarmoniaCheatManager::HarmoniaTeleport(float X, float Y, float Z)
{
	ACharacter* PlayerChar = GetPlayerCharacter();
	if (PlayerChar)
	{
		FVector NewLocation(X, Y, Z);
		PlayerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		LogCheat(FString::Printf(TEXT("Teleported to coordinates (%.0f, %.0f, %.0f)."), X, Y, Z));
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
			// Time setting handled via delegate broadcast to Blueprint
			Hour = FMath::Clamp(Hour, 0, 23);
			Minute = FMath::Clamp(Minute, 0, 59);
			
			EHarmoniaTimeOfDay TimeOfDay = UHarmoniaTimeWeatherManager::HourToTimeOfDay(Hour);
			TimeWeatherManager->BroadcastTimeChange(TimeWeatherManager->GetCurrentTimeOfDay(), TimeOfDay, Hour, Minute);
			
			LogCheat(FString::Printf(TEXT("Set time to %02d:%02d."), Hour, Minute));
		}
		else
		{
			LogCheat(TEXT("Time Weather Manager not found."));
		}
	}
}

void UHarmoniaCheatManager::HarmoniaSetTimeScale(float Multiplier)
{
	TimeScale = FMath::Max(0.0f, Multiplier);
	
	if (UWorld* World = GetWorld())
	{
		// This changes game time system speed, not World time dilation
		LogCheat(FString::Printf(TEXT("Time scale: %.1fx"), TimeScale));
		LogCheat(TEXT("Actual time flow change should be implemented by referencing TimeScale in the time system."));
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
			LogCheat(FString::Printf(TEXT("Set weather to '%s'."), *WeatherType));
		}
		else
		{
			LogCheat(TEXT("Time Weather Manager not found."));
		}
	}
}

// ==================== Quest & Achievement ====================

void UHarmoniaCheatManager::HarmoniaCompleteQuest(const FString& QuestName)
{
	LogCheat(FString::Printf(TEXT("Completed quest '%s'."), *QuestName));
	LogCheat(TEXT("This feature requires implementation with quest system."));
}

void UHarmoniaCheatManager::HarmoniaCompleteAllQuests()
{
	LogCheat(TEXT("Completed all quests."));
	LogCheat(TEXT("This feature requires implementation with quest system."));
}

void UHarmoniaCheatManager::HarmoniaUnlockAchievement(const FString& AchievementName)
{
	LogCheat(FString::Printf(TEXT("Unlocked achievement '%s'."), *AchievementName));
	LogCheat(TEXT("This feature requires implementation with achievement system."));
}

// ==================== Debug ====================

void UHarmoniaCheatManager::HarmoniaToggleDebugInfo()
{
	bShowDebugInfo = !bShowDebugInfo;
	LogCheat(FString::Printf(TEXT("Debug info display: %s"), bShowDebugInfo ? TEXT("ON") : TEXT("OFF")));
}

void UHarmoniaCheatManager::HarmoniaKillAllEnemies()
{
	LogCheat(TEXT("Killed all enemies."));
	LogCheat(TEXT("This feature requires implementation to find and kill enemy actors."));
}

void UHarmoniaCheatManager::HarmoniaSpawnEnemy(const FString& EnemyName, int32 Count)
{
	LogCheat(FString::Printf(TEXT("Spawned %d '%s' enemy(s)."), Count, *EnemyName));
	LogCheat(TEXT("This feature requires implementation with enemy spawn system."));
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
	
	// Reset speed as well
	HarmoniaSetSpeed(1.0f);
	
	LogCheat(TEXT("All cheats have been reset."));
}

void UHarmoniaCheatManager::HarmoniaHelp()
{
	LogCheat(TEXT("========== HarmoniaKit Cheat Command List =========="));
	LogCheat(TEXT(""));
	LogCheat(TEXT("== Health/Mana/Stamina =="));
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
	LogCheat(TEXT("== Equipment =="));
	LogCheat(TEXT("HarmoniaEquipItem <id> - Equip item by ID"));
	LogCheat(TEXT("HarmoniaUnequipSlot <slot> - Unequip slot"));
	LogCheat(TEXT("HarmoniaUnequipAll - Unequip all"));
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
#endif // !UE_BUILD_SHIPPING
