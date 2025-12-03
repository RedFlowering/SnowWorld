// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaTitleSystemDefinitions.h"
#include "HarmoniaTitleSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleUnlocked, const FHarmoniaID&, TitleId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleEquipped, const FHarmoniaID&, TitleId);

/**
 * Subsystem for managing titles
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaTitleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Unlock a title
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Title")
	void UnlockTitle(const FHarmoniaID& TitleId);

	// Equip a title
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Title")
	void EquipTitle(const FHarmoniaID& TitleId);

	// Get currently equipped title
	UFUNCTION(BlueprintPure, Category = "Harmonia|Title")
	FHarmoniaID GetEquippedTitle() const;

	// Check if title is unlocked
	UFUNCTION(BlueprintPure, Category = "Harmonia|Title")
	bool IsTitleUnlocked(const FHarmoniaID& TitleId) const;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Title")
	FOnTitleUnlocked OnTitleUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Title")
	FOnTitleEquipped OnTitleEquipped;

protected:
	UPROPERTY()
	TMap<FHarmoniaID, FTitleState> TitleStates;

	UPROPERTY()
	FHarmoniaID EquippedTitleId;
};
