// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/HarmoniaDifficultyLibrary.h"
#include "System/HarmoniaDifficultySubsystem.h"
#include "Engine/World.h"

UHarmoniaDifficultySubsystem* UHarmoniaDifficultyLibrary::GetDifficultySubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UHarmoniaDifficultySubsystem>();
}

// ========================================
// Pact Management
// ========================================

bool UHarmoniaDifficultyLibrary::ToggleDifficultyPact(const UObject* WorldContextObject, EHarmoniaDifficultyPactType PactType, bool bActivate)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return false;
	}

	return DifficultySubsystem->TogglePact(PactType, bActivate);
}

bool UHarmoniaDifficultyLibrary::IsDifficultyPactActive(const UObject* WorldContextObject, EHarmoniaDifficultyPactType PactType)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return false;
	}

	return DifficultySubsystem->IsPactActive(PactType);
}

TArray<FHarmoniaDifficultyPactConfig> UHarmoniaDifficultyLibrary::GetActiveDifficultyPacts(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return TArray<FHarmoniaDifficultyPactConfig>();
	}

	return DifficultySubsystem->GetActivePacts();
}

TArray<FHarmoniaDifficultyPactConfig> UHarmoniaDifficultyLibrary::GetAllDifficultyPacts(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return TArray<FHarmoniaDifficultyPactConfig>();
	}

	return DifficultySubsystem->GetAllPactConfigs();
}

float UHarmoniaDifficultyLibrary::GetTotalDifficultyMultiplier(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return 1.0f;
	}

	return DifficultySubsystem->GetTotalDifficultyMultiplier();
}

// ========================================
// Combat Modifiers
// ========================================

float UHarmoniaDifficultyLibrary::ApplyPlayerDamageModifier(const UObject* WorldContextObject, float BaseDamage)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BaseDamage;
	}

	return BaseDamage * DifficultySubsystem->GetPlayerDamageMultiplier();
}

float UHarmoniaDifficultyLibrary::ApplyPlayerDamageTakenModifier(const UObject* WorldContextObject, float BaseDamage)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BaseDamage;
	}

	return BaseDamage * DifficultySubsystem->GetPlayerDamageTakenMultiplier();
}

float UHarmoniaDifficultyLibrary::ApplyEnemyDamageModifier(const UObject* WorldContextObject, float BaseDamage)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BaseDamage;
	}

	return BaseDamage * DifficultySubsystem->GetEnemyDamageMultiplier();
}

float UHarmoniaDifficultyLibrary::ApplyEnemyHealthModifier(const UObject* WorldContextObject, float BaseHealth)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BaseHealth;
	}

	return BaseHealth * DifficultySubsystem->GetEnemyHealthMultiplier();
}

bool UHarmoniaDifficultyLibrary::CanPlayerBlock(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return true;
	}

	return !DifficultySubsystem->IsBlockingDisabled();
}

bool UHarmoniaDifficultyLibrary::CanPlayerParry(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return true;
	}

	return !DifficultySubsystem->IsParryingDisabled();
}

// ========================================
// Economy Modifiers
// ========================================

int32 UHarmoniaDifficultyLibrary::ApplySoulGainModifier(const UObject* WorldContextObject, int32 BaseSouls)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BaseSouls;
	}

	return FMath::RoundToInt32(BaseSouls * DifficultySubsystem->GetSoulGainMultiplier());
}

int32 UHarmoniaDifficultyLibrary::ApplyExperienceGainModifier(const UObject* WorldContextObject, int32 BaseExperience)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BaseExperience;
	}

	return FMath::RoundToInt32(BaseExperience * DifficultySubsystem->GetExperienceGainMultiplier());
}

int32 UHarmoniaDifficultyLibrary::ApplyVendorPriceModifier(const UObject* WorldContextObject, int32 BasePrice)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BasePrice;
	}

	return FMath::RoundToInt32(BasePrice * DifficultySubsystem->GetVendorPriceMultiplier());
}

