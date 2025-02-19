// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "CosmeticSkeletalMeshComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = (Cosmetic), meta = (BlueprintSpawnableComponent))
class COSMETICSYSTEM_API UCosmeticSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UCosmeticSkeletalMeshComponent();
};
