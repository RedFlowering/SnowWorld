// Copyright 2025 Snow Game Studio.

#include "HarmoniaLogCategories.h"
#include "Components/HarmoniaEnhancementSystemComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Interfaces/IRepairStation.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "GenericTeamAgentInterface.h"

UHarmoniaEnhancementSystemComponent::UHarmoniaEnhancementSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHarmoniaEnhancementSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find ability system component
	AActor* Owner = GetOwner();
	if (Owner)
	{
		AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	}
}

void UHarmoniaEnhancementSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaEnhancementSystemComponent, EnhancedItems);
}

void UHarmoniaEnhancementSystemComponent::Initialize(UHarmoniaInventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;
}

// ============================================================================
// Enhancement System
// ============================================================================

bool UHarmoniaEnhancementSystemComponent::GetEnhancementData(FGuid ItemGUID, FEnhancedItemData& OutData) const
{
	if (const FEnhancedItemData* Data = FindEnhancedItem(ItemGUID))
	{
		OutData = *Data;
		return true;
	}
	return false;
}

bool UHarmoniaEnhancementSystemComponent::CanEnhanceItem(FGuid ItemGUID, int32 TargetLevel, FString& OutReason) const
{
	// Check if item exists
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		OutReason = TEXT("Item not found");
		return false;
	}

	// Check if already at target level
	if (ItemData->EnhancementLevel >= TargetLevel)
	{
		OutReason = TEXT("Item already at or above target level");
		return false;
	}

	// Check if target level is valid
	if (TargetLevel > MaxEnhancementLevel)
	{
		OutReason = FString::Printf(TEXT("Target level exceeds maximum (%d)"), MaxEnhancementLevel);
		return false;
	}

	// Check if we can only enhance one level at a time
	if (TargetLevel != ItemData->EnhancementLevel + 1)
	{
		OutReason = TEXT("Can only enhance one level at a time");
		return false;
	}

	// Get config for target level
	FEnhancementLevelConfig Config;
	if (!GetEnhancementLevelConfig(TargetLevel, Config))
	{
		OutReason = TEXT("Enhancement level config not found");
		return false;
	}

	// Check materials
	if (!HasMaterials(Config.RequiredMaterials))
	{
		OutReason = TEXT("Insufficient materials");
		return false;
	}

	// Check currency
	if (!HasCurrency(Config.RequiredCurrency))
	{
		OutReason = TEXT("Insufficient currency");
		return false;
	}

	return true;
}

bool UHarmoniaEnhancementSystemComponent::EnhanceItem(FGuid ItemGUID, int32 TargetLevel, bool bUseProtection)
{
	if (GetOwner()->HasAuthority())
	{
		return ServerEnhanceItem(ItemGUID, TargetLevel, bUseProtection), true;
	}
	else
	{
		ServerEnhanceItem(ItemGUID, TargetLevel, bUseProtection);
		return false; // Wait for server response
	}
}

