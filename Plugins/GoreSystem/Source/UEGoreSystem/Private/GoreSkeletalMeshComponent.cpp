// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.


#include "GoreSkeletalMeshComponent.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Engine/StreamableManager.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkinWeightVertexBuffer.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "Engine/SkeletalMesh.h"



static void GetAdditionalSocketsFromList(const TArray<USkeletalMeshSocket*> SocketsList, TArray<FName>& NewList)
{	
	const TArray<USkeletalMeshSocket*> MeshSocketList = SocketsList;
	NewList.AddUninitialized(MeshSocketList.Num());

	for(int32 Idx = 0; Idx < MeshSocketList.Num(); Idx++)
	{
		NewList[Idx] = MeshSocketList[Idx]->SocketName;
	}
};

UGoreSkeletalMeshComponent::UGoreSkeletalMeshComponent(const FObjectInitializer& ObjInit)
	:Super(ObjInit)
{
	//...
	SetTickGroup(ETickingGroup::TG_EndPhysics);
}

void UGoreSkeletalMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (StretchFixEnabled) {
		if (!InitialVisibilityFired) {
			InitialVisibilityFired = true;
			SetVisibility(true);
			SetAllBodiesBelowSimulatePhysics(NewRootBone, true, true);
			AddImpulse(DetachImpulse, NewRootBone);
			On_DetachPhysicsEnabled.Broadcast(NewRootBone);
		}
		else
		{
			if (IsSimulatingPhysics(NewRootBone) && !GetComponentLocation().Equals(GetBoneLocation(NewRootBone), 0.01f) && GetPhysicsAsset())
				SetWorldLocation(GetBoneLocation(NewRootBone), false, (FHitResult*)nullptr, ETeleportType::ResetPhysics);
		}
	}
}

void UGoreSkeletalMeshComponent::HideBoneByNameGore(const FName BoneName, const EPhysBodyOp PhysBodyOption, const bool bAvoidChildren)
{
	// Find appropriate BoneIndex
	TArray<int32> BoneIndexes;

	//Decide if hide children or just itself
	if (bAvoidChildren)
	{
		//Only itself
		BoneIndexes.Add(GetBoneIndex(BoneName));
	}
	else
	{
		//Remove not-skeletal sockets
		TArray<FName> ActualBoneNames;
		GetBoneNames(ActualBoneNames);
		//With children
		for (auto& CBoneName : ActualBoneNames)
		{
			if (BoneIsChildOf(CBoneName, BoneName) || CBoneName == BoneName)
				BoneIndexes.Add(GetBoneIndex(CBoneName));
		}
	}

	//If list has at least 1 entry, hide
	if (BoneIndexes.Num() > 0)
	{
		HideBonesGore(BoneIndexes, PhysBodyOption, bAvoidChildren);
	}
}

void UGoreSkeletalMeshComponent::HideBoneBeforeNameGore(const FName BoneName, const EPhysBodyOp PhysBodyOption)
{
	// Find appropriate BoneIndex
	int32 BoneIndex = GetBoneIndex(BoneName);
	TArray<int32>BonesToHide;

	//Check index is valid
	if (BoneIndex != INDEX_NONE && BoneIndex > 0)
	{
		//Remove not-skeletal sockets (not bone sockets)
		TArray<FName> ActualBoneNames;
		GetBoneNames(ActualBoneNames);
		//Find not-children
		for (auto& Socket : ActualBoneNames)
		{
			if (!BoneIsChildOf(Socket, BoneName) && Socket != BoneName) {
				BonesToHide.Add(GetBoneIndex(Socket));
			}
		}

		//Hide all bones before, preserve bones below (bAvoidChildren == true)
		if (!BonesToHide.Contains(GetBoneIndex(FName(TEXT("Pelvis"))))) {
			UE_LOG(LogTemp, Error, TEXT("DOES NOT CONTAIN PELVIS, Index Is: %i"), GetBoneIndex(FName(TEXT("Pelvis"))));
		}

		HideBonesGore(BonesToHide, PhysBodyOption, true);
	}	
}

