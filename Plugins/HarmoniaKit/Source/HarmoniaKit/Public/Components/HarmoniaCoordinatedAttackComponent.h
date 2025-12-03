// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaCoordinatedAttackComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Coordinated attack roles
 */
UENUM(BlueprintType)
enum class EHarmoniaCoordinatedRole : uint8
{
	Aggro		UMETA(DisplayName = "Aggro - Draw Attention"),
	Flanker		UMETA(DisplayName = "Flanker - Attack from Behind"),
	Ambusher	UMETA(DisplayName = "Ambusher - Surprise Attack"),
	Support		UMETA(DisplayName = "Support - Buff/Heal Allies"),
	Controller	UMETA(DisplayName = "Controller - CC Target")
};

/**
 * Coordinated attack pattern
 */
USTRUCT(BlueprintType)
struct FHarmoniaCoordinatedAttackPattern
{
	GENERATED_BODY()

	/** Pattern name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PatternName;

	/** Required number of participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredParticipants = 2;

	/** Role assignments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EHarmoniaCoordinatedRole, int32> RoleAssignments;

	/** Attack delay between participants (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDelay = 0.5f;

	/** Success bonus damage multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SuccessBonusMultiplier = 1.5f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoordinatedAttackStarted, FString, PatternName, int32, Participants);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoordinatedAttackCompleted, bool, bSuccess);

/**
 * Component for coordinating attacks between multiple monsters
 * Implements tactical teamwork patterns
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaCoordinatedAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaCoordinatedAttackComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Enable coordinated attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinated Attack")
	bool bEnableCoordination = true;

	/** Detection radius for allies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinated Attack")
	float AllyDetectionRadius = 1000.0f;

	/** Available attack patterns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinated Attack")
	TArray<FHarmoniaCoordinatedAttackPattern> AttackPatterns;

	/** Cooldown between coordinated attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinated Attack")
	float CoordinationCooldown = 10.0f;

	UPROPERTY(BlueprintAssignable, Category = "Coordinated Attack")
	FOnCoordinatedAttackStarted OnAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Coordinated Attack")
	FOnCoordinatedAttackCompleted OnAttackCompleted;

	/** Initiate coordinated attack */
	UFUNCTION(BlueprintCallable, Category = "Coordinated Attack")
	bool InitiateCoordinatedAttack(AActor* Target);

	/** Assign role to participant */
	UFUNCTION(BlueprintCallable, Category = "Coordinated Attack")
	void AssignRole(AHarmoniaMonsterBase* Monster, EHarmoniaCoordinatedRole Role);

	/** Get nearby allies */
	UFUNCTION(BlueprintCallable, Category = "Coordinated Attack")
	TArray<AHarmoniaMonsterBase*> GetNearbyAllies() const;

protected:
	UPROPERTY()
	AHarmoniaMonsterBase* OwnerMonster;

	UPROPERTY()
	TMap<AHarmoniaMonsterBase*, EHarmoniaCoordinatedRole> ActiveRoles;

	bool bCoordinationActive = false;
	float CooldownTimer = 0.0f;
};
