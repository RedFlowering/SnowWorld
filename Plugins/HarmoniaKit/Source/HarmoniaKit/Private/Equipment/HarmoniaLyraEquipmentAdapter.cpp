// Copyright 2025 Snow Game Studio.

#include "Equipment/HarmoniaLyraEquipmentAdapter.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "GameFramework/Actor.h"

FString UHarmoniaLyraEquipmentAdapter::ConvertEquipmentSlotToString(EEquipmentSlot Slot)
{
	switch (Slot)
	{
	case EEquipmentSlot::Head: return TEXT("Head");
	case EEquipmentSlot::Chest: return TEXT("Chest");
	case EEquipmentSlot::Hands: return TEXT("Hands");
	case EEquipmentSlot::Legs: return TEXT("Legs");
	case EEquipmentSlot::Feet: return TEXT("Feet");
	case EEquipmentSlot::MainHand: return TEXT("MainHand");
	case EEquipmentSlot::OffHand: return TEXT("OffHand");
	case EEquipmentSlot::Back: return TEXT("Back");
	case EEquipmentSlot::Accessory1: return TEXT("Accessory1");
	case EEquipmentSlot::Accessory2: return TEXT("Accessory2");
	case EEquipmentSlot::Accessory3: return TEXT("Accessory3");
	case EEquipmentSlot::Accessory4: return TEXT("Accessory4");
	default: return TEXT("None");
	}
}

bool UHarmoniaLyraEquipmentAdapter::SyncHarmoniaToLyra(UHarmoniaEquipmentComponent* HarmoniaEquipment, ULyraEquipmentManagerComponent* LyraEquipment)
{
	if (!HarmoniaEquipment || !LyraEquipment)
	{
		return false;
	}

	// This would require mapping Harmonia equipment IDs to Lyra equipment definitions
	// Implementation depends on your specific data structure
	// For now, this is a placeholder for the migration system

	return true;
}

bool UHarmoniaLyraEquipmentAdapter::SyncLyraToHarmonia(ULyraEquipmentManagerComponent* LyraEquipment, UHarmoniaEquipmentComponent* HarmoniaEquipment)
{
	if (!LyraEquipment || !HarmoniaEquipment)
	{
		return false;
	}

	// This would require mapping Lyra equipment definitions to Harmonia equipment IDs
	// Implementation depends on your specific data structure
	// For now, this is a placeholder for the migration system

	return true;
}

bool UHarmoniaLyraEquipmentAdapter::HasBothEquipmentSystems(AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	UHarmoniaEquipmentComponent* HarmoniaComp = Actor->FindComponentByClass<UHarmoniaEquipmentComponent>();
	ULyraEquipmentManagerComponent* LyraComp = Actor->FindComponentByClass<ULyraEquipmentManagerComponent>();

	return HarmoniaComp != nullptr && LyraComp != nullptr;
}

ULyraEquipmentManagerComponent* UHarmoniaLyraEquipmentAdapter::GetOrCreateLyraEquipmentManager(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	// Try to find existing component
	ULyraEquipmentManagerComponent* LyraComp = Actor->FindComponentByClass<ULyraEquipmentManagerComponent>();
	if (LyraComp)
	{
		return LyraComp;
	}

	// Create new component if it doesn't exist
	// Note: This should only be done on authority/server
	if (Actor->HasAuthority())
	{
		LyraComp = NewObject<ULyraEquipmentManagerComponent>(Actor, ULyraEquipmentManagerComponent::StaticClass());
		if (LyraComp)
		{
			LyraComp->RegisterComponent();
			Actor->AddInstanceComponent(LyraComp);
		}
	}

	return LyraComp;
}
