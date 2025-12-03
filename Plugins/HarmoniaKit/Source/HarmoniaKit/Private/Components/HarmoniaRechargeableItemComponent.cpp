// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaRechargeableItemComponent.h"
#include "System/HarmoniaCheckpointSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UHarmoniaRechargeableItemComponent::UHarmoniaRechargeableItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHarmoniaRechargeableItemComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeCheckpointIntegration();
}

void UHarmoniaRechargeableItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaRechargeableItemComponent, ItemStates);
}

// ============================================================================
// Item Registration
// ============================================================================

bool UHarmoniaRechargeableItemComponent::RegisterRecoveryItem(EHarmoniaRecoveryItemType ItemType, const FHarmoniaRecoveryItemConfig& Config)
{
	if (IsItemRegistered(ItemType))
	{
		UE_LOG(LogTemp, Warning, TEXT("Item type %d is already registered"), static_cast<int32>(ItemType));
		return false;
	}

	// Record settings
	RegisteredItems.Add(ItemType, Config);

	// Generate initial state
	FHarmoniaRecoveryItemState NewState;
	NewState.ItemType = ItemType;
	NewState.CurrentCharges = Config.InitialCharges;
	NewState.MaxCharges = Config.MaxCharges;
	NewState.ShardFrequency = EHarmoniaResonanceFrequency::Azure; // Default
	ItemStates.Add(NewState);

	OnChargesChanged.Broadcast(ItemType, Config.InitialCharges);

	return true;
}

bool UHarmoniaRechargeableItemComponent::RegisterResonanceShard(EHarmoniaResonanceFrequency Frequency, const FHarmoniaResonanceShardVariant& VariantConfig)
{
	if (ResonanceShardVariants.Contains(Frequency))
	{
		UE_LOG(LogTemp, Warning, TEXT("Resonance Shard with frequency %d is already registered"), static_cast<int32>(Frequency));
		return false;
	}

	// Record changes
	ResonanceShardVariants.Add(Frequency, VariantConfig);

	// Generate state (all resonance shards use ResonanceShard type)
	FHarmoniaRecoveryItemState NewState;
	NewState.ItemType = EHarmoniaRecoveryItemType::ResonanceShard;
	NewState.ShardFrequency = Frequency;
	NewState.CurrentCharges = 5; // Default initial charges
	NewState.MaxCharges = 5; // Default max charges
	ItemStates.Add(NewState);

	OnChargesChanged.Broadcast(EHarmoniaRecoveryItemType::ResonanceShard, NewState.CurrentCharges);

	return true;
}

void UHarmoniaRechargeableItemComponent::UnregisterRecoveryItem(EHarmoniaRecoveryItemType ItemType)
{
	RegisteredItems.Remove(ItemType);
	ItemStates.RemoveAll([ItemType](const FHarmoniaRecoveryItemState& State) { return State.ItemType == ItemType; });
}

// ============================================================================
// Item Usage
// ============================================================================

bool UHarmoniaRechargeableItemComponent::UseRecoveryItem(EHarmoniaRecoveryItemType ItemType)
{
	FText Reason;
	if (!CanUseRecoveryItem(ItemType, Reason))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot use recovery item: %s"), *Reason.ToString());
		OnItemUsed.Broadcast(ItemType, 0, false);
		return false;
	}

	// Find charge count
	for (FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == ItemType)
		{
			State.CurrentCharges--;
			State.LastUsedTime = FDateTime::Now();

			OnItemUsed.Broadcast(ItemType, State.CurrentCharges, true);
			OnChargesChanged.Broadcast(ItemType, State.CurrentCharges);

			return true;
		}
	}

	return false;
}

