// Copyright Epic Games, Inc. All Rights Reserved.

#include "HarmoniaDeathPenaltyLibrary.h"
#include "Components/HarmoniaDeathPenaltyComponent.h"
#include "Components/HarmoniaCurrencyManagerComponent.h"
#include "Actors/HarmoniaMemoryEchoActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"

UHarmoniaDeathPenaltyComponent* UHarmoniaDeathPenaltyLibrary::GetDeathPenaltyComponent(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UHarmoniaDeathPenaltyComponent>();
}

UHarmoniaCurrencyManagerComponent* UHarmoniaDeathPenaltyLibrary::GetCurrencyManagerComponent(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UHarmoniaCurrencyManagerComponent>();
}

void UHarmoniaDeathPenaltyLibrary::HandlePlayerDeath(AActor* PlayerCharacter, const FVector& DeathLocation)
{
	UHarmoniaDeathPenaltyComponent* DeathPenalty = GetDeathPenaltyComponent(PlayerCharacter);
	if (DeathPenalty)
	{
		DeathPenalty->OnPlayerDeath(DeathLocation);
	}
}

bool UHarmoniaDeathPenaltyLibrary::RecoverCurrencies(AActor* PlayerCharacter)
{
	UHarmoniaDeathPenaltyComponent* DeathPenalty = GetDeathPenaltyComponent(PlayerCharacter);
	if (DeathPenalty)
	{
		return DeathPenalty->RecoverCurrenciesFromMemoryEcho();
	}
	return false;
}

bool UHarmoniaDeathPenaltyLibrary::IsPlayerInEtherealState(AActor* PlayerCharacter)
{
	UHarmoniaDeathPenaltyComponent* DeathPenalty = GetDeathPenaltyComponent(PlayerCharacter);
	if (DeathPenalty)
	{
		return DeathPenalty->IsInEtherealState();
	}
	return false;
}

float UHarmoniaDeathPenaltyLibrary::GetDistanceToMemoryEcho(AActor* PlayerCharacter)
{
	UHarmoniaDeathPenaltyComponent* DeathPenalty = GetDeathPenaltyComponent(PlayerCharacter);
	if (DeathPenalty)
	{
		return DeathPenalty->GetDistanceToMemoryEcho();
	}
	return -1.0f;
}

void UHarmoniaDeathPenaltyLibrary::ResetDeathPenalties(AActor* PlayerCharacter)
{
	UHarmoniaDeathPenaltyComponent* DeathPenalty = GetDeathPenaltyComponent(PlayerCharacter);
	if (DeathPenalty)
	{
		DeathPenalty->ResetAllPenalties();
	}
}

bool UHarmoniaDeathPenaltyLibrary::AddCurrency(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	UHarmoniaCurrencyManagerComponent* CurrencyManager = GetCurrencyManagerComponent(PlayerCharacter);
	if (CurrencyManager)
	{
		return CurrencyManager->AddCurrency(CurrencyType, Amount);
	}
	return false;
}

bool UHarmoniaDeathPenaltyLibrary::RemoveCurrency(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	UHarmoniaCurrencyManagerComponent* CurrencyManager = GetCurrencyManagerComponent(PlayerCharacter);
	if (CurrencyManager)
	{
		return CurrencyManager->RemoveCurrency(CurrencyType, Amount);
	}
	return false;
}

int32 UHarmoniaDeathPenaltyLibrary::GetCurrencyAmount(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType)
{
	UHarmoniaCurrencyManagerComponent* CurrencyManager = GetCurrencyManagerComponent(PlayerCharacter);
	if (CurrencyManager)
	{
		return CurrencyManager->GetCurrencyAmount(CurrencyType);
	}
	return 0;
}

bool UHarmoniaDeathPenaltyLibrary::HasCurrency(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	UHarmoniaCurrencyManagerComponent* CurrencyManager = GetCurrencyManagerComponent(PlayerCharacter);
	if (CurrencyManager)
	{
		return CurrencyManager->HasCurrency(CurrencyType, Amount);
	}
	return false;
}

FText UHarmoniaDeathPenaltyLibrary::GetCurrencyName(EHarmoniaCurrencyType CurrencyType)
{
	switch (CurrencyType)
	{
	case EHarmoniaCurrencyType::MemoryEssence:
		return FText::FromString(TEXT("Memory Essence"));
	case EHarmoniaCurrencyType::SoulCrystals:
		return FText::FromString(TEXT("Soul Crystals"));
	case EHarmoniaCurrencyType::ForgottenKnowledge:
		return FText::FromString(TEXT("Forgotten Knowledge"));
	case EHarmoniaCurrencyType::TimeFragments:
		return FText::FromString(TEXT("Time Fragments"));
	case EHarmoniaCurrencyType::Gold:
		return FText::FromString(TEXT("Gold"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

EHarmoniaCurrencyType UHarmoniaDeathPenaltyLibrary::GetCurrencyTypeFromString(const FString& TypeString)
{
	if (TypeString.Equals(TEXT("MemoryEssence"), ESearchCase::IgnoreCase))
	{
		return EHarmoniaCurrencyType::MemoryEssence;
	}
	else if (TypeString.Equals(TEXT("SoulCrystals"), ESearchCase::IgnoreCase))
	{
		return EHarmoniaCurrencyType::SoulCrystals;
	}
	else if (TypeString.Equals(TEXT("ForgottenKnowledge"), ESearchCase::IgnoreCase))
	{
		return EHarmoniaCurrencyType::ForgottenKnowledge;
	}
	else if (TypeString.Equals(TEXT("TimeFragments"), ESearchCase::IgnoreCase))
	{
		return EHarmoniaCurrencyType::TimeFragments;
	}
	else if (TypeString.Equals(TEXT("Gold"), ESearchCase::IgnoreCase))
	{
		return EHarmoniaCurrencyType::Gold;
	}

	return EHarmoniaCurrencyType::None;
}

FText UHarmoniaDeathPenaltyLibrary::FormatCurrencyAmount(EHarmoniaCurrencyType CurrencyType, int32 Amount)
{
	FText CurrencyName = GetCurrencyName(CurrencyType);

	return FText::Format(
		FText::FromString(TEXT("{0} x{1}")),
		CurrencyName,
		FText::AsNumber(Amount)
	);
}

AHarmoniaMemoryEchoActor* UHarmoniaDeathPenaltyLibrary::FindNearestMemoryEcho(UObject* WorldContextObject, const FVector& Location, float MaxDistance)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	AHarmoniaMemoryEchoActor* NearestEcho = nullptr;
	float NearestDistance = MaxDistance;

	for (TActorIterator<AHarmoniaMemoryEchoActor> It(World); It; ++It)
	{
		AHarmoniaMemoryEchoActor* Echo = *It;
		if (Echo)
		{
			const float Distance = FVector::Distance(Location, Echo->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestEcho = Echo;
			}
		}
	}

	return NearestEcho;
}

TArray<AHarmoniaMemoryEchoActor*> UHarmoniaDeathPenaltyLibrary::GetAllMemoryEchoes(UObject* WorldContextObject)
{
	TArray<AHarmoniaMemoryEchoActor*> MemoryEchoes;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return MemoryEchoes;
	}

	for (TActorIterator<AHarmoniaMemoryEchoActor> It(World); It; ++It)
	{
		AHarmoniaMemoryEchoActor* Echo = *It;
		if (Echo)
		{
			MemoryEchoes.Add(Echo);
		}
	}

	return MemoryEchoes;
}
