// Copyright 2025 Snow Game Studio. All Rights Reserved.

#include "Core/HarmoniaCoreBFL.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"

// ============================================================================
// Authority & Server Checks
// ============================================================================

bool UHarmoniaCoreBFL::HasServerAuthority(const AActor* Actor)
{
	return Actor && Actor->HasAuthority();
}

bool UHarmoniaCoreBFL::IsComponentServerAuthoritative(const UActorComponent* Component)
{
	if (!Component)
	{
		return false;
	}

	AActor* Owner = Component->GetOwner();
	return Owner && Owner->HasAuthority();
}

bool UHarmoniaCoreBFL::CheckServerAuthority(const UActorComponent* Component, const FString& FunctionName)
{
	if (!IsComponentServerAuthoritative(Component))
	{
		if (!FunctionName.IsEmpty())
		{
			UE_LOG(LogHarmoniaKit, Warning, TEXT("[%s] Called on non-authoritative client. Ignoring."), *FunctionName);
		}
		return false;
	}
	return true;
}

bool UHarmoniaCoreBFL::ShouldCallServerRPC(const AActor* Actor)
{
	return Actor && !Actor->HasAuthority();
}

// ============================================================================
// World & GameInstance Access
// ============================================================================

UWorld* UHarmoniaCoreBFL::GetWorldSafe(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	return WorldContextObject->GetWorld();
}

UGameInstance* UHarmoniaCoreBFL::GetGameInstanceSafe(const UObject* WorldContextObject)
{
	UWorld* World = GetWorldSafe(WorldContextObject);
	if (!World)
	{
		return nullptr;
	}
	return World->GetGameInstance();
}

// ============================================================================
// Component Access
// ============================================================================

UActorComponent* UHarmoniaCoreBFL::GetComponentByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass)
{
	if (!Actor || !ComponentClass)
	{
		return nullptr;
	}
	return Actor->FindComponentByClass(ComponentClass);
}

// ============================================================================
// Ability System Access
// ============================================================================

UAbilitySystemComponent* UHarmoniaCoreBFL::GetASC(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
}

bool UHarmoniaCoreBFL::HasASC(const AActor* Actor)
{
	// const_cast??UAbilitySystemGlobals APIê°€ non-constë¥??”êµ¬?˜ê¸° ?Œë¬¸
	return GetASC(const_cast<AActor*>(Actor)) != nullptr;
}

bool UHarmoniaCoreBFL::HasGameplayTag(AActor* Actor, FGameplayTag Tag)
{
	if (UAbilitySystemComponent* ASC = GetASC(Actor))
	{
		return ASC->HasMatchingGameplayTag(Tag);
	}
	return false;
}

void UHarmoniaCoreBFL::AddGameplayTag(AActor* Actor, FGameplayTag Tag)
{
	if (UAbilitySystemComponent* ASC = GetASC(Actor))
	{
		ASC->AddLooseGameplayTag(Tag);
	}
}

void UHarmoniaCoreBFL::RemoveGameplayTag(AActor* Actor, FGameplayTag Tag)
{
	if (UAbilitySystemComponent* ASC = GetASC(Actor))
	{
		ASC->RemoveLooseGameplayTag(Tag);
	}
}

// ============================================================================
// Player Access
// ============================================================================

APlayerController* UHarmoniaCoreBFL::GetLocalPlayerController(const UObject* WorldContextObject, int32 PlayerIndex)
{
	UWorld* World = GetWorldSafe(WorldContextObject);
	if (!World)
	{
		return nullptr;
	}

	return World->GetFirstPlayerController();
}

APawn* UHarmoniaCoreBFL::GetLocalPlayerPawn(const UObject* WorldContextObject, int32 PlayerIndex)
{
	APlayerController* PC = GetLocalPlayerController(WorldContextObject, PlayerIndex);
	if (!PC)
	{
		return nullptr;
	}
	return PC->GetPawn();
}

// ============================================================================
// Validation & Null Checks
// ============================================================================

bool UHarmoniaCoreBFL::IsValidObject(const UObject* Object)
{
	return IsValid(Object);
}

bool UHarmoniaCoreBFL::IsValidActor(const AActor* Actor)
{
	return IsValid(Actor) && !Actor->IsPendingKillPending();
}

bool UHarmoniaCoreBFL::AreAllValid(std::initializer_list<const UObject*> Objects)
{
	for (const UObject* Object : Objects)
	{
		if (!IsValidObject(Object))
		{
			return false;
		}
	}
	return true;
}

// ============================================================================
// Debug & Logging Helpers
// ============================================================================

void UHarmoniaCoreBFL::LogIfDevelopment(const FString& Message, ELogVerbosity::Type Verbosity)
{
#if !UE_BUILD_SHIPPING
	switch (Verbosity)
	{
	case ELogVerbosity::Error:
		UE_LOG(LogHarmoniaKit, Error, TEXT("%s"), *Message);
		break;
	case ELogVerbosity::Warning:
		UE_LOG(LogHarmoniaKit, Warning, TEXT("%s"), *Message);
		break;
	default:
		UE_LOG(LogHarmoniaKit, Log, TEXT("%s"), *Message);
		break;
	}
#endif
}

bool UHarmoniaCoreBFL::LogErrorAndReturnFalse(const FString& ErrorMessage)
{
	UE_LOG(LogHarmoniaKit, Error, TEXT("%s"), *ErrorMessage);
	return false;
}
