// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaTeamSystemDefinitions.h"
#include "HarmoniaTeamSetupData.generated.h"

/**
 * Team Setup Entry
 * Defines a single team with its properties
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaTeamSetupEntry
{
	GENERATED_BODY()

	/** Team identification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FHarmoniaTeamIdentification TeamID;

	/** Optional: Team configuration asset for advanced settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TObjectPtr<UHarmoniaTeamConfigData> TeamConfig = nullptr;

	/** Whether members can attack each other */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	bool bAllowFriendlyFire = false;

	FHarmoniaTeamSetupEntry()
		: bAllowFriendlyFire(false)
	{
	}
};

/**
 * Team Relationship Entry
 * Defines relationship between two teams
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaTeamRelationshipEntry
{
	GENERATED_BODY()

	/** First team */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	FGameplayTag TeamA;

	/** Second team */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	FGameplayTag TeamB;

	/** Relationship type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	EHarmoniaTeamRelationship Relationship = EHarmoniaTeamRelationship::Neutral;

	/** Whether relationship is bidirectional */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	bool bBidirectional = true;

	FHarmoniaTeamRelationshipEntry()
		: Relationship(EHarmoniaTeamRelationship::Neutral)
		, bBidirectional(true)
	{
	}
};

/**
 * UHarmoniaTeamSetupData
 * 
 * Data Asset that defines the initial team setup for a game.
 * Replaces hardcoded team initialization.
 * 
 * Usage:
 * 1. Create a Data Asset of this type in Content Browser
 * 2. Configure teams and relationships
 * 3. Set it in Project Settings or load via code
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaTeamSetupData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Teams
	// ============================================================================

	/** List of teams to register */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teams", meta = (TitleProperty = "TeamID.TeamName"))
	TArray<FHarmoniaTeamSetupEntry> Teams;

	// ============================================================================
	// Relationships
	// ============================================================================

	/** List of team relationships to set */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationships")
	TArray<FHarmoniaTeamRelationshipEntry> Relationships;

	// ============================================================================
	// Utility
	// ============================================================================

	/** Find team entry by tag */
	UFUNCTION(BlueprintCallable, Category = "Team Setup")
	bool FindTeamByTag(FGameplayTag TeamTag, FHarmoniaTeamSetupEntry& OutEntry) const;

	/** Get all team tags */
	UFUNCTION(BlueprintCallable, Category = "Team Setup")
	TArray<FGameplayTag> GetAllTeamTags() const;
};
