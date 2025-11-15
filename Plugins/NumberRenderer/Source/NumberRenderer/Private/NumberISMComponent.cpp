// Copyright 2022 HGsofts, Ltd. All Rights Reserved.


#include "NumberISMComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "StaticMeshResources.h"
#include "AI/NavigationSystemBase.h"
#include "AI/NavigationSystemHelpers.h"
#include "AI/Navigation/NavCollisionBase.h"
#include "StaticMeshResources.h"
#include "InstancedStaticMesh/ISMInstanceDataManager.h"


UNumberISMComponent::UNumberISMComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(false);

	bNeverDistanceCull = true;
	bNavigationRelevant = false;
	bPhysicsStateCreated = false;
	bNetAddressable = false;
}

void UNumberISMComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNumberISMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UNumberISMComponent::ResetInstancesTransforms(const TArray<FNumberInstance>& Instances, int32 MaxCount)
{
	int32 NewCount = Instances.Num();
	int32 CurrentCount = GetInstanceCount();
	MaxDrawCount = MaxCount;

	if (CurrentCount > NewCount)
	{
		if (NewCount == 0)
		{
			ClearInstances();
			return;
		}

		int32 Diff = CurrentCount - NewCount;

		for (int32 i = 0; i < Diff; ++i)
		{
			int32 InstanceIndex = CurrentCount - 1 - i;
			RemoveInstance(InstanceIndex);
		}

		UpdateInstancesMesh(Instances);
	}
	else if (CurrentCount < NewCount)
	{
		int32 Diff = NewCount - CurrentCount;
		TArray<FTransform> TempTransforms;
		TempTransforms.Reserve(Diff);
		for (int32 i = 0; i < Diff; ++i)
		{
			TempTransforms.Add(FTransform::Identity);
		}
		AddInstancesInternal(TempTransforms, false, true);
		UpdateInstancesMesh(Instances);
	}
	else // Same Size
	{
		// Only View
		UpdateInstancesMesh(Instances);
	}
}

void UNumberISMComponent::ClearInstances()
{
	Super::ClearInstances();
}

bool UNumberISMComponent::UpdateInstancesMesh(const TArray<FNumberInstance>& Instances)
{
	if (!PerInstanceSMData.IsValidIndex(0) || !PerInstanceSMData.IsValidIndex(0 + Instances.Num() - 1))
	{
		return false;
	}

	Modify();

	int32 InstanceIndex = 0;
	TArray<float> CustomData;
	CustomData.SetNumZeroed(6);

	for (const FNumberInstance& Instance : Instances)
	{
		FInstancedStaticMeshInstanceData& InstanceData = PerInstanceSMData[InstanceIndex];
		FTransform LocalTransform = Instance.CurrentTranform;
		InstanceData.Transform = LocalTransform.ToMatrixWithScale();

		UpdateInstanceTransform(InstanceIndex, Instance.CurrentTranform, false, false, true);

		CustomData[0] = Instance.Color.R;
		CustomData[1] = Instance.Color.G;
		CustomData[2] = Instance.Color.B;
		CustomData[3] = Instance.Alpha;
		CustomData[4] = Instance.Number;
		CustomData[5] = Instance.NumberCount;

		SetCustomData(InstanceIndex, CustomData, false);

		++InstanceIndex;
	}

	MarkRenderStateDirty();

	return true;
}

