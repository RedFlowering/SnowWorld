// Copyright 2025 Snow Game Studio.

#include "Utils/HarmoniaTeamAgentHelper.h"
#include "Definitions/HarmoniaTeamSystemDefinitions.h"

EHarmoniaTeamRelationship UHarmoniaTeamAgentHelper::GetRelationshipWith(const UObject* TeamAgent, AActor* OtherActor)
{
	if (!TeamAgent || !OtherActor)
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	// Check if other actor implements interface
	if (OtherActor->Implements<UHarmoniaTeamAgentInterface>())
	{
		FHarmoniaTeamIdentification MyTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(TeamAgent);
		FHarmoniaTeamIdentification OtherTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(OtherActor);

		if (MyTeam.IsSameTeam(OtherTeam))
		{
			return EHarmoniaTeamRelationship::Ally;
		}
	}

	return EHarmoniaTeamRelationship::Neutral;
}

bool UHarmoniaTeamAgentHelper::CanAttackActor(const UObject* TeamAgent, AActor* OtherActor)
{
	if (!TeamAgent || !OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = GetRelationshipWith(TeamAgent, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}

bool UHarmoniaTeamAgentHelper::ShouldHelpActor(const UObject* TeamAgent, AActor* OtherActor)
{
	if (!TeamAgent || !OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = GetRelationshipWith(TeamAgent, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

bool UHarmoniaTeamAgentHelper::IsSameTeamAs(const UObject* TeamAgent, AActor* OtherActor)
{
	if (!TeamAgent || !OtherActor || !OtherActor->Implements<UHarmoniaTeamAgentInterface>())
	{
		return false;
	}

	FHarmoniaTeamIdentification MyTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(TeamAgent);
	FHarmoniaTeamIdentification OtherTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(OtherActor);

	return MyTeam.IsSameTeam(OtherTeam);
}

bool UHarmoniaTeamAgentHelper::IsAllyWith(const UObject* TeamAgent, AActor* OtherActor)
{
	if (!TeamAgent || !OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = GetRelationshipWith(TeamAgent, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

bool UHarmoniaTeamAgentHelper::IsEnemyWith(const UObject* TeamAgent, AActor* OtherActor)
{
	if (!TeamAgent || !OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = GetRelationshipWith(TeamAgent, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}

FHarmoniaTeamIdentification UHarmoniaTeamAgentHelper::GetTeamIDFromActor(AActor* Actor)
{
	if (!Actor || !Actor->Implements<UHarmoniaTeamAgentInterface>())
	{
		return FHarmoniaTeamIdentification();
	}

	return IHarmoniaTeamAgentInterface::Execute_GetTeamID(Actor);
}

bool UHarmoniaTeamAgentHelper::DoesActorImplementTeamInterface(AActor* Actor)
{
	return Actor && Actor->Implements<UHarmoniaTeamAgentInterface>();
}
