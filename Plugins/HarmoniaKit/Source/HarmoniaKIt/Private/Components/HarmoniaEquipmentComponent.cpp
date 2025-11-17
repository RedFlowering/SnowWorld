// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaEquipmentComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"

UHarmoniaEquipmentComponent::UHarmoniaEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UHarmoniaEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up all equipment
	UnequipAll();
	Super::EndPlay(EndPlayReason);
}

void UHarmoniaEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHarmoniaEquipmentComponent, EquippedItems);
}

// ============================================================================
// Equipment Management
// ============================================================================

bool UHarmoniaEquipmentComponent::EquipItem(const FHarmoniaID& EquipmentId, EEquipmentSlot Slot)
{
	// Server authority
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerEquipItem(EquipmentId, Slot);
		return true;
	}

	if (!EquipmentId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipItem: Invalid EquipmentId"));
		return false;
	}

	// Get equipment data
	FEquipmentData EquipmentData;
	if (!GetEquipmentData(EquipmentId, EquipmentData))
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipItem: Equipment data not found for ID: %s"), *EquipmentId.ToString());
		return false;
	}

	// Determine slot
	EEquipmentSlot TargetSlot = (Slot == EEquipmentSlot::None) ? EquipmentData.EquipmentSlot : Slot;
	if (TargetSlot == EEquipmentSlot::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipItem: Invalid equipment slot"));
		return false;
	}

	// Unequip existing item in the slot
	FHarmoniaID OldEquipmentId;
	FEquippedItem* ExistingItem = EquippedItems.FindByPredicate([TargetSlot](const FEquippedItem& Item)
	{
		return Item.Slot == TargetSlot;
	});
	if (ExistingItem)
	{
		OldEquipmentId = ExistingItem->EquipmentId;
		UnequipItem(TargetSlot);
	}

	// Create equipped item
	FEquippedItem NewEquippedItem;
	NewEquippedItem.EquipmentId = EquipmentId;
	NewEquippedItem.Slot = TargetSlot;
	NewEquippedItem.CurrentDurability = EquipmentData.MaxDurability;
	NewEquippedItem.InstanceGUID = FGuid::NewGuid();

	// Apply stat modifiers
	ApplyStatModifiers(EquipmentData);

	// Apply gameplay effects
	ApplyGameplayEffects(EquipmentData, NewEquippedItem);

	// Apply visual mesh
	ApplyVisualMesh(EquipmentData);

	// Store equipped item
	EquippedItems.Add(NewEquippedItem);

	// Broadcast event
	OnEquipmentChanged.Broadcast(TargetSlot, OldEquipmentId, EquipmentId);
	OnEquipmentStatsChanged.Broadcast(TargetSlot, EquipmentData.StatModifiers);

	UE_LOG(LogTemp, Log, TEXT("EquipItem: Successfully equipped %s to slot %d"),
		*EquipmentData.DisplayName.ToString(), static_cast<int32>(TargetSlot));

	return true;
}

bool UHarmoniaEquipmentComponent::UnequipItem(EEquipmentSlot Slot)
{
	// Server authority
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerUnequipItem(Slot);
		return true;
	}

	int32 ItemIndex = EquippedItems.FindLastByPredicate([Slot](const FEquippedItem& Item)
	{
		return Item.Slot == Slot;
	});

	if (ItemIndex == INDEX_NONE)
	{
		return false;
	}

	FEquippedItem EquippedItem = EquippedItems[ItemIndex];
	FHarmoniaID OldEquipmentId = EquippedItem.EquipmentId;

	// Get equipment data
	FEquipmentData EquipmentData;
	if (GetEquipmentData(EquippedItem.EquipmentId, EquipmentData))
	{
		// Remove stat modifiers
		RemoveStatModifiers(EquipmentData);

		// Remove gameplay effects
		RemoveGameplayEffects(EquippedItem);

		// Remove visual mesh
		RemoveVisualMesh(Slot);
	}

	// Remove from equipped items
	EquippedItems.RemoveAt(ItemIndex);

	// Broadcast event
	OnEquipmentChanged.Broadcast(Slot, OldEquipmentId, FHarmoniaID());

	UE_LOG(LogTemp, Log, TEXT("UnequipItem: Successfully unequipped from slot %d"), static_cast<int32>(Slot));

	return true;
}

bool UHarmoniaEquipmentComponent::SwapEquipment(EEquipmentSlot SlotA, EEquipmentSlot SlotB)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	int32 IndexA = EquippedItems.FindLastByPredicate([SlotA](const FEquippedItem& Item)
	{
		return Item.Slot == SlotA;
	});

	int32 IndexB = EquippedItems.FindLastByPredicate([SlotB](const FEquippedItem& Item)
	{
		return Item.Slot == SlotB;
	});

	if (IndexA == INDEX_NONE && IndexB == INDEX_NONE)
	{
		return false;
	}

	// Simple swap
	if (IndexA != INDEX_NONE && IndexB != INDEX_NONE)
	{
		Swap(EquippedItems[IndexA], EquippedItems[IndexB]);
	}
	else if (IndexA != INDEX_NONE)
	{
		EquippedItems[IndexA].Slot = SlotB;
	}
	else if (IndexB != INDEX_NONE)
	{
		EquippedItems[IndexB].Slot = SlotA;
	}

	return true;
}

FEquippedItem UHarmoniaEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	const FEquippedItem* FoundItem = EquippedItems.FindByPredicate([Slot](const FEquippedItem& Item)
	{
		return Item.Slot == Slot;
	});
	return FoundItem ? *FoundItem : FEquippedItem();
}

bool UHarmoniaEquipmentComponent::IsSlotEquipped(EEquipmentSlot Slot) const
{
	const FEquippedItem* FoundItem = EquippedItems.FindByPredicate([Slot](const FEquippedItem& Item)
	{
		return Item.Slot == Slot;
	});
	return FoundItem && FoundItem->IsValid();
}

TArray<FEquippedItem> UHarmoniaEquipmentComponent::GetAllEquippedItems() const
{
	return EquippedItems;
}

bool UHarmoniaEquipmentComponent::GetEquipmentData(const FHarmoniaID& EquipmentId, FEquipmentData& OutData) const
{
	if (!EquipmentDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetEquipmentData: EquipmentDataTable is null"));
		return false;
	}

	FString ContextString = TEXT("GetEquipmentData");
	FEquipmentData* FoundData = EquipmentDataTable->FindRow<FEquipmentData>(FName(*EquipmentId.ToString()), ContextString, false);

	if (!FoundData)
	{
		return false;
	}

	OutData = *FoundData;
	return true;
}

float UHarmoniaEquipmentComponent::GetTotalStatModifier(const FString& AttributeName) const
{
	float TotalFlat = 0.f;
	float TotalPercentage = 0.f;

	for (const FEquippedItem& Item : EquippedItems)
	{
		FEquipmentData EquipmentData;
		if (GetEquipmentData(Item.EquipmentId, EquipmentData))
		{
			for (const FEquipmentStatModifier& Modifier : EquipmentData.StatModifiers)
			{
				if (Modifier.AttributeName == AttributeName)
				{
					if (Modifier.ModifierType == EStatModifierType::Flat)
					{
						TotalFlat += Modifier.Value;
					}
					else if (Modifier.ModifierType == EStatModifierType::Percentage)
					{
						TotalPercentage += Modifier.Value;
					}
				}
			}
		}
	}

	// For now, return flat value. Percentage would need base value context
	return TotalFlat;
}

void UHarmoniaEquipmentComponent::UnequipAll()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// Make a copy to avoid modifying array during iteration
	TArray<FEquippedItem> ItemsCopy = EquippedItems;

	for (const FEquippedItem& Item : ItemsCopy)
	{
		UnequipItem(Item.Slot);
	}
}

