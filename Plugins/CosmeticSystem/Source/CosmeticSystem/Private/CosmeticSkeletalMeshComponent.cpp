// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticSkeletalMeshComponent.h"

UCosmeticSkeletalMeshComponent::UCosmeticSkeletalMeshComponent()	
{	
	SetCanEverAffectNavigation(false);
	SetGenerateOverlapEvents(false);

	//ClothTickFunction.bCanEverTick = true;
	//bWaitForParallelClothTask = false;
}
