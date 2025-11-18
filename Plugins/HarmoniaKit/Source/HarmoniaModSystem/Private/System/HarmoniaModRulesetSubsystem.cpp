// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaModRulesetSubsystem.h"

DEFINE_LOG_CATEGORY(LogHarmoniaModRuleset);

void UHarmoniaModRulesetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaModRuleset, Log, TEXT("Initializing Harmonia Mod Ruleset Subsystem"));

	RegisteredRulesets.Empty();
	ActiveRulesetIds.Empty();
	RulesetsByMod.Empty();
	ActiveRuleTags = FGameplayTagContainer();
}

void UHarmoniaModRulesetSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaModRuleset, Log, TEXT("Deinitializing Harmonia Mod Ruleset Subsystem"));

	DeactivateAllRulesets();
	RegisteredRulesets.Empty();
	RulesetsByMod.Empty();

	Super::Deinitialize();
}

bool UHarmoniaModRulesetSubsystem::RegisterRuleset(const FHarmoniaCustomRuleset& Ruleset, FName ModId)
{
	if (!ValidateRuleset(Ruleset))
	{
		UE_LOG(LogHarmoniaModRuleset, Error, TEXT("Invalid ruleset: %s"), *Ruleset.RulesetId.ToString());
		return false;
	}

	if (RegisteredRulesets.Contains(Ruleset.RulesetId))
	{
		UE_LOG(LogHarmoniaModRuleset, Warning, TEXT("Ruleset '%s' already registered, replacing..."),
			*Ruleset.RulesetId.ToString());
	}

	RegisteredRulesets.Add(Ruleset.RulesetId, Ruleset);

	// Track by mod
	if (!RulesetsByMod.Contains(ModId))
	{
		RulesetsByMod.Add(ModId, TArray<FName>());
	}
	RulesetsByMod[ModId].AddUnique(Ruleset.RulesetId);

	UE_LOG(LogHarmoniaModRuleset, Log, TEXT("Registered ruleset: %s (%s) [Mod: %s]"),
		*Ruleset.RulesetId.ToString(),
		*Ruleset.DisplayName.ToString(),
		*ModId.ToString());

	return true;
}

bool UHarmoniaModRulesetSubsystem::UnregisterRuleset(FName RulesetId)
{
	if (!RegisteredRulesets.Contains(RulesetId))
	{
		UE_LOG(LogHarmoniaModRuleset, Warning, TEXT("Ruleset '%s' not registered"), *RulesetId.ToString());
		return false;
	}

	// Deactivate if active
	if (IsRulesetActive(RulesetId))
	{
		DeactivateRuleset(RulesetId);
	}

	// Remove from registered
	RegisteredRulesets.Remove(RulesetId);

	// Remove from mod tracking
	for (auto& Pair : RulesetsByMod)
	{
		Pair.Value.Remove(RulesetId);
	}

	UE_LOG(LogHarmoniaModRuleset, Log, TEXT("Unregistered ruleset: %s"), *RulesetId.ToString());

	return true;
}

bool UHarmoniaModRulesetSubsystem::ActivateRuleset(FName RulesetId)
{
	if (!RegisteredRulesets.Contains(RulesetId))
	{
		UE_LOG(LogHarmoniaModRuleset, Error, TEXT("Ruleset '%s' not registered"), *RulesetId.ToString());
		return false;
	}

	if (IsRulesetActive(RulesetId))
	{
		UE_LOG(LogHarmoniaModRuleset, Warning, TEXT("Ruleset '%s' is already active"), *RulesetId.ToString());
		return true;
	}

	const FHarmoniaCustomRuleset& Ruleset = RegisteredRulesets[RulesetId];

	// Add to active
	ActiveRulesetIds.Add(RulesetId);

	// Add rule tags to combined tags
	ActiveRuleTags.AppendTags(Ruleset.RuleTags);

	// Apply config
	ApplyRulesetConfig(RulesetId);

	UE_LOG(LogHarmoniaModRuleset, Log, TEXT("Activated ruleset: %s (%s)"),
		*RulesetId.ToString(),
		*Ruleset.DisplayName.ToString());

	// Broadcast event
	OnRulesetActivated.Broadcast(Ruleset);

	return true;
}

bool UHarmoniaModRulesetSubsystem::DeactivateRuleset(FName RulesetId)
{
	if (!IsRulesetActive(RulesetId))
	{
		UE_LOG(LogHarmoniaModRuleset, Warning, TEXT("Ruleset '%s' is not active"), *RulesetId.ToString());
		return false;
	}

	const FHarmoniaCustomRuleset& Ruleset = RegisteredRulesets[RulesetId];

	// Remove from active
	ActiveRulesetIds.Remove(RulesetId);

	// Rebuild active tags from remaining active rulesets
	ActiveRuleTags = FGameplayTagContainer();
	for (const FName& ActiveRulesetId : ActiveRulesetIds)
	{
		if (RegisteredRulesets.Contains(ActiveRulesetId))
		{
			ActiveRuleTags.AppendTags(RegisteredRulesets[ActiveRulesetId].RuleTags);
		}
	}

	UE_LOG(LogHarmoniaModRuleset, Log, TEXT("Deactivated ruleset: %s"), *RulesetId.ToString());

	// Broadcast event
	OnRulesetDeactivated.Broadcast(Ruleset);

	return true;
}