// ============================================================================
// Durability
// ============================================================================

void UHarmoniaEquipmentComponent::DamageEquipment(EEquipmentSlot Slot, float DamageAmount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	int32 ItemIndex = EquippedItems.FindLastByPredicate([Slot](const FEquippedItem& Item)
	{
		return Item.Slot == Slot;
	});

	if (ItemIndex == INDEX_NONE)
	{
		return;
	}

	FEquippedItem& EquippedItem = EquippedItems[ItemIndex];
	EquippedItem.CurrentDurability = FMath::Max(0.f, EquippedItem.CurrentDurability - DamageAmount);

	// Auto-unequip if broken
	if (EquippedItem.CurrentDurability <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment in slot %d is broken!"), static_cast<int32>(Slot));
		UnequipItem(Slot);
	}
}

void UHarmoniaEquipmentComponent::RepairEquipment(EEquipmentSlot Slot, float RepairAmount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	int32 ItemIndex = EquippedItems.FindLastByPredicate([Slot](const FEquippedItem& Item)
	{
		return Item.Slot == Slot;
	});

	if (ItemIndex == INDEX_NONE)
	{
		return;
	}

	FEquippedItem& EquippedItem = EquippedItems[ItemIndex];
	FEquipmentData EquipmentData;
	if (GetEquipmentData(EquippedItem.EquipmentId, EquipmentData))
	{
		EquippedItem.CurrentDurability = FMath::Min(EquipmentData.MaxDurability,
			EquippedItem.CurrentDurability + RepairAmount);
	}
}

float UHarmoniaEquipmentComponent::GetEquipmentDurabilityPercent(EEquipmentSlot Slot) const
{
	const FEquippedItem* FoundItem = EquippedItems.FindByPredicate([Slot](const FEquippedItem& Item)
	{
		return Item.Slot == Slot;
	});

	if (!FoundItem)
	{
		return 0.f;
	}

	FEquipmentData EquipmentData;
	if (GetEquipmentData(FoundItem->EquipmentId, EquipmentData) && EquipmentData.MaxDurability > 0.f)
	{
		return FoundItem->CurrentDurability / EquipmentData.MaxDurability;
	}

	return 1.f;
}

// ============================================================================
// Save/Load
// ============================================================================

FEquipmentSaveData UHarmoniaEquipmentComponent::GetSaveData() const
{
	FEquipmentSaveData SaveData;
	for (const FEquippedItem& Item : EquippedItems)
	{
		SaveData.EquippedItems.Add(Item.Slot, Item);
	}
	return SaveData;
}

void UHarmoniaEquipmentComponent::LoadFromSaveData(const FEquipmentSaveData& SaveData)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// Unequip all first
	UnequipAll();

	// Equip saved items
	for (const auto& Pair : SaveData.EquippedItems)
	{
		EquipItem(Pair.Value.EquipmentId, Pair.Key);

		// Restore durability
		FEquippedItem* FoundItem = EquippedItems.FindByPredicate([&Pair](const FEquippedItem& Item)
		{
			return Item.Slot == Pair.Key;
		});
		if (FoundItem)
		{
			FoundItem->CurrentDurability = Pair.Value.CurrentDurability;
		}
	}
}

// ============================================================================
// Configuration
// ============================================================================

void UHarmoniaEquipmentComponent::SetEquipmentDataTable(UDataTable* InDataTable)
{
	EquipmentDataTable = InDataTable;
}

// ============================================================================
// Internal Functions
// ============================================================================

FGameplayAttribute UHarmoniaEquipmentComponent::GetAttributeFromName(const FString& AttributeName) const
{
	if (AttributeName == "MaxHealth")
	{
		return UHarmoniaAttributeSet::GetMaxHealthAttribute();
	}
	else if (AttributeName == "MaxStamina")
	{
		return UHarmoniaAttributeSet::GetMaxStaminaAttribute();
	}
	else if (AttributeName == "AttackPower")
	{
		return UHarmoniaAttributeSet::GetAttackPowerAttribute();
	}
	else if (AttributeName == "Defense")
	{
		return UHarmoniaAttributeSet::GetDefenseAttribute();
	}
	else if (AttributeName == "CriticalChance")
	{
		return UHarmoniaAttributeSet::GetCriticalChanceAttribute();
	}
	else if (AttributeName == "CriticalDamage")
	{
		return UHarmoniaAttributeSet::GetCriticalDamageAttribute();
	}
	else if (AttributeName == "MovementSpeed")
	{
		return UHarmoniaAttributeSet::GetMovementSpeedAttribute();
	}
	else if (AttributeName == "AttackSpeed")
	{
		return UHarmoniaAttributeSet::GetAttackSpeedAttribute();
	}

	return FGameplayAttribute();
}

void UHarmoniaEquipmentComponent::ApplyStatModifiers(const FEquipmentData& EquipmentData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UHarmoniaAttributeSet* AttributeSet = GetAttributeSet();

	if (!ASC || !AttributeSet)
	{
		return;
	}

	for (const FEquipmentStatModifier& Modifier : EquipmentData.StatModifiers)
	{
		FGameplayAttribute Attribute = GetAttributeFromName(Modifier.AttributeName);

		if (!Attribute.IsValid())
		{
			continue;
		}

		// Apply modifier
		float CurrentValue = ASC->GetNumericAttribute(Attribute);
		float NewValue = CurrentValue;

		if (Modifier.ModifierType == EStatModifierType::Flat)
		{
			NewValue += Modifier.Value;
		}
		else if (Modifier.ModifierType == EStatModifierType::Percentage)
		{
			NewValue *= (1.0f + Modifier.Value / 100.0f);
		}
		else if (Modifier.ModifierType == EStatModifierType::Override)
		{
			NewValue = Modifier.Value;
		}

		ASC->SetNumericAttributeBase(Attribute, NewValue);
	}
}

void UHarmoniaEquipmentComponent::RemoveStatModifiers(const FEquipmentData& EquipmentData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UHarmoniaAttributeSet* AttributeSet = GetAttributeSet();

	if (!ASC || !AttributeSet)
	{
		return;
	}

	for (const FEquipmentStatModifier& Modifier : EquipmentData.StatModifiers)
	{
		FGameplayAttribute Attribute = GetAttributeFromName(Modifier.AttributeName);

		if (!Attribute.IsValid())
		{
			continue;
		}

		// Remove modifier (inverse operation)
		float CurrentValue = ASC->GetNumericAttribute(Attribute);
		float NewValue = CurrentValue;

		if (Modifier.ModifierType == EStatModifierType::Flat)
		{
			NewValue -= Modifier.Value;
		}
		else if (Modifier.ModifierType == EStatModifierType::Percentage)
		{
			NewValue /= (1.0f + Modifier.Value / 100.0f);
		}

		ASC->SetNumericAttributeBase(Attribute, NewValue);
	}
}

void UHarmoniaEquipmentComponent::ApplyGameplayEffects(const FEquipmentData& EquipmentData, FEquippedItem& OutEquippedItem)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Apply granted effects
	for (TSubclassOf<UGameplayEffect> EffectClass : EquipmentData.GrantedEffects)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			OutEquippedItem.ActiveEffectHandles.Add(ActiveHandle);
		}
	}

	// Apply granted tags
	if (EquipmentData.GrantedTags.Num() > 0)
	{
		ASC->AddLooseGameplayTags(EquipmentData.GrantedTags);
	}
}

void UHarmoniaEquipmentComponent::RemoveGameplayEffects(FEquippedItem& EquippedItem)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Remove active effects
	for (FActiveGameplayEffectHandle& Handle : EquippedItem.ActiveEffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}
	EquippedItem.ActiveEffectHandles.Empty();

	// Remove granted tags
	FEquipmentData EquipmentData;
	if (GetEquipmentData(EquippedItem.EquipmentId, EquipmentData))
	{
		if (EquipmentData.GrantedTags.Num() > 0)
		{
			ASC->RemoveLooseGameplayTags(EquipmentData.GrantedTags);
		}
	}
}

