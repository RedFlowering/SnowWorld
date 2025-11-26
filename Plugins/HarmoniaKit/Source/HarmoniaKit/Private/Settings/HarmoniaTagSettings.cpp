// Copyright 2025 Snow Game Studio.

#include "Settings/HarmoniaTagSettings.h"

UHarmoniaTagSettings::UHarmoniaTagSettings()
{
	CustomTagsIniPath = TEXT("Plugins/HarmoniaKit/Config/HarmoniaGameplayTags.ini");
	bAutoRegisterIniTags = true;
	bAutoRegisterDataTableTags = true;
	bValidateAttributeValues = true;
	bLogRegisteredTags = false;
	bLogRegisteredAttributes = false;
}

UHarmoniaTagSettings* UHarmoniaTagSettings::Get()
{
	return GetMutableDefault<UHarmoniaTagSettings>();
}
