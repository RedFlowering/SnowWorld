// Copyright 2025 Snow Game Studio.

#include "Definitions/HarmoniaTeamSystemDefinitions.h"

// ============================================================================
// UHarmoniaTeamConfigData
// ============================================================================

EHarmoniaTeamRelationship UHarmoniaTeamConfigData::GetRelationshipWith(const FHarmoniaTeamIdentification& OtherTeam) const
{
	// Check if same team
	if (TeamID.IsSameTeam(OtherTeam))
	{
		return bAllowFriendlyFire ? EHarmoniaTeamRelationship::Neutral : EHarmoniaTeamRelationship::Ally;
	}

	// Check custom relationships
	for (const FHarmoniaTeamRelationshipData& RelData : DefaultRelationships)
	{
		if (RelData.TargetTeam.IsSameTeam(OtherTeam))
		{
			return RelData.Relationship;
		}
	}

	// Fall back to default attitude
	switch (TeamID.DefaultAttitude)
	{
	case EHarmoniaTeamAttitude::Friendly:
		return EHarmoniaTeamRelationship::Ally;
	case EHarmoniaTeamAttitude::Hostile:
		return EHarmoniaTeamRelationship::Enemy;
	case EHarmoniaTeamAttitude::Neutral:
	case EHarmoniaTeamAttitude::Defensive:
	default:
		return EHarmoniaTeamRelationship::Neutral;
	}
}

bool UHarmoniaTeamConfigData::CanAttack(const FHarmoniaTeamIdentification& OtherTeam) const
{
	// Same team check
	if (TeamID.IsSameTeam(OtherTeam))
	{
		return bAllowFriendlyFire;
	}

	EHarmoniaTeamRelationship Relationship = GetRelationshipWith(OtherTeam);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}

bool UHarmoniaTeamConfigData::ShouldHelp(const FHarmoniaTeamIdentification& OtherTeam) const
{
	// Same team always helps (unless friendly fire is on)
	if (TeamID.IsSameTeam(OtherTeam))
	{
		return !bAllowFriendlyFire;
	}

	EHarmoniaTeamRelationship Relationship = GetRelationshipWith(OtherTeam);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

// ============================================================================
// IHarmoniaTeamAgentInterface Default Implementations
// ============================================================================

FHarmoniaTeamIdentification IHarmoniaTeamAgentInterface::GetTeamID_Implementation() const
{
	return FHarmoniaTeamIdentification();
}

void IHarmoniaTeamAgentInterface::SetTeamID_Implementation(const FHarmoniaTeamIdentification& NewTeamID)
{
	// Default implementation does nothing
}

EHarmoniaTeamRelationship IHarmoniaTeamAgentInterface::GetRelationshipWith_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	// Check if other actor implements interface
	if (OtherActor->Implements<UHarmoniaTeamAgentInterface>())
	{
		FHarmoniaTeamIdentification MyTeam = Execute_GetTeamID(Cast<UObject>(this));
		FHarmoniaTeamIdentification OtherTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(OtherActor);

		if (MyTeam.IsSameTeam(OtherTeam))
		{
			return EHarmoniaTeamRelationship::Ally;
		}
	}

	return EHarmoniaTeamRelationship::Neutral;
}

bool IHarmoniaTeamAgentInterface::CanAttackActor_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(Cast<UObject>(this), OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}

bool IHarmoniaTeamAgentInterface::ShouldHelpActor_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(Cast<UObject>(this), OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

bool IHarmoniaTeamAgentInterface::IsSameTeamAs_Implementation(AActor* OtherActor) const
{
	if (!OtherActor || !OtherActor->Implements<UHarmoniaTeamAgentInterface>())
	{
		return false;
	}

	FHarmoniaTeamIdentification MyTeam = Execute_GetTeamID(Cast<UObject>(this));
	FHarmoniaTeamIdentification OtherTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(OtherActor);

	return MyTeam.IsSameTeam(OtherTeam);
}

bool IHarmoniaTeamAgentInterface::IsAllyWith_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(Cast<UObject>(this), OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

bool IHarmoniaTeamAgentInterface::IsEnemyWith_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(Cast<UObject>(this), OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}