void UHarmoniaEquipmentComponent::ApplyVisualMesh(const FEquipmentData& EquipmentData)
{
	USkeletalMeshComponent* OwnerMesh = GetOwnerMesh();
	if (!OwnerMesh || EquipmentData.EquipmentMesh.IsNull())
	{
		return;
	}

	// Load mesh
	USkeletalMesh* LoadedMesh = EquipmentData.EquipmentMesh.LoadSynchronous();
	if (!LoadedMesh)
	{
		return;
	}

	// Create equipment mesh component
	USkeletalMeshComponent* EquipmentMeshComponent = NewObject<USkeletalMeshComponent>(GetOwner());
	if (!EquipmentMeshComponent)
	{
		return;
	}

	EquipmentMeshComponent->SetSkeletalMesh(LoadedMesh);
	EquipmentMeshComponent->SetupAttachment(OwnerMesh, EquipmentData.AttachSocketName);
	EquipmentMeshComponent->SetLeaderPoseComponent(OwnerMesh);

	// Tag with slot info for later removal
	EquipmentMeshComponent->Rename(*FString::Printf(TEXT("EquipmentMesh_%d"), static_cast<int32>(EquipmentData.EquipmentSlot)));

	EquipmentMeshComponent->RegisterComponent();
	EquipmentMeshes.Add(EquipmentMeshComponent);
}

void UHarmoniaEquipmentComponent::RemoveVisualMesh(EEquipmentSlot Slot)
{
	// Find and remove mesh component for this slot
	for (int32 i = EquipmentMeshes.Num() - 1; i >= 0; --i)
	{
		USkeletalMeshComponent* MeshComponent = EquipmentMeshes[i];
		if (MeshComponent && MeshComponent->GetName().Contains(FString::Printf(TEXT("EquipmentMesh_%d"), static_cast<int32>(Slot))))
		{
			MeshComponent->DestroyComponent();
			EquipmentMeshes.RemoveAt(i);
			break;
		}
	}
}

UAbilitySystemComponent* UHarmoniaEquipmentComponent::GetAbilitySystemComponent() const
{
	if (CachedASC.IsValid())
	{
		return CachedASC.Get();
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Try to get from owner
	UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC)
	{
		const_cast<UHarmoniaEquipmentComponent*>(this)->CachedASC = ASC;
		return ASC;
	}

	return nullptr;
}

UHarmoniaAttributeSet* UHarmoniaEquipmentComponent::GetAttributeSet() const
{
	if (CachedAttributeSet.IsValid())
	{
		return CachedAttributeSet.Get();
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return nullptr;
	}

	UHarmoniaAttributeSet* AttributeSet = const_cast<UHarmoniaAttributeSet*>(
		ASC->GetSet<UHarmoniaAttributeSet>());

	if (AttributeSet)
	{
		const_cast<UHarmoniaEquipmentComponent*>(this)->CachedAttributeSet = AttributeSet;
	}

	return AttributeSet;
}

USkeletalMeshComponent* UHarmoniaEquipmentComponent::GetOwnerMesh() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		return Character->GetMesh();
	}
	return nullptr;
}

void UHarmoniaEquipmentComponent::OnRep_EquippedItems()
{
	// Notify clients of equipment changes
	// Visual updates would happen here
}

void UHarmoniaEquipmentComponent::ServerEquipItem_Implementation(const FHarmoniaID& EquipmentId, EEquipmentSlot Slot)
{
	EquipItem(EquipmentId, Slot);
}

void UHarmoniaEquipmentComponent::ServerUnequipItem_Implementation(EEquipmentSlot Slot)
{
	UnequipItem(Slot);
}
