// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticChildActorComponent.h"
#include "GameFramework/Actor.h"

UCosmeticChildActorComponent::UCosmeticChildActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)	
{
	
}

void UCosmeticChildActorComponent::CreateChildActor(TFunction<void(AActor*)> CustomizerFunc /*= nullptr*/)
{
	Super::CreateChildActor(CustomizerFunc);

	if (GetChildActor())
	{		
		GetChildActor()->SetOwner(GetOwner());
	}	
}
