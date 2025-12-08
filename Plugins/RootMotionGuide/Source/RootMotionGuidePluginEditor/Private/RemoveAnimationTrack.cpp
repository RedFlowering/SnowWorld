// Copyright 2017 Lee Ju Sik

#include "RemoveAnimationTrack.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "RemoveAnimationTrack"



FReply URemoveAnimationTrack::RemoveAnimationTrack()
{
	if (AnimSequence)
	{
		USkeleton* Skeleton = AnimSequence->GetSkeleton();
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();

		int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();
		if (NumKeys == 0)
		{
			Notification(LOCTEXT("Animation frame is zero", "Animation frame is zero"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		int32 RemoveBoneIndex = RefSkeleton.FindBoneIndex(RemoveBoneName);
		if (RemoveBoneIndex == INDEX_NONE)
		{
			Notification(LOCTEXT("Can't find bone", "Can't find bone"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}


		{
			FScopedTransaction Transation(LOCTEXT("RemoveAnimationTrack", "RemoveAnimationTrack"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("RemoveAnimationTrack")));

			for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
			{
				if (BoneIndex == RemoveBoneIndex || (bRemoveAnimationTrackOfAllChildBones && RefSkeleton.BoneIsChildOf(BoneIndex, RemoveBoneIndex)))
				{
					if (DefaultValueType == ERemoveAnimationTrackDefaultValueType::AnimFirstFrame)
					{
						FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
						
						if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
						{
							FTransform FirstAnimKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, 0, AnimSequence->Interpolation);
							
							TArray< FVector > AnimTrackPosKeys;
							TArray< FQuat > AnimTrackRotKeys;
							TArray< FVector > AnimTrackScaleKeys;

							AnimTrackPosKeys.Add(FirstAnimKey.GetLocation());
							AnimTrackRotKeys.Add(FirstAnimKey.GetRotation());
							AnimTrackScaleKeys.Add(FirstAnimKey.GetScale3D());

							AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
						}
					}
					else if (DefaultValueType == ERemoveAnimationTrackDefaultValueType::RefPose)
					{
						FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
						
						if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
						{
							FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

							TArray< FVector > AnimTrackPosKeys;
							TArray< FQuat > AnimTrackRotKeys;
							TArray< FVector > AnimTrackScaleKeys;

							AnimTrackPosKeys.Add(LocalTransform.GetLocation());
							AnimTrackRotKeys.Add(LocalTransform.GetRotation());
							AnimTrackScaleKeys.Add(LocalTransform.GetScale3D());

							AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
						}
					}
				}
			}


			AnimSequence->GetController().CloseBracket();

			Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
		}
	}


	return FReply::Handled();
}



#undef LOCTEXT_NAMESPACE