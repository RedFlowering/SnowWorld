// Copyright 2017 Lee Ju Sik

#include "ResetRootBoneTransform.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"


#define LOCTEXT_NAMESPACE "ResetRootBoneTransform"


FReply UResetRootBoneTransform::ResetRootBoneTransform()
{
	if (SkeletalMesh)
	{
		USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
		FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
		int32 NumBones = RefSkeleton.GetNum();
		if (NumBones < 2)
		{
			Notification(LOCTEXT("Num Bone is lower then 2", "Num Bone is lower then 2"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}




		{
			FScopedTransaction Transation(LOCTEXT("ResetRootBoneTransform", "ResetRootBoneTransform"));
			Skeleton->Modify();
			SkeletalMesh->Modify();

			{
				FReferenceSkeletonModifier RefSkelModifier(RefSkeleton, Skeleton);

				TArray< int32 > ChildBoneIndexArray;
				for (int32 BoneIndex = 1; BoneIndex < NumBones; BoneIndex++)
				{
					if (RefSkeleton.GetParentIndex(BoneIndex) == 0)
					{
						ChildBoneIndexArray.Add(BoneIndex);
					}
				}

				FTransform RootBoneTransform = RefSkeleton.GetRefBonePose()[0];

				for (int32 BoneIndex : ChildBoneIndexArray)
				{
					FTransform ChildBoneTransform = RefSkeleton.GetRefBonePose()[BoneIndex] * RootBoneTransform;
					ChildBoneTransform.NormalizeRotation();

					RefSkelModifier.UpdateRefPoseTransform(BoneIndex, ChildBoneTransform);
				}

				RefSkelModifier.UpdateRefPoseTransform(0, FTransform::Identity);
			}
			

			Skeleton->UpdateReferencePoseFromMesh(SkeletalMesh);
		}

		
		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}



#undef LOCTEXT_NAMESPACE