void UHarmoniaEnhancementSystemComponent::ServerEnhanceItem_Implementation(FGuid ItemGUID, int32 TargetLevel, bool bUseProtection)
{
	FString Reason;
	if (!CanEnhanceItem(ItemGUID, TargetLevel, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Cannot enhance item: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	// Get config
	FEnhancementLevelConfig Config;
	if (!GetEnhancementLevelConfig(TargetLevel, Config))
	{
		return;
	}

	// Consume materials and currency
	if (!ConsumeMaterials(Config.RequiredMaterials))
	{
		return;
	}

	if (!ConsumeCurrency(Config.RequiredCurrency))
	{
		return;
	}

	// Consume protection stone if used
	if (bUseProtection && bAllowProtectionStones && ProtectionStoneItemId.IsValid())
	{
		TMap<FHarmoniaID, int32> ProtectionStone;
		ProtectionStone.Add(ProtectionStoneItemId, 1);
		ConsumeMaterials(ProtectionStone);
	}

	// Start enhancement
	CurrentSession = FEnhancementSession();
	CurrentSession.TargetItemGUID = ItemGUID;
	CurrentSession.CurrentLevel = ItemData->EnhancementLevel;
	CurrentSession.TargetLevel = TargetLevel;

	OnEnhancementStarted.Broadcast(ItemGUID, TargetLevel);

	// Roll result
	EEnhancementResult Result = RollEnhancementResult(Config, bUseProtection);

	int32 NewLevel = ItemData->EnhancementLevel;

	switch (Result)
	{
	case EEnhancementResult::Success:
		NewLevel = TargetLevel;
		ApplyEnhancement(*ItemData, NewLevel);
		break;

	case EEnhancementResult::GreatSuccess:
		// Great success gives bonus level
		NewLevel = FMath::Min(TargetLevel + 1, MaxEnhancementLevel);
		ApplyEnhancement(*ItemData, NewLevel);
		break;

	case EEnhancementResult::Failure:
		// No change
		break;

	case EEnhancementResult::LevelDown:
		NewLevel = FMath::Max(0, ItemData->EnhancementLevel - 1);
		ApplyEnhancement(*ItemData, NewLevel);
		break;

	case EEnhancementResult::Destruction:
		DestroyEnhancedItem(ItemGUID, true);
		NewLevel = -1; // Item destroyed
		break;

	default:
		break;
	}

	OnEnhancementCompleted.Broadcast(ItemGUID, Result, NewLevel);
}

bool UHarmoniaEnhancementSystemComponent::GetEnhancementLevelConfig(int32 Level, FEnhancementLevelConfig& OutConfig) const
{
	if (!EnhancementLevelConfigTable)
	{
		return false;
	}

	FString ContextString(TEXT("GetEnhancementLevelConfig"));
	FEnhancementLevelConfig* Config = EnhancementLevelConfigTable->FindRow<FEnhancementLevelConfig>(
		FName(*FString::FromInt(Level)), ContextString);

	if (Config)
	{
		OutConfig = *Config;
		return true;
	}

	return false;
}

float UHarmoniaEnhancementSystemComponent::GetTotalStatMultiplier(const FEnhancedItemData& ItemData) const
{
	float Multiplier = 1.0f;

	// Enhancement multiplier
	FEnhancementLevelConfig EnhanceConfig;
	if (GetEnhancementLevelConfig(ItemData.EnhancementLevel, EnhanceConfig))
	{
		Multiplier *= EnhanceConfig.StatMultiplier;
	}

	// Transcendence multiplier
	if (ItemData.TranscendenceTier != ETranscendenceTier::None)
	{
		FTranscendenceConfig TransConfig;
		if (GetTranscendenceConfig(ItemData.TranscendenceTier, TransConfig))
		{
			Multiplier *= (1.0f + TransConfig.StatMultiplierBonus);
		}
	}

	return Multiplier;
}

void UHarmoniaEnhancementSystemComponent::GetEnhancedStatModifiers(const FEnhancedItemData& ItemData, TArray<FEquipmentStatModifier>& OutModifiers) const
{
	OutModifiers.Reset();

	// Get base item stats
	FEquipmentData BaseData;
	if (GetItemBaseData(ItemData.ItemId, BaseData))
	{
		OutModifiers = BaseData.StatModifiers;
	}

	// Apply enhancement multiplier
	float StatMultiplier = GetTotalStatMultiplier(ItemData);
	for (FEquipmentStatModifier& Modifier : OutModifiers)
	{
		Modifier.Value *= StatMultiplier;
	}

	// Add enhancement bonus stats
	FEnhancementLevelConfig EnhanceConfig;
	if (GetEnhancementLevelConfig(ItemData.EnhancementLevel, EnhanceConfig))
	{
		OutModifiers.Append(EnhanceConfig.BonusStats);
	}

	// Add transcendence bonus stats
	if (ItemData.TranscendenceTier != ETranscendenceTier::None)
	{
		FTranscendenceConfig TransConfig;
		if (GetTranscendenceConfig(ItemData.TranscendenceTier, TransConfig))
		{
			OutModifiers.Append(TransConfig.BonusStats);
		}
	}

	// Add gem stats
	for (const FSocketData& Socket : ItemData.Sockets)
	{
		if (Socket.HasGem())
		{
			FGemData GemData;
			if (GetGemData(Socket.InsertedGemId, GemData))
			{
				OutModifiers.Append(GemData.StatModifiers);
			}
		}
	}

	// Add reforged stats
	for (const FReforgeStatEntry& ReforgeStat : ItemData.ReforgedStats)
	{
		FEquipmentStatModifier Modifier;

		// Convert EReforgeStatType to attribute name
		switch (ReforgeStat.StatType)
		{
		case EReforgeStatType::Attack:
			Modifier.AttributeName = TEXT("AttackPower");
			break;
		case EReforgeStatType::Defense:
			Modifier.AttributeName = TEXT("Defense");
			break;
		case EReforgeStatType::MaxHealth:
			Modifier.AttributeName = TEXT("MaxHealth");
			break;
		case EReforgeStatType::MaxMana:
			Modifier.AttributeName = TEXT("MaxMana");
			break;
		case EReforgeStatType::CriticalChance:
			Modifier.AttributeName = TEXT("CriticalChance");
			break;
		case EReforgeStatType::CriticalDamage:
			Modifier.AttributeName = TEXT("CriticalDamage");
			break;
		case EReforgeStatType::AttackSpeed:
			Modifier.AttributeName = TEXT("AttackSpeed");
			break;
		case EReforgeStatType::MovementSpeed:
			Modifier.AttributeName = TEXT("MovementSpeed");
			break;
		default:
			continue;
		}

		Modifier.ModifierType = ReforgeStat.bIsPercentage ? EStatModifierType::Percentage : EStatModifierType::Flat;
		Modifier.Value = ReforgeStat.Value;
		OutModifiers.Add(Modifier);
	}

	// Apply durability penalty
	if (bEnableDurabilityPenalty)
	{
		ApplyDurabilityPenalty(ItemData.ItemGUID, OutModifiers);
	}
}

// ============================================================================
// Socket/Gem System
// ============================================================================

bool UHarmoniaEnhancementSystemComponent::AddSocket(FGuid ItemGUID, EGemSocketType SocketType, bool bUnlocked)
{
	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return false;
	}

	ItemData->Sockets.Add(FSocketData(SocketType, bUnlocked));
	return true;
}

bool UHarmoniaEnhancementSystemComponent::UnlockSocket(FGuid ItemGUID, int32 SocketIndex)
{
	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData || !ItemData->Sockets.IsValidIndex(SocketIndex))
	{
		return false;
	}

	ItemData->Sockets[SocketIndex].bIsUnlocked = true;
	return true;
}

