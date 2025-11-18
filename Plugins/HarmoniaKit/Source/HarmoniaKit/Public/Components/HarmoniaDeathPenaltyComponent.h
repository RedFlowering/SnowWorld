// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "GameplayEffectTypes.h"
#include "HarmoniaDeathPenaltyComponent.generated.h"

class AHarmoniaMemoryEchoActor;
class UHarmoniaDeathPenaltyConfigAsset;
class UHarmoniaCurrencyDataAsset;
class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * Delegate fired when player enters/exits ethereal state
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathStateChanged, EHarmoniaPlayerDeathState, NewState);

/**
 * Delegate fired when currencies are dropped on death
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrenciesDropped, const TArray<FHarmoniaCurrencyAmount>&, DroppedCurrencies);

/**
 * Delegate fired when currencies are recovered
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrenciesRecovered, const TArray<FHarmoniaCurrencyAmount>&, RecoveredCurrencies, bool, bWasFastRecovery);

/**
 * Component that manages death penalties and currency recovery mechanics
 * Attach to player character to enable souls-like death penalty system
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaDeathPenaltyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaDeathPenaltyComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//~ Configuration
	/** Death penalty configuration asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Config")
	TObjectPtr<UHarmoniaDeathPenaltyConfigAsset> DeathPenaltyConfig = nullptr;

	/** Currency data assets for lookup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Config")
	TMap<EHarmoniaCurrencyType, TObjectPtr<UHarmoniaCurrencyDataAsset>> CurrencyDataAssets;

	/** Memory Echo actor class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Config")
	TSubclassOf<AHarmoniaMemoryEchoActor> MemoryEchoActorClass;

	//~ Current State
	/** Current death state */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentDeathState, BlueprintReadOnly, Category = "Death Penalty|State")
	EHarmoniaPlayerDeathState CurrentDeathState = EHarmoniaPlayerDeathState::Normal;

	/** Current memory echo actor (where currencies are dropped) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMemoryEcho, BlueprintReadOnly, Category = "Death Penalty|State")
	TObjectPtr<AHarmoniaMemoryEchoActor> CurrentMemoryEcho = nullptr;

	/** Number of consecutive deaths without recovery */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Death Penalty|State")
	int32 ConsecutiveDeaths = 0;

	/** Active ethereal state penalty effect */
	UPROPERTY(BlueprintReadOnly, Category = "Death Penalty|State")
	FActiveGameplayEffectHandle EtherealStatePenaltyHandle;

	//~ Delegates
	/** Fired when death state changes */
	UPROPERTY(BlueprintAssignable, Category = "Death Penalty|Events")
	FOnDeathStateChanged OnDeathStateChanged;

	/** Fired when currencies are dropped */
	UPROPERTY(BlueprintAssignable, Category = "Death Penalty|Events")
	FOnCurrenciesDropped OnCurrenciesDropped;

	/** Fired when currencies are recovered */
	UPROPERTY(BlueprintAssignable, Category = "Death Penalty|Events")
	FOnCurrenciesRecovered OnCurrenciesRecovered;

	//~ Main API
	/** Called when player dies - handles currency drop and penalty application */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty", meta = (AutoCreateRefTerm = "DeathLocation"))
	void OnPlayerDeath(const FVector& DeathLocation);

	/** Attempt to recover currencies from memory echo */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	bool RecoverCurrenciesFromMemoryEcho();

	/** Clear ethereal state (called after successful recovery or resting) */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	void ClearEtherealState();

	/** Reset all death penalties (called at safe zones/rest points) */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	void ResetAllPenalties();

	//~ State Queries
	/** Check if player is in ethereal state */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	bool IsInEtherealState() const { return CurrentDeathState == EHarmoniaPlayerDeathState::Ethereal; }

	/** Get distance to current memory echo */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetDistanceToMemoryEcho() const;

	/** Get direction to memory echo */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	FVector GetDirectionToMemoryEcho() const;

	/** Check if memory echo exists */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	bool HasActiveMemoryEcho() const { return CurrentMemoryEcho != nullptr; }

	//~ Currency Management Integration
	/** Get reference to currency manager component (should be on same actor) */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	class UHarmoniaCurrencyManagerComponent* GetCurrencyManager() const;

	/** Calculate currencies to drop based on current inventory */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	TArray<FHarmoniaCurrencyAmount> CalculateCurrenciesToDrop() const;

	/** Get currency data asset for a specific type */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	UHarmoniaCurrencyDataAsset* GetCurrencyDataAsset(EHarmoniaCurrencyType CurrencyType) const;

protected:
	//~ Internal Implementation
	/** Spawn memory echo at death location */
	UFUNCTION(Server, Reliable)
	void Server_SpawnMemoryEcho(const FVector& Location, const TArray<FHarmoniaCurrencyAmount>& Currencies);

	/** Apply ethereal state penalties */
	void ApplyEtherealStatePenalties();

	/** Remove ethereal state penalties */
	void RemoveEtherealStatePenalties();

	/** Apply max health penalty for consecutive deaths */
	void ApplyConsecutiveDeathPenalty();

	/** Remove max health penalty */
	void RemoveConsecutiveDeathPenalty();

	/** Handle double death scenario */
	void HandleDoubleDeathLoss();

	/** Change death state */
	void SetDeathState(EHarmoniaPlayerDeathState NewState);

	/** Get ability system component */
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	//~ Replication
	UFUNCTION()
	void OnRep_CurrentDeathState();

	UFUNCTION()
	void OnRep_CurrentMemoryEcho();

	//~ Cached References
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent = nullptr;
};
