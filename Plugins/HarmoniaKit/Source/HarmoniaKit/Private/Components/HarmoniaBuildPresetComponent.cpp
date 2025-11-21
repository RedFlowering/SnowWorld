// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaBuildPresetComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "GameFramework/Character.h"

UHarmoniaBuildPresetComponent::UHarmoniaBuildPresetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHarmoniaBuildPresetComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaBuildPresetComponent::SavePreset(FName PresetName)
{
	if (PresetName.IsNone())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FBuildPreset NewPreset;
	NewPreset.PresetName = PresetName;

	// Capture Equipment
	UHarmoniaEquipmentComponent* EquipmentComp = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>();
	if (EquipmentComp)
	{
		NewPreset.Equipment = EquipmentComp->GetAllEquippedItems();
	}

	// Capture Abilities (Placeholder logic)
	// In a real implementation, we would get the AbilitySystemComponent and save active abilities
	// For now, we just leave it empty or add a dummy tag
	// NewPreset.AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Placeholder")));

	// Update existing or add new
	int32 ExistingIndex = SavedPresets.IndexOfByPredicate([PresetName](const FBuildPreset& Preset) {
		return Preset.PresetName == PresetName;
	});

	if (ExistingIndex != INDEX_NONE)
	{
		SavedPresets[ExistingIndex] = NewPreset;
	}
	else
	{
		SavedPresets.Add(NewPreset);
	}

	UE_LOG(LogTemp, Log, TEXT("Saved Build Preset: %s"), *PresetName.ToString());
}

void UHarmoniaBuildPresetComponent::LoadPreset(FName PresetName)
{
	const FBuildPreset* Preset = SavedPresets.FindByPredicate([PresetName](const FBuildPreset& P) {
		return P.PresetName == PresetName;
	});

	if (Preset)
	{
		ApplyEquipment(Preset->Equipment);
		ApplyAbilities(Preset->AbilityTags);
		UE_LOG(LogTemp, Log, TEXT("Loaded Build Preset: %s"), *PresetName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Build Preset not found: %s"), *PresetName.ToString());
	}
}

void UHarmoniaBuildPresetComponent::DeletePreset(FName PresetName)
{
	SavedPresets.RemoveAll([PresetName](const FBuildPreset& Preset) {
		return Preset.PresetName == PresetName;
	});
}

void UHarmoniaBuildPresetComponent::ApplyEquipment(const TArray<FEquippedItem>& Equipment)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UHarmoniaEquipmentComponent* EquipmentComp = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>();
	if (!EquipmentComp) return;

	// Unequip everything first
	EquipmentComp->UnequipAll();

	// Equip items from preset
	for (const FEquippedItem& Item : Equipment)
	{
		if (Item.EquipmentId.IsValid())
		{
			// Note: In a real game, we should check if the player actually HAS this item in their inventory
			// or if the item was moved to storage.
			// For this implementation, we assume if it's in the preset, we try to equip it.
			// If the item is not in inventory, EquipItem might fail depending on its implementation.
			// However, HarmoniaEquipmentComponent::EquipItem usually checks inventory.
			
			EquipmentComp->EquipItem(Item.EquipmentId, Item.Slot);
		}
	}
}

void UHarmoniaBuildPresetComponent::ApplyAbilities(const FGameplayTagContainer& AbilityTags)
{
	// Placeholder for ability application
	// Would involve clearing current abilities and granting new ones based on tags
}
