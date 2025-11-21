// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaRelationshipComponent.generated.h"

class UHarmoniaRelationshipData;

/**
 * Delegate for affinity changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAffinityChanged, FHarmoniaID, TargetId, int32, NewAffinity, int32, Delta);

/**
 * Component to track relationship/affinity with NPCs or Factions
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIASTORY_API UHarmoniaRelationshipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRelationshipComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** Modify affinity with a target */
	UFUNCTION(BlueprintCallable, Category = "Relationship")
	void ModifyAffinity(FHarmoniaID TargetId, int32 Amount);

	/** Set affinity with a target */
	UFUNCTION(BlueprintCallable, Category = "Relationship")
	void SetAffinity(FHarmoniaID TargetId, int32 Amount);

	/** Get current affinity with a target */
	UFUNCTION(BlueprintPure, Category = "Relationship")
	int32 GetAffinity(FHarmoniaID TargetId) const;

	/** Get relationship tier with a target */
	UFUNCTION(BlueprintPure, Category = "Relationship")
	FText GetAffinityTierName(FHarmoniaID TargetId) const;

public:
	/** Called when affinity changes */
	UPROPERTY(BlueprintAssignable, Category = "Relationship|Events")
	FOnAffinityChanged OnAffinityChanged;

	/** Relationship data asset (defines tiers) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship")
	UHarmoniaRelationshipData* RelationshipData;

protected:
	/** Map of TargetID -> Affinity Value */
	UPROPERTY(SaveGame)
	TMap<FHarmoniaID, int32> AffinityMap;
};
