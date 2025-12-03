// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HarmoniaRelationshipData.generated.h"

/**
 * Relationship tier definition
 */
USTRUCT(BlueprintType)
struct FRelationshipTier
{
	GENERATED_BODY()

	/** Tier name (e.g., "Friendly", "Hostile") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship")
	FText TierName;

	/** Minimum affinity value for this tier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship")
	int32 MinAffinity = 0;

	/** Maximum affinity value for this tier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship")
	int32 MaxAffinity = 100;

	/** Color for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship")
	FLinearColor TierColor = FLinearColor::White;

	FRelationshipTier()
		: TierName(FText::FromString("Neutral"))
		, MinAffinity(0)
		, MaxAffinity(100)
	{}
};

/**
 * Data asset defining relationship tiers and settings
 */
UCLASS(BlueprintType)
class HARMONIASTORY_API UHarmoniaRelationshipData : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaRelationshipData();

	/** List of relationship tiers, sorted by affinity (lowest to highest) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship")
	TArray<FRelationshipTier> Tiers;

	/** Get tier for a given affinity value */
	UFUNCTION(BlueprintPure, Category = "Relationship")
	FRelationshipTier GetTierForAffinity(int32 Affinity) const;
};
