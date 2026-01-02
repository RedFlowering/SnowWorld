// Copyright 2025 Snow Game Studio.

#include "Data/HarmoniaTeamSetupData.h"

bool UHarmoniaTeamSetupData::FindTeamByTag(FGameplayTag TeamTag, FHarmoniaTeamSetupEntry& OutEntry) const
{
	for (const FHarmoniaTeamSetupEntry& Entry : Teams)
	{
		if (Entry.TeamID.TeamID == TeamTag)
		{
			OutEntry = Entry;
			return true;
		}
	}
	return false;
}

TArray<FGameplayTag> UHarmoniaTeamSetupData::GetAllTeamTags() const
{
	TArray<FGameplayTag> Tags;
	for (const FHarmoniaTeamSetupEntry& Entry : Teams)
	{
		if (Entry.TeamID.TeamID.IsValid())
		{
			Tags.Add(Entry.TeamID.TeamID);
		}
	}
	return Tags;
}