bool UHarmoniaEnhancementSystemComponent::CanInsertGem(FGuid ItemGUID, int32 SocketIndex, FHarmoniaID GemId, FString& OutReason) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		OutReason = TEXT("Item not found");
		return false;
	}

	if (!ItemData->Sockets.IsValidIndex(SocketIndex))
	{
		OutReason = TEXT("Invalid socket index");
		return false;
	}

	const FSocketData& Socket = ItemData->Sockets[SocketIndex];

	if (!Socket.bIsUnlocked)
	{
		OutReason = TEXT("Socket is locked");
		return false;
	}

	if (Socket.HasGem())
	{
		OutReason = TEXT("Socket already has a gem");
		return false;
	}

	FGemData GemData;
	if (!GetGemData(GemId, GemData))
	{
		OutReason = TEXT("Gem data not found");
		return false;
	}

	if (!GemData.CanInsertIntoSocket(Socket.SocketType))
	{
		OutReason = TEXT("Gem is not compatible with this socket type");
		return false;
	}

	return true;
}

bool UHarmoniaEnhancementSystemComponent::InsertGem(FGuid ItemGUID, int32 SocketIndex, FHarmoniaID GemId)
{
	if (GetOwner()->HasAuthority())
	{
		ServerInsertGem(ItemGUID, SocketIndex, GemId);
		return true;
	}
	else
	{
		ServerInsertGem(ItemGUID, SocketIndex, GemId);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerInsertGem_Implementation(FGuid ItemGUID, int32 SocketIndex, FHarmoniaID GemId)
{
	FString Reason;
	if (!CanInsertGem(ItemGUID, SocketIndex, GemId, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Cannot insert gem: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	// TODO: Remove gem from inventory

	ItemData->Sockets[SocketIndex].InsertedGemId = GemId;
	OnGemInserted.Broadcast(ItemGUID, SocketIndex, GemId);
}

bool UHarmoniaEnhancementSystemComponent::RemoveGem(FGuid ItemGUID, int32 SocketIndex, bool bDestroyGem)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRemoveGem(ItemGUID, SocketIndex, bDestroyGem);
		return true;
	}
	else
	{
		ServerRemoveGem(ItemGUID, SocketIndex, bDestroyGem);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerRemoveGem_Implementation(FGuid ItemGUID, int32 SocketIndex, bool bDestroyGem)
{
	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData || !ItemData->Sockets.IsValidIndex(SocketIndex))
	{
		return;
	}

	FSocketData& Socket = ItemData->Sockets[SocketIndex];
	if (!Socket.HasGem())
	{
		return;
	}

	FHarmoniaID RemovedGemId = Socket.InsertedGemId;

	// Return gem to inventory if not destroying
	if (!bDestroyGem)
	{
		// TODO: Add gem back to inventory
	}

	Socket.InsertedGemId = FHarmoniaID();
	OnGemRemoved.Broadcast(ItemGUID, SocketIndex, RemovedGemId);
}

bool UHarmoniaEnhancementSystemComponent::GetGemData(FHarmoniaID GemId, FGemData& OutData) const
{
	if (!GemDataTable)
	{
		return false;
	}

	FString ContextString(TEXT("GetGemData"));
	FGemData* Data = GemDataTable->FindRow<FGemData>(FName(*GemId.ToString()), ContextString);

	if (Data)
	{
		OutData = *Data;
		return true;
	}

	return false;
}

void UHarmoniaEnhancementSystemComponent::GetInsertedGems(FGuid ItemGUID, TArray<FGemData>& OutGems) const
{
	OutGems.Reset();

	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	for (const FSocketData& Socket : ItemData->Sockets)
	{
		if (Socket.HasGem())
		{
			FGemData GemData;
			if (GetGemData(Socket.InsertedGemId, GemData))
			{
				OutGems.Add(GemData);
			}
		}
	}
}

// ============================================================================
// Reforge System
// ============================================================================

bool UHarmoniaEnhancementSystemComponent::CanReforgeItem(FGuid ItemGUID, FString& OutReason) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		OutReason = TEXT("Item not found");
		return false;
	}

	FEquipmentData BaseData;
	if (!GetItemBaseData(ItemData->ItemId, BaseData))
	{
		OutReason = TEXT("Base item data not found");
		return false;
	}

	FReforgeConfig Config;
	if (!GetReforgeConfig(BaseData.Grade, Config))
	{
		OutReason = TEXT("Reforge config not found");
		return false;
	}

	if (!HasMaterials(Config.RequiredMaterials))
	{
		OutReason = TEXT("Insufficient materials");
		return false;
	}

	if (!HasCurrency(Config.RequiredCurrency))
	{
		OutReason = TEXT("Insufficient currency");
		return false;
	}

	return true;
}

bool UHarmoniaEnhancementSystemComponent::ReforgeItem(FGuid ItemGUID, bool bLockCurrentStats)
{
	if (GetOwner()->HasAuthority())
	{
		ServerReforgeItem(ItemGUID);
		return true;
	}
	else
	{
		ServerReforgeItem(ItemGUID);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerReforgeItem_Implementation(FGuid ItemGUID)
{
	FString Reason;
	if (!CanReforgeItem(ItemGUID, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Cannot reforge item: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	FEquipmentData BaseData;
	if (!GetItemBaseData(ItemData->ItemId, BaseData))
	{
		return;
	}

	FReforgeConfig Config;
	if (!GetReforgeConfig(BaseData.Grade, Config))
	{
		return;
	}

	// Consume materials and currency
	if (!ConsumeMaterials(Config.RequiredMaterials))
	{
		return;
	}

	if (!ConsumeCurrency(Config.RequiredCurrency))
	{
		return;
	}

	// Roll new stats
	TArray<FReforgeStatEntry> NewStats = RollReforgeStats(Config);
	ItemData->ReforgedStats = NewStats;

	OnItemReforged.Broadcast(ItemGUID, NewStats);
}

bool UHarmoniaEnhancementSystemComponent::GetReforgeConfig(EItemGrade ItemGrade, FReforgeConfig& OutConfig) const
{
	if (!ReforgeConfigTable)
	{
		return false;
	}

	FString ContextString(TEXT("GetReforgeConfig"));

	// Try to find by exact grade
	FString GradeName = UEnum::GetValueAsString(ItemGrade);
	GradeName = GradeName.Right(GradeName.Len() - GradeName.Find(TEXT("::")) - 2);

	FReforgeConfig* Config = ReforgeConfigTable->FindRow<FReforgeConfig>(FName(*GradeName), ContextString);

	if (Config)
	{
		OutConfig = *Config;
		return true;
	}

	return false;
}

bool UHarmoniaEnhancementSystemComponent::LockReforgeStat(FGuid ItemGUID, int32 StatIndex, bool bLocked)
{
	// TODO: Implement stat locking system
	return false;
}

// ============================================================================
// Transcendence System
// ============================================================================

bool UHarmoniaEnhancementSystemComponent::CanTranscendItem(FGuid ItemGUID, ETranscendenceTier TargetTier, FString& OutReason) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		OutReason = TEXT("Item not found");
		return false;
	}

	if (ItemData->TranscendenceTier >= TargetTier)
	{
		OutReason = TEXT("Item already at or above target tier");
		return false;
	}

	FTranscendenceConfig Config;
	if (!GetTranscendenceConfig(TargetTier, Config))
	{
		OutReason = TEXT("Transcendence config not found");
		return false;
	}

	FEquipmentData BaseData;
	if (!GetItemBaseData(ItemData->ItemId, BaseData))
	{
		OutReason = TEXT("Base item data not found");
		return false;
	}

	if (BaseData.Grade < Config.RequiredGrade)
	{
		OutReason = TEXT("Item grade too low");
		return false;
	}

	if (ItemData->EnhancementLevel < Config.RequiredEnhancementLevel)
	{
		OutReason = FString::Printf(TEXT("Enhancement level too low (requires +%d)"), Config.RequiredEnhancementLevel);
		return false;
	}

	// Check materials
	TMap<FHarmoniaID, int32> MaterialMap;
	for (const FTranscendenceMaterial& Mat : Config.RequiredMaterials)
	{
		MaterialMap.Add(Mat.MaterialId, Mat.Amount);
	}

	if (!HasMaterials(MaterialMap))
	{
		OutReason = TEXT("Insufficient materials");
		return false;
	}

	if (!HasCurrency(Config.RequiredCurrency))
	{
		OutReason = TEXT("Insufficient currency");
		return false;
	}

	return true;
}

bool UHarmoniaEnhancementSystemComponent::TranscendItem(FGuid ItemGUID, ETranscendenceTier TargetTier)
{
	if (GetOwner()->HasAuthority())
	{
		ServerTranscendItem(ItemGUID, TargetTier);
		return true;
	}
	else
	{
		ServerTranscendItem(ItemGUID, TargetTier);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerTranscendItem_Implementation(FGuid ItemGUID, ETranscendenceTier TargetTier)
{
	FString Reason;
	if (!CanTranscendItem(ItemGUID, TargetTier, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Cannot transcend item: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	FTranscendenceConfig Config;
	if (!GetTranscendenceConfig(TargetTier, Config))
	{
		return;
	}

	// Consume materials
	TMap<FHarmoniaID, int32> MaterialMap;
	for (const FTranscendenceMaterial& Mat : Config.RequiredMaterials)
	{
		MaterialMap.Add(Mat.MaterialId, Mat.Amount);
	}

	if (!ConsumeMaterials(MaterialMap))
	{
		return;
	}

	if (!ConsumeCurrency(Config.RequiredCurrency))
	{
		return;
	}

	// Apply transcendence
	ItemData->TranscendenceTier = TargetTier;

	OnItemTranscended.Broadcast(ItemGUID, TargetTier);
}

bool UHarmoniaEnhancementSystemComponent::GetTranscendenceConfig(ETranscendenceTier Tier, FTranscendenceConfig& OutConfig) const
{
	if (!TranscendenceConfigTable)
	{
		return false;
	}

	FString ContextString(TEXT("GetTranscendenceConfig"));

	FString TierName = UEnum::GetValueAsString(Tier);
	TierName = TierName.Right(TierName.Len() - TierName.Find(TEXT("::")) - 2);

	FTranscendenceConfig* Config = TranscendenceConfigTable->FindRow<FTranscendenceConfig>(FName(*TierName), ContextString);

	if (Config)
	{
		OutConfig = *Config;
		return true;
	}

	return false;
}

// ============================================================================
// Transmog System
// ============================================================================

bool UHarmoniaEnhancementSystemComponent::CanApplyTransmog(FGuid TargetItemGUID, FHarmoniaID AppearanceItemId, FString& OutReason) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(TargetItemGUID);
	if (!ItemData)
	{
		OutReason = TEXT("Item not found");
		return false;
	}

	FEquipmentData AppearanceData;
	if (!GetItemBaseData(AppearanceItemId, AppearanceData))
	{
		OutReason = TEXT("Appearance item data not found");
		return false;
	}

	FEquipmentData BaseData;
	if (!GetItemBaseData(ItemData->ItemId, BaseData))
	{
		OutReason = TEXT("Base item data not found");
		return false;
	}

	// Check if same slot
	if (AppearanceData.EquipmentSlot != BaseData.EquipmentSlot)
	{
		OutReason = TEXT("Appearance item must be same equipment slot");
		return false;
	}

	if (!HasCurrency(TransmogCost))
	{
		OutReason = TEXT("Insufficient currency");
		return false;
	}

	return true;
}

bool UHarmoniaEnhancementSystemComponent::ApplyTransmog(FGuid TargetItemGUID, FHarmoniaID AppearanceItemId)
{
	if (GetOwner()->HasAuthority())
	{
		ServerApplyTransmog(TargetItemGUID, AppearanceItemId);
		return true;
	}
	else
	{
		ServerApplyTransmog(TargetItemGUID, AppearanceItemId);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerApplyTransmog_Implementation(FGuid TargetItemGUID, FHarmoniaID AppearanceItemId)
{
	FString Reason;
	if (!CanApplyTransmog(TargetItemGUID, AppearanceItemId, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Cannot apply transmog: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(TargetItemGUID);
	if (!ItemData)
	{
		return;
	}

	if (!ConsumeCurrency(TransmogCost))
	{
		return;
	}

	FEquipmentData AppearanceData;
	if (!GetItemBaseData(AppearanceItemId, AppearanceData))
	{
		return;
	}

	// Apply transmog
	ItemData->Transmog.OriginalEquipmentId = AppearanceItemId;
	ItemData->Transmog.OverrideMesh = AppearanceData.EquipmentMesh;
	ItemData->Transmog.OverrideMaterials = AppearanceData.MaterialInstances;
	ItemData->Transmog.bIsActive = true;

	OnTransmogApplied.Broadcast(TargetItemGUID, AppearanceItemId);
}

bool UHarmoniaEnhancementSystemComponent::RemoveTransmog(FGuid ItemGUID)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRemoveTransmog(ItemGUID);
		return true;
	}
	else
	{
		ServerRemoveTransmog(ItemGUID);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerRemoveTransmog_Implementation(FGuid ItemGUID)
{
	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	ItemData->Transmog = FTransmogData();
	OnTransmogRemoved.Broadcast(ItemGUID);
}

bool UHarmoniaEnhancementSystemComponent::GetTransmogData(FGuid ItemGUID, FTransmogData& OutData) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return false;
	}

	OutData = ItemData->Transmog;
	return true;
}

// ============================================================================
// Repair System
// ============================================================================

void UHarmoniaEnhancementSystemComponent::DamageItemDurability(FGuid ItemGUID, float DamageAmount)
{
	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	ItemData->CurrentDurability = FMath::Max(0.0f, ItemData->CurrentDurability - DamageAmount);

	if (ItemData->IsBroken())
	{
		OnItemDestroyed.Broadcast(ItemGUID, false);
	}
	else if (bAutoRepairEnabled && ItemData->GetDurabilityPercent() < AutoRepairThreshold)
	{
		TryAutoRepair(ItemGUID);
	}
}

bool UHarmoniaEnhancementSystemComponent::CanRepairItem(FGuid ItemGUID, FString& OutReason) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		OutReason = TEXT("Item not found");
		return false;
	}

	if (!ItemData->IsDamaged())
	{
		OutReason = TEXT("Item is not damaged");
		return false;
	}

	int32 Cost = GetRepairCost(ItemGUID, true, 0.0f);
	if (!HasCurrency(Cost))
	{
		OutReason = TEXT("Insufficient currency");
		return false;
	}

	return true;
}

bool UHarmoniaEnhancementSystemComponent::RepairItem(FGuid ItemGUID, bool bFullRepair, float RepairAmount)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRepairItem(ItemGUID, bFullRepair, RepairAmount);
		return true;
	}
	else
	{
		ServerRepairItem(ItemGUID, bFullRepair, RepairAmount);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerRepairItem_Implementation(FGuid ItemGUID, bool bFullRepair, float RepairAmount)
{
	FString Reason;
	if (!CanRepairItem(ItemGUID, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Cannot repair item: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	int32 Cost = GetRepairCost(ItemGUID, bFullRepair, RepairAmount);
	if (!ConsumeCurrency(Cost))
	{
		return;
	}

	if (bFullRepair)
	{
		ItemData->CurrentDurability = ItemData->MaxDurability;
	}
	else
	{
		ItemData->CurrentDurability = FMath::Min(ItemData->MaxDurability, ItemData->CurrentDurability + RepairAmount);
	}

	OnItemRepaired.Broadcast(ItemGUID, ItemData->CurrentDurability);
}

int32 UHarmoniaEnhancementSystemComponent::GetRepairCost(FGuid ItemGUID, bool bFullRepair, float RepairAmount) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return 0;
	}

	FEquipmentData BaseData;
	if (!GetItemBaseData(ItemData->ItemId, BaseData))
	{
		return 0;
	}

	FRepairConfig Config;
	if (!GetRepairConfig(BaseData.Grade, Config))
	{
		return 0;
	}

	float DurabilityToRepair = bFullRepair ? (ItemData->MaxDurability - ItemData->CurrentDurability) : RepairAmount;
	return FMath::CeilToInt(DurabilityToRepair * Config.CostPerDurability);
}

bool UHarmoniaEnhancementSystemComponent::GetRepairConfig(EItemGrade ItemGrade, FRepairConfig& OutConfig) const
{
	if (!RepairConfigTable)
	{
		return false;
	}

	FString ContextString(TEXT("GetRepairConfig"));

	FString GradeName = UEnum::GetValueAsString(ItemGrade);
	GradeName = GradeName.Right(GradeName.Len() - GradeName.Find(TEXT("::")) - 2);

	FRepairConfig* Config = RepairConfigTable->FindRow<FRepairConfig>(FName(*GradeName), ContextString);

	if (Config)
	{
		OutConfig = *Config;
		return true;
	}

	return false;
}

bool UHarmoniaEnhancementSystemComponent::GetRepairKitData(FHarmoniaID RepairKitId, FRepairKitData& OutData) const
{
	if (!RepairKitDataTable)
	{
		return false;
	}

	FString ContextString(TEXT("GetRepairKitData"));
	FRepairKitData* Data = RepairKitDataTable->FindRow<FRepairKitData>(FName(*RepairKitId.ToString()), ContextString);

	if (Data)
	{
		OutData = *Data;
		return true;
	}

	return false;
}

bool UHarmoniaEnhancementSystemComponent::CanUseRepairKit(FGuid ItemGUID, FHarmoniaID RepairKitId, FString& OutReason) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		OutReason = TEXT("Item not found");
		return false;
	}

	if (!ItemData->IsDamaged())
	{
		OutReason = TEXT("Item is not damaged");
		return false;
	}

	FRepairKitData KitData;
	if (!GetRepairKitData(RepairKitId, KitData))
	{
		OutReason = TEXT("Repair kit data not found");
		return false;
	}

	FEquipmentData BaseData;
	if (!GetItemBaseData(ItemData->ItemId, BaseData))
	{
		OutReason = TEXT("Base item data not found");
		return false;
	}

	// Check grade compatibility
	if (!KitData.CanRepairGrade(BaseData.Grade))
	{
		OutReason = TEXT("Repair kit cannot repair this item grade");
		return false;
	}

	// Check slot compatibility
	if (!KitData.CanRepairSlot(BaseData.EquipmentSlot))
	{
		OutReason = TEXT("Repair kit cannot repair this equipment slot");
		return false;
	}

	// Check combat restriction
	if (!KitData.bCanUseInCombat && GetOwner() && GetOwner()->Implements<UGenericTeamAgentInterface>())
	{
		// TODO: Check if in combat
		// For now, always allow
	}

	return true;
}

bool UHarmoniaEnhancementSystemComponent::RepairItemWithKit(FGuid ItemGUID, FHarmoniaID RepairKitId)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRepairItemWithKit(ItemGUID, RepairKitId);
		return true;
	}
	else
	{
		ServerRepairItemWithKit(ItemGUID, RepairKitId);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerRepairItemWithKit_Implementation(FGuid ItemGUID, FHarmoniaID RepairKitId)
{
	FString Reason;
	if (!CanUseRepairKit(ItemGUID, RepairKitId, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Cannot use repair kit: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return;
	}

	FRepairKitData KitData;
	if (!GetRepairKitData(RepairKitId, KitData))
	{
		return;
	}

	// Consume repair kit from inventory
	// TODO: Implement inventory consumption

	// Calculate repair amount
	float RepairAmount = ItemData->MaxDurability * KitData.DurabilityRestored;
	RepairAmount += ItemData->MaxDurability * KitData.QualityBonus;

	// Apply repair
	ItemData->CurrentDurability = FMath::Min(ItemData->MaxDurability, ItemData->CurrentDurability + RepairAmount);

	OnItemRepaired.Broadcast(ItemGUID, ItemData->CurrentDurability);
}

bool UHarmoniaEnhancementSystemComponent::RepairItemAtStation(FGuid ItemGUID, AActor* RepairStation, bool bFullRepair)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRepairItemAtStation(ItemGUID, RepairStation, bFullRepair);
		return true;
	}
	else
	{
		ServerRepairItemAtStation(ItemGUID, RepairStation, bFullRepair);
		return false;
	}
}

void UHarmoniaEnhancementSystemComponent::ServerRepairItemAtStation_Implementation(FGuid ItemGUID, AActor* RepairStation, bool bFullRepair)
{
	if (!RepairStation)
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Repair station is null"));
		return;
	}

	// Check if actor implements repair station interface
	if (!RepairStation->Implements<URepairStation>())
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Actor does not implement IRepairStation interface"));
		return;
	}

	IRepairStation* Station = Cast<IRepairStation>(RepairStation);
	if (!Station)
	{
		return;
	}

	// Check if station is available
	if (!Station->Execute_IsAvailableForRepair(RepairStation, GetOwner()))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Repair station is not available"));
		return;
	}

	// Check if station can repair this item
	FString Reason;
	if (!Station->Execute_CanRepairItem(RepairStation, ItemGUID, Reason))
	{
		UE_LOG(LogHarmoniaEnhancement, Warning, TEXT("Station cannot repair item: %s"), *Reason);
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData || !ItemData->IsDamaged())
	{
		return;
	}

	// Get discounted cost from station
	int32 BaseCost = GetRepairCost(ItemGUID, bFullRepair, 0.0f);
	int32 DiscountedCost = Station->Execute_GetDiscountedRepairCost(RepairStation, ItemGUID, BaseCost);

	// Consume currency
	if (!ConsumeCurrency(DiscountedCost))
	{
		return;
	}

	// Notify station
	Station->Execute_OnRepairStarted(RepairStation, GetOwner(), ItemGUID);

	// Get station data for quality bonus
	FRepairStationData StationData = Station->Execute_GetRepairStationData(RepairStation);

	float DurabilityToRepair;
	if (bFullRepair)
	{
		DurabilityToRepair = ItemData->MaxDurability - ItemData->CurrentDurability;
	}
	else
	{
		// Partial repair based on cost
		DurabilityToRepair = ItemData->MaxDurability * 0.5f; // 50% repair
	}

	// Apply quality bonus
	DurabilityToRepair += ItemData->MaxDurability * StationData.QualityBonus;

	// Apply repair
	ItemData->CurrentDurability = FMath::Min(ItemData->MaxDurability, ItemData->CurrentDurability + DurabilityToRepair);

	// Notify station
	Station->Execute_OnRepairCompleted(RepairStation, GetOwner(), ItemGUID, DurabilityToRepair);

	OnItemRepaired.Broadcast(ItemGUID, ItemData->CurrentDurability);
}