void UGoreSkeletalMeshComponent::HideBonesGore(const TArray<int32> BoneIndexes, const EPhysBodyOp PhysBodyOption, const bool bAvoidChildren)
{
	//Return if no indexes
	if (BoneIndexes.Num() == 0)
	{
		return;
	}

	TArray<uint8>& EditableBoneVisibilityStates = GetEditableBoneVisibilityStates();
	TArray<int32> NewBoneIndexes;
	//Fix invalid entries
	for (auto& CBone : BoneIndexes)
	{
		if (CBone < EditableBoneVisibilityStates.Num() && CBone >= 0)
		{
			NewBoneIndexes.Add(CBone);
		}
	}

	//Set all target bones as "BVS_ExplicitlyHidden"
	if (!bDisableMultiThread)
	{
		ParallelFor(BoneIndexes.Num(), [&](int32 Idx)
		{
			//checkSlow(CBone != INDEX_NONE && );
			EditableBoneVisibilityStates[NewBoneIndexes[Idx]] = BVS_ExplicitlyHidden;
		});
	}
	else
	{
		for (int32 Idx = 0; Idx < BoneIndexes.Num(); Idx++)
		{
			//checkSlow(CBone != INDEX_NONE && );
			EditableBoneVisibilityStates[NewBoneIndexes[Idx]] = BVS_ExplicitlyHidden;
		}
	}

	//NON MULTI-THREAD
	/*for (int32 i = 0; i < BoneIndexes.Num(); i++)
	{
		checkSlow(CBone != INDEX_NONE && );
		EditableBoneVisibilityStates[NewBoneIndexes[i]] = BVS_ExplicitlyHidden;
		
	}*/

	//Mark as dirty
	MarkRenderStateDirty();
	MarkRenderDynamicDataDirty();
	MarkRenderTransformDirty();

	//Rebuild visibility with forced method
	NewRebuildVisibilityArray(bAvoidChildren);
}

void UGoreSkeletalMeshComponent::UnhideBonesGore(const TArray<int32> BoneIndexes)
{
	//Return if no indexes
	if (BoneIndexes.Num() == 0)
	{
		return;
	}

	TArray<uint8>& EditableBoneVisibilityStates = GetEditableBoneVisibilityStates();
	TArray<int32> NewBoneIndexes;

	//Fix invalid entries
	for (auto& CBone : BoneIndexes)
	{
		if (CBone < EditableBoneVisibilityStates.Num())
		{
			NewBoneIndexes.Add(CBone);
		}
	}

	//Set all target bones as "BVS_ExplicitlyHidden"
	if (!bDisableMultiThread)
	{
		ParallelFor(BoneIndexes.Num(), [&](int32 Idx)
		{
			//checkSlow(CBone != INDEX_NONE && );
			EditableBoneVisibilityStates[NewBoneIndexes[Idx]] = BVS_Visible;
		});
	}
	else
	{
		for (int32 Idx = 0; Idx < BoneIndexes.Num(); Idx++)
		{
			//checkSlow(CBone != INDEX_NONE && );
			EditableBoneVisibilityStates[NewBoneIndexes[Idx]] = BVS_Visible;
		}
	}

	//Mark as dirty
	MarkRenderStateDirty();
	MarkRenderDynamicDataDirty();
	MarkRenderTransformDirty();

	//Rebuild visibility with forced method
	NewRebuildVisibilityArray(true);
}

void UGoreSkeletalMeshComponent::NewRebuildVisibilityArray(const bool bShouldAvoidChildren)
{
	// BoneVisibility needs update if MasterComponent == NULL
	// if MaterComponent, it should follow MaterPoseComponent
	if (ShouldUpdateBoneVisibility())
	{
		// If the BoneVisibilityStates array has a 0 for a parent bone, all children bones are meant to be hidden as well
		// (as the concatenated matrix will have scale 0).  This code propagates explicitly hidden parents to children.

		// On the first read of any cell of BoneVisibilityStates, BVS_HiddenByParent and BVS_Visible are treated as visible.
		// If it starts out visible, the value written back will be BVS_Visible if the parent is visible; otherwise BVS_HiddenByParent.
		// If it starts out hidden, the BVS_ExplicitlyHidden value stays in place

		// The following code relies on a complete hierarchy sorted from parent to children
		TArray<uint8>& EditableBoneVisibilityStates = GetEditableBoneVisibilityStates();
		if (EditableBoneVisibilityStates.Num() != GetSkeletalMeshAsset()->GetRefSkeleton().GetNum())
		{
			UE_LOG(LogTemp, Warning, TEXT("RebuildVisibilityArray() failed because EditableBoneVisibilityStates size: %d not equal to RefSkeleton bone count: %d."), EditableBoneVisibilityStates.Num(), GetSkeletalMeshAsset()->GetRefSkeleton().GetNum());
			return;
		}	

		bBoneVisibilityDirty = true;
	}
	
}
