// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaCollectionSystemDefinitions.h"
#include "HarmoniaCollectionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectionUpdated, const FHarmoniaID&, CollectionId);

/**
 * Subsystem for managing collections (encyclopedia)
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Mark an item as seen
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Collection")
	void MarkAsSeen(const FHarmoniaID& CollectionId);

	// Mark an item as collected
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Collection")
	void MarkAsCollected(const FHarmoniaID& CollectionId);

	// Get collection state
	UFUNCTION(BlueprintPure, Category = "Harmonia|Collection")
	ECollectionState GetCollectionState(const FHarmoniaID& CollectionId) const;

	// Get collection count
	UFUNCTION(BlueprintPure, Category = "Harmonia|Collection")
	int32 GetCollectionCount(const FHarmoniaID& CollectionId) const;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Collection")
	FOnCollectionUpdated OnCollectionUpdated;

protected:
	UPROPERTY()
	TMap<FHarmoniaID, FCollectionState> CollectionStates;
};
