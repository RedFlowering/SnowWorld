// Copyright 2025 Snow Game Studio.

#include "ItemGeneration/HarmoniaRandomItemSubsystem.h"
#include "ItemGeneration/HarmoniaRandomItemConfigDataAsset.h"

void UHarmoniaRandomItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaRandomItemSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaRandomItemSubsystem::SetConfigDataAsset(UHarmoniaRandomItemConfigDataAsset* InConfig)
{
	ConfigAsset = InConfig;

	if (ConfigAsset)
	{
		for (const FHarmoniaAffixData& Affix : ConfigAsset->DefaultAffixes)
		{
			RegisterAffix(Affix);
		}
	}
}

void UHarmoniaRandomItemSubsystem::RegisterAffix(const FHarmoniaAffixData& Affix)
{
	if (!Affix.AffixId.IsValid())
	{
		return;
	}

	Affixes.Add(Affix.AffixId.GetID(), Affix);
}

void UHarmoniaRandomItemSubsystem::UnregisterAffix(FName AffixID)
{
	Affixes.Remove(AffixID);
}

bool UHarmoniaRandomItemSubsystem::GetAffix(FName AffixID, FHarmoniaAffixData& OutAffix) const
{
	if (const FHarmoniaAffixData* Found = Affixes.Find(AffixID))
	{
		OutAffix = *Found;
		return true;
	}
	return false;
}

TArray<FHarmoniaAffixData> UHarmoniaRandomItemSubsystem::GetAllAffixes() const
{
	TArray<FHarmoniaAffixData> Result;
	Affixes.GenerateValueArray(Result);
	return Result;
}

