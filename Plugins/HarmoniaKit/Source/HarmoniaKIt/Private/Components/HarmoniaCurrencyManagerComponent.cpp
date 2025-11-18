// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaCurrencyManagerComponent.h"
#include "Data/HarmoniaCurrencyDataAsset.h"
#include "Net/UnrealNetwork.h"

UHarmoniaCurrencyManagerComponent::UHarmoniaCurrencyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHarmoniaCurrencyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize all configured currency types to 0 if not already set
	for (const auto& Pair : CurrencyDataAssets)
	{
		if (!CurrencyAmounts.Contains(Pair.Key))
		{
			CurrencyAmounts.Add(Pair.Key, 0);
		}
	}
}

void UHarmoniaCurrencyManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaCurrencyManagerComponent, CurrencyAmounts);
}

bool UHarmoniaCurrencyManagerComponent::AddCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	// Check carry limit
	if (!IsWithinCarryLimit(CurrencyType, Amount))
	{
		OnCurrencyTransactionFailed.Broadcast(CurrencyType, Amount);
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		const int32 CurrentAmount = GetCurrencyAmount(CurrencyType);
		const int32 NewAmount = CurrentAmount + Amount;
		const int32 ValidatedAmount = ValidateCurrencyAmount(CurrencyType, NewAmount);

		CurrencyAmounts.Add(CurrencyType, ValidatedAmount);
		NotifyCurrencyChanged(CurrencyType, ValidatedAmount, Amount);

		return true;
	}
	else
	{
		Server_AddCurrency(CurrencyType, Amount);
		return true; // Assume success on client, will be corrected by replication if failed
	}
}

bool UHarmoniaCurrencyManagerComponent::RemoveCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	// Check if we have enough
	if (!HasCurrency(CurrencyType, Amount))
	{
		OnCurrencyTransactionFailed.Broadcast(CurrencyType, Amount);
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		const int32 CurrentAmount = GetCurrencyAmount(CurrencyType);
		const int32 NewAmount = FMath::Max(0, CurrentAmount - Amount);

		CurrencyAmounts.Add(CurrencyType, NewAmount);
		NotifyCurrencyChanged(CurrencyType, NewAmount, -Amount);

		return true;
	}
	else
	{
		Server_RemoveCurrency(CurrencyType, Amount);
		return true; // Assume success on client
	}
}

void UHarmoniaCurrencyManagerComponent::SetCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	if (GetOwner()->HasAuthority())
	{
		const int32 CurrentAmount = GetCurrencyAmount(CurrencyType);
		const int32 ValidatedAmount = ValidateCurrencyAmount(CurrencyType, Amount);
		const int32 DeltaAmount = ValidatedAmount - CurrentAmount;

		CurrencyAmounts.Add(CurrencyType, ValidatedAmount);
		NotifyCurrencyChanged(CurrencyType, ValidatedAmount, DeltaAmount);
	}
	else
	{
		Server_SetCurrency(CurrencyType, Amount);
	}
}

bool UHarmoniaCurrencyManagerComponent::TransferCurrency(UHarmoniaCurrencyManagerComponent* TargetManager, EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	if (!TargetManager || Amount <= 0)
	{
		return false;
	}

	// Check if we have enough
	if (!HasCurrency(CurrencyType, Amount))
	{
		return false;
	}

	// Check target carry limit
	if (!TargetManager->IsWithinCarryLimit(CurrencyType, Amount))
	{
		return false;
	}

	// Remove from source
	if (RemoveCurrency(CurrencyType, Amount))
	{
		// Add to target
		return TargetManager->AddCurrency(CurrencyType, Amount);
	}

	return false;
}

bool UHarmoniaCurrencyManagerComponent::ConvertCurrency(EHarmoniaCurrencyType FromType, EHarmoniaCurrencyType ToType, int32 Amount)
{
	// Get currency data assets
	UHarmoniaCurrencyDataAsset* FromDataAsset = GetCurrencyDataAsset(FromType);
	if (!FromDataAsset)
	{
		return false;
	}

	// Check if we have enough source currency
	if (!HasCurrency(FromType, Amount))
	{
		return false;
	}

	// Calculate conversion
	const int32 ConvertedAmount = FromDataAsset->ConvertTo(ToType, Amount);
	if (ConvertedAmount <= 0)
	{
		return false;
	}

	// Check target carry limit
	if (!IsWithinCarryLimit(ToType, ConvertedAmount))
	{
		return false;
	}

	// Perform conversion
	if (RemoveCurrency(FromType, Amount))
	{
		return AddCurrency(ToType, ConvertedAmount);
	}

	return false;
}

