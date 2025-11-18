// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaDeathPenaltyComponent.h"
#include "Components/HarmoniaCurrencyManagerComponent.h"
#include "Actors/HarmoniaMemoryEchoActor.h"
#include "Data/HarmoniaDeathPenaltyConfigAsset.h"
#include "Data/HarmoniaCurrencyDataAsset.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UHarmoniaDeathPenaltyComponent::UHarmoniaDeathPenaltyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentDeathState = EHarmoniaPlayerDeathState::Normal;
	ConsecutiveDeaths = 0;
}

void UHarmoniaDeathPenaltyComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache ability system component
	CachedAbilitySystemComponent = GetAbilitySystemComponent();
}

void UHarmoniaDeathPenaltyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cleanup any active effects
	RemoveEtherealStatePenalties();
	RemoveConsecutiveDeathPenalty();

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaDeathPenaltyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaDeathPenaltyComponent, CurrentDeathState);
	DOREPLIFETIME(UHarmoniaDeathPenaltyComponent, CurrentMemoryEcho);
	DOREPLIFETIME(UHarmoniaDeathPenaltyComponent, ConsecutiveDeaths);
}

void UHarmoniaDeathPenaltyComponent::OnPlayerDeath(const FVector& DeathLocation)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!DeathPenaltyConfig || !DeathPenaltyConfig->bEnableDeathPenalty)
	{
		return;
	}

	// Handle double death scenario
	if (CurrentDeathState == EHarmoniaPlayerDeathState::Ethereal && CurrentMemoryEcho)
	{
		HandleDoubleDeathLoss();
	}

	// Calculate currencies to drop
	TArray<FHarmoniaCurrencyAmount> CurrenciesToDrop = CalculateCurrenciesToDrop();

	if (CurrenciesToDrop.Num() > 0)
	{
		// Spawn memory echo
		Server_SpawnMemoryEcho(DeathLocation, CurrenciesToDrop);

		// Remove currencies from player
		UHarmoniaCurrencyManagerComponent* CurrencyManager = GetCurrencyManager();
		if (CurrencyManager)
		{
			for (const FHarmoniaCurrencyAmount& Currency : CurrenciesToDrop)
			{
				CurrencyManager->RemoveCurrency(Currency.CurrencyType, Currency.Amount);
			}
		}

		OnCurrenciesDropped.Broadcast(CurrenciesToDrop);
	}

	// Apply ethereal state
	SetDeathState(EHarmoniaPlayerDeathState::Ethereal);
	ApplyEtherealStatePenalties();

	// Increment consecutive deaths
	ConsecutiveDeaths++;
	if (ConsecutiveDeaths > 1)
	{
		ApplyConsecutiveDeathPenalty();
	}
}

bool UHarmoniaDeathPenaltyComponent::RecoverCurrenciesFromMemoryEcho()
{
	if (!CurrentMemoryEcho)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PC)
	{
		return false;
	}

	// Check if player can interact
	if (!CurrentMemoryEcho->CanPlayerInteract(PC))
	{
		return false;
	}

	// Recover currencies
	TArray<FHarmoniaCurrencyAmount> RecoveredCurrencies = CurrentMemoryEcho->RecoverCurrencies(PC);

	if (RecoveredCurrencies.Num() > 0)
	{
		// Add currencies back to player
		UHarmoniaCurrencyManagerComponent* CurrencyManager = GetCurrencyManager();
		if (CurrencyManager)
		{
			for (const FHarmoniaCurrencyAmount& Currency : RecoveredCurrencies)
			{
				CurrencyManager->AddCurrency(Currency.CurrencyType, Currency.Amount);
			}
		}

		// Check if it was fast recovery
		const bool bWasFastRecovery = CurrentMemoryEcho->IsFastRecovery();

		// Clear ethereal state
		ClearEtherealState();

		// Broadcast event
		OnCurrenciesRecovered.Broadcast(RecoveredCurrencies, bWasFastRecovery);

		return true;
	}

	return false;
}

void UHarmoniaDeathPenaltyComponent::ClearEtherealState()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Remove penalties
	RemoveEtherealStatePenalties();

	// Clear memory echo reference
	CurrentMemoryEcho = nullptr;

	// Reset consecutive deaths
	ConsecutiveDeaths = 0;
	RemoveConsecutiveDeathPenalty();

	// Set state to normal
	SetDeathState(EHarmoniaPlayerDeathState::Normal);
}

void UHarmoniaDeathPenaltyComponent::ResetAllPenalties()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Remove all penalties
	RemoveEtherealStatePenalties();
	RemoveConsecutiveDeathPenalty();

	// Destroy memory echo if exists
	if (CurrentMemoryEcho)
	{
		CurrentMemoryEcho->Destroy();
		CurrentMemoryEcho = nullptr;
	}

	// Reset state
	ConsecutiveDeaths = 0;
	SetDeathState(EHarmoniaPlayerDeathState::Normal);
}

