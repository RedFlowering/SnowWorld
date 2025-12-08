// Copyright 2017 Lee Ju Sik

#include "RootMotionAdjustScale.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "RootMotionAdjustScale"

URootMotionAdjustScale::URootMotionAdjustScale()
{
	AdjustScale = FVector::OneVector;
}

FReply URootMotionAdjustScale::AdjustRootMotionScale()
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

		int32 RootBoneIndex = 0;
		FName BoneName = RefSkeleton.GetBoneName(RootBoneIndex);

		if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false)
		{
			Notification(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}


		// Adjust
		{
			FScopedTransaction Transation(LOCTEXT("AdjustRootMotionScale", "AdjustRootMotionScale"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AdjustRootMotionScale")));

			AdjustScaleStartFrame = FMath::Max(AdjustScaleStartFrame, 0);
			AdjustScaleEndFrame = FMath::Max(AdjustScaleStartFrame, AdjustScaleEndFrame);


			TArray<FVector> AnimTrackPosKeys;
			TArray<FQuat> AnimTrackRotKeys;
			TArray<FVector> AnimTrackScaleKeys;
			AnimTrackPosKeys.Empty(NumKeys);
			AnimTrackRotKeys.Empty(NumKeys);
			AnimTrackScaleKeys.Empty(NumKeys);

			FVector PrevLocationOrig = FVector::ZeroVector;
			FVector PrevLocation = FVector::ZeroVector;


			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);

				FVector MoveDelta = AnimatedLocalKey.GetLocation() - PrevLocationOrig;
				PrevLocationOrig = AnimatedLocalKey.GetLocation();


				if (key >= AdjustScaleStartFrame  && key <= AdjustScaleEndFrame)
				{
					MoveDelta *= AdjustScale;
				}

				AnimatedLocalKey.SetLocation(PrevLocation + MoveDelta);

				AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
				AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
				AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

				PrevLocation = AnimatedLocalKey.GetLocation();
			}


			AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE