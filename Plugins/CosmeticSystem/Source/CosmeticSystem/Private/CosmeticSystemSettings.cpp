// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticSystemSettings.h"
#include "UObject/UnrealType.h"

void UCosmeticSystemSettings::PostInitProperties()
{
	Super::PostInitProperties();
/*
#if WITH_EDITOR
	if (IsTemplate())
	{
		ImportConsoleVariableValues();
	}
#endif
*/
}

FName UCosmeticSystemSettings::GetCategoryName() const
{
	return FName(TEXT("Plugins"));
}

#if WITH_EDITOR
void UCosmeticSystemSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	/*
	if (PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}
	*/
}
#endif