bool UHarmoniaRechargeableItemComponent::UseResonanceShard(EHarmoniaResonanceFrequency Frequency)
{
	// Find resonance shard of the frequency
	for (FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == EHarmoniaRecoveryItemType::ResonanceShard && State.ShardFrequency == Frequency)
		{
			if (State.CurrentCharges <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Resonance Shard (Frequency: %d) has no charges left"), static_cast<int32>(Frequency));
				OnItemUsed.Broadcast(EHarmoniaRecoveryItemType::ResonanceShard, 0, false);
				return false;
			}

			State.CurrentCharges--;
			State.LastUsedTime = FDateTime::Now();

			OnItemUsed.Broadcast(EHarmoniaRecoveryItemType::ResonanceShard, State.CurrentCharges, true);
			OnChargesChanged.Broadcast(EHarmoniaRecoveryItemType::ResonanceShard, State.CurrentCharges);

			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Resonance Shard with frequency %d is not registered"), static_cast<int32>(Frequency));
	return false;
}

bool UHarmoniaRechargeableItemComponent::CanUseRecoveryItem(EHarmoniaRecoveryItemType ItemType, FText& OutReason) const
{
	if (!IsItemRegistered(ItemType))
	{
		OutReason = FText::FromString(TEXT("Item is not registered"));
		return false;
	}

	// Check charge count
	for (const FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == ItemType)
		{
			if (State.CurrentCharges <= 0)
			{
				OutReason = FText::FromString(TEXT("No charges remaining"));
				return false;
			}
			return true;
		}
	}

	OutReason = FText::FromString(TEXT("Item state not found"));
	return false;
}

int32 UHarmoniaRechargeableItemComponent::GetRemainingCharges(EHarmoniaRecoveryItemType ItemType) const
{
	for (const FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == ItemType)
		{
			return State.CurrentCharges;
		}
	}
	return 0;
}

int32 UHarmoniaRechargeableItemComponent::GetMaxCharges(EHarmoniaRecoveryItemType ItemType) const
{
	for (const FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == ItemType)
		{
			return State.MaxCharges;
		}
	}
	return 0;
}

bool UHarmoniaRechargeableItemComponent::GetRecoveryItemConfig(EHarmoniaRecoveryItemType ItemType, FHarmoniaRecoveryItemConfig& OutConfig) const
{
	if (const FHarmoniaRecoveryItemConfig* Config = RegisteredItems.Find(ItemType))
	{
		OutConfig = *Config;
		return true;
	}
	return false;
}

bool UHarmoniaRechargeableItemComponent::GetResonanceShardVariant(EHarmoniaResonanceFrequency Frequency, FHarmoniaResonanceShardVariant& OutVariant) const
{
	if (const FHarmoniaResonanceShardVariant* Variant = ResonanceShardVariants.Find(Frequency))
	{
		OutVariant = *Variant;
		return true;
	}
	return false;
}

// ============================================================================
// Item Recharge
// ============================================================================

int32 UHarmoniaRechargeableItemComponent::RechargeItem(EHarmoniaRecoveryItemType ItemType, int32 Amount)
{
	int32 TotalRecharged = 0;

	for (FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == ItemType)
		{
			int32 OldCharges = State.CurrentCharges;

			if (Amount < 0)
			{
				// 최�?치까지 충전
				State.CurrentCharges = State.MaxCharges;
			}
			else
			{
				// Charge only specified amount
				State.CurrentCharges = FMath::Min(State.CurrentCharges + Amount, State.MaxCharges);
			}

			TotalRecharged = State.CurrentCharges - OldCharges;

			if (TotalRecharged > 0)
			{
				OnItemRecharged.Broadcast(ItemType, State.CurrentCharges, State.MaxCharges);
				OnChargesChanged.Broadcast(ItemType, State.CurrentCharges);
			}

			break;
		}
	}

	return TotalRecharged;
}

int32 UHarmoniaRechargeableItemComponent::RechargeAllItems()
{
	int32 TotalRecharged = 0;

	for (FHarmoniaRecoveryItemState& State : ItemStates)
	{
		int32 OldCharges = State.CurrentCharges;
		State.CurrentCharges = State.MaxCharges;
		int32 Recharged = State.CurrentCharges - OldCharges;

		if (Recharged > 0)
		{
			TotalRecharged += Recharged;
			OnItemRecharged.Broadcast(State.ItemType, State.CurrentCharges, State.MaxCharges);
			OnChargesChanged.Broadcast(State.ItemType, State.CurrentCharges);
		}
	}

	return TotalRecharged;
}