float UHarmoniaDifficultyLibrary::ApplyHealingModifier(const UObject* WorldContextObject, float BaseHealing)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return BaseHealing;
	}

	return BaseHealing * DifficultySubsystem->GetHealingEffectivenessMultiplier();
}

bool UHarmoniaDifficultyLibrary::ShouldItemDrop(const UObject* WorldContextObject, float BaseDropChance, bool bIsRare, bool bIsLegendary)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return FMath::FRand() < BaseDropChance;
	}

	float ModifiedChance = BaseDropChance * DifficultySubsystem->GetItemDropRateMultiplier();

	if (bIsLegendary)
	{
		ModifiedChance *= DifficultySubsystem->GetLegendaryDropRateMultiplier();
	}
	else if (bIsRare)
	{
		ModifiedChance *= DifficultySubsystem->GetRareDropRateMultiplier();
	}

	return FMath::FRand() < FMath::Clamp(ModifiedChance, 0.0f, 1.0f);
}

// ========================================
// New Game Plus
// ========================================

void UHarmoniaDifficultyLibrary::StartNewGamePlus(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (DifficultySubsystem)
	{
		DifficultySubsystem->StartNewGamePlus();
	}
}

void UHarmoniaDifficultyLibrary::CompletePlaythrough(const UObject* WorldContextObject, float CompletionTime)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (DifficultySubsystem)
	{
		DifficultySubsystem->CompletePlaythrough(CompletionTime);
	}
}

EHarmoniaNewGamePlusTier UHarmoniaDifficultyLibrary::GetNewGamePlusTier(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return EHarmoniaNewGamePlusTier::BaseGame;
	}

	return DifficultySubsystem->GetNewGamePlusTier();
}

FHarmoniaNewGamePlusLegacy UHarmoniaDifficultyLibrary::GetNewGamePlusLegacy(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return FHarmoniaNewGamePlusLegacy();
	}

	return DifficultySubsystem->GetNewGamePlusLegacy();
}

bool UHarmoniaDifficultyLibrary::IsNewGamePlusItemUnlocked(const UObject* WorldContextObject, FName ItemName, const FString& ItemType)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (!DifficultySubsystem)
	{
		return false;
	}

	FHarmoniaNewGamePlusLegacy Legacy = DifficultySubsystem->GetNewGamePlusLegacy();

	if (ItemType == TEXT("Cosmetic"))
	{
		return Legacy.UnlockedCosmetics.Contains(ItemName);
	}
	else if (ItemType == TEXT("Weapon"))
	{
		return Legacy.UnlockedWeapons.Contains(ItemName);
	}
	else if (ItemType == TEXT("Ability"))
	{
		return Legacy.UnlockedAbilities.Contains(ItemName);
	}

	return false;
}

// ========================================
// Dynamic Difficulty
// ========================================

void UHarmoniaDifficultyLibrary::SetDynamicDifficultyMode(const UObject* WorldContextObject, EHarmoniaDynamicDifficultyMode Mode)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (DifficultySubsystem)
	{
		DifficultySubsystem->SetDynamicDifficultyMode(Mode);
	}
}

void UHarmoniaDifficultyLibrary::ReportPlayerDeath(const UObject* WorldContextObject)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (DifficultySubsystem)
	{
		DifficultySubsystem->ReportPlayerDeath();
	}
}

void UHarmoniaDifficultyLibrary::ReportPlayerVictory(const UObject* WorldContextObject, float HealthRemainingPercent, float CombatDuration)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (DifficultySubsystem)
	{
		DifficultySubsystem->ReportPlayerVictory(HealthRemainingPercent, CombatDuration);
	}
}

void UHarmoniaDifficultyLibrary::ReportPerfectDefense(const UObject* WorldContextObject, bool bWasParry)
{
	UHarmoniaDifficultySubsystem* DifficultySubsystem = GetDifficultySubsystem(WorldContextObject);
	if (DifficultySubsystem)
	{
		DifficultySubsystem->ReportPerfectDefense(bWasParry);
	}
}