TArray<FHarmoniaAffixData> UHarmoniaRandomItemSubsystem::GetAffixesByType(EAffixType Type) const
{
	TArray<FHarmoniaAffixData> Result;
	for (const auto& Pair : Affixes)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FGeneratedItemData UHarmoniaRandomItemSubsystem::GenerateItem(const FItemGenerationParams& Settings)
{
	FGeneratedItemData Item;
	Item.ItemLevel = Settings.ItemLevel;
	Item.Rarity = RollRarity(Settings.MagicFindBonus);
	
	// Generate affixes based on rarity
	int32 AffixCount = GetAffixCountForRarity(Item.Rarity);
	
	// Roll prefixes
	int32 PrefixCount = FMath::RandRange(0, FMath::Min(AffixCount, 3));
	for (int32 i = 0; i < PrefixCount; ++i)
	{
		TArray<FHarmoniaAffixData> ValidPrefixes = GetValidAffixes(EAffixType::Prefix, EAffixType::Prefix, Settings.ItemLevel, Settings.ExcludedItemTags);
		if (const FHarmoniaAffixData* SelectedAffix = SelectWeightedAffix(ValidPrefixes))
		{
			Item.Affixes.Add(RollAffix(*SelectedAffix));
		}
	}
	
	// Roll suffixes
	int32 SuffixCount = AffixCount - PrefixCount;
	for (int32 i = 0; i < SuffixCount; ++i)
	{
		TArray<FHarmoniaAffixData> ValidSuffixes = GetValidAffixes(EAffixType::Suffix, EAffixType::Suffix, Settings.ItemLevel, Settings.ExcludedItemTags);
		if (const FHarmoniaAffixData* SelectedAffix = SelectWeightedAffix(ValidSuffixes))
		{
			Item.Affixes.Add(RollAffix(*SelectedAffix));
		}
	}
	
	UpdateCombinedModifiers(Item);
	OnItemGenerated.Broadcast(Item);
	
	return Item;
}

FGeneratedItemData UHarmoniaRandomItemSubsystem::GenerateItemWithRarity(const FItemGenerationParams& Settings, EItemRarity Rarity)
{
	FItemGenerationParams ModifiedSettings = Settings;
	ModifiedSettings.MinRarity = Rarity;
	ModifiedSettings.MaxRarity = Rarity;
	
	FGeneratedItemData Item = GenerateItem(ModifiedSettings);
	Item.Rarity = Rarity;
	return Item;
}

EItemRarity UHarmoniaRandomItemSubsystem::RollRarity(float MagicFindBonus) const
{
	// Simple rarity roll based on magic find
	float Roll = FMath::FRand() * (1.0f - MagicFindBonus);
	
	if (Roll < 0.01f) return EItemRarity::Legendary;
	if (Roll < 0.05f) return EItemRarity::Epic;
	if (Roll < 0.15f) return EItemRarity::Rare;
	if (Roll < 0.35f) return EItemRarity::Uncommon;
	return EItemRarity::Common;
}

int32 UHarmoniaRandomItemSubsystem::GetAffixCountForRarity(EItemRarity Rarity) const
{
	switch (Rarity)
	{
	case EItemRarity::Common: return 0;
	case EItemRarity::Uncommon: return 1;
	case EItemRarity::Rare: return 2;
	case EItemRarity::Epic: return 4;
	case EItemRarity::Legendary: return 6;
	default: return 0;
	}
}

FLinearColor UHarmoniaRandomItemSubsystem::GetRarityColor(EItemRarity Rarity) const
{
	if (ConfigAsset)
	{
		switch (Rarity)
		{
		case EItemRarity::Common: return ConfigAsset->CommonColor;
		case EItemRarity::Uncommon: return ConfigAsset->UncommonColor;
		case EItemRarity::Rare: return ConfigAsset->RareColor;
		case EItemRarity::Epic: return ConfigAsset->EpicColor;
		case EItemRarity::Legendary: return ConfigAsset->LegendaryColor;
		default: return FLinearColor::White;
		}
	}
	return FLinearColor::White;
}

TArray<FAffixStatModifier> UHarmoniaRandomItemSubsystem::GetCombinedModifiers(const FGeneratedItemData& Item) const
{
	TArray<FAffixStatModifier> Combined;
	// Combine stat modifiers from all affixes
	for (const FAppliedAffix& Affix : Item.Affixes)
	{
		// Get affix definition and add its modifiers
		FHarmoniaAffixData AffixDef;
		if (GetAffix(Affix.AffixId.GetID(), AffixDef))
		{
			Combined.Append(AffixDef.StatModifiers);
		}
	}
	return Combined;
}

FText UHarmoniaRandomItemSubsystem::GenerateItemName(const FGeneratedItemData& Item, const FText& BaseName) const
{
	// Simple name generation - prefix + base + suffix
	FString NameStr = BaseName.ToString();
	
	for (const FAppliedAffix& Affix : Item.Affixes)
	{
		FHarmoniaAffixData AffixDef;
		if (GetAffix(Affix.AffixId.GetID(), AffixDef))
		{
			if (Affix.Type == EAffixType::Prefix)
			{
				NameStr = AffixDef.DisplayName.ToString() + TEXT(" ") + NameStr;
			}
			else if (Affix.Type == EAffixType::Suffix)
			{
				NameStr = NameStr + TEXT(" ") + AffixDef.DisplayName.ToString();
			}
		}
	}
	
	return FText::FromString(NameStr);
}

FText UHarmoniaRandomItemSubsystem::GetAffixDescription(const FAppliedAffix& Affix) const
{
	FHarmoniaAffixData AffixDef;
	if (GetAffix(Affix.AffixId.GetID(), AffixDef))
	{
		return AffixDef.DisplayName;
	}
	return FText::GetEmpty();
}

FAppliedAffix UHarmoniaRandomItemSubsystem::RollAffix(const FHarmoniaAffixData& Affix) const
{
	FAppliedAffix Applied;
	Applied.AffixId = Affix.AffixId;
	Applied.Type = Affix.Type;
	
	// Roll values for each stat modifier
	for (const FAffixStatModifier& Modifier : Affix.StatModifiers)
	{
		float RolledValue = FMath::RandRange(Modifier.MinValue, Modifier.MaxValue);
		Applied.RolledValues.Add(Modifier.StatTag, RolledValue);
	}
	
	return Applied;
}

TArray<FHarmoniaAffixData> UHarmoniaRandomItemSubsystem::GetValidAffixes(EAffixType Type, EAffixType Slot, int32 ItemLevel, const FGameplayTagContainer& ExcludedTags) const
{
	TArray<FHarmoniaAffixData> ValidAffixes;
	
	for (const auto& Pair : Affixes)
	{
		const FHarmoniaAffixData& Affix = Pair.Value;
		
		// Check type
		if (Affix.Type != Type)
		{
			continue;
		}
		
		// Check item level
		if (ItemLevel < Affix.MinItemLevel || ItemLevel > Affix.MaxItemLevel)
		{
			continue;
		}
		
		// Check excluded tags
		if (Affix.ExcludedItemTags.HasAny(ExcludedTags))
		{
			continue;
		}
		
		ValidAffixes.Add(Affix);
	}
	
	return ValidAffixes;
}

const FHarmoniaAffixData* UHarmoniaRandomItemSubsystem::SelectWeightedAffix(const TArray<FHarmoniaAffixData>& ValidAffixes) const
{
	if (ValidAffixes.Num() == 0)
	{
		return nullptr;
	}
	
	// Calculate total weight
	int32 TotalWeight = 0;
	for (const FHarmoniaAffixData& Affix : ValidAffixes)
	{
		TotalWeight += Affix.Weight;
	}
	
	if (TotalWeight <= 0)
	{
		return &ValidAffixes[FMath::RandRange(0, ValidAffixes.Num() - 1)];
	}
	
	// Roll
	int32 Roll = FMath::RandRange(0, TotalWeight - 1);
	int32 CurrentWeight = 0;
	
	for (const FHarmoniaAffixData& Affix : ValidAffixes)
	{
		CurrentWeight += Affix.Weight;
		if (Roll < CurrentWeight)
		{
			return &Affix;
		}
	}
	
	return &ValidAffixes.Last();
}

void UHarmoniaRandomItemSubsystem::UpdateCombinedModifiers(FGeneratedItemData& Item) const
{
	Item.TotalStats.Empty();
	
	
	for (const FAppliedAffix& Affix : Item.Affixes)
	{
		for (const auto& Pair : Affix.RolledValues)
		{
			if (float* ExistingValue = Item.TotalStats.Find(Pair.Key))
			{
				*ExistingValue += Pair.Value;
			}
			else
			{
				Item.TotalStats.Add(Pair.Key, Pair.Value);
			}
		}
	}
}

TArray<FGeneratedItemData> UHarmoniaRandomItemSubsystem::GenerateItems(const FItemGenerationParams& Settings, int32 Count)
{
	TArray<FGeneratedItemData> Results;
	for (int32 i = 0; i < Count; ++i)
	{
		Results.Add(GenerateItem(Settings));
	}
	return Results;
}

bool UHarmoniaRandomItemSubsystem::RerollAffixes(FGeneratedItemData& Item, bool bKeepRarity)
{
	EItemRarity OriginalRarity = Item.Rarity;
	
	// Clear existing affixes
	Item.Affixes.Empty();
	Item.TotalStats.Empty();
	
	// Re-roll rarity if not keeping
	if (!bKeepRarity)
	{
		Item.Rarity = RollRarity(0.0f);
	}
	
	// Generate new affixes
	int32 AffixCount = GetAffixCountForRarity(Item.Rarity);
	
	int32 PrefixCount = FMath::RandRange(0, FMath::Min(AffixCount, 3));
	for (int32 i = 0; i < PrefixCount; ++i)
	{
		AddRandomAffix(Item, EAffixType::Prefix);
	}
	
	int32 SuffixCount = AffixCount - PrefixCount;
	for (int32 i = 0; i < SuffixCount; ++i)
	{
		AddRandomAffix(Item, EAffixType::Suffix);
	}
	
	UpdateCombinedModifiers(Item);
	return true;
}

bool UHarmoniaRandomItemSubsystem::AddRandomAffix(FGeneratedItemData& Item, EAffixType Type)
{
	TArray<FHarmoniaAffixData> ValidAffixes = GetValidAffixes(Type, Type, Item.ItemLevel, FGameplayTagContainer());
	if (const FHarmoniaAffixData* SelectedAffix = SelectWeightedAffix(ValidAffixes))
	{
		Item.Affixes.Add(RollAffix(*SelectedAffix));
		UpdateCombinedModifiers(Item);
		return true;
	}
	return false;
}

bool UHarmoniaRandomItemSubsystem::RemoveAffix(FGeneratedItemData& Item, int32 AffixIndex, EAffixType Type)
{
	if (AffixIndex >= 0 && AffixIndex < Item.Affixes.Num())
	{
		if (Item.Affixes[AffixIndex].Type == Type)
		{
			Item.Affixes.RemoveAt(AffixIndex);
			UpdateCombinedModifiers(Item);
			return true;
		}
	}
	return false;
}

void UHarmoniaRandomItemSubsystem::GetMaxAffixesForRarity(EItemRarity Rarity, int32& OutMaxPrefixes, int32& OutMaxSuffixes) const
{
	switch (Rarity)
	{
	case EItemRarity::Common:
		OutMaxPrefixes = 0;
		OutMaxSuffixes = 0;
		break;
	case EItemRarity::Uncommon:
		OutMaxPrefixes = 1;
		OutMaxSuffixes = 0;
		break;
	case EItemRarity::Rare:
		OutMaxPrefixes = 1;
		OutMaxSuffixes = 1;
		break;
	case EItemRarity::Epic:
		OutMaxPrefixes = 2;
		OutMaxSuffixes = 2;
		break;
	case EItemRarity::Legendary:
		OutMaxPrefixes = 3;
		OutMaxSuffixes = 3;
		break;
	default:
		OutMaxPrefixes = 0;
		OutMaxSuffixes = 0;
		break;
	}
}
