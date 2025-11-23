// Copyright Epic Games, Inc. All Rights Reserved.

#include "Utils/HarmoniaComponentUtils.h"
#include "HarmoniaLogCategories.h"

UActorComponent* UHarmoniaComponentUtils::FindComponentByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass)
{
	if (!Actor || !ComponentClass)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass(ComponentClass);
}

UActorComponent* UHarmoniaComponentUtils::FindComponentByClassWithValidation(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass, bool& bFound)
{
	UActorComponent* Component = FindComponentByClass(Actor, ComponentClass);
	bFound = (Component != nullptr);

	if (!Component)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("FindComponentByClassWithValidation: Component %s not found on actor %s"),
			ComponentClass ? *ComponentClass->GetName() : TEXT("NULL"),
			Actor ? *Actor->GetName() : TEXT("NULL"));
	}

	return Component;
}