void UHarmoniaModRulesetSubsystem::DeactivateAllRulesets()
{
	UE_LOG(LogHarmoniaModRuleset, Log, TEXT("Deactivating all rulesets..."));

	TArray<FName> ActiveIds = ActiveRulesetIds.Array();

	for (const FName& RulesetId : ActiveIds)
	{
		DeactivateRuleset(RulesetId);
	}

	ActiveRulesetIds.Empty();
	ActiveRuleTags = FGameplayTagContainer();
}

TArray<FHarmoniaCustomRuleset> UHarmoniaModRulesetSubsystem::GetAllRulesets() const
{
	TArray<FHarmoniaCustomRuleset> Rulesets;
	RegisteredRulesets.GenerateValueArray(Rulesets);
	return Rulesets;
}

TArray<FHarmoniaCustomRuleset> UHarmoniaModRulesetSubsystem::GetActiveRulesets() const
{
	TArray<FHarmoniaCustomRuleset> ActiveRulesets;

	for (const FName& RulesetId : ActiveRulesetIds)
	{
		if (RegisteredRulesets.Contains(RulesetId))
		{
			ActiveRulesets.Add(RegisteredRulesets[RulesetId]);
		}
	}

	return ActiveRulesets;
}

bool UHarmoniaModRulesetSubsystem::GetRuleset(FName RulesetId, FHarmoniaCustomRuleset& OutRuleset) const
{
	if (RegisteredRulesets.Contains(RulesetId))
	{
		OutRuleset = RegisteredRulesets[RulesetId];
		return true;
	}

	return false;
}

bool UHarmoniaModRulesetSubsystem::IsRulesetActive(FName RulesetId) const
{
	return ActiveRulesetIds.Contains(RulesetId);
}

bool UHarmoniaModRulesetSubsystem::IsRuleTagActive(FGameplayTag Tag) const
{
	return ActiveRuleTags.HasTag(Tag);
}

bool UHarmoniaModRulesetSubsystem::GetRuleConfigValue(FName ConfigKey, FString& OutValue) const
{
	// Search through active rulesets (last one wins)
	for (const FName& RulesetId : ActiveRulesetIds)
	{
		if (RegisteredRulesets.Contains(RulesetId))
		{
			const FHarmoniaCustomRuleset& Ruleset = RegisteredRulesets[RulesetId];

			if (Ruleset.ConfigValues.Contains(ConfigKey))
			{
				OutValue = Ruleset.ConfigValues[ConfigKey];
				return true;
			}
		}
	}

	return false;
}

FGameplayTagContainer UHarmoniaModRulesetSubsystem::GetActiveRuleTags() const
{
	return ActiveRuleTags;
}

float UHarmoniaModRulesetSubsystem::GetDifficultyMultiplier() const
{
	float Multiplier = 1.0f;

	for (const FName& RulesetId : ActiveRulesetIds)
	{
		if (RegisteredRulesets.Contains(RulesetId))
		{
			const FHarmoniaCustomRuleset& Ruleset = RegisteredRulesets[RulesetId];

			if (Ruleset.bIsDifficultyModifier)
			{
				// Check for difficulty multiplier in config
				if (Ruleset.ConfigValues.Contains(TEXT("DifficultyMultiplier")))
				{
					float RulesetMultiplier = FCString::Atof(*Ruleset.ConfigValues[TEXT("DifficultyMultiplier")]);
					Multiplier *= RulesetMultiplier;
				}
			}
		}
	}

	return Multiplier;
}

bool UHarmoniaModRulesetSubsystem::ApplyRulesetConfig(FName RulesetId)
{
	if (!RegisteredRulesets.Contains(RulesetId))
	{
		return false;
	}

	const FHarmoniaCustomRuleset& Ruleset = RegisteredRulesets[RulesetId];

	// Log config values
	for (const auto& ConfigPair : Ruleset.ConfigValues)
	{
		UE_LOG(LogHarmoniaModRuleset, Verbose, TEXT("  Config: %s = %s"),
			*ConfigPair.Key.ToString(),
			*ConfigPair.Value);
	}

	// Config values will be queried by game systems as needed
	// We don't directly modify game settings here to avoid coupling

	return true;
}

bool UHarmoniaModRulesetSubsystem::ValidateRuleset(const FHarmoniaCustomRuleset& Ruleset) const
{
	if (Ruleset.RulesetId == NAME_None)
	{
		UE_LOG(LogHarmoniaModRuleset, Error, TEXT("Ruleset ID is invalid"));
		return false;
	}

	if (Ruleset.DisplayName.IsEmpty())
	{
		UE_LOG(LogHarmoniaModRuleset, Error, TEXT("Ruleset display name is empty"));
		return false;
	}

	return true;
}