float UHarmoniaDeathPenaltyComponent::GetDistanceToMemoryEcho() const
{
	if (!CurrentMemoryEcho || !GetOwner())
	{
		return -1.0f;
	}

	return FVector::Distance(GetOwner()->GetActorLocation(), CurrentMemoryEcho->GetActorLocation());
}

FVector UHarmoniaDeathPenaltyComponent::GetDirectionToMemoryEcho() const
{
	if (!CurrentMemoryEcho || !GetOwner())
	{
		return FVector::ZeroVector;
	}

	return (CurrentMemoryEcho->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
}

UHarmoniaCurrencyManagerComponent* UHarmoniaDeathPenaltyComponent::GetCurrencyManager() const
{
	if (!GetOwner())
	{
		return nullptr;
	}

	return GetOwner()->FindComponentByClass<UHarmoniaCurrencyManagerComponent>();
}

TArray<FHarmoniaCurrencyAmount> UHarmoniaDeathPenaltyComponent::CalculateCurrenciesToDrop() const
{
	TArray<FHarmoniaCurrencyAmount> CurrenciesToDrop;

	if (!DeathPenaltyConfig)
	{
		return CurrenciesToDrop;
	}

	UHarmoniaCurrencyManagerComponent* CurrencyManager = GetCurrencyManager();
	if (!CurrencyManager)
	{
		return CurrenciesToDrop;
	}

	// Calculate drop for each configured currency
	for (const FHarmoniaCurrencyDropConfig& DropConfig : DeathPenaltyConfig->Config.CurrencyDropConfigs)
	{
		const int32 CurrentAmount = CurrencyManager->GetCurrencyAmount(DropConfig.CurrencyType);

		if (CurrentAmount > 0 && DropConfig.DropPercentage > 0.0f)
		{
			const int32 DropAmount = FMath::CeilToInt32(CurrentAmount * DropConfig.DropPercentage);

			if (DropAmount > 0)
			{
				CurrenciesToDrop.Add(FHarmoniaCurrencyAmount(DropConfig.CurrencyType, DropAmount));
			}
		}
	}

	return CurrenciesToDrop;
}

UHarmoniaCurrencyDataAsset* UHarmoniaDeathPenaltyComponent::GetCurrencyDataAsset(EHarmoniaCurrencyType CurrencyType) const
{
	if (const TObjectPtr<UHarmoniaCurrencyDataAsset>* DataAsset = CurrencyDataAssets.Find(CurrencyType))
	{
		return *DataAsset;
	}
	return nullptr;
}

void UHarmoniaDeathPenaltyComponent::Server_SpawnMemoryEcho_Implementation(const FVector& Location, const TArray<FHarmoniaCurrencyAmount>& Currencies)
{
	if (!MemoryEchoActorClass || !GetWorld())
	{
		return;
	}

	// Destroy previous memory echo if exists
	if (CurrentMemoryEcho)
	{
		CurrentMemoryEcho->Destroy();
	}

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn memory echo
	CurrentMemoryEcho = GetWorld()->SpawnActor<AHarmoniaMemoryEchoActor>(
		MemoryEchoActorClass,
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (CurrentMemoryEcho)
	{
		APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
		CurrentMemoryEcho->Initialize(Currencies, PC, DeathPenaltyConfig);
	}
}

void UHarmoniaDeathPenaltyComponent::ApplyEtherealStatePenalties()
{
	if (!DeathPenaltyConfig)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Apply ethereal state attribute penalties via gameplay effect
	// Note: For full implementation, create a UGameplayEffect Blueprint/DataAsset with:
	// - Duration: Infinite
	// - Modifiers for MaxHealth, AttackPower, StaminaRegen, MovementSpeed (multiplicative)
	// - Apply via: ASC->ApplyGameplayEffectToSelf(...)
	//
	// Example of proper GE application (requires GE asset reference):
	// if (EtherealPenaltyGameplayEffect)
	// {
	//     FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	//     FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
	//         EtherealPenaltyGameplayEffect, 1.0f, ContextHandle);
	//     if (SpecHandle.IsValid())
	//     {
	//         EtherealStatePenaltyHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	//     }
	// }

	const FHarmoniaDeathAttributePenalty& Penalties = DeathPenaltyConfig->Config.AttributePenalties;

	UE_LOG(LogTemp, Log, TEXT("Applied ethereal state penalties: Health=%f, Damage=%f, StaminaRegen=%f, MovementSpeed=%f"),
		Penalties.HealthMultiplier,
		Penalties.DamageMultiplier,
		Penalties.StaminaRegenMultiplier,
		Penalties.MovementSpeedMultiplier
	);
}

void UHarmoniaDeathPenaltyComponent::RemoveEtherealStatePenalties()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Remove the ethereal state gameplay effect
	if (EtherealStatePenaltyHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(EtherealStatePenaltyHandle);
		EtherealStatePenaltyHandle.Invalidate();
	}

	UE_LOG(LogTemp, Log, TEXT("Removed ethereal state penalties"));
}

void UHarmoniaDeathPenaltyComponent::ApplyConsecutiveDeathPenalty()
{
	if (!DeathPenaltyConfig)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Apply max health penalty for consecutive deaths
	// Note: For full implementation, create a stacking UGameplayEffect with:
	// - Duration: Infinite (until recovered)
	// - Modifier: MaxHealth (additive or multiplicative based on design)
	// - Stack count based on consecutive deaths
	//
	// Example implementation (requires GE asset reference):
	// if (ConsecutiveDeathPenaltyGameplayEffect)
	// {
	//     FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	//     FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
	//         ConsecutiveDeathPenaltyGameplayEffect, EffectiveStacks, ContextHandle);
	//     if (SpecHandle.IsValid())
	//     {
	//         ConsecutiveDeathPenaltyHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	//     }
	// }

	const FHarmoniaDeathAttributePenalty& Penalties = DeathPenaltyConfig->Config.AttributePenalties;
	const int32 EffectiveStacks = FMath::Min(ConsecutiveDeaths - 1, Penalties.MaxHealthPenaltyStacks);
	const float TotalPenalty = EffectiveStacks * Penalties.MaxHealthPenaltyPerDeath;

	UE_LOG(LogTemp, Log, TEXT("Applied consecutive death penalty: %d deaths, %f%% max health penalty"),
		ConsecutiveDeaths, TotalPenalty * 100.0f);
}

void UHarmoniaDeathPenaltyComponent::RemoveConsecutiveDeathPenalty()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Remove the max health penalty gameplay effect
	// In full implementation with GE: ASC->RemoveActiveGameplayEffect(ConsecutiveDeathPenaltyHandle);
	if (ConsecutiveDeathPenaltyHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ConsecutiveDeathPenaltyHandle);
		ConsecutiveDeathPenaltyHandle.Invalidate();
	}

	UE_LOG(LogTemp, Log, TEXT("Removed consecutive death penalty"));
}