int32 UHarmoniaEnhancementSystemComponent::GetRepairCostAtStation(FGuid ItemGUID, AActor* RepairStation, bool bFullRepair) const
{
	if (!RepairStation || !RepairStation->Implements<URepairStation>())
	{
		return GetRepairCost(ItemGUID, bFullRepair, 0.0f);
	}

	IRepairStation* Station = Cast<IRepairStation>(RepairStation);
	if (!Station)
	{
		return GetRepairCost(ItemGUID, bFullRepair, 0.0f);
	}

	int32 BaseCost = GetRepairCost(ItemGUID, bFullRepair, 0.0f);
	return Station->Execute_GetDiscountedRepairCost(RepairStation, ItemGUID, BaseCost);
}

float UHarmoniaEnhancementSystemComponent::GetDurabilityPenaltyMultiplier(FGuid ItemGUID) const
{
	if (!bEnableDurabilityPenalty)
	{
		return 1.0f;
	}

	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return 1.0f;
	}

	float DurabilityPercent = ItemData->GetDurabilityPercent();

	// No penalty above threshold
	if (DurabilityPercent >= DurabilityPenaltyThreshold)
	{
		return 1.0f;
	}

	// Linear penalty from threshold to 0
	float PenaltyRange = DurabilityPenaltyThreshold;
	float CurrentBelowThreshold = DurabilityPenaltyThreshold - DurabilityPercent;
	float PenaltyRatio = CurrentBelowThreshold / PenaltyRange;

	// Calculate final multiplier
	float Penalty = MaxDurabilityPenalty * PenaltyRatio;
	return FMath::Clamp(1.0f - Penalty, 0.0f, 1.0f);
}

