// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Definitions/HarmoniaTeamSystemDefinitions.h"
#include "HarmoniaTeamManagementSubsystem.generated.h"

class UHarmoniaTeamConfigData;

/**
 * Wrapper struct for nested TMap value
 */
USTRUCT()
struct FHarmoniaTeamRelationshipMap
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FHarmoniaTeamIdentification, EHarmoniaTeamRelationship> Relationships;
};

/**
 * Team Relationship Changed Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamRelationshipChangedDelegate,
	FHarmoniaTeamIdentification, SourceTeam,
	FHarmoniaTeamIdentification, TargetTeam,
	EHarmoniaTeamRelationship, NewRelationship);

/**
 * Team Registered Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamRegisteredDelegate,
	FHarmoniaTeamIdentification, TeamID);

/**
 * UHarmoniaTeamManagementSubsystem
 *
 * World subsystem that manages teams and their relationships
 * Provides centralized team management and friend-or-foe identification
 *
 * Features:
 * - Dynamic team creation and registration
 * - Team relationship management (ally, neutral, enemy)
 * - Friend-or-foe identification between actors
 * - Team-based targeting and combat logic
 * - Supports both tag-based and numeric team IDs
 *
 * Usage:
 * 1. Register teams using RegisterTeam() or load from UHarmoniaTeamConfigData
 * 2. Set relationships using SetTeamRelationship()
 * 3. Query relationships using GetRelationship(), CanAttack(), ShouldHelp()
 * 4. Use with monsters and players that implement IHarmoniaTeamAgentInterface
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaTeamManagementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	// ============================================================================
	// Team Registration
	// ============================================================================

	/**
	 * Register a new team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	bool RegisterTeam(const FHarmoniaTeamIdentification& TeamID, UHarmoniaTeamConfigData* TeamConfig = nullptr);

	/**
	 * Unregister a team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	void UnregisterTeam(const FHarmoniaTeamIdentification& TeamID);

	/**
	 * Check if a team is registered
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	bool IsTeamRegistered(const FHarmoniaTeamIdentification& TeamID) const;

	/**
	 * Get team configuration data
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	UHarmoniaTeamConfigData* GetTeamConfig(const FHarmoniaTeamIdentification& TeamID) const;

	/**
	 * Get all registered teams
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	TArray<FHarmoniaTeamIdentification> GetAllTeams() const;

	/**
	 * Load team configuration from data asset
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	bool LoadTeamConfig(UHarmoniaTeamConfigData* TeamConfig);

	/**
	 * Load multiple team configurations
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	void LoadTeamConfigs(const TArray<UHarmoniaTeamConfigData*>& TeamConfigs);

	// ============================================================================
	// Team Relationship Management
	// ============================================================================

	/**
	 * Set relationship between two teams
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	void SetTeamRelationship(const FHarmoniaTeamIdentification& SourceTeam,
		const FHarmoniaTeamIdentification& TargetTeam,
		EHarmoniaTeamRelationship Relationship);

	/**
	 * Get relationship between two teams
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	EHarmoniaTeamRelationship GetTeamRelationship(const FHarmoniaTeamIdentification& SourceTeam,
		const FHarmoniaTeamIdentification& TargetTeam) const;

	/**
	 * Check if two teams are allies
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	bool AreTeamsAllies(const FHarmoniaTeamIdentification& TeamA,
		const FHarmoniaTeamIdentification& TeamB) const;

	/**
	 * Check if two teams are enemies
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	bool AreTeamsEnemies(const FHarmoniaTeamIdentification& TeamA,
		const FHarmoniaTeamIdentification& TeamB) const;

	/**
	 * Check if source team can attack target team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	bool CanTeamAttack(const FHarmoniaTeamIdentification& SourceTeam,
		const FHarmoniaTeamIdentification& TargetTeam) const;

	/**
	 * Check if source team should help target team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	bool ShouldTeamHelp(const FHarmoniaTeamIdentification& SourceTeam,
		const FHarmoniaTeamIdentification& TargetTeam) const;

	/**
	 * Set mutual relationship (bidirectional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	void SetMutualRelationship(const FHarmoniaTeamIdentification& TeamA,
		const FHarmoniaTeamIdentification& TeamB,
		EHarmoniaTeamRelationship Relationship);

	/**
	 * Make teams allies (bidirectional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	void MakeTeamsAllies(const FHarmoniaTeamIdentification& TeamA,
		const FHarmoniaTeamIdentification& TeamB);

	/**
	 * Make teams enemies (bidirectional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Relationships")
	void MakeTeamsEnemies(const FHarmoniaTeamIdentification& TeamA,
		const FHarmoniaTeamIdentification& TeamB);

	// ============================================================================
	// Actor-Based Queries (Friend-or-Foe Identification)
	// ============================================================================

	/**
	 * Get team ID of an actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	FHarmoniaTeamIdentification GetActorTeamID(AActor* Actor) const;

	/**
	 * Get relationship between two actors
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	EHarmoniaTeamRelationship GetActorRelationship(AActor* SourceActor, AActor* TargetActor) const;

	/**
	 * Check if two actors are on the same team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	bool AreSameTeam(AActor* ActorA, AActor* ActorB) const;

	/**
	 * Check if two actors are allies
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	bool AreAllies(AActor* ActorA, AActor* ActorB) const;

	/**
	 * Check if two actors are enemies
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	bool AreEnemies(AActor* ActorA, AActor* ActorB) const;

	/**
	 * Check if source actor can attack target actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	bool CanActorAttack(AActor* SourceActor, AActor* TargetActor) const;

	/**
	 * Check if source actor should help target actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	bool ShouldActorHelp(AActor* SourceActor, AActor* TargetActor) const;

	/**
	 * Get all actors on a specific team
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	TArray<AActor*> GetActorsOnTeam(const FHarmoniaTeamIdentification& TeamID,
		TSubclassOf<AActor> ActorClass = nullptr) const;

	/**
	 * Get all allies of an actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	TArray<AActor*> GetAlliesOf(AActor* Actor, float SearchRadius = 0.0f,
		TSubclassOf<AActor> ActorClass = nullptr) const;

	/**
	 * Get all enemies of an actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Actors")
	TArray<AActor*> GetEnemiesOf(AActor* Actor, float SearchRadius = 0.0f,
		TSubclassOf<AActor> ActorClass = nullptr) const;

	// ============================================================================
	// Utility Functions
	// ============================================================================

	/**
	 * Generate unique numeric team ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Utility")
	int32 GenerateUniqueTeamID();

	/**
	 * Create team identification from tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Utility")
	FHarmoniaTeamIdentification CreateTeamID(FGameplayTag TeamTag, FText TeamName = FText::GetEmpty());

	/**
	 * Create team identification from numeric ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Utility")
	FHarmoniaTeamIdentification CreateTeamIDFromNumeric(int32 NumericID, FText TeamName = FText::GetEmpty());

	/**
	 * Reset all team relationships to default
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	void ResetAllRelationships();

	/**
	 * Clear all teams and relationships
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management")
	void ClearAllTeams();

	// ============================================================================
	// Advanced Team Management (Dynamic Faction System)
	// ============================================================================

	/**
	 * Create a new faction dynamically
	 * @param FactionName Display name for the faction
	 * @param DefaultAttitude Default attitude towards other factions
	 * @param TeamColor Color for UI representation
	 * @return Created team identification
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	FHarmoniaTeamIdentification CreateFaction(FText FactionName,
		EHarmoniaTeamAttitude DefaultAttitude = EHarmoniaTeamAttitude::Neutral,
		FLinearColor TeamColor = FLinearColor::White);

	/**
	 * Create multiple factions and set their relationships
	 * @param FactionDefinitions Array of faction definitions with relationships
	 * @return Array of created team IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	TArray<FHarmoniaTeamIdentification> CreateFactionNetwork(const TArray<UHarmoniaTeamConfigData*>& FactionDefinitions);

	/**
	 * Make a faction hostile to multiple other factions
	 * @param SourceTeam The faction to set as hostile
	 * @param HostileFactions Array of factions to be hostile towards
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	void SetFactionHostileToMany(const FHarmoniaTeamIdentification& SourceTeam,
		const TArray<FHarmoniaTeamIdentification>& HostileFactions);

	/**
	 * Create alliance between multiple factions
	 * @param AllianceFactions Array of factions that should be allies with each other
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	void CreateAlliance(const TArray<FHarmoniaTeamIdentification>& AllianceFactions);

	/**
	 * Break alliance between two factions (set to neutral)
	 * @param TeamA First faction
	 * @param TeamB Second faction
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	void BreakAlliance(const FHarmoniaTeamIdentification& TeamA,
		const FHarmoniaTeamIdentification& TeamB);

	/**
	 * Get all factions that are enemies of a specific faction
	 * @param Team The faction to query
	 * @return Array of enemy faction IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	TArray<FHarmoniaTeamIdentification> GetEnemyFactions(const FHarmoniaTeamIdentification& Team) const;

	/**
	 * Get all factions that are allies of a specific faction
	 * @param Team The faction to query
	 * @return Array of allied faction IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	TArray<FHarmoniaTeamIdentification> GetAlliedFactions(const FHarmoniaTeamIdentification& Team) const;

	/**
	 * Change faction relationship dynamically (for story events, quests, etc.)
	 * @param FactionA First faction
	 * @param FactionB Second faction
	 * @param NewRelationship New relationship to set
	 * @param bBidirectional Whether to set relationship in both directions
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	void ChangeFactionRelationship(const FHarmoniaTeamIdentification& FactionA,
		const FHarmoniaTeamIdentification& FactionB,
		EHarmoniaTeamRelationship NewRelationship,
		bool bBidirectional = true);

	/**
	 * Get total number of registered factions
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Factions")
	int32 GetFactionCount() const;

	// ============================================================================
	// Delegates
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Team Management|Events")
	FOnTeamRelationshipChangedDelegate OnTeamRelationshipChanged;

	UPROPERTY(BlueprintAssignable, Category = "Team Management|Events")
	FOnTeamRegisteredDelegate OnTeamRegistered;

	// ============================================================================
	// Debug Functions
	// ============================================================================

	/**
	 * Print all teams and their relationships (for debugging)
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Debug")
	void DebugPrintTeams() const;

	/**
	 * Print relationship matrix (for debugging)
	 */
	UFUNCTION(BlueprintCallable, Category = "Team Management|Debug")
	void DebugPrintRelationshipMatrix() const;

protected:
	// ============================================================================
	// Internal Data
	// ============================================================================

	/** Registered teams */
	UPROPERTY()
	TMap<FHarmoniaTeamIdentification, TObjectPtr<UHarmoniaTeamConfigData>> RegisteredTeams;

	/** Team relationship matrix */
	UPROPERTY()
	TMap<FHarmoniaTeamIdentification, FHarmoniaTeamRelationshipMap> RelationshipMatrix;

	/** Next available numeric team ID */
	int32 NextNumericTeamID = 1;

	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Get default relationship based on team attitudes
	 */
	EHarmoniaTeamRelationship GetDefaultRelationship(const FHarmoniaTeamIdentification& SourceTeam,
		const FHarmoniaTeamIdentification& TargetTeam) const;

	/**
	 * Initialize default teams (Player, Neutral, etc.)
	 */
	void InitializeDefaultTeams();
};
