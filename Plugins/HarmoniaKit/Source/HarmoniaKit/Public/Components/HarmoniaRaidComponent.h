// Copyright (c) 2025 RedFlowering. All Rights Reserved.

/**
 * @file HarmoniaRaidComponent.h
 * @brief Raid combat management component
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaRaidComponent.generated.h"

/**
 * @struct FRaidMemberInfo
 * @brief Raid member information structure
 */
USTRUCT(BlueprintType)
struct FRaidMemberInfo
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerID;

	/** Player name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerName;

	/** Role assignment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	ERaidRole Role = ERaidRole::DPS;

	/** Ready status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsReady = false;

	/** Alive status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsAlive = true;

	/** Remaining revive count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 RemainingRevives = 0;
};

/**
 * @struct FRaidPhaseInfo
 * @brief Raid phase information structure
 */
USTRUCT(BlueprintType)
struct FRaidPhaseInfo
{
	GENERATED_BODY()

	/** Phase number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 PhaseNumber = 1;

	/** Phase name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseName;

	/** Boss health threshold (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	float HealthThreshold = 100.0f;

	/** Phase description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseDescription;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberJoined, const FRaidMemberInfo&, MemberInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberLeft, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidPhaseChanged, int32, OldPhase, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidMemberDied, const FString&, PlayerID, int32, RemainingRevives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidWipe, int32, CurrentPhase);

/**
 * @class UHarmoniaRaidComponent
 * @brief Raid combat management component
 * 
 * Manages raid battles including member management, phase transitions, and combat state.
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaRaidComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRaidComponent();

protected:
	virtual void BeginPlay() override;

public:
	//~ Delegates
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidMemberJoined OnRaidMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidMemberLeft OnRaidMemberLeft;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidPhaseChanged OnRaidPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidMemberDied OnRaidMemberDied;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidWipe OnRaidWipe;

	//~ Member Management

	/** Add member to raid */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool AddMember(const FRaidMemberInfo& MemberInfo);

	/** Remove member from raid */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool RemoveMember(const FString& PlayerID);

	/** Get member info by player ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidMemberInfo GetMemberInfo(const FString& PlayerID) const;

	/** Get all raid members */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetAllMembers() const;

	/** Get members by role */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetMembersByRole(ERaidRole Role) const;

	/** Get raid size */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetRaidSize() const { return RaidMembers.Num(); }

	//~ Ready Status

	/** Set member ready status */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void SetMemberReady(const FString& PlayerID, bool bReady);

	/** Check if all members are ready */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool AreAllMembersReady() const;

	/** Get count of ready members */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetReadyMemberCount() const;

	//~ Phase Management

	/** Get current phase */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	/** Advance to next phase */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void AdvanceToNextPhase();

	/** Get phase info by number */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidPhaseInfo GetPhaseInfo(int32 PhaseNumber) const;

	//~ Combat Management

	/** Process member death */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void OnMemberDeath(const FString& PlayerID);

	/** Revive member */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ReviveMember(const FString& PlayerID);

	/** Get count of alive members */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetAliveMemberCount() const;

	/** Check if raid is wiped */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool IsWiped() const;

	//~ Role Composition Validation

	/** Validate role composition */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ValidateRoleComposition(const URaidDataAsset* RaidData) const;

	/** Get role distribution */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TMap<ERaidRole, int32> GetRoleDistribution() const;

protected:
	/** Raid members map */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TMap<FString, FRaidMemberInfo> RaidMembers;

	/** Current phase number */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 CurrentPhase;

	/** Phase info list */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TArray<FRaidPhaseInfo> PhaseInfos;

	/** Global revive limit */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 GlobalReviveLimit;

	/** Used revive count */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 UsedRevives;

	/** Current raid data */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	const URaidDataAsset* CurrentRaidData;

private:
	/** Process wipe */
	void ProcessWipe();
};
