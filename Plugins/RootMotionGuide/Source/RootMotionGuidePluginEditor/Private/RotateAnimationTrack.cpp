// Copyright 2017 Lee Ju Sik

#include "RotateAnimationTrack.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "RotateAnimationTrack"

FReply URotateAnimationTrack::RotateAnimationTrack()
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

		int32 AdjustBoneIndex = RefSkeleton.FindBoneIndex(BoneName);
		if (AdjustBoneIndex == INDEX_NONE)
		{
			Notification(LOCTEXT("Can't find adjust bone", "Can't find adjust bone"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false)
		{
			Notification(LOCTEXT("Can't find animation track", "Can't find animation track"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}


		// Adjust
		{
			FScopedTransaction Transation(LOCTEXT("AdjustAnimationTrack", "AdjustAnimationTrack"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AdjustAnimationTrack")));


			TArray<FVector> AnimTrackPosKeys;
			TArray<FQuat> AnimTrackRotKeys;
			TArray<FVector> AnimTrackScaleKeys;
			AnimTrackPosKeys.Empty(NumKeys);
			AnimTrackRotKeys.Empty(NumKeys);
			AnimTrackScaleKeys.Empty(NumKeys);

			FTransform AdjustRotationTM = FTransform::Identity;
			AdjustRotationTM.SetRotation(AdjustRotation.Quaternion());

			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);

				if (AdjustRotation.IsNearlyZero() == false)
				{
					AnimatedLocalKey = AnimatedLocalKey * AdjustRotationTM;
					AnimatedLocalKey.NormalizeRotation();
				}

				AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
				AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
				AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());
			}


			AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE