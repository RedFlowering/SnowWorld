// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/HarmoniaDifficultySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UHarmoniaDifficultySubsystem::UHarmoniaDifficultySubsystem()
{
}

void UHarmoniaDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default pact configurations
	InitializeDefaultPacts();

	// Set up dynamic difficulty update timer (every 30 seconds)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DynamicDifficultyUpdateTimer,
			this,
			&UHarmoniaDifficultySubsystem::UpdateDynamicDifficulty,
			30.0f,
			true
		);
	}
}

void UHarmoniaDifficultySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DynamicDifficultyUpdateTimer);
	}

	Super::Deinitialize();
}

// ========================================
// Difficulty Pacts
// ========================================

void UHarmoniaDifficultySubsystem::InitializeDefaultPacts()
{
	PactConfigs.Empty();

	// Create all default pact configurations
	SetupGlassCannonPact();
	SetupIronWillPact();
	SetupFragileBonesPact();
	SetupBerserkersRagePact();
	SetupSwiftFoesPact();
	SetupEliteUprisingPact();
	SetupIntelligentEnemiesPact();
	SetupRelentlessPursuitPact();
	SetupScarceRecoveryPact();
	SetupFragileEquipmentPact();
	SetupHungerOfTheVoidPact();
	SetupPovertysCursePact();
	SetupGamblersFatePact();
	SetupPermadeathThreatPact();
	SetupNoHUDPact();
	SetupFogOfWarPact();
}

bool UHarmoniaDifficultySubsystem::TogglePact(EHarmoniaDifficultyPactType PactType, bool bActivate)
{
	if (!PactConfigs.Contains(PactType))
	{
		return false;
	}

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[PactType];

	if (Config.bIsActive == bActivate)
	{
		return false; // Already in desired state
	}

	Config.bIsActive = bActivate;

	// Update active pacts list
	if (bActivate)
	{
		CurrentConfiguration.ActivePacts.Add(Config);
	}
	else
	{
		CurrentConfiguration.ActivePacts.RemoveAll([PactType](const FHarmoniaDifficultyPactConfig& Pact)
		{
			return Pact.PactType == PactType;
		});
	}

	OnPactToggled.Broadcast(PactType);

	return true;
}

bool UHarmoniaDifficultySubsystem::IsPactActive(EHarmoniaDifficultyPactType PactType) const
{
	if (const FHarmoniaDifficultyPactConfig* Config = PactConfigs.Find(PactType))
	{
		return Config->bIsActive;
	}
	return false;
}

FHarmoniaDifficultyPactConfig UHarmoniaDifficultySubsystem::GetPactConfig(EHarmoniaDifficultyPactType PactType) const
{
	if (const FHarmoniaDifficultyPactConfig* Config = PactConfigs.Find(PactType))
	{
		return *Config;
	}
	return FHarmoniaDifficultyPactConfig();
}

TArray<FHarmoniaDifficultyPactConfig> UHarmoniaDifficultySubsystem::GetActivePacts() const
{
	return CurrentConfiguration.ActivePacts;
}

TArray<FHarmoniaDifficultyPactConfig> UHarmoniaDifficultySubsystem::GetAllPactConfigs() const
{
	TArray<FHarmoniaDifficultyPactConfig> AllConfigs;
	PactConfigs.GenerateValueArray(AllConfigs);
	return AllConfigs;
}

float UHarmoniaDifficultySubsystem::GetTotalDifficultyMultiplier() const
{
	float TotalMultiplier = 1.0f;

	for (const FHarmoniaDifficultyPactConfig& Pact : CurrentConfiguration.ActivePacts)
	{
		TotalMultiplier += Pact.DifficultyRating - 1.0f;
	}

	// Apply New Game Plus multiplier
	TotalMultiplier *= GetNewGamePlusDifficultyMultiplier();

	// Apply dynamic difficulty
	TotalMultiplier *= CurrentConfiguration.DynamicDifficulty.CurrentAdjustment;

	return TotalMultiplier;
}

// ========================================
// Modifier Getters
// ========================================

float UHarmoniaDifficultySubsystem::GetPlayerDamageMultiplier() const
{
	float Multiplier = CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.PlayerDamageMultiplier;
	});

	// Add New Game Plus bonus
	Multiplier += CurrentConfiguration.NewGamePlusLegacy.PermanentDamageBonus;

	return Multiplier;
}

float UHarmoniaDifficultySubsystem::GetPlayerDamageTakenMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.PlayerDamageTakenMultiplier;
	}) * CurrentConfiguration.DynamicDifficulty.CurrentAdjustment;
}

float UHarmoniaDifficultySubsystem::GetPlayerPoiseMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.PlayerPoiseMultiplier;
	});
}

bool UHarmoniaDifficultySubsystem::IsBlockingDisabled() const
{
	return CheckAnyPactCondition([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.bDisableBlocking;
	});
}

bool UHarmoniaDifficultySubsystem::IsParryingDisabled() const
{
	return CheckAnyPactCondition([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.bDisableParrying;
	});
}

float UHarmoniaDifficultySubsystem::GetEnemyHealthMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.EnemyHealthMultiplier;
	}) * GetNewGamePlusDifficultyMultiplier();
}

float UHarmoniaDifficultySubsystem::GetEnemyDamageMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.EnemyDamageMultiplier;
	}) * GetNewGamePlusDifficultyMultiplier() * CurrentConfiguration.DynamicDifficulty.CurrentAdjustment;
}

float UHarmoniaDifficultySubsystem::GetEnemyMovementSpeedMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.EnemyMovementSpeedMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetEnemyAttackSpeedMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.EnemyAttackSpeedMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetEliteSpawnChanceMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.EliteSpawnChanceMultiplier;
	});
}

bool UHarmoniaDifficultySubsystem::IsEnhancedAIEnabled() const
{
	return CheckAnyPactCondition([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.bEnhancedAI;
	});
}

float UHarmoniaDifficultySubsystem::GetHealingEffectivenessMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.HealingEffectivenessMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetStaminaDrainMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.StaminaDrainMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetDurabilityLossMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.DurabilityLossMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetVendorPriceMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.VendorPriceMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetSoulGainMultiplier() const
{
	float Multiplier = CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.SoulGainMultiplier;
	});

	// Add New Game Plus bonus
	Multiplier += CurrentConfiguration.NewGamePlusLegacy.PermanentSoulGainBonus;

	return Multiplier;
}

float UHarmoniaDifficultySubsystem::GetExperienceGainMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.ExperienceGainMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetItemDropRateMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.ItemDropRateMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetRareDropRateMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.RareDropRateMultiplier;
	});
}

float UHarmoniaDifficultySubsystem::GetLegendaryDropRateMultiplier() const
{
	return CalculateCombinedMultiplier([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.LegendaryDropRateMultiplier;
	});
}

int32 UHarmoniaDifficultySubsystem::GetMaxCheckpointUses() const
{
	int32 MinUses = -1; // -1 = unlimited

	for (const FHarmoniaDifficultyPactConfig& Pact : CurrentConfiguration.ActivePacts)
	{
		if (Pact.MaxCheckpointUses >= 0)
		{
			if (MinUses < 0 || Pact.MaxCheckpointUses < MinUses)
			{
				MinUses = Pact.MaxCheckpointUses;
			}
		}
	}

	return MinUses;
}

bool UHarmoniaDifficultySubsystem::IsPermadeathEnabled() const
{
	return CheckAnyPactCondition([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.bPermadeathEnabled;
	});
}

bool UHarmoniaDifficultySubsystem::ShouldHideHUD() const
{
	return CheckAnyPactCondition([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.bHideHUD;
	});
}

bool UHarmoniaDifficultySubsystem::IsMinimapLimited() const
{
	return CheckAnyPactCondition([](const FHarmoniaDifficultyPactConfig& Pact)
	{
		return Pact.bLimitedMinimap;
	});
}

// ========================================
// New Game Plus
// ========================================

void UHarmoniaDifficultySubsystem::StartNewGamePlus()
{
	// Increment tier
	int32 CurrentTierValue = static_cast<int32>(CurrentConfiguration.NewGamePlusLegacy.CurrentTier);
	CurrentTierValue = FMath::Min(CurrentTierValue + 1, static_cast<int32>(EHarmoniaNewGamePlusTier::MAX) - 1);
	CurrentConfiguration.NewGamePlusLegacy.CurrentTier = static_cast<EHarmoniaNewGamePlusTier>(CurrentTierValue);

	OnNewGamePlusTierChanged.Broadcast(CurrentConfiguration.NewGamePlusLegacy.CurrentTier);
}

void UHarmoniaDifficultySubsystem::CompletePlaythrough(float CompletionTime)
{
	FHarmoniaNewGamePlusLegacy& Legacy = CurrentConfiguration.NewGamePlusLegacy;

	Legacy.CompletionCount++;

	// Award permanent bonuses based on completion count
	Legacy.PermanentHealthBonus += 0.05f;		// +5% max health per completion
	Legacy.PermanentStaminaBonus += 0.05f;		// +5% max stamina per completion
	Legacy.PermanentDamageBonus += 0.03f;		// +3% damage per completion
	Legacy.PermanentSoulGainBonus += 0.10f;		// +10% soul gain per completion
	Legacy.BonusSkillPoints += 1;				// +1 skill point per completion

	// Track fastest completion time
	if (Legacy.FastestCompletionTime == 0.0f || CompletionTime < Legacy.FastestCompletionTime)
	{
		Legacy.FastestCompletionTime = CompletionTime;
	}

	// Award special unlocks based on milestones
	if (Legacy.CompletionCount == 1)
	{
		AwardLegacyUnlock(FName("NG_Armor_Set"), TEXT("Cosmetic"));
		AwardLegacyUnlock(FName("NG_Special_Weapon"), TEXT("Weapon"));
	}
	else if (Legacy.CompletionCount == 3)
	{
		AwardLegacyUnlock(FName("NG_Master_Armor"), TEXT("Cosmetic"));
		AwardLegacyUnlock(FName("NG_Divine_Weapon"), TEXT("Weapon"));
	}
	else if (Legacy.CompletionCount >= 5)
	{
		AwardLegacyUnlock(FName("NG_Legendary_Armor"), TEXT("Cosmetic"));
		AwardLegacyUnlock(FName("NG_Ultimate_Ability"), TEXT("Ability"));
	}
}

EHarmoniaNewGamePlusTier UHarmoniaDifficultySubsystem::GetNewGamePlusTier() const
{
	return CurrentConfiguration.NewGamePlusLegacy.CurrentTier;
}

FHarmoniaNewGamePlusLegacy UHarmoniaDifficultySubsystem::GetNewGamePlusLegacy() const
{
	return CurrentConfiguration.NewGamePlusLegacy;
}

float UHarmoniaDifficultySubsystem::GetNewGamePlusDifficultyMultiplier() const
{
	int32 TierValue = static_cast<int32>(CurrentConfiguration.NewGamePlusLegacy.CurrentTier);

	// Each NG+ tier adds 15% difficulty
	return 1.0f + (TierValue * 0.15f);
}

void UHarmoniaDifficultySubsystem::AwardLegacyUnlock(const FName& UnlockName, const FString& UnlockType)
{
	FHarmoniaNewGamePlusLegacy& Legacy = CurrentConfiguration.NewGamePlusLegacy;

	if (UnlockType == TEXT("Cosmetic"))
	{
		Legacy.UnlockedCosmetics.AddUnique(UnlockName);
	}
	else if (UnlockType == TEXT("Weapon"))
	{
		Legacy.UnlockedWeapons.AddUnique(UnlockName);
	}
	else if (UnlockType == TEXT("Ability"))
	{
		Legacy.UnlockedAbilities.AddUnique(UnlockName);
	}
}

// ========================================
// Dynamic Difficulty
// ========================================

void UHarmoniaDifficultySubsystem::SetDynamicDifficultyMode(EHarmoniaDynamicDifficultyMode Mode)
{
	CurrentConfiguration.DynamicDifficulty.Mode = Mode;

	// Reset adjustment when changing modes
	if (Mode == EHarmoniaDynamicDifficultyMode::Disabled)
	{
		CurrentConfiguration.DynamicDifficulty.CurrentAdjustment = 1.0f;
	}
}

EHarmoniaDynamicDifficultyMode UHarmoniaDifficultySubsystem::GetDynamicDifficultyMode() const
{
	return CurrentConfiguration.DynamicDifficulty.Mode;
}

float UHarmoniaDifficultySubsystem::GetDynamicDifficultyAdjustment() const
{
	return CurrentConfiguration.DynamicDifficulty.CurrentAdjustment;
}

void UHarmoniaDifficultySubsystem::ReportPlayerDeath()
{
	CurrentConfiguration.DynamicDifficulty.RecentDeaths++;
	CurrentConfiguration.NewGamePlusLegacy.TotalDeaths++;
}