int32 UHarmoniaRechargeableItemComponent::RechargeRechargeableItems()
{
	int32 TotalRecharged = 0;

	for (FHarmoniaRecoveryItemState& State : ItemStates)
	{
		// Check if can be charged
		const FHarmoniaRecoveryItemConfig* Config = RegisteredItems.Find(State.ItemType);
		if (Config && Config->bRechargeableAtCheckpoint)
		{
			int32 OldCharges = State.CurrentCharges;
			State.CurrentCharges = State.MaxCharges;
			int32 Recharged = State.CurrentCharges - OldCharges;

			if (Recharged > 0)
			{
				TotalRecharged += Recharged;
				OnItemRecharged.Broadcast(State.ItemType, State.CurrentCharges, State.MaxCharges);
				OnChargesChanged.Broadcast(State.ItemType, State.CurrentCharges);
			}
		}
	}

	return TotalRecharged;
}

// ============================================================================
// Item Upgrade
// ============================================================================

bool UHarmoniaRechargeableItemComponent::UpgradeItemMaxCharges(EHarmoniaRecoveryItemType ItemType, int32 AdditionalMaxCharges)
{
	for (FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == ItemType)
		{
			State.MaxCharges += AdditionalMaxCharges;
			State.CurrentCharges = FMath::Min(State.CurrentCharges, State.MaxCharges); // Clamp current charges

			OnItemUpgraded.Broadcast(ItemType, State.MaxCharges);
			return true;
		}
	}

	return false;
}

// ============================================================================
// Checkpoint Integration
// ============================================================================

void UHarmoniaRechargeableItemComponent::OnCheckpointResonanceCompleted(FName CheckpointID, const FHarmoniaResonanceResult& Result)
{
	if (Result.bSuccess)
	{
		// Charge all chargeable items when checkpoint resonance succeeds
		int32 TotalRecharged = RechargeRechargeableItems();

		UE_LOG(LogTemp, Log, TEXT("Checkpoint resonance completed. Recharged %d items at checkpoint %s"), TotalRecharged, *CheckpointID.ToString());
	}
}

void UHarmoniaRechargeableItemComponent::InitializeCheckpointIntegration()
{
	// Get checkpoint subsystem
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>();

			if (CheckpointSubsystem)
			{
				// Bind to resonance complete event
				CheckpointSubsystem->OnResonanceCompleted.AddDynamic(this, &UHarmoniaRechargeableItemComponent::OnCheckpointResonanceCompleted);

				UE_LOG(LogTemp, Log, TEXT("Rechargeable Item Component integrated with Checkpoint Subsystem"));
			}
		}
	}
}

// ============================================================================
// Data Persistence
// ============================================================================

TArray<FHarmoniaRecoveryItemState> UHarmoniaRechargeableItemComponent::GetItemStatesForSave() const
{
	return ItemStates;
}

void UHarmoniaRechargeableItemComponent::LoadItemStates(const TArray<FHarmoniaRecoveryItemState>& LoadedStates)
{
	ItemStates = LoadedStates;

	// Broadcast event on load
	for (const FHarmoniaRecoveryItemState& State : ItemStates)
	{
		OnChargesChanged.Broadcast(State.ItemType, State.CurrentCharges);
	}
}

// ============================================================================
// Internal
// ============================================================================

bool UHarmoniaRechargeableItemComponent::IsItemRegistered(EHarmoniaRecoveryItemType ItemType) const
{
	return RegisteredItems.Contains(ItemType);
}

void UHarmoniaRechargeableItemComponent::SetCharges(EHarmoniaRecoveryItemType ItemType, int32 NewCharges)
{
	for (FHarmoniaRecoveryItemState& State : ItemStates)
	{
		if (State.ItemType == ItemType)
		{
			State.CurrentCharges = FMath::Clamp(NewCharges, 0, State.MaxCharges);
			OnChargesChanged.Broadcast(ItemType, State.CurrentCharges);
			break;
		}
	}
}

void UHarmoniaRechargeableItemComponent::OnRep_ItemStates()
{
	// Update UI after replication
	for (const FHarmoniaRecoveryItemState& State : ItemStates)
	{
		OnChargesChanged.Broadcast(State.ItemType, State.CurrentCharges);
	}
}
