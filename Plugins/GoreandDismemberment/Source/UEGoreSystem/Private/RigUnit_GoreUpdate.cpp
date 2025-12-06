// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.


#include "RigUnit_GoreUpdate.h"
#include "Units/RigUnitContext.h"
#include "UEGoreSystemComponent.h"
#include "GoreSkeletalMeshComponent.h"


//#include UE_INLINE_GENERATED_CPP_BY_NAME(RigUnit_GoreUpdate)

FRigUnit_GoreUpdate_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()

	TArray<FCachedRigElement>& CachedItems = WorkData.CachedItems;

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;

	if (Hierarchy)
	{
		if (!ExecuteContext.GetOwningActor()) {
			return;
		}

		if (ExecuteContext.GetOwningComponent()->GetClass() == UGoreSkeletalMeshComponent::StaticClass()) {
			return;
		}

		AActor* ParentActor = ExecuteContext.GetOwningActor()->GetAttachParentActor();

		UActorComponent* ActorComps = ParentActor ? ParentActor->GetComponentByClass(UUEGoreSystemComponent::StaticClass()) : ExecuteContext.GetOwningActor()->GetComponentByClass(UUEGoreSystemComponent::StaticClass());
		UUEGoreSystemComponent* GoreComp = Cast<UUEGoreSystemComponent>(ActorComps);

		if (!GoreComp) {
			return;
		}

		if (CachedItems.Num() != GoreComp->DestroyedBones.Num())
		{
			CachedItems.SetNum(GoreComp->DestroyedBones.Num(), EAllowShrinking::Yes);
		}

		if (CachedItems.Num() == GoreComp->DestroyedBones.Num())
		{
			int32 EntryIndex = 0;
			for (const FName& Entry : GoreComp->DestroyedBones)
			{
				FCachedRigElement& CachedItem = CachedItems[EntryIndex];
				if (!CachedItem.UpdateCache(FRigElementKey(Entry, ERigElementType::Bone), Hierarchy))
				{
					continue;
				}

				FVector NLoc = Hierarchy->GetLocalTransform(CachedItem.GetKey(), ERigTransformType::CurrentLocal).GetLocation();

				FTransform Transform = FTransform(FQuat::Identity, NLoc, FVector(0.0f));
				Hierarchy->SetLocalTransform(CachedItem, Transform, true);

				EntryIndex++;
			}
		}
	}
}
