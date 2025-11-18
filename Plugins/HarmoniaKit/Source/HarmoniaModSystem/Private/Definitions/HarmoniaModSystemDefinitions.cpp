// Copyright 2025 Snow Game Studio.

#include "Definitions/HarmoniaModSystemDefinitions.h"

bool FHarmoniaModInfo::IsVersionCompatible(const FString& RequiredVersion) const
{
	// Parse version strings (format: Major.Minor.Patch)
	TArray<FString> CurrentParts;
	Version.ParseIntoArray(CurrentParts, TEXT("."), true);

	TArray<FString> RequiredParts;
	RequiredVersion.ParseIntoArray(RequiredParts, TEXT("."), true);

	// Pad with zeros if needed
	while (CurrentParts.Num() < 3)
	{
		CurrentParts.Add(TEXT("0"));
	}

	while (RequiredParts.Num() < 3)
	{
		RequiredParts.Add(TEXT("0"));
	}

	// Convert to integers
	int32 CurrentMajor = FCString::Atoi(*CurrentParts[0]);
	int32 CurrentMinor = FCString::Atoi(*CurrentParts[1]);
	int32 CurrentPatch = FCString::Atoi(*CurrentParts[2]);

	int32 RequiredMajor = FCString::Atoi(*RequiredParts[0]);
	int32 RequiredMinor = FCString::Atoi(*RequiredParts[1]);
	int32 RequiredPatch = FCString::Atoi(*RequiredParts[2]);

	// Compare versions
	if (CurrentMajor > RequiredMajor)
	{
		return true;
	}
	else if (CurrentMajor == RequiredMajor)
	{
		if (CurrentMinor > RequiredMinor)
		{
			return true;
		}
		else if (CurrentMinor == RequiredMinor)
		{
			return CurrentPatch >= RequiredPatch;
		}
	}

	return false;
}
