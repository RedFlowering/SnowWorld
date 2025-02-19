// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "Components/ChildActorComponent.h"
#include "CosmeticChildActorComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (Cosmetic), meta = (BlueprintSpawnableComponent))
class COSMETICSYSTEM_API UCosmeticChildActorComponent : public UChildActorComponent
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void CreateChildActor(TFunction<void(AActor*)> CustomizerFunc = nullptr);
};