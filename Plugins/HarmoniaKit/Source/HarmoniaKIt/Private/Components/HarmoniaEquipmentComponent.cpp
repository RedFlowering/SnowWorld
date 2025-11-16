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
	SetIsReplicatedUsing(true);
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
	if (EquippedItems.Contains(TargetSlot))
	{
		OldEquipmentId = EquippedItems[TargetSlot].EquipmentId;
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
	EquippedItems.Add(TargetSlot, NewEquippedItem);

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

	if (!EquippedItems.Contains(Slot))
	{
		return false;
	}

	FEquippedItem& EquippedItem = EquippedItems[Slot];
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
	EquippedItems.Remove(Slot);

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

	FEquippedItem* ItemA = EquippedItems.Find(SlotA);
	FEquippedItem* ItemB = EquippedItems.Find(SlotB);

	if (!ItemA && !ItemB)
	{
		return false;
	}

	// Simple swap
	FEquippedItem TempItem = ItemA ? *ItemA : FEquippedItem();
	if (ItemB)
	{
		EquippedItems.Add(SlotA, *ItemB);
	}
	else
	{
		EquippedItems.Remove(SlotA);
	}

	if (TempItem.IsValid())
	{
		EquippedItems.Add(SlotB, TempItem);
	}
	else
	{
		EquippedItems.Remove(SlotB);
	}

	return true;
}

FEquippedItem UHarmoniaEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	if (EquippedItems.Contains(Slot))
	{
		return EquippedItems[Slot];
	}
	return FEquippedItem();
}

bool UHarmoniaEquipmentComponent::IsSlotEquipped(EEquipmentSlot Slot) const
{
	return EquippedItems.Contains(Slot) && EquippedItems[Slot].IsValid();
}

TMap<EEquipmentSlot, FEquippedItem> UHarmoniaEquipmentComponent::GetAllEquippedItems() const
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

	for (const auto& Pair : EquippedItems)
	{
		FEquipmentData EquipmentData;
		if (GetEquipmentData(Pair.Value.EquipmentId, EquipmentData))
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

	// Copy keys to avoid modifying map during iteration
	TArray<EEquipmentSlot> Slots;
	EquippedItems.GetKeys(Slots);

	for (EEquipmentSlot Slot : Slots)
	{
		UnequipItem(Slot);
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

	if (!EquippedItems.Contains(Slot))
	{
		return;
	}

	FEquippedItem& EquippedItem = EquippedItems[Slot];
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

	if (!EquippedItems.Contains(Slot))
	{
		return;
	}

	FEquippedItem& EquippedItem = EquippedItems[Slot];
	FEquipmentData EquipmentData;
	if (GetEquipmentData(EquippedItem.EquipmentId, EquipmentData))
	{
		EquippedItem.CurrentDurability = FMath::Min(EquipmentData.MaxDurability,
			EquippedItem.CurrentDurability + RepairAmount);
	}
}

float UHarmoniaEquipmentComponent::GetEquipmentDurabilityPercent(EEquipmentSlot Slot) const
{
	if (!EquippedItems.Contains(Slot))
	{
		return 0.f;
	}

	const FEquippedItem& EquippedItem = EquippedItems[Slot];
	FEquipmentData EquipmentData;
	if (GetEquipmentData(EquippedItem.EquipmentId, EquipmentData) && EquipmentData.MaxDurability > 0.f)
	{
		return EquippedItem.CurrentDurability / EquipmentData.MaxDurability;
	}

	return 1.f;
}

// ============================================================================
// Save/Load
// ============================================================================

FEquipmentSaveData UHarmoniaEquipmentComponent::GetSaveData() const
{
	FEquipmentSaveData SaveData;
	SaveData.EquippedItems = EquippedItems;
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
		if (EquippedItems.Contains(Pair.Key))
		{
			EquippedItems[Pair.Key].CurrentDurability = Pair.Value.CurrentDurability;
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
		FGameplayAttribute Attribute;

		// Map attribute name to FGameplayAttribute
		if (Modifier.AttributeName == "MaxHealth")
		{
			Attribute = UHarmoniaAttributeSet::GetMaxHealthAttribute();
		}
		else if (Modifier.AttributeName == "MaxStamina")
		{
			Attribute = UHarmoniaAttributeSet::GetMaxStaminaAttribute();
		}
		else if (Modifier.AttributeName == "AttackPower")
		{
			Attribute = UHarmoniaAttributeSet::GetAttackPowerAttribute();
		}
		else if (Modifier.AttributeName == "Defense")
		{
			Attribute = UHarmoniaAttributeSet::GetDefenseAttribute();
		}
		else if (Modifier.AttributeName == "CriticalChance")
		{
			Attribute = UHarmoniaAttributeSet::GetCriticalChanceAttribute();
		}
		else if (Modifier.AttributeName == "CriticalDamage")
		{
			Attribute = UHarmoniaAttributeSet::GetCriticalDamageAttribute();
		}
		else if (Modifier.AttributeName == "MovementSpeed")
		{
			Attribute = UHarmoniaAttributeSet::GetMovementSpeedAttribute();
		}
		else if (Modifier.AttributeName == "AttackSpeed")
		{
			Attribute = UHarmoniaAttributeSet::GetAttackSpeedAttribute();
		}
		else
		{
			continue;
		}

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
		FGameplayAttribute Attribute;

		// Map attribute name to FGameplayAttribute
		if (Modifier.AttributeName == "MaxHealth")
		{
			Attribute = UHarmoniaAttributeSet::GetMaxHealthAttribute();
		}
		else if (Modifier.AttributeName == "MaxStamina")
		{
			Attribute = UHarmoniaAttributeSet::GetMaxStaminaAttribute();
		}
		else if (Modifier.AttributeName == "AttackPower")
		{
			Attribute = UHarmoniaAttributeSet::GetAttackPowerAttribute();
		}
		else if (Modifier.AttributeName == "Defense")
		{
			Attribute = UHarmoniaAttributeSet::GetDefenseAttribute();
		}
		else if (Modifier.AttributeName == "CriticalChance")
		{
			Attribute = UHarmoniaAttributeSet::GetCriticalChanceAttribute();
		}
		else if (Modifier.AttributeName == "CriticalDamage")
		{
			Attribute = UHarmoniaAttributeSet::GetCriticalDamageAttribute();
		}
		else if (Modifier.AttributeName == "MovementSpeed")
		{
			Attribute = UHarmoniaAttributeSet::GetMovementSpeedAttribute();
		}
		else if (Modifier.AttributeName == "AttackSpeed")
		{
			Attribute = UHarmoniaAttributeSet::GetAttackSpeedAttribute();
		}
		else
		{
			continue;
		}

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
	EquipmentMeshComponent->SetMasterPoseComponent(OwnerMesh);
	EquipmentMeshComponent->RegisterComponent();

	// Store for later removal
	EquipmentMeshes.Add(EquipmentData.EquipmentSlot, EquipmentMeshComponent);
}

void UHarmoniaEquipmentComponent::RemoveVisualMesh(EEquipmentSlot Slot)
{
	if (EquipmentMeshes.Contains(Slot))
	{
		USkeletalMeshComponent* MeshComponent = EquipmentMeshes[Slot];
		if (MeshComponent)
		{
			MeshComponent->DestroyComponent();
		}
		EquipmentMeshes.Remove(Slot);
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
