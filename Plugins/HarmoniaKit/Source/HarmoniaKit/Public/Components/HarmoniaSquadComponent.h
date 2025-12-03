// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseAIComponent.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "HarmoniaSquadComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Squad State Changed Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSquadStateChangedDelegate, AActor*, SquadLeader, int32, MemberCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSquadMemberAddedDelegate, AActor*, NewMember, EHarmoniaSquadRole, AssignedRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSquadMemberRemovedDelegate, AActor*, RemovedMember);

/**
 * UHarmoniaSquadComponent
 *
 * Component that manages squad/group behavior for monsters
 * Allows monsters to coordinate attacks, maintain formations, and work together
 * Can be added to monster actors to enable group AI
 *
 * Features:
 * - Squad formation (circle, line, wedge, flanking)
 * - Role assignment (leader, tank, DPS, support, scout)
 * - Coordinated attacks
 * - Regrouping behavior
 * - Leader following
 *
 * Inherits common AI functionality from UHarmoniaBaseAIComponent
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaSquadComponent : public UHarmoniaBaseAIComponent
{
	GENERATED_BODY()

public:
	UHarmoniaSquadComponent();

protected:
	//~UHarmoniaBaseAIComponent interface
	virtual void InitializeAIComponent() override;
	virtual void UpdateAIComponent(float DeltaTime) override;
	virtual bool IsInCombat() const override;
	//~End of UHarmoniaBaseAIComponent interface

public:
	//~UActorComponent interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Whether this component is active
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	bool bSquadEnabled = true;

	/**
	 * Maximum distance to maintain squad cohesion
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	float MaxSquadDistance = 2000.0f;

	/**
	 * How often to update formation (in seconds)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	float FormationUpdateInterval = 0.5f;

	/**
	 * Whether to auto-assign roles based on monster stats
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	bool bAutoAssignRoles = true;

	// ============================================================================
	// Squad State
	// ============================================================================

	/**
	 * Current squad leader (nullptr if this is the leader)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad", Replicated)
	TObjectPtr<AActor> SquadLeader = nullptr;

	/**
	 * Squad members (only valid on leader)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad", Replicated)
	TArray<FHarmoniaSquadMemberInfo> SquadMembers;

	/**
	 * Current formation type
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad", Replicated)
	EHarmoniaSquadFormationType CurrentFormation = EHarmoniaSquadFormationType::Loose;

	/**
	 * This member's assigned role
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad", Replicated)
	EHarmoniaSquadRole MyRole = EHarmoniaSquadRole::DPS;

	/**
	 * Shared target for the squad
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad", Replicated)
	TObjectPtr<AActor> SharedTarget = nullptr;

	// ============================================================================
	// Delegates
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Squad|Events")
	FOnSquadStateChangedDelegate OnSquadStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Squad|Events")
	FOnSquadMemberAddedDelegate OnSquadMemberAdded;

	UPROPERTY(BlueprintAssignable, Category = "Squad|Events")
	FOnSquadMemberRemovedDelegate OnSquadMemberRemoved;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * Check if this monster is the squad leader
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	bool IsSquadLeader() const;

	/**
	 * Get the number of alive squad members
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	int32 GetSquadSize() const;

	/**
	 * Join a squad (as follower)
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	bool JoinSquad(AActor* Leader);

	/**
	 * Leave current squad
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void LeaveSquad();

	/**
	 * Add a member to this squad (only works if this is the leader)
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	bool AddSquadMember(AActor* NewMember, EHarmoniaSquadRole Role = EHarmoniaSquadRole::DPS);

	/**
	 * Remove a member from squad
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void RemoveSquadMember(AActor* Member);

	/**
	 * Get formation position for this member
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	FVector GetFormationPosition() const;

	/**
	 * Set shared target for the squad
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void SetSquadTarget(AActor* NewTarget);

	/**
	 * Request coordinated attack on target
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void RequestCoordinatedAttack(AActor* Target);

	/**
	 * Get all alive squad members
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	TArray<AActor*> GetAliveSquadMembers() const;

	/**
	 * Find nearby monsters to form a squad with
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	TArray<AActor*> FindNearbyMonsters(float SearchRadius = 2000.0f) const;

	/**
	 * Try to form a squad with nearby monsters
	 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	bool TryFormSquad();

protected:
	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Update formation positions for all members
	 */
	void UpdateFormation();

	/**
	 * Calculate formation offset for a member
	 */
	FVector CalculateFormationOffset(int32 MemberIndex, int32 TotalMembers) const;

	/**
	 * Auto-assign role based on monster stats
	 */
	EHarmoniaSquadRole DetermineRole(AActor* Monster) const;

	/**
	 * Check if squad members are too far apart
	 */
	void CheckSquadCohesion();

	/**
	 * Handle squad member death
	 */
	UFUNCTION()
	void OnMemberDeath(AHarmoniaMonsterBase* DeadMember, AActor* Killer);

	// ============================================================================
	// Internal State
	// ============================================================================

	/**
	 * Time since last formation update
	 */
	float TimeSinceFormationUpdate = 0.0f;

	/**
	 * Whether squad has been initialized
	 */
	bool bSquadInitialized = false;
};
