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
// NOTE: UE 5.7 doesn't support default implementations for BlueprintNativeEvent
// in interfaces. Each implementing class must provide its own _Implementation methods.