bool UHarmoniaEnhancementSystemComponent::ShouldShowDurabilityWarning(FGuid ItemGUID, float WarningThreshold) const
{
	const FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData)
	{
		return false;
	}

	return ItemData->GetDurabilityPercent() < WarningThreshold;
}

// ============================================================================
// Item Management
// ============================================================================

FGuid UHarmoniaEnhancementSystemComponent::CreateEnhancedItem(FHarmoniaID ItemId, int32 InitialEnhancementLevel)
{
	FEnhancedItemData NewItem;
	NewItem.ItemGUID = FGuid::NewGuid();
	NewItem.ItemId = ItemId;
	NewItem.EnhancementLevel = InitialEnhancementLevel;

	// Get base data for durability
	FEquipmentData BaseData;
	if (GetItemBaseData(ItemId, BaseData))
	{
		NewItem.MaxDurability = BaseData.MaxDurability;
		NewItem.CurrentDurability = BaseData.MaxDurability;
	}

	EnhancedItems.Add(NewItem);
	return NewItem.ItemGUID;
}

void UHarmoniaEnhancementSystemComponent::DestroyEnhancedItem(FGuid ItemGUID, bool bWasEnhancementFailure)
{
	int32 RemovedCount = EnhancedItems.RemoveAll([ItemGUID](const FEnhancedItemData& Item)
	{
		return Item.ItemGUID == ItemGUID;
	});

	if (RemovedCount > 0)
	{
		OnItemDestroyed.Broadcast(ItemGUID, bWasEnhancementFailure);
	}
}

