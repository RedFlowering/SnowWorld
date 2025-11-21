// Copyright 2025 Snow Game Studio.

#include "Relationship/HarmoniaRelationshipComponent.h"
#include "Relationship/HarmoniaRelationshipData.h"
#include "HarmoniaStoryLog.h"

UHarmoniaRelationshipComponent::UHarmoniaRelationshipComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHarmoniaRelationshipComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaRelationshipComponent::ModifyAffinity(FHarmoniaID TargetId, int32 Amount)
{
	int32 CurrentAffinity = GetAffinity(TargetId);
	SetAffinity(TargetId, CurrentAffinity + Amount);
}

void UHarmoniaRelationshipComponent::SetAffinity(FHarmoniaID TargetId, int32 Amount)
{
	int32 OldAffinity = GetAffinity(TargetId);
	
	// Clamp if needed, but for now just set
	AffinityMap.Add(TargetId, Amount);

	if (OldAffinity != Amount)
	{
		OnAffinityChanged.Broadcast(TargetId, Amount, Amount - OldAffinity);
		UE_LOG(LogHarmoniaStory, Log, TEXT("Affinity with %s changed to %d (Delta: %d)"), *TargetId.ToString(), Amount, Amount - OldAffinity);
	}
}

int32 UHarmoniaRelationshipComponent::GetAffinity(FHarmoniaID TargetId) const
{
	if (AffinityMap.Contains(TargetId))
	{
		return AffinityMap[TargetId];
	}
	return 0; // Default neutral
}

FText UHarmoniaRelationshipComponent::GetAffinityTierName(FHarmoniaID TargetId) const
{
	if (RelationshipData)
	{
		int32 Affinity = GetAffinity(TargetId);
		return RelationshipData->GetTierForAffinity(Affinity).TierName;
	}
	return FText::FromString("Unknown");
}