// ========================================
// Utility Functions
// ========================================

FText UHarmoniaDifficultyLibrary::GetPactTypeName(EHarmoniaDifficultyPactType PactType)
{
	switch (PactType)
	{
	case EHarmoniaDifficultyPactType::GlassCannon:
		return FText::FromString(TEXT("Glass Cannon"));
	case EHarmoniaDifficultyPactType::IronWill:
		return FText::FromString(TEXT("Iron Will"));
	case EHarmoniaDifficultyPactType::FragileBones:
		return FText::FromString(TEXT("Fragile Bones"));
	case EHarmoniaDifficultyPactType::BerserkersRage:
		return FText::FromString(TEXT("Berserker's Rage"));
	case EHarmoniaDifficultyPactType::SwiftFoes:
		return FText::FromString(TEXT("Swift Foes"));
	case EHarmoniaDifficultyPactType::EliteUprising:
		return FText::FromString(TEXT("Elite Uprising"));
	case EHarmoniaDifficultyPactType::IntelligentEnemies:
		return FText::FromString(TEXT("Intelligent Enemies"));
	case EHarmoniaDifficultyPactType::RelentlessPursuit:
		return FText::FromString(TEXT("Relentless Pursuit"));
	case EHarmoniaDifficultyPactType::ScarceRecovery:
		return FText::FromString(TEXT("Scarce Recovery"));
	case EHarmoniaDifficultyPactType::FragileEquipment:
		return FText::FromString(TEXT("Fragile Equipment"));
	case EHarmoniaDifficultyPactType::HungerOfTheVoid:
		return FText::FromString(TEXT("Hunger of the Void"));
	case EHarmoniaDifficultyPactType::PovertysCurse:
		return FText::FromString(TEXT("Poverty's Curse"));
	case EHarmoniaDifficultyPactType::GamblersFate:
		return FText::FromString(TEXT("Gambler's Fate"));
	case EHarmoniaDifficultyPactType::PermadealthThreat:
		return FText::FromString(TEXT("Permadeath Threat"));
	case EHarmoniaDifficultyPactType::NoHUD:
		return FText::FromString(TEXT("No HUD"));
	case EHarmoniaDifficultyPactType::FogOfWar:
		return FText::FromString(TEXT("Fog of War"));
	default:
		return FText::FromString(TEXT("None"));
	}
}

FText UHarmoniaDifficultyLibrary::GetNewGamePlusTierName(EHarmoniaNewGamePlusTier Tier)
{
	switch (Tier)
	{
	case EHarmoniaNewGamePlusTier::BaseGame:
		return FText::FromString(TEXT("Base Game"));
	case EHarmoniaNewGamePlusTier::NewGamePlus1:
		return FText::FromString(TEXT("New Game+"));
	case EHarmoniaNewGamePlusTier::NewGamePlus2:
		return FText::FromString(TEXT("New Game++"));
	case EHarmoniaNewGamePlusTier::NewGamePlus3:
		return FText::FromString(TEXT("New Game+++"));
	case EHarmoniaNewGamePlusTier::NewGamePlus4:
		return FText::FromString(TEXT("New Game++++"));
	case EHarmoniaNewGamePlusTier::NewGamePlus5:
		return FText::FromString(TEXT("New Game+++++"));
	case EHarmoniaNewGamePlusTier::NewGamePlus6:
		return FText::FromString(TEXT("New Game++++++"));
	case EHarmoniaNewGamePlusTier::NewGamePlus7:
		return FText::FromString(TEXT("New Game+++++++"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

int32 UHarmoniaDifficultyLibrary::GetRecommendedLevelForTier(EHarmoniaNewGamePlusTier Tier)
{
	// Base recommended level is 1 for base game
	// Each NG+ tier adds 20 levels
	int32 TierValue = static_cast<int32>(Tier);
	return 1 + (TierValue * 20);
}