bool UHarmoniaEnhancementSystemComponent::ItemExists(FGuid ItemGUID) const
{
	return FindEnhancedItem(ItemGUID) != nullptr;
}

bool UHarmoniaEnhancementSystemComponent::GetItemBaseData(FHarmoniaID ItemId, FEquipmentData& OutData) const
{
	if (!EquipmentDataTable)
	{
		return false;
	}

	FString ContextString(TEXT("GetItemBaseData"));
	FEquipmentData* Data = EquipmentDataTable->FindRow<FEquipmentData>(FName(*ItemId.ToString()), ContextString);

	if (Data)
	{
		OutData = *Data;
		return true;
	}

	return false;
}

// ============================================================================
// Internal Methods
// ============================================================================

EEnhancementResult UHarmoniaEnhancementSystemComponent::RollEnhancementResult(const FEnhancementLevelConfig& Config, bool bUseProtection) const
{
	float Roll = FMath::FRand();

	// Check for great success first
	if (Roll < Config.GreatSuccessChance)
	{
		return EEnhancementResult::GreatSuccess;
	}

	// Check for success
	if (Roll < Config.SuccessChance)
	{
		return EEnhancementResult::Success;
	}

	// Failed - now check failure type
	Roll = FMath::FRand();

	// Protection prevents destruction and level down
	if (bUseProtection)
	{
		return EEnhancementResult::Failure;
	}

	// Check for destruction
	if (Roll < Config.DestructionChance)
	{
		return EEnhancementResult::Destruction;
	}

	// Check for level down
	if (Roll < Config.DestructionChance + Config.LevelDownChance)
	{
		return EEnhancementResult::LevelDown;
	}

	return EEnhancementResult::Failure;
}

