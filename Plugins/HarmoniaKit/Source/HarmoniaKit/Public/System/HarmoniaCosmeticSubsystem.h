// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaCosmeticSystemDefinitions.h"
#include "HarmoniaCosmeticSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCosmeticUnlocked, const FHarmoniaID&, CosmeticId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCosmeticEquipped, const FHarmoniaID&, CosmeticId, ECosmeticSlotType, Slot);

/**
 * Subsystem for managing cosmetic items
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCosmeticSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Unlock a cosmetic
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cosmetic")
	void UnlockCosmetic(const FHarmoniaID& CosmeticId);

	// Equip a cosmetic
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cosmetic")
	void EquipCosmetic(const FHarmoniaID& CosmeticId, ECosmeticSlotType Slot);

	// Get equipped cosmetic for slot
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cosmetic")
	FHarmoniaID GetEquippedCosmetic(ECosmeticSlotType Slot) const;

	// Check if cosmetic is unlocked
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cosmetic")
	bool IsCosmeticUnlocked(const FHarmoniaID& CosmeticId) const;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cosmetic")
	FOnCosmeticUnlocked OnCosmeticUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cosmetic")
	FOnCosmeticEquipped OnCosmeticEquipped;

protected:
	UPROPERTY()
	TMap<FHarmoniaID, FCosmeticState> CosmeticStates;

	UPROPERTY()
	TMap<ECosmeticSlotType, FHarmoniaID> EquippedCosmetics;
};
