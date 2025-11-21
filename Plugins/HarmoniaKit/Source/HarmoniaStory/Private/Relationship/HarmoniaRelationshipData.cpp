// Copyright 2025 Snow Game Studio.

#include "Relationship/HarmoniaRelationshipData.h"

UHarmoniaRelationshipData::UHarmoniaRelationshipData()
{
}

FRelationshipTier UHarmoniaRelationshipData::GetTierForAffinity(int32 Affinity) const
{
	for (const FRelationshipTier& Tier : Tiers)
	{
		if (Affinity >= Tier.MinAffinity && Affinity <= Tier.MaxAffinity)
		{
			return Tier;
		}
	}

	// Fallback if not found (return neutral or first)
	if (Tiers.Num() > 0)
	{
		return Tiers[0];
	}

	return FRelationshipTier();
}
