// Copyright 2025 Snow Game Studio.

#include "Enhancement/HarmoniaEnhancementSubsystem.h"
#include "Enhancement/HarmoniaEnhancementConfigDataAsset.h"
#include "GameFramework/PlayerController.h"

void UHarmoniaEnhancementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaEnhancementSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaEnhancementSubsystem::SetConfigDataAsset(UHarmoniaEnhancementConfigDataAsset* InConfig)
{
	ConfigAsset = InConfig;

	if (ConfigAsset)
	{
		for (const FEnhancementLevelConfig& Level : ConfigAsset->DefaultEnhancementLevels)
		{
			RegisterEnhancementLevel(Level);
		}

		for (const FEnchantmentDefinition& Enchant : ConfigAsset->DefaultEnchantments)
		{
			RegisterEnchantment(Enchant);
		}
	}
}

void UHarmoniaEnhancementSubsystem::RegisterEnhancementLevel(const FEnhancementLevelConfig& Level)
{
	EnhancementLevels.Add(Level.Level, Level);
}

bool UHarmoniaEnhancementSubsystem::GetEnhancementLevel(int32 Level, FEnhancementLevelConfig& OutLevel) const
{
	if (const FEnhancementLevelConfig* Found = EnhancementLevels.Find(Level))
	{
		OutLevel = *Found;
		return true;
	}
	return false;
}

int32 UHarmoniaEnhancementSubsystem::GetMaxEnhancementLevel() const
{
	int32 MaxLevel = 0;
	for (const auto& Pair : EnhancementLevels)
	{
		if (Pair.Key > MaxLevel)
		{
			MaxLevel = Pair.Key;
		}
	}
	return MaxLevel;
}

void UHarmoniaEnhancementSubsystem::RegisterEnchantment(const FEnchantmentDefinition& Enchantment)
{
	if (Enchantment.EnchantmentID.IsNone())
	{
		return;
	}

	Enchantments.Add(Enchantment.EnchantmentID, Enchantment);
}

bool UHarmoniaEnhancementSubsystem::GetEnchantment(FName EnchantmentID, FEnchantmentDefinition& OutEnchantment) const
{
	if (const FEnchantmentDefinition* Found = Enchantments.Find(EnchantmentID))
	{
		OutEnchantment = *Found;
		return true;
	}
	return false;
}

TArray<FEnchantmentDefinition> UHarmoniaEnhancementSubsystem::GetAllEnchantments() const
{
	TArray<FEnchantmentDefinition> Result;
	Enchantments.GenerateValueArray(Result);
	return Result;
}

