// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaEquipmentComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "HarmoniaGameplayTags.h"
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

	// Remove equipment load penalty effect
	if (ActiveEquipLoadPenaltyHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->RemoveActiveGameplayEffect(ActiveEquipLoadPenaltyHandle);
		}
		ActiveEquipLoadPenaltyHandle.Invalidate();
	}

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// [BANDWIDTH OPTIMIZATION] Only replicate equipment to the owning client
	// Other players don't need to know about this player's full equipment details
	// Visual meshes are still replicated separately for all clients
	DOREPLIFETIME_CONDITION(UHarmoniaEquipmentComponent, EquippedItems, COND_OwnerOnly);
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
	FHarmoniaEquipmentData EquipmentData;
	if (!GetEquipmentData(EquipmentId, EquipmentData))
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipItem: Equipment data not found for ID: %s"), *EquipmentId.ToString());
		return false;
	}

	// Check stat requirements
	FText FailureReason;
	if (!CanEquipItem(EquipmentId, FailureReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipItem: Cannot equip item - %s"), *FailureReason.ToString());
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

	// Apply stat modifiers as GameplayEffects
	ApplyStatModifiers(EquipmentData, NewEquippedItem);

	// Apply gameplay effects (GrantedEffects)
	ApplyGameplayEffects(EquipmentData, NewEquippedItem);

	// Apply visual mesh
	ApplyVisualMesh(EquipmentData);

	// Store equipped item
	EquippedItems.Add(NewEquippedItem);

	// Update equipment load
	UpdateEquipLoad();

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
	FHarmoniaEquipmentData EquipmentData;
	if (GetEquipmentData(EquippedItem.EquipmentId, EquipmentData))
	{
		// Remove all gameplay effects (including stat modifiers)
		RemoveGameplayEffects(EquippedItem);

		// Remove visual mesh
		RemoveVisualMesh(Slot);
	}

	// Remove from equipped items
	EquippedItems.RemoveAt(ItemIndex);

	// Update equipment load
	UpdateEquipLoad();

	// Broadcast event
	OnEquipmentChanged.Broadcast(Slot, OldEquipmentId, FHarmoniaID());

	UE_LOG(LogTemp, Log, TEXT("UnequipItem: Successfully unequipped from slot %d"), static_cast<int32>(Slot));

	return true;
}

void UHarmoniaEquipmentComponent::RequestSwapEquipment(EEquipmentSlot SlotA, EEquipmentSlot SlotB)
{
	// Server authority
	if (GetOwnerRole() == ROLE_Authority)
	{
		SwapEquipment(SlotA, SlotB);
	}
	else
	{
		ServerSwapEquipment(SlotA, SlotB);
	}
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

bool UHarmoniaEquipmentComponent::GetEquipmentData(const FHarmoniaID& EquipmentId, FHarmoniaEquipmentData& OutData) const
{
	if (!EquipmentDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetEquipmentData: EquipmentDataTable is null"));
		return false;
	}

	FString ContextString = TEXT("GetEquipmentData");
	FHarmoniaEquipmentData* FoundData = EquipmentDataTable->FindRow<FHarmoniaEquipmentData>(FName(*EquipmentId.ToString()), ContextString, false);

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
		FHarmoniaEquipmentData EquipmentData;
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
	FHarmoniaEquipmentData EquipmentData;
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

	FHarmoniaEquipmentData EquipmentData;
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

bool UHarmoniaEquipmentComponent::CanEquipItem(const FHarmoniaID& EquipmentId, FText& OutFailureReason) const
{
	// Get equipment data
	FHarmoniaEquipmentData EquipmentData;
	if (!GetEquipmentData(EquipmentId, EquipmentData))
	{
		OutFailureReason = FText::FromString(TEXT("Equipment data not found"));
		return false;
	}

	// Get attribute set
	UHarmoniaAttributeSet* AttributeSet = GetAttributeSet();
	if (!AttributeSet)
	{
		OutFailureReason = FText::FromString(TEXT("Attribute set not found"));
		return false;
	}

	// Check stat requirements
	if (EquipmentData.RequiredStrength > 0 && AttributeSet->GetStrength() < EquipmentData.RequiredStrength)
	{
		OutFailureReason = FText::Format(
			FText::FromString(TEXT("Requires {0} Strength (you have {1})")),
			FText::AsNumber(EquipmentData.RequiredStrength),
			FText::AsNumber(static_cast<int32>(AttributeSet->GetStrength()))
		);
		return false;
	}

	if (EquipmentData.RequiredDexterity > 0 && AttributeSet->GetDexterity() < EquipmentData.RequiredDexterity)
	{
		OutFailureReason = FText::Format(
			FText::FromString(TEXT("Requires {0} Dexterity (you have {1})")),
			FText::AsNumber(EquipmentData.RequiredDexterity),
			FText::AsNumber(static_cast<int32>(AttributeSet->GetDexterity()))
		);
		return false;
	}

	if (EquipmentData.RequiredIntelligence > 0 && AttributeSet->GetIntelligence() < EquipmentData.RequiredIntelligence)
	{
		OutFailureReason = FText::Format(
			FText::FromString(TEXT("Requires {0} Intelligence (you have {1})")),
			FText::AsNumber(EquipmentData.RequiredIntelligence),
			FText::AsNumber(static_cast<int32>(AttributeSet->GetIntelligence()))
		);
		return false;
	}

	if (EquipmentData.RequiredFaith > 0 && AttributeSet->GetFaith() < EquipmentData.RequiredFaith)
	{
		OutFailureReason = FText::Format(
			FText::FromString(TEXT("Requires {0} Faith (you have {1})")),
			FText::AsNumber(EquipmentData.RequiredFaith),
			FText::AsNumber(static_cast<int32>(AttributeSet->GetFaith()))
		);
		return false;
	}

	return true;
}

float UHarmoniaEquipmentComponent::GetTotalEquipmentLoad() const
{
	float TotalWeight = 0.f;

	for (const FEquippedItem& Item : EquippedItems)
	{
		FHarmoniaEquipmentData EquipmentData;
		if (GetEquipmentData(Item.EquipmentId, EquipmentData))
		{
			TotalWeight += EquipmentData.Weight;
		}
	}

	return TotalWeight;
}

// ============================================================================
// Internal Functions
// ============================================================================

void UHarmoniaEquipmentComponent::UpdateEquipLoad()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UHarmoniaAttributeSet* AttributeSet = GetAttributeSet();

	if (!ASC || !AttributeSet)
	{
		return;
	}

	// Calculate total equipment weight
	float TotalWeight = GetTotalEquipmentLoad();

	// Update EquipLoad attribute
	ASC->SetNumericAttributeBase(UHarmoniaAttributeSet::GetEquipLoadAttribute(), TotalWeight);

	// Apply movement speed penalty based on equipment load ratio
	ApplyEquipLoadPenalty();

	UE_LOG(LogTemp, Verbose, TEXT("UpdateEquipLoad: Total weight = %.2f / %.2f (%.1f%%)"),
		TotalWeight, AttributeSet->GetMaxEquipLoad(),
		(TotalWeight / FMath::Max(1.f, AttributeSet->GetMaxEquipLoad())) * 100.f);
}

void UHarmoniaEquipmentComponent::ApplyEquipLoadPenalty()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UHarmoniaAttributeSet* AttributeSet = GetAttributeSet();

	if (!ASC || !AttributeSet)
	{
		return;
	}

	float EquipLoad = AttributeSet->GetEquipLoad();
	float MaxEquipLoad = AttributeSet->GetMaxEquipLoad();

	if (MaxEquipLoad <= 0.f)
	{
		return;
	}

	// Calculate equipment load ratio (0.0 - 1.0+)
	float LoadRatio = EquipLoad / MaxEquipLoad;

	// Calculate movement speed penalty based on load ratio
	// Light Load (0-30%): No penalty
	// Medium Load (30-70%): -10% speed
	// Heavy Load (70-100%): -20% speed
	// Overload (100%+): -40% speed
	float SpeedPenalty = 0.f;

	if (LoadRatio <= 0.3f)
	{
		// Light load - no penalty
		SpeedPenalty = 0.f;
	}
	else if (LoadRatio <= 0.7f)
	{
		// Medium load - 10% penalty
		SpeedPenalty = -0.1f;
	}
	else if (LoadRatio <= 1.0f)
	{
		// Heavy load - 20% penalty
		SpeedPenalty = -0.2f;
	}
	else
	{
		// Overload - 40% penalty
		SpeedPenalty = -0.4f;
	}

	// Remove previous penalty effect if it exists
	if (ActiveEquipLoadPenaltyHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveEquipLoadPenaltyHandle);
		ActiveEquipLoadPenaltyHandle.Invalidate();
	}

	// Apply penalty as a gameplay effect
	if (FMath::Abs(SpeedPenalty) > KINDA_SMALL_NUMBER)
	{
		if (!EquipLoadPenaltyEffectClass)
		{
			// Only log once to avoid spam, or if you want to warn the designer
			// UE_LOG(LogTemp, Warning, TEXT("ApplyEquipLoadPenalty: EquipLoadPenaltyEffectClass is not set in HarmoniaEquipmentComponent!"));
			return;
		}

		// Apply new penalty using SetByCaller
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EquipLoadPenaltyEffectClass, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Set the magnitude for the movement speed penalty
			// The GE should be set up to use SetByCaller with this tag for MovementSpeed attribute
			SpecHandle.Data->SetSetByCallerMagnitude(HarmoniaGameplayTags::Stat_Movement_Speed, SpeedPenalty);

			ActiveEquipLoadPenaltyHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			UE_LOG(LogTemp, Verbose, TEXT("ApplyEquipLoadPenalty: Applied %.1f%% movement speed penalty (Load: %.1f%%)"),
				SpeedPenalty * 100.f, LoadRatio * 100.f);
		}
	}
}

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

void UHarmoniaEquipmentComponent::ApplyStatModifiers(const FHarmoniaEquipmentData& EquipmentData, FEquippedItem& OutEquippedItem)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UHarmoniaAttributeSet* AttributeSet = GetAttributeSet();

	if (!ASC || !AttributeSet)
	{
		return;
	}

	if (!DefaultStatModifierEffectClass)
	{
		// UE_LOG(LogTemp, Warning, TEXT("ApplyStatModifiers: DefaultStatModifierEffectClass not set!"));
		return;
	}

	// Create the spec
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DefaultStatModifierEffectClass, 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	bool bHasModifiers = false;

	// Create and apply a GameplayEffect for each stat modifier
	for (const FEquipmentStatModifier& Modifier : EquipmentData.StatModifiers)
	{
		FGameplayTag StatTag;
		if (Modifier.AttributeName == "Strength") StatTag = HarmoniaGameplayTags::Stat_Primary_Strength;
		else if (Modifier.AttributeName == "Dexterity") StatTag = HarmoniaGameplayTags::Stat_Primary_Dexterity;
		else if (Modifier.AttributeName == "Intelligence") StatTag = HarmoniaGameplayTags::Stat_Primary_Intelligence;
		else if (Modifier.AttributeName == "Faith") StatTag = HarmoniaGameplayTags::Stat_Primary_Faith;
		else if (Modifier.AttributeName == "MaxHealth") StatTag = HarmoniaGameplayTags::Stat_Resource_MaxHealth;
		else if (Modifier.AttributeName == "MaxStamina") StatTag = HarmoniaGameplayTags::Stat_Resource_MaxStamina;
		else if (Modifier.AttributeName == "AttackPower") StatTag = HarmoniaGameplayTags::Stat_Combat_AttackPower;
		else if (Modifier.AttributeName == "Defense") StatTag = HarmoniaGameplayTags::Stat_Combat_Defense;
		else if (Modifier.AttributeName == "CriticalChance") StatTag = HarmoniaGameplayTags::Stat_Combat_CriticalChance;
		else if (Modifier.AttributeName == "CriticalDamage") StatTag = HarmoniaGameplayTags::Stat_Combat_CriticalDamage;
		else if (Modifier.AttributeName == "MovementSpeed") StatTag = HarmoniaGameplayTags::Stat_Movement_Speed;
		else if (Modifier.AttributeName == "AttackSpeed") StatTag = HarmoniaGameplayTags::Stat_Movement_AttackSpeed;

		if (!StatTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("ApplyStatModifiers: Invalid attribute name or no tag mapping for '%s'"), *Modifier.AttributeName);
			continue;
		}

		float ModifierMagnitude = Modifier.Value;

		if (Modifier.ModifierType == EStatModifierType::Percentage)
		{
			// Get current attribute value to calculate percentage
			FGameplayAttribute Attribute = GetAttributeFromName(Modifier.AttributeName);
			if (Attribute.IsValid())
			{
				float CurrentValue = ASC->GetNumericAttribute(Attribute);
				ModifierMagnitude = CurrentValue * (Modifier.Value / 100.0f);
			}
		}
		else if (Modifier.ModifierType == EStatModifierType::Override)
		{
			// Override is not supported with the single SetByCaller GE approach currently
			UE_LOG(LogTemp, Warning, TEXT("ApplyStatModifiers: Override modifier type not supported for %s"), *Modifier.AttributeName);
			continue;
		}

		// Set the magnitude for the tag
		SpecHandle.Data->SetSetByCallerMagnitude(StatTag, ModifierMagnitude);
		bHasModifiers = true;

		UE_LOG(LogTemp, Verbose, TEXT("ApplyStatModifiers: Setting %s to %.2f"), *StatTag.ToString(), ModifierMagnitude);
	}

	if (bHasModifiers)
	{
		FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		if (ActiveHandle.IsValid())
		{
			OutEquippedItem.ActiveEffectHandles.Add(ActiveHandle);
			UE_LOG(LogTemp, Verbose, TEXT("ApplyStatModifiers: Applied equipment stats"));
		}
	}
}

void UHarmoniaEquipmentComponent::ApplyGameplayEffects(const FHarmoniaEquipmentData& EquipmentData, FEquippedItem& OutEquippedItem)
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
	FHarmoniaEquipmentData EquipmentData;
	if (GetEquipmentData(EquippedItem.EquipmentId, EquipmentData))
	{
		if (EquipmentData.GrantedTags.Num() > 0)
		{
			ASC->RemoveLooseGameplayTags(EquipmentData.GrantedTags);
		}
	}
}

void UHarmoniaEquipmentComponent::ApplyVisualMesh(const FHarmoniaEquipmentData& EquipmentData)
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

bool UHarmoniaEquipmentComponent::ServerEquipItem_Validate(const FHarmoniaID& EquipmentId, EEquipmentSlot Slot)
{
	// Anti-cheat: Validate equipment ID
	if (!EquipmentId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerEquipItem: Invalid EquipmentId"));
		return false;
	}

	// Validate equipment exists in data table
	FHarmoniaEquipmentData EquipmentData;
	if (!GetEquipmentData(EquipmentId, EquipmentData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerEquipItem: Equipment data not found for ID: %s"), *EquipmentId.ToString());
		return false;
	}

	// [SECURITY FIX] Validate player owns the equipment in their inventory
	// This prevents cheating clients from equipping items they don't have
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UHarmoniaInventoryComponent* InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
		if (InventoryComponent)
		{
			// Check if player has at least 1 of this equipment in inventory
			int32 ItemCount = InventoryComponent->GetTotalCount(EquipmentId);
			if (ItemCount <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerEquipItem: Player %s doesn't own equipment %s in inventory"),
					*Owner->GetName(), *EquipmentId.ToString());
				return false;
			}
		}
		else
		{
			// If no inventory component, allow for backwards compatibility or special cases
			// But log a warning for investigation
			UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerEquipItem: Player %s has no inventory component, allowing equip for compatibility"),
				*Owner->GetName());
		}
	}

	return true;
}

void UHarmoniaEquipmentComponent::ServerUnequipItem_Implementation(EEquipmentSlot Slot)
{
	UnequipItem(Slot);
}

bool UHarmoniaEquipmentComponent::ServerUnequipItem_Validate(EEquipmentSlot Slot)
{
	// Slot validation happens in implementation
	return true;
}

void UHarmoniaEquipmentComponent::ServerSwapEquipment_Implementation(EEquipmentSlot SlotA, EEquipmentSlot SlotB)
{
	SwapEquipment(SlotA, SlotB);
}

bool UHarmoniaEquipmentComponent::ServerSwapEquipment_Validate(EEquipmentSlot SlotA, EEquipmentSlot SlotB)
{
	// Basic validation - slot equality check
	if (SlotA == SlotB)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSwapEquipment: Cannot swap slot with itself"));
		return false;
	}

	return true;
}