void UHarmoniaDifficultySubsystem::ReportPlayerVictory(float HealthRemainingPercent, float CombatDuration)
{
	FHarmoniaDynamicDifficultyState& DynDiff = CurrentConfiguration.DynamicDifficulty;

	DynDiff.RecentVictories++;

	// Update running averages
	const float Alpha = 0.3f; // Smoothing factor
	DynDiff.AverageHealthRemaining = FMath::Lerp(DynDiff.AverageHealthRemaining, HealthRemainingPercent, Alpha);
	DynDiff.AverageCombatDuration = FMath::Lerp(DynDiff.AverageCombatDuration, CombatDuration, Alpha);
}

void UHarmoniaDifficultySubsystem::ReportPerfectDefense(bool bWasParry)
{
	if (bWasParry)
	{
		CurrentConfiguration.DynamicDifficulty.PerfectParries++;
	}
	else
	{
		CurrentConfiguration.DynamicDifficulty.PerfectDodges++;
	}
}

void UHarmoniaDifficultySubsystem::UpdateDynamicDifficulty()
{
	FHarmoniaDynamicDifficultyState& DynDiff = CurrentConfiguration.DynamicDifficulty;

	if (DynDiff.Mode == EHarmoniaDynamicDifficultyMode::Disabled)
	{
		return;
	}

	// Calculate performance score (0.0 = struggling, 1.0 = dominating)
	float PerformanceScore = 0.5f;

	const int32 TotalEncounters = DynDiff.RecentDeaths + DynDiff.RecentVictories;
	if (TotalEncounters > 0)
	{
		const float WinRate = static_cast<float>(DynDiff.RecentVictories) / TotalEncounters;
		PerformanceScore = WinRate;

		// Factor in health remaining
		PerformanceScore = (PerformanceScore * 0.7f) + (DynDiff.AverageHealthRemaining * 0.3f);

		// Bonus for perfect defenses
		const float PerfectDefenseBonus = FMath::Min((DynDiff.PerfectParries + DynDiff.PerfectDodges) * 0.01f, 0.1f);
		PerformanceScore += PerfectDefenseBonus;

		PerformanceScore = FMath::Clamp(PerformanceScore, 0.0f, 1.0f);
	}

	// Determine adjustment range based on mode
	float MaxAdjustment = 0.05f;
	switch (DynDiff.Mode)
	{
	case EHarmoniaDynamicDifficultyMode::Subtle:
		MaxAdjustment = 0.05f;
		break;
	case EHarmoniaDynamicDifficultyMode::Moderate:
		MaxAdjustment = 0.10f;
		break;
	case EHarmoniaDynamicDifficultyMode::Adaptive:
		MaxAdjustment = 0.20f;
		break;
	}

	// Adjust difficulty (lower performance = easier, higher performance = harder)
	float TargetAdjustment = 1.0f;
	if (PerformanceScore < 0.3f)
	{
		// Struggling - make easier
		TargetAdjustment = 1.0f - MaxAdjustment;
	}
	else if (PerformanceScore > 0.7f)
	{
		// Dominating - make harder
		TargetAdjustment = 1.0f + MaxAdjustment;
	}

	// Smooth transition
	DynDiff.CurrentAdjustment = FMath::Lerp(DynDiff.CurrentAdjustment, TargetAdjustment, 0.2f);

	// Reset counters for next period
	DynDiff.RecentDeaths = 0;
	DynDiff.RecentVictories = 0;
	DynDiff.PerfectParries = 0;
	DynDiff.PerfectDodges = 0;

	OnDynamicDifficultyAdjusted.Broadcast(DynDiff.CurrentAdjustment);
}

// ========================================
// Configuration Management
// ========================================

FHarmoniaDifficultyConfiguration UHarmoniaDifficultySubsystem::GetDifficultyConfiguration() const
{
	return CurrentConfiguration;
}

void UHarmoniaDifficultySubsystem::SetDifficultyConfiguration(const FHarmoniaDifficultyConfiguration& Config)
{
	CurrentConfiguration = Config;

	// Update pact configs to match
	for (const FHarmoniaDifficultyPactConfig& ActivePact : Config.ActivePacts)
	{
		if (FHarmoniaDifficultyPactConfig* PactConfig = PactConfigs.Find(ActivePact.PactType))
		{
			PactConfig->bIsActive = true;
		}
	}
}

void UHarmoniaDifficultySubsystem::ResetToDefault()
{
	CurrentConfiguration = FHarmoniaDifficultyConfiguration();
	InitializeDefaultPacts();
}

// ========================================
// Helper Functions
// ========================================

