// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "HarmoniaTeamSystemDefinitions.generated.h"

// ============================================================================
// Team Identification System
// ============================================================================

/**
 * Team Relationship Type
 * Defines how one team relates to another
 */
UENUM(BlueprintType)
enum class EHarmoniaTeamRelationship : uint8
{
	/** Friendly - Will help, won't attack */
	Ally UMETA(DisplayName = "Ally"),

	/** Neutral - Will ignore unless provoked */
	Neutral UMETA(DisplayName = "Neutral"),

	/** Hostile - Will attack on sight */
	Enemy UMETA(DisplayName = "Enemy")
};

/**
 * Team Attitude
 * Attitude that affects relationship behavior
 */
UENUM(BlueprintType)
enum class EHarmoniaTeamAttitude : uint8
{
	/** Friendly - Cooperative behavior */
	Friendly UMETA(DisplayName = "Friendly"),

	/** Neutral - Indifferent behavior */
	Neutral UMETA(DisplayName = "Neutral"),

	/** Hostile - Aggressive behavior */
	Hostile UMETA(DisplayName = "Hostile"),

	/** Defensive - Will retaliate when attacked */
	Defensive UMETA(DisplayName = "Defensive")
};

/**
 * Team Identification
 * Identifies a team using a flexible tag-based system
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaTeamIdentification
{
	GENERATED_BODY()

	/**
	 * Team ID (using GameplayTag for flexibility)
	 * Examples: Team.Player, Team.Monster, Team.NPC.Guard
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGameplayTag TeamID;

	/**
	 * Team display name
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FText TeamName;

	/**
	 * Team color (for UI representation)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FLinearColor TeamColor = FLinearColor::White;

	/**
	 * Default attitude toward other teams
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	EHarmoniaTeamAttitude DefaultAttitude = EHarmoniaTeamAttitude::Neutral;

	FHarmoniaTeamIdentification()
		: TeamColor(FLinearColor::White)
		, DefaultAttitude(EHarmoniaTeamAttitude::Neutral)
	{
	}

	FHarmoniaTeamIdentification(const FGameplayTag& InTeamID)
		: TeamID(InTeamID)
		, TeamColor(FLinearColor::White)
		, DefaultAttitude(EHarmoniaTeamAttitude::Neutral)
	{
	}

	/**
	 * Check if this team ID is valid
	 */
	bool IsValid() const
	{
		return TeamID.IsValid();
	}

	/**
	 * Check if this is the same team
	 */
	bool IsSameTeam(const FHarmoniaTeamIdentification& Other) const
	{
		return TeamID.IsValid() && TeamID == Other.TeamID;
	}

	/**
	 * Equality operator
	 */
	bool operator==(const FHarmoniaTeamIdentification& Other) const
	{
		return IsSameTeam(Other);
	}

	bool operator!=(const FHarmoniaTeamIdentification& Other) const
	{
		return !IsSameTeam(Other);
	}

	/**
	 * Get hash for use in maps
	 */
	friend uint32 GetTypeHash(const FHarmoniaTeamIdentification& TeamIdentification)
	{
		return GetTypeHash(TeamIdentification.TeamID);
	}
};

/**
 * Team Relationship Data
 * Defines relationship between two teams
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaTeamRelationshipData
{
	GENERATED_BODY()

	/** Source team ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	FHarmoniaTeamIdentification SourceTeam;

	/** Target team ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	FHarmoniaTeamIdentification TargetTeam;

	/** Relationship type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	EHarmoniaTeamRelationship Relationship = EHarmoniaTeamRelationship::Neutral;

	/** Whether to retaliate when attacked by this team */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	bool bRetaliateWhenAttacked = true;

	/** Whether to defend allies from this team */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	bool bDefendAlliesAgainst = true;

	FHarmoniaTeamRelationshipData()
		: Relationship(EHarmoniaTeamRelationship::Neutral)
		, bRetaliateWhenAttacked(true)
		, bDefendAlliesAgainst(true)
	{
	}
};

/**
 * Team Configuration Data Asset
 * Defines a team and its default relationships
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaTeamConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Team identification */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	FHarmoniaTeamIdentification TeamID;

	/** Team description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team", meta = (MultiLine = true))
	FText Description;

	/** Default relationships with other teams */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationships")
	TArray<FHarmoniaTeamRelationshipData> DefaultRelationships;

	/** Whether members of same team can attack each other */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	bool bAllowFriendlyFire = false;

	/** Whether this team automatically targets players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	bool bAutoTargetPlayers = false;

	/** Whether this team can form squads with same team members */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	bool bCanFormSquads = true;

	/**
	 * Get relationship with another team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team")
	EHarmoniaTeamRelationship GetRelationshipWith(const FHarmoniaTeamIdentification& OtherTeam) const;

	/**
	 * Check if can attack another team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team")
	bool CanAttack(const FHarmoniaTeamIdentification& OtherTeam) const;

	/**
	 * Check if should help another team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team")
	bool ShouldHelp(const FHarmoniaTeamIdentification& OtherTeam) const;
};

/**
 * Team Agent Interface
 * Implemented by actors that belong to a team
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UHarmoniaTeamAgentInterface : public UInterface
{
	GENERATED_BODY()
};

class HARMONIAKIT_API IHarmoniaTeamAgentInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get this actor's team ID
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	FHarmoniaTeamIdentification GetTeamID() const;

	/**
	 * Set this actor's team ID
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	void SetTeamID(const FHarmoniaTeamIdentification& NewTeamID);

	/**
	 * Get relationship with another actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	EHarmoniaTeamRelationship GetRelationshipWith(AActor* OtherActor) const;

	/**
	 * Check if can attack another actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	bool CanAttackActor(AActor* OtherActor) const;

	/**
	 * Check if should help another actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	bool ShouldHelpActor(AActor* OtherActor) const;

	/**
	 * Check if is same team as another actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	bool IsSameTeamAs(AActor* OtherActor) const;

	/**
	 * Check if is ally with another actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	bool IsAllyWith(AActor* OtherActor) const;

	/**
	 * Check if is enemy with another actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Team")
	bool IsEnemyWith(AActor* OtherActor) const;
};