void UHarmoniaDeathPenaltyComponent::HandleDoubleDeathLoss()
{
	if (!CurrentMemoryEcho || !DeathPenaltyConfig)
	{
		return;
	}

	// Calculate permanent loss for each currency in the memory echo
	TArray<FHarmoniaCurrencyAmount> LostCurrencies;

	for (const FHarmoniaCurrencyAmount& Currency : CurrentMemoryEcho->StoredCurrencies)
	{
		if (Currency.Amount > 0)
		{
			const float LossPercentage = DeathPenaltyConfig->GetCurrencyPermanentLossPercentage(Currency.CurrencyType);
			const int32 LostAmount = FMath::CeilToInt32(Currency.Amount * LossPercentage);

			if (LostAmount > 0)
			{
				LostCurrencies.Add(FHarmoniaCurrencyAmount(Currency.CurrencyType, LostAmount));
			}
		}
	}

	// Destroy old memory echo
	CurrentMemoryEcho->Destroy();
	CurrentMemoryEcho = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("Double death! Lost currencies permanently."));
}

void UHarmoniaDeathPenaltyComponent::SetDeathState(EHarmoniaPlayerDeathState NewState)
{
	if (CurrentDeathState == NewState)
	{
		return;
	}

	const EHarmoniaPlayerDeathState OldState = CurrentDeathState;
	CurrentDeathState = NewState;

	OnRep_CurrentDeathState();
	OnDeathStateChanged.Broadcast(NewState);

	UE_LOG(LogTemp, Log, TEXT("Death state changed: %d -> %d"),
		static_cast<int32>(OldState),
		static_cast<int32>(NewState)
	);
}

UAbilitySystemComponent* UHarmoniaDeathPenaltyComponent::GetAbilitySystemComponent() const
{
	if (CachedAbilitySystemComponent)
	{
		return CachedAbilitySystemComponent;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Try to get ASC from owner
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
}

void UHarmoniaDeathPenaltyComponent::OnRep_CurrentDeathState()
{
	// Update visual effects, UI, etc. based on new state
	// This runs on clients when state replicates
}

void UHarmoniaDeathPenaltyComponent::OnRep_CurrentMemoryEcho()
{
	// Update UI markers, navigation hints, etc.
	// This runs on clients when memory echo changes
}