float UHarmoniaDifficultySubsystem::CalculateCombinedMultiplier(TFunction<float(const FHarmoniaDifficultyPactConfig&)> MultiplierGetter) const
{
	float CombinedMultiplier = 1.0f;

	for (const FHarmoniaDifficultyPactConfig& Pact : CurrentConfiguration.ActivePacts)
	{
		float PactMultiplier = MultiplierGetter(Pact);
		if (PactMultiplier != 1.0f)
		{
			// Multiplicative stacking for most modifiers
			CombinedMultiplier *= PactMultiplier;
		}
	}

	return CombinedMultiplier;
}

bool UHarmoniaDifficultySubsystem::CheckAnyPactCondition(TFunction<bool(const FHarmoniaDifficultyPactConfig&)> ConditionGetter) const
{
	for (const FHarmoniaDifficultyPactConfig& Pact : CurrentConfiguration.ActivePacts)
	{
		if (ConditionGetter(Pact))
		{
			return true;
		}
	}
	return false;
}

void UHarmoniaDifficultySubsystem::CreateDefaultPactConfig(EHarmoniaDifficultyPactType PactType, const FText& Name, const FText& Description, float DifficultyRating)
{
	FHarmoniaDifficultyPactConfig Config;
	Config.PactType = PactType;
	Config.PactName = Name;
	Config.PactDescription = Description;
	Config.DifficultyRating = DifficultyRating;
	PactConfigs.Add(PactType, Config);
}

// ========================================
// Pact Setup Functions
// ========================================

void UHarmoniaDifficultySubsystem::SetupGlassCannonPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::GlassCannon,
		FText::FromString(TEXT("Glass Cannon")),
		FText::FromString(TEXT("Deal 30% more damage but take 50% more damage. Gain 20% more souls.")),
		1.3f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::GlassCannon];
	Config.PlayerDamageMultiplier = 1.3f;
	Config.PlayerDamageTakenMultiplier = 1.5f;
	Config.SoulGainMultiplier = 1.2f;
}

void UHarmoniaDifficultySubsystem::SetupIronWillPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::IronWill,
		FText::FromString(TEXT("Iron Will")),
		FText::FromString(TEXT("Can only rest at checkpoints 3 times. All rewards doubled.")),
		2.0f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::IronWill];
	Config.MaxCheckpointUses = 3;
	Config.SoulGainMultiplier = 2.0f;
	Config.ExperienceGainMultiplier = 2.0f;
	Config.ItemDropRateMultiplier = 2.0f;
}

void UHarmoniaDifficultySubsystem::SetupFragileBonesPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::FragileBones,
		FText::FromString(TEXT("Fragile Bones")),
		FText::FromString(TEXT("Poise reduced by 50%. Gain 25% more souls.")),
		1.25f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::FragileBones];
	Config.PlayerPoiseMultiplier = 0.5f;
	Config.SoulGainMultiplier = 1.25f;
}

void UHarmoniaDifficultySubsystem::SetupBerserkersRagePact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::BerserkersRage,
		FText::FromString(TEXT("Berserker's Rage")),
		FText::FromString(TEXT("Cannot block or parry. Deal 40% more damage. Gain 30% more souls.")),
		1.5f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::BerserkersRage];
	Config.bDisableBlocking = true;
	Config.bDisableParrying = true;
	Config.PlayerDamageMultiplier = 1.4f;
	Config.SoulGainMultiplier = 1.3f;
}

void UHarmoniaDifficultySubsystem::SetupSwiftFoesPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::SwiftFoes,
		FText::FromString(TEXT("Swift Foes")),
		FText::FromString(TEXT("Enemies move and attack 30% faster. Gain 25% more experience.")),
		1.3f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::SwiftFoes];
	Config.EnemyMovementSpeedMultiplier = 1.3f;
	Config.EnemyAttackSpeedMultiplier = 1.3f;
	Config.ExperienceGainMultiplier = 1.25f;
}

void UHarmoniaDifficultySubsystem::SetupEliteUprisingPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::EliteUprising,
		FText::FromString(TEXT("Elite Uprising")),
		FText::FromString(TEXT("50% more elite monsters spawn. Rare item drop rate increased by 30%.")),
		1.4f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::EliteUprising];
	Config.EliteSpawnChanceMultiplier = 1.5f;
	Config.RareDropRateMultiplier = 1.3f;
}

