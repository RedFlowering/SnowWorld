// Copyright Epic Games, Inc. All Rights Reserved.

#include "Utils/HarmoniaReplicationUtils.h"
#include "HarmoniaLogCategories.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

bool UHarmoniaReplicationUtils::IsServer(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	return World->GetNetMode() != NM_Client;
}

bool UHarmoniaReplicationUtils::IsClient(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	return World->GetNetMode() == NM_Client;
}

bool UHarmoniaReplicationUtils::IsStandalone(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	return World->GetNetMode() == NM_Standalone;
}

bool UHarmoniaReplicationUtils::IsLocallyControlled(const APawn* Pawn)
{
	if (!Pawn)
	{
		return false;
	}

	return Pawn->IsLocallyControlled();
}

bool UHarmoniaReplicationUtils::HasAuthority(const AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	return Actor->HasAuthority();
}

bool UHarmoniaReplicationUtils::IsOwnedByLocalPlayer(const AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	// Check if the actor's owner is a locally controlled pawn
	AActor* Owner = Actor->GetOwner();
	while (Owner)
	{
		if (APawn* PawnOwner = Cast<APawn>(Owner))
		{
			return PawnOwner->IsLocallyControlled();
		}
		Owner = Owner->GetOwner();
	}

	return false;
}

FString UHarmoniaReplicationUtils::GetActorRoleString(const AActor* Actor)
{
	if (!Actor)
	{
		return TEXT("Invalid Actor");
	}

	switch (Actor->GetLocalRole())
	{
	case ROLE_Authority:
		return TEXT("Authority");
	case ROLE_AutonomousProxy:
		return TEXT("AutonomousProxy");
	case ROLE_SimulatedProxy:
		return TEXT("SimulatedProxy");
	case ROLE_None:
		return TEXT("None");
	default:
		return TEXT("Unknown");
	}
}

bool UHarmoniaReplicationUtils::ShouldReplicateToPlayer(const AActor* Actor, const APlayerController* PlayerController)
{
	if (!Actor || !PlayerController)
	{
		return false;
	}

	// Basic check - can be extended for more complex visibility rules
	return Actor->IsNetRelevantFor(PlayerController->GetPawn(), PlayerController, Actor->GetActorLocation());
}

bool UHarmoniaReplicationUtils::IsRPCRateLimited(float& LastCallTime, float WorldTime, float MinInterval)
{
	const float TimeSinceLastCall = WorldTime - LastCallTime;

	if (TimeSinceLastCall < MinInterval)
	{
		UE_LOG(LogHarmoniaNetwork, Warning, TEXT("RPC rate limited: %.2fs since last call (min: %.2fs)"),
			TimeSinceLastCall, MinInterval);
		return true;
	}

	LastCallTime = WorldTime;
	return false;
}

bool UHarmoniaReplicationUtils::ValidateActorForRPC(const AActor* Actor, bool bRequireAuthority)
{
	if (!Actor)
	{
		UE_LOG(LogHarmoniaNetwork, Error, TEXT("ValidateActorForRPC: Null actor"));
		return false;
	}

	if (!Actor->IsValidLowLevel())
	{
		UE_LOG(LogHarmoniaNetwork, Error, TEXT("ValidateActorForRPC: Invalid actor %s"), *Actor->GetName());
		return false;
	}

	if (Actor->IsPendingKillPending())
	{
		UE_LOG(LogHarmoniaNetwork, Error, TEXT("ValidateActorForRPC: Actor pending kill %s"), *Actor->GetName());
		return false;
	}

	if (bRequireAuthority && !Actor->HasAuthority())
	{
		UE_LOG(LogHarmoniaNetwork, Warning, TEXT("ValidateActorForRPC: Actor %s does not have authority"), *Actor->GetName());
		return false;
	}

	return true;
}

void UHarmoniaReplicationUtils::AddReplicatedProperty(
	TArray<FLifetimeProperty>& OutLifetimeProps,
	const UClass* InClass,
	const FProperty* Property,
	ELifetimeCondition Condition)
{
	if (!Property)
	{
		return;
	}

	FDoRepLifetimeParams Params;
	Params.Condition = Condition;

	OutLifetimeProps.Add(FLifetimeProperty(Property->RepIndex, Condition));
}