void UHarmoniaEnhancementSystemComponent::ApplyEnhancement(FEnhancedItemData& ItemData, int32 NewLevel)
{
	ItemData.EnhancementLevel = NewLevel;

	// Optionally increase max durability with enhancement
	FEnhancementLevelConfig Config;
	if (GetEnhancementLevelConfig(NewLevel, Config))
	{
		// Increase max durability by 5% per level
		ItemData.MaxDurability *= (1.0f + 0.05f * NewLevel);
	}
}

TArray<FReforgeStatEntry> UHarmoniaEnhancementSystemComponent::RollReforgeStats(const FReforgeConfig& Config) const
{
	TArray<FReforgeStatEntry> Stats;

	if (Config.PossibleStats.Num() == 0)
	{
		return Stats;
	}

	int32 StatCount = FMath::RandRange(Config.MinStatCount, Config.MaxStatCount);
	StatCount = FMath::Min(StatCount, Config.PossibleStats.Num());

	TArray<EReforgeStatType> AvailableStats = Config.PossibleStats;

	for (int32 i = 0; i < StatCount; ++i)
	{
		int32 Index = FMath::RandRange(0, AvailableStats.Num() - 1);
		EReforgeStatType StatType = AvailableStats[Index];
		AvailableStats.RemoveAt(Index); // No duplicate stats

		float Value = FMath::FRandRange(Config.MinStatValue, Config.MaxStatValue);
		bool bIsPercentage = FMath::RandBool();

		Stats.Add(FReforgeStatEntry(StatType, Value, bIsPercentage));
	}

	return Stats;
}