void UHarmoniaDifficultySubsystem::SetupIntelligentEnemiesPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::IntelligentEnemies,
		FText::FromString(TEXT("Intelligent Enemies")),
		FText::FromString(TEXT("Enemies use advanced AI tactics. Gain 35% more experience.")),
		1.4f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::IntelligentEnemies];
	Config.bEnhancedAI = true;
	Config.ExperienceGainMultiplier = 1.35f;
}

void UHarmoniaDifficultySubsystem::SetupRelentlessPursuitPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::RelentlessPursuit,
		FText::FromString(TEXT("Relentless Pursuit")),
		FText::FromString(TEXT("Enemies pursue you much longer. Gain 20% more souls.")),
		1.2f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::RelentlessPursuit];
	Config.AggroResetTimeMultiplier = 2.0f;
	Config.SoulGainMultiplier = 1.2f;
}

void UHarmoniaDifficultySubsystem::SetupScarceRecoveryPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::ScarceRecovery,
		FText::FromString(TEXT("Scarce Recovery")),
		FText::FromString(TEXT("Healing items 50% less effective. Item drop rate increased by 40%.")),
		1.3f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::ScarceRecovery];
	Config.HealingEffectivenessMultiplier = 0.5f;
	Config.ItemDropRateMultiplier = 1.4f;
}

void UHarmoniaDifficultySubsystem::SetupFragileEquipmentPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::FragileEquipment,
		FText::FromString(TEXT("Fragile Equipment")),
		FText::FromString(TEXT("Equipment loses durability twice as fast. Upgrade materials drop 50% more.")),
		1.25f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::FragileEquipment];
	Config.DurabilityLossMultiplier = 2.0f;
	Config.UpgradeMaterialDropMultiplier = 1.5f;
}

void UHarmoniaDifficultySubsystem::SetupHungerOfTheVoidPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::HungerOfTheVoid,
		FText::FromString(TEXT("Hunger of the Void")),
		FText::FromString(TEXT("Stamina drains 30% faster. Stamina items drop more frequently.")),
		1.2f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::HungerOfTheVoid];
	Config.StaminaDrainMultiplier = 1.3f;
	Config.ItemDropRateMultiplier = 1.25f;
}

void UHarmoniaDifficultySubsystem::SetupPovertysCursePact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::PovertysCurse,
		FText::FromString(TEXT("Poverty's Curse")),
		FText::FromString(TEXT("Vendors charge 50% more. Soul drops increased by 60%.")),
		1.3f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::PovertysCurse];
	Config.VendorPriceMultiplier = 1.5f;
	Config.SoulGainMultiplier = 1.6f;
}

void UHarmoniaDifficultySubsystem::SetupGamblersFatePact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::GamblersFate,
		FText::FromString(TEXT("Gambler's Fate")),
		FText::FromString(TEXT("Loot quality varies randomly. Legendary drop chance increased by 50%.")),
		1.2f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::GamblersFate];
	Config.LootQualityVariance = 0.5f;
	Config.LegendaryDropRateMultiplier = 1.5f;
}

void UHarmoniaDifficultySubsystem::SetupPermadeathThreatPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::PermadealthThreat,
		FText::FromString(TEXT("Permadeath Threat")),
		FText::FromString(TEXT("Death deletes your save (hardcore mode). All rewards tripled.")),
		3.0f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::PermadealthThreat];
	Config.bPermadeathEnabled = true;
	Config.SoulGainMultiplier = 3.0f;
	Config.ExperienceGainMultiplier = 3.0f;
	Config.ItemDropRateMultiplier = 3.0f;
}

void UHarmoniaDifficultySubsystem::SetupNoHUDPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::NoHUD,
		FText::FromString(TEXT("No HUD")),
		FText::FromString(TEXT("Minimal UI elements displayed. All rewards increased by 15%.")),
		1.15f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::NoHUD];
	Config.bHideHUD = true;
	Config.SoulGainMultiplier = 1.15f;
	Config.ExperienceGainMultiplier = 1.15f;
}

void UHarmoniaDifficultySubsystem::SetupFogOfWarPact()
{
	CreateDefaultPactConfig(
		EHarmoniaDifficultyPactType::FogOfWar,
		FText::FromString(TEXT("Fog of War")),
		FText::FromString(TEXT("Minimap and markers limited. Exploration rewards increased by 20%.")),
		1.2f
	);

	FHarmoniaDifficultyPactConfig& Config = PactConfigs[EHarmoniaDifficultyPactType::FogOfWar];
	Config.bLimitedMinimap = true;
	Config.ItemDropRateMultiplier = 1.2f;
}