int32 UHarmoniaCurrencyManagerComponent::GetCurrencyAmount(EHarmoniaCurrencyType CurrencyType) const
{
	if (const int32* Amount = CurrencyAmounts.Find(CurrencyType))
	{
		return *Amount;
	}
	return 0;
}

bool UHarmoniaCurrencyManagerComponent::HasCurrency(EHarmoniaCurrencyType CurrencyType, int32 Amount) const
{
	return GetCurrencyAmount(CurrencyType) >= Amount;
}

bool UHarmoniaCurrencyManagerComponent::IsWithinCarryLimit(EHarmoniaCurrencyType CurrencyType, int32 AdditionalAmount) const
{
	const int32 MaxCarry = GetMaxCarryAmount(CurrencyType);

	// 0 = unlimited
	if (MaxCarry <= 0)
	{
		return true;
	}

	const int32 CurrentAmount = GetCurrencyAmount(CurrencyType);
	return (CurrentAmount + AdditionalAmount) <= MaxCarry;
}

int32 UHarmoniaCurrencyManagerComponent::GetMaxCarryAmount(EHarmoniaCurrencyType CurrencyType) const
{
	UHarmoniaCurrencyDataAsset* DataAsset = GetCurrencyDataAsset(CurrencyType);
	if (DataAsset)
	{
		return DataAsset->MaxCarryAmount;
	}
	return 0; // Unlimited if no data asset
}

TArray<FHarmoniaCurrencyAmount> UHarmoniaCurrencyManagerComponent::GetAllCurrencies() const
{
	TArray<FHarmoniaCurrencyAmount> Result;

	for (const auto& Pair : CurrencyAmounts)
	{
		if (Pair.Value > 0)
		{
			Result.Add(FHarmoniaCurrencyAmount(Pair.Key, Pair.Value));
		}
	}

	return Result;
}

UHarmoniaCurrencyDataAsset* UHarmoniaCurrencyManagerComponent::GetCurrencyDataAsset(EHarmoniaCurrencyType CurrencyType) const
{
	if (const TObjectPtr<UHarmoniaCurrencyDataAsset>* DataAsset = CurrencyDataAssets.Find(CurrencyType))
	{
		return *DataAsset;
	}
	return nullptr;
}

TMap<EHarmoniaCurrencyType, int32> UHarmoniaCurrencyManagerComponent::ExportCurrencyData() const
{
	return CurrencyAmounts;
}

void UHarmoniaCurrencyManagerComponent::ImportCurrencyData(const TMap<EHarmoniaCurrencyType, int32>& SavedCurrencies)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Clear current currencies
	CurrencyAmounts.Empty();

	// Import saved data
	for (const auto& Pair : SavedCurrencies)
	{
		const int32 ValidatedAmount = ValidateCurrencyAmount(Pair.Key, Pair.Value);
		CurrencyAmounts.Add(Pair.Key, ValidatedAmount);
	}

	OnRep_CurrencyAmounts();
}

int32 UHarmoniaCurrencyManagerComponent::ValidateCurrencyAmount(EHarmoniaCurrencyType CurrencyType, int32 Amount) const
{
	// Clamp to non-negative
	int32 ValidatedAmount = FMath::Max(0, Amount);

	// Check carry limit
	const int32 MaxCarry = GetMaxCarryAmount(CurrencyType);
	if (MaxCarry > 0)
	{
		ValidatedAmount = FMath::Min(ValidatedAmount, MaxCarry);
	}

	return ValidatedAmount;
}

void UHarmoniaCurrencyManagerComponent::NotifyCurrencyChanged(EHarmoniaCurrencyType CurrencyType, int32 NewAmount, int32 DeltaAmount)
{
	OnCurrencyChanged.Broadcast(CurrencyType, NewAmount, DeltaAmount);

	UE_LOG(LogTemp, Log, TEXT("Currency changed: Type=%d, NewAmount=%d, Delta=%d"),
		static_cast<int32>(CurrencyType), NewAmount, DeltaAmount);
}

void UHarmoniaCurrencyManagerComponent::Server_AddCurrency_Implementation(EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	AddCurrency(CurrencyType, Amount);
}

void UHarmoniaCurrencyManagerComponent::Server_RemoveCurrency_Implementation(EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	RemoveCurrency(CurrencyType, Amount);
}

void UHarmoniaCurrencyManagerComponent::Server_SetCurrency_Implementation(EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	SetCurrency(CurrencyType, Amount);
}

void UHarmoniaCurrencyManagerComponent::OnRep_CurrencyAmounts()
{
	// Notify UI and other systems that currencies have changed
	// This runs on clients when CurrencyAmounts replicates
}
