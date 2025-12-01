// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaCurrencyManagerComponent.generated.h"

class UHarmoniaCurrencyDataAsset;

/**
 * Delegate fired when currency amount changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCurrencyChanged, EHarmoniaCurrencyType, CurrencyType, int32, NewAmount, int32, DeltaAmount);

/**
 * Delegate fired when currency transaction fails
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyTransactionFailed, EHarmoniaCurrencyType, CurrencyType, int32, AttemptedAmount);

/**
 * Component that manages player's currency inventory
 * Handles adding, removing, and querying different currency types
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaCurrencyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaCurrencyManagerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//~ Currency Storage
	/** Current currency amounts */
	UPROPERTY(ReplicatedUsing = OnRep_CurrencyAmounts, BlueprintReadOnly, Category = "Currency")
	TArray<FHarmoniaCurrencyAmount> CurrencyAmounts;

	/** Currency data assets for lookup and validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Config")
	TMap<EHarmoniaCurrencyType, TObjectPtr<UHarmoniaCurrencyDataAsset>> CurrencyDataAssets;

	//~ Delegates
	/** Fired when currency amount changes */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FOnCurrencyChanged OnCurrencyChanged;

	/** Fired when currency transaction fails (insufficient funds, carry limit, etc.) */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FOnCurrencyTransactionFailed OnCurrencyTransactionFailed;

	//~ Currency Operations
	/** Add currency to inventory */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool AddCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	/** Remove currency from inventory */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool RemoveCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	/** Set currency amount directly (use with caution) */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void SetCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	/** Transfer currency to another currency manager */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool TransferCurrency(UHarmoniaCurrencyManagerComponent* TargetManager, EHarmoniaCurrencyType CurrencyType, int32 Amount);

	/** Convert one currency to another using conversion rates */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool ConvertCurrency(EHarmoniaCurrencyType FromType, EHarmoniaCurrencyType ToType, int32 Amount);

	//~ Queries
	/** Get current amount of a currency */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurrencyAmount(EHarmoniaCurrencyType CurrencyType) const;

	/** Check if player has enough of a currency */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool HasCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount) const;

	/** Check if adding amount would exceed carry limit */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool IsWithinCarryLimit(EHarmoniaCurrencyType CurrencyType, int32 AdditionalAmount) const;

	/** Get maximum carry amount for a currency (0 = unlimited) */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetMaxCarryAmount(EHarmoniaCurrencyType CurrencyType) const;

	/** Get all non-zero currencies */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	TArray<FHarmoniaCurrencyAmount> GetAllCurrencies() const;

	/** Get currency data asset for a type */
	UFUNCTION(BlueprintPure, Category = "Currency")
	UHarmoniaCurrencyDataAsset* GetCurrencyDataAsset(EHarmoniaCurrencyType CurrencyType) const;

	//~ Save/Load Support
	/** Export currency data for save game */
	UFUNCTION(BlueprintCallable, Category = "Currency|Save")
	TMap<EHarmoniaCurrencyType, int32> ExportCurrencyData() const;

	/** Import currency data from save game */
	UFUNCTION(BlueprintCallable, Category = "Currency|Save")
	void ImportCurrencyData(const TMap<EHarmoniaCurrencyType, int32>& SavedCurrencies);

protected:
	//~ Internal
	/** Validate and clamp currency amount based on carry limits */
	int32 ValidateCurrencyAmount(EHarmoniaCurrencyType CurrencyType, int32 Amount) const;

	/** Notify currency changed */
	void NotifyCurrencyChanged(EHarmoniaCurrencyType CurrencyType, int32 NewAmount, int32 DeltaAmount);

	/** Internal helper to set currency amount in array */
	void SetCurrencyInternal(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	//~ Server RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RemoveCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	//~ Replication
	UFUNCTION()
	void OnRep_CurrencyAmounts();
};
