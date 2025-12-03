// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaSwarmDirectorComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Swarm command types
 */
UENUM(BlueprintType)
enum class EHarmoniaSwarmCommand : uint8
{
	Attack			UMETA(DisplayName = "Attack Target"),
	Surround		UMETA(DisplayName = "Surround Target"),
	WaveAttack		UMETA(DisplayName = "Wave Attack"),
	Retreat			UMETA(DisplayName = "Retreat"),
	Sacrifice		UMETA(DisplayName = "Sacrifice to Block"),
	Regroup			UMETA(DisplayName = "Regroup")
};

/**
 * Swarm formation types
 */
UENUM(BlueprintType)
enum class EHarmoniaSwarmFormation : uint8
{
	Swarm			UMETA(DisplayName = "Loose Swarm"),
	Circle			UMETA(DisplayName = "Encircle"),
	Line			UMETA(DisplayName = "Line Formation"),
	Wave			UMETA(DisplayName = "Wave Pattern"),
	Pincer			UMETA(DisplayName = "Pincer Attack")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSwarmCommandIssued, EHarmoniaSwarmCommand, Command, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwarmLeaderDied, AHarmoniaMonsterBase*, OldLeader);

/**
 * Swarm Director Component
 * One monster acts as director/leader commanding the swarm
 * If leader dies, swarm becomes disorganized
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaSwarmDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaSwarmDirectorComponent();

	//~UActorComponent interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Is this the swarm leader */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm Director")
	bool bIsSwarmLeader = false;

	/** Auto-elect leader if none exists */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm Director")
	bool bAutoElectLeader = true;

	/** Swarm detection radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm Director")
	float SwarmRadius = 1500.0f;

	/** Command update interval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm Director")
	float CommandInterval = 1.0f;

	/** Current formation */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Swarm Director")
	EHarmoniaSwarmFormation CurrentFormation;

	/** Swarm members */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Swarm Director")
	TArray<AHarmoniaMonsterBase*> SwarmMembers;

	UPROPERTY(BlueprintAssignable, Category = "Swarm Director")
	FOnSwarmCommandIssued OnCommandIssued;

	UPROPERTY(BlueprintAssignable, Category = "Swarm Director")
	FOnSwarmLeaderDied OnLeaderDied;

	/** Issue command to swarm */
	UFUNCTION(BlueprintCallable, Category = "Swarm Director")
	void IssueCommand(EHarmoniaSwarmCommand Command, AActor* Target = nullptr);

	/** Set swarm formation */
	UFUNCTION(BlueprintCallable, Category = "Swarm Director")
	void SetFormation(EHarmoniaSwarmFormation Formation);

	/** Get formation position for member */
	UFUNCTION(BlueprintCallable, Category = "Swarm Director")
	FVector GetFormationPosition(AHarmoniaMonsterBase* Member, AActor* Target) const;

	/** Elect new leader */
	UFUNCTION(BlueprintCallable, Category = "Swarm Director")
	void ElectNewLeader();

protected:
	/** Update swarm members */
	void UpdateSwarmMembers();

	/** Execute current command */
	void ExecuteCommand(float DeltaTime);

	/** Handle leader death */
	UFUNCTION()
	void OnLeaderDeath(AHarmoniaMonsterBase* DeadLeader);

	float CommandTimer = 0.0f;
	float UpdateTimer = 0.0f;

	EHarmoniaSwarmCommand CurrentCommand = EHarmoniaSwarmCommand::Attack;

	UPROPERTY()
	AActor* CommandTarget = nullptr;

	UPROPERTY()
	AHarmoniaMonsterBase* OwnerMonster = nullptr;
};