bool UHarmoniaEnhancementSystemComponent::ConsumeMaterials(const TMap<FHarmoniaID, int32>& Materials)
{
	if (!InventoryComponent)
	{
		return false;
	}

	// TODO: Implement material consumption via inventory component
	return true;
}

bool UHarmoniaEnhancementSystemComponent::ConsumeCurrency(int32 Amount)
{
	// TODO: Implement currency consumption
	return true;
}

bool UHarmoniaEnhancementSystemComponent::HasMaterials(const TMap<FHarmoniaID, int32>& Materials) const
{
	if (!InventoryComponent)
	{
		return false;
	}

	// TODO: Implement material check via inventory component
	return true;
}

bool UHarmoniaEnhancementSystemComponent::HasCurrency(int32 Amount) const
{
	// TODO: Implement currency check
	return true;
}

void UHarmoniaEnhancementSystemComponent::ApplyStatModifiers(const TArray<FEquipmentStatModifier>& Modifiers)
{
	// TODO: Apply stat modifiers to ability system component
}

void UHarmoniaEnhancementSystemComponent::RemoveStatModifiers(const TArray<FEquipmentStatModifier>& Modifiers)
{
	// TODO: Remove stat modifiers from ability system component
}

void UHarmoniaEnhancementSystemComponent::OnRep_EnhancedItems()
{
	// Handle replication
}

// ============================================================================
// Helper Methods
// ============================================================================

void UHarmoniaEnhancementSystemComponent::TryAutoRepair(FGuid ItemGUID)
{
	if (!bAutoRepairEnabled)
	{
		return;
	}

	FEnhancedItemData* ItemData = FindEnhancedItem(ItemGUID);
	if (!ItemData || !ItemData->IsDamaged())
	{
		return;
	}

	// Try repair kits first if preferred
	if (bAutoRepairPreferKits)
	{
		// TODO: Search inventory for appropriate repair kit
		// For now, skip to currency repair
	}

	// Fall back to currency repair
	FString Reason;
	if (CanRepairItem(ItemGUID, Reason))
	{
		RepairItem(ItemGUID, true, 0.0f);
	}
}

void UHarmoniaEnhancementSystemComponent::ApplyDurabilityPenalty(FGuid ItemGUID, TArray<FEquipmentStatModifier>& InOutModifiers) const
{
	float PenaltyMultiplier = GetDurabilityPenaltyMultiplier(ItemGUID);

	// Apply penalty to all stat modifiers
	for (FEquipmentStatModifier& Modifier : InOutModifiers)
	{
		// Only apply to flat and percentage modifiers, not overrides
		if (Modifier.ModifierType != EStatModifierType::Override)
		{
			Modifier.Value *= PenaltyMultiplier;
		}
	}
}

FEnhancedItemData* UHarmoniaEnhancementSystemComponent::FindEnhancedItem(FGuid ItemGUID)
{
	for (FEnhancedItemData& Item : EnhancedItems)
	{
		if (Item.ItemGUID == ItemGUID)
		{
			return &Item;
		}
	}
	return nullptr;
}

const FEnhancedItemData* UHarmoniaEnhancementSystemComponent::FindEnhancedItem(FGuid ItemGUID) const
{
	for (const FEnhancedItemData& Item : EnhancedItems)
	{
		if (Item.ItemGUID == ItemGUID)
		{
			return &Item;
		}
	}
	return nullptr;
}