TArray<FEnchantmentDefinition> UHarmoniaEnhancementSubsystem::GetEnchantmentsBySlot(EEnchantSlot Slot) const
{
	TArray<FEnchantmentDefinition> Result;
	for (const auto& Pair : Enchantments)
	{
		if (Pair.Value.Slot == Slot)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FEnhancedItemData UHarmoniaEnhancementSubsystem::GetItemState(FGuid ItemID) const
{
	if (const FEnhancedItemData* Found = ItemStates.Find(ItemID))
	{
		return *Found;
	}

	FEnhancedItemData DefaultState;
	DefaultState.ItemGUID = ItemID;
	return DefaultState;
}

void UHarmoniaEnhancementSubsystem::SetItemState(FGuid ItemID, const FEnhancedItemData& State)
{
	ItemStates.Add(ItemID, State);
}

FEnhancedItemData UHarmoniaEnhancementSubsystem::InitializeItemState(int32 InitialSockets)
{
	FEnhancedItemData State;
	State.ItemGUID = FGuid::NewGuid();
	State.Sockets.SetNum(InitialSockets);
	ItemStates.Add(State.ItemGUID, State);
	return State;
}

bool UHarmoniaEnhancementSubsystem::CanEnhance(APlayerController* Player, FGuid ItemID, FText& OutReason) const
{
	if (!Player)
	{
		OutReason = NSLOCTEXT("Enhancement", "NoPlayer", "Player not found.");
		return false;
	}

	FEnhancedItemData State = GetItemState(ItemID);
	int32 MaxLevel = GetMaxEnhancementLevel();

	if (State.EnhancementLevel >= MaxLevel)
	{
		OutReason = NSLOCTEXT("Enhancement", "MaxLevel", "Maximum enhancement level reached.");
		return false;
	}

	FEnhancementLevelConfig NextLevel;
	if (!GetEnhancementLevel(State.EnhancementLevel + 1, NextLevel))
	{
		OutReason = NSLOCTEXT("Enhancement", "NoLevelData", "Enhancement data not found.");
		return false;
	}

	// TODO: Check materials and currency

	OutReason = FText::GetEmpty();
	return true;
}

FHarmoniaEnhancementSessionResult UHarmoniaEnhancementSubsystem::AttemptEnhance(APlayerController* Player, FGuid ItemID, bool bUseProtection)
{
	FHarmoniaEnhancementSessionResult Result;

	FText Reason;
	if (!CanEnhance(Player, ItemID, Reason))
	{
		Result.Result = EEnhancementResult::InvalidItem;
		Result.ErrorMessage = Reason;
		return Result;
	}

	FEnhancedItemData& State = ItemStates.FindOrAdd(ItemID);
	if (!State.ItemGUID.IsValid())
	{
		State.ItemGUID = ItemID;
	}

	Result.PreviousLevel = State.EnhancementLevel;

	FEnhancementLevelConfig LevelData;
	if (!GetEnhancementLevel(State.EnhancementLevel + 1, LevelData))
	{
		Result.Result = EEnhancementResult::InvalidItem;
		return Result;
	}

	// Convert materials map to array for consumption
	TArray<FEnhancementMaterial> MaterialsArray;
	for (const auto& Pair : LevelData.RequiredMaterials)
	{
		FEnhancementMaterial Material;
		Material.ItemId = Pair.Key;
		Material.Quantity = Pair.Value;
		MaterialsArray.Add(Material);
	}

	// Consume materials and currency
	if (!ConsumeMaterials(Player, MaterialsArray))
	{
		Result.Result = EEnhancementResult::InsufficientMaterials;
		return Result;
	}

	if (!ConsumeCurrency(Player, LevelData.RequiredCurrency))
	{
		Result.Result = EEnhancementResult::InsufficientCurrency;
		return Result;
	}

	Result.ConsumedMaterials = LevelData.RequiredMaterials;
	Result.CurrencySpent = LevelData.RequiredCurrency;

	// Check pity
	bool bPityActive = IsPityActive(ItemID);

	// Use protection if requested
	if (bUseProtection && State.ProtectionCount > 0)
	{
		State.ProtectionCount--;
		Result.bProtectionUsed = true;
	}

	// Roll result
	Result.Result = RollEnhancementResult(LevelData, bPityActive, bUseProtection);

	// Apply result
	switch (Result.Result)
	{
		case EEnhancementResult::Success:
			State.EnhancementLevel++;
			State.PityCounter = 0;
			break;

		case EEnhancementResult::GreatSuccess:
			State.EnhancementLevel += 2;
			State.EnhancementLevel = FMath::Min(State.EnhancementLevel, GetMaxEnhancementLevel());
			State.PityCounter = 0;
			break;

		case EEnhancementResult::Failure:
			State.FailedAttempts++;
			State.PityCounter++;
			break;

		case EEnhancementResult::Downgrade:
			State.EnhancementLevel = FMath::Max(0, State.EnhancementLevel - 1);
			State.FailedAttempts++;
			State.PityCounter++;
			break;

		case EEnhancementResult::Destruction:
			OnItemDestroyed.Broadcast(Player, ItemID);
			ItemStates.Remove(ItemID);
			break;

		case EEnhancementResult::Protected:
			State.FailedAttempts++;
			// No pity increase when protected
			break;

		default:
			break;
	}

	Result.NewLevel = State.EnhancementLevel;
	OnEnhancementAttempted.Broadcast(Player, Result);
	return Result;
}

float UHarmoniaEnhancementSubsystem::GetSuccessRate(FGuid ItemID, float BonusRate) const
{
	FEnhancedItemData State = GetItemState(ItemID);

	FEnhancementLevelConfig LevelData;
	if (!GetEnhancementLevel(State.EnhancementLevel + 1, LevelData))
	{
		return 0.0f;
	}

	float Rate = LevelData.SuccessChance + BonusRate;

	// Apply pity bonus
	if (ConfigAsset && ConfigAsset->PityBonusPerFailure > 0.0f)
	{
		Rate += State.PityCounter * ConfigAsset->PityBonusPerFailure;
	}

	return FMath::Clamp(Rate, 0.0f, 1.0f);
}

TArray<FEnhancementMaterial> UHarmoniaEnhancementSubsystem::GetRequiredMaterials(FGuid ItemID) const
{
	FEnhancedItemData State = GetItemState(ItemID);

	FEnhancementLevelConfig LevelData;
	if (GetEnhancementLevel(State.EnhancementLevel + 1, LevelData))
	{
		// Convert TMap to TArray
		TArray<FEnhancementMaterial> Materials;
		for (const auto& Pair : LevelData.RequiredMaterials)
		{
			FEnhancementMaterial Material;
			Material.ItemId = Pair.Key;
			Material.Quantity = Pair.Value;
			Materials.Add(Material);
		}
		return Materials;
	}

	return TArray<FEnhancementMaterial>();
}

int64 UHarmoniaEnhancementSubsystem::GetEnhancementCost(FGuid ItemID) const
{
	FEnhancedItemData State = GetItemState(ItemID);

	FEnhancementLevelConfig LevelData;
	if (GetEnhancementLevel(State.EnhancementLevel + 1, LevelData))
	{
		return LevelData.RequiredCurrency;
	}

	return 0;
}

bool UHarmoniaEnhancementSubsystem::ApplyEnchantment(APlayerController* Player, FGuid ItemID, FName EnchantmentID, EEnchantSlot Slot)
{
	if (!Player || !IsEnchantmentCompatible(ItemID, EnchantmentID, Slot))
	{
		return false;
	}

	FEnhancedItemData& State = ItemStates.FindOrAdd(ItemID);
	if (!State.ItemGUID.IsValid())
	{
		State.ItemGUID = ItemID;
	}

	State.AppliedEnchantments.Add(Slot, EnchantmentID);
	OnEnchantmentApplied.Broadcast(Player, ItemID, EnchantmentID);
	return true;
}

bool UHarmoniaEnhancementSubsystem::RemoveEnchantment(APlayerController* Player, FGuid ItemID, EEnchantSlot Slot)
{
	FEnhancedItemData* State = ItemStates.Find(ItemID);
	if (!State)
	{
		return false;
	}

	return State->AppliedEnchantments.Remove(Slot) > 0;
}

TMap<EEnchantSlot, FName> UHarmoniaEnhancementSubsystem::GetAppliedEnchantments(FGuid ItemID) const
{
	FEnhancedItemData State = GetItemState(ItemID);
	return State.AppliedEnchantments;
}

bool UHarmoniaEnhancementSubsystem::IsEnchantmentCompatible(FGuid ItemID, FName EnchantmentID, EEnchantSlot Slot) const
{
	FEnchantmentDefinition Enchant;
	if (!GetEnchantment(EnchantmentID, Enchant))
	{
		return false;
	}

	if (Enchant.Slot != Slot)
	{
		return false;
	}

	FEnhancedItemData State = GetItemState(ItemID);

	// Check exclusives
	for (const auto& Pair : State.AppliedEnchantments)
	{
		if (Enchant.ExclusiveWith.Contains(Pair.Value))
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaEnhancementSubsystem::AddSocket(APlayerController* Player, FGuid ItemID)
{
	FEnhancedItemData& State = ItemStates.FindOrAdd(ItemID);
	if (!State.ItemGUID.IsValid())
	{
		State.ItemGUID = ItemID;
	}

	int32 MaxSockets = ConfigAsset ? ConfigAsset->MaxSockets : 3;
	if (State.Sockets.Num() >= MaxSockets)
	{
		return false;
	}

	State.Sockets.Add(FSocketData());
	return true;
}

bool UHarmoniaEnhancementSubsystem::InsertGem(APlayerController* Player, FGuid ItemID, int32 SocketIndex, FGameplayTag GemTag)
{
	FEnhancedItemData& State = ItemStates.FindOrAdd(ItemID);
	if (SocketIndex < 0 || SocketIndex >= State.Sockets.Num())
	{
		return false;
	}

	// Map socket index to slot
	EEnchantSlot Slot;
	switch (SocketIndex)
	{
		case 0: Slot = EEnchantSlot::Socket1; break;
		case 1: Slot = EEnchantSlot::Socket2; break;
		case 2: Slot = EEnchantSlot::Socket3; break;
		default: return false;
	}

	// Use gem tag as enchantment ID
	State.AppliedEnchantments.Add(Slot, GemTag.GetTagName());
	return true;
}

bool UHarmoniaEnhancementSubsystem::RemoveGem(APlayerController* Player, FGuid ItemID, int32 SocketIndex)
{
	EEnchantSlot Slot;
	switch (SocketIndex)
	{
		case 0: Slot = EEnchantSlot::Socket1; break;
		case 1: Slot = EEnchantSlot::Socket2; break;
		case 2: Slot = EEnchantSlot::Socket3; break;
		default: return false;
	}

	return RemoveEnchantment(Player, ItemID, Slot);
}

bool UHarmoniaEnhancementSubsystem::ApplyProtection(APlayerController* Player, FGuid ItemID, EProtectionType ProtectionType)
{
	FEnhancedItemData& State = ItemStates.FindOrAdd(ItemID);
	if (!State.ItemGUID.IsValid())
	{
		State.ItemGUID = ItemID;
	}

	State.ProtectionCount++;
	return true;
}

int32 UHarmoniaEnhancementSubsystem::GetProtectionCount(FGuid ItemID) const
{
	return GetItemState(ItemID).ProtectionCount;
}

int32 UHarmoniaEnhancementSubsystem::GetPityCounter(FGuid ItemID) const
{
	return GetItemState(ItemID).PityCounter;
}

int32 UHarmoniaEnhancementSubsystem::GetPityThreshold(int32 EnhancementLevel) const
{
	if (ConfigAsset)
	{
		return ConfigAsset->BasePityThreshold + (EnhancementLevel * ConfigAsset->PityThresholdPerLevel);
	}
	return 10 + EnhancementLevel;
}

bool UHarmoniaEnhancementSubsystem::IsPityActive(FGuid ItemID) const
{
	FEnhancedItemData State = GetItemState(ItemID);
	return State.PityCounter >= GetPityThreshold(State.EnhancementLevel);
}

float UHarmoniaEnhancementSubsystem::GetStatMultiplier(int32 EnhancementLevel) const
{
	FEnhancementLevelConfig LevelData;
	if (GetEnhancementLevel(EnhancementLevel, LevelData))
	{
		return LevelData.StatMultiplier;
	}

	// Default: 5% per level
	return 1.0f + (EnhancementLevel * 0.05f);
}

TMap<FGameplayTag, float> UHarmoniaEnhancementSubsystem::GetEnchantmentBonuses(FGuid ItemID) const
{
	TMap<FGameplayTag, float> Result;

	FEnhancedItemData State = GetItemState(ItemID);
	for (const auto& Pair : State.AppliedEnchantments)
	{
		FEnchantmentDefinition Enchant;
		if (GetEnchantment(Pair.Value, Enchant))
		{
			for (const FEquipmentStatModifier& Modifier : Enchant.StatModifiers)
			{
				FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Modifier.AttributeName), false);
				if (Tag.IsValid())
				{
					float& CurrentValue = Result.FindOrAdd(Tag);
					CurrentValue += Modifier.Value;
				}
			}
		}
	}

	return Result;
}

bool UHarmoniaEnhancementSubsystem::ConsumeMaterials(APlayerController* Player, const TArray<FEnhancementMaterial>& Materials)
{
	// TODO: Integrate with inventory system
	return true;
}

bool UHarmoniaEnhancementSubsystem::ConsumeCurrency(APlayerController* Player, int64 Amount)
{
	// TODO: Integrate with shop/currency system
	return true;
}

EEnhancementResult UHarmoniaEnhancementSubsystem::RollEnhancementResult(const FEnhancementLevelConfig& Level, bool bPityActive, bool bUseProtection)
{
	// Pity guarantees success
	if (bPityActive)
	{
		return EEnhancementResult::Success;
	}

	// Roll for great success first
	if (Level.GreatSuccessChance > 0.0f && FMath::FRand() <= Level.GreatSuccessChance)
	{
		return EEnhancementResult::GreatSuccess;
	}

	// Roll for success
	if (FMath::FRand() <= Level.SuccessChance)
	{
		return EEnhancementResult::Success;
	}

	// Failed - determine penalty
	if (bUseProtection)
	{
		return EEnhancementResult::Protected;
	}

	// Roll for destruction
	if (Level.DestructionChance > 0.0f && FMath::FRand() <= Level.DestructionChance)
	{
		return EEnhancementResult::Destruction;
	}

	// Roll for downgrade
	if (Level.LevelDownChance > 0.0f && FMath::FRand() <= Level.LevelDownChance)
	{
		return EEnhancementResult::Downgrade;
	}

	return EEnhancementResult::Failure;
}
