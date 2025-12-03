// Copyright 2025 Snow Game Studio.

#include "Definitions/HarmoniaMonsterSystemDefinitions.h"

// ============================================================================
// FHarmoniaFactionSettings Implementation
// ============================================================================

EFactionRelationship FHarmoniaFactionSettings::GetRelationshipWith(EHarmoniaMonsterFaction OtherFaction) const
{
	// Check custom relationships first
	if (const EFactionRelationship* CustomRelation = CustomRelationships.Find(OtherFaction))
	{
		return *CustomRelation;
	}

	// Default relationship logic
	if (Faction == OtherFaction)
	{
		return bCanAttackSameFaction ? EFactionRelationship::Neutral : EFactionRelationship::Friendly;
	}

	// Player Friendly is hostile to Player Hostile
	if (Faction == EHarmoniaMonsterFaction::PlayerFriendly && OtherFaction == EHarmoniaMonsterFaction::PlayerHostile)
	{
		return EFactionRelationship::Hostile;
	}

	// Player Hostile is hostile to Player Friendly
	if (Faction == EHarmoniaMonsterFaction::PlayerHostile && OtherFaction == EHarmoniaMonsterFaction::PlayerFriendly)
	{
		return EFactionRelationship::Hostile;
	}

	// Monster factions are neutral to each other by default
	if ((Faction >= EHarmoniaMonsterFaction::Monster1 && Faction <= EHarmoniaMonsterFaction::Monster4) &&
		(OtherFaction >= EHarmoniaMonsterFaction::Monster1 && OtherFaction <= EHarmoniaMonsterFaction::Monster4))
	{
		return EFactionRelationship::Neutral;
	}

	// Neutral is neutral to everyone
	if (Faction == EHarmoniaMonsterFaction::Neutral)
	{
		return EFactionRelationship::Neutral;
	}

	// Default: Neutral
	return EFactionRelationship::Neutral;
}

bool FHarmoniaFactionSettings::CanAttack(EHarmoniaMonsterFaction OtherFaction) const
{
	EFactionRelationship Relationship = GetRelationshipWith(OtherFaction);
	return Relationship == EFactionRelationship::Hostile;
}

bool FHarmoniaFactionSettings::ShouldHelp(EHarmoniaMonsterFaction OtherFaction) const
{
	if (!bDefendAllies)
	{
		return false;
	}

	EFactionRelationship Relationship = GetRelationshipWith(OtherFaction);
	return Relationship == EFactionRelationship::Friendly;
}

// ============================================================================
// UHarmoniaLootTableData Implementation
// ============================================================================

TArray<FHarmoniaLootTableRow> UHarmoniaLootTableData::GenerateLoot(int32 MonsterLevel, float LuckModifier) const
{
	TArray<FHarmoniaLootTableRow> GeneratedLoot;

	// Track how many items we've generated
	int32 ItemsGenerated = 0;

	// Process each loot entry
	for (const FHarmoniaLootTableRow& LootEntry : LootEntries)
	{
		// Check level requirement
		if (MonsterLevel < LootEntry.MinMonsterLevel)
		{
			continue;
		}

		// Check if we've reached max drops (guaranteed drops don't count)
		if (!LootEntry.bGuaranteedDrop && ItemsGenerated >= MaxDrops)
		{
			break;
		}

		// Calculate drop chance with luck modifier
		float FinalDropChance = FMath::Clamp(LootEntry.DropChance + LuckModifier, 0.0f, 1.0f);

		// Roll for drop
		bool bShouldDrop = LootEntry.bGuaranteedDrop || FMath::FRand() <= FinalDropChance;

		if (bShouldDrop)
		{
			// Create a copy of the loot entry with randomized quantity
			FHarmoniaLootTableRow GeneratedEntry = LootEntry;

			// Randomize quantity
			if (LootEntry.MaxQuantity > LootEntry.MinQuantity)
			{
				GeneratedEntry.MinQuantity = FMath::RandRange(LootEntry.MinQuantity, LootEntry.MaxQuantity);
				GeneratedEntry.MaxQuantity = GeneratedEntry.MinQuantity; // Set to same value for consistency
			}

			GeneratedLoot.Add(GeneratedEntry);

			if (!LootEntry.bGuaranteedDrop)
			{
				ItemsGenerated++;
			}
		}
	}

	// Add guaranteed gold as a loot entry if specified
	if (MaxGold > 0)
	{
		int32 GoldAmount = FMath::RandRange(MinGold, MaxGold);
		if (GoldAmount > 0)
		{
			FHarmoniaLootTableRow GoldEntry;
			GoldEntry.LootType = EHarmoniaLootItemType::Currency;
			GoldEntry.ItemID = FName("Gold");
			GoldEntry.MinQuantity = GoldAmount;
			GoldEntry.MaxQuantity = GoldAmount;
			GoldEntry.bGuaranteedDrop = true;
			GeneratedLoot.Add(GoldEntry);
		}
	}

	// Add experience as a loot entry if specified
	if (BaseExperience > 0)
	{
		int32 ExpAmount = FMath::RoundToInt32(BaseExperience * (1.0f + ExperiencePerLevel * FMath::Max(0, MonsterLevel - 1)));

		FHarmoniaLootTableRow ExpEntry;
		ExpEntry.LootType = EHarmoniaLootItemType::Experience;
		ExpEntry.ItemID = FName("Experience");
		ExpEntry.MinQuantity = ExpAmount;
		ExpEntry.MaxQuantity = ExpAmount;
		ExpEntry.bGuaranteedDrop = true;
		GeneratedLoot.Add(ExpEntry);
	}

	return GeneratedLoot;
}
