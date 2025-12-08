// Copyright 2017 Lee Ju Sik

#include "BlendAnimationLastPose.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "BlendAnimationLastPose"






FReply UBlendAnimationLastPose::BlendAnimationLastPose()
{
	if (IsValid(AnimSequence) == false)
	{
		Notification(LOCTEXT("No AnimSequence", "No AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (IsValid(NextAnimSequence) == false)
	{
		Notification(LOCTEXT("No Next AnimSequence", "No Next AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	USkeleton* Skeleton = AnimSequence->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 NumBones = RefSkeleton.GetNum();

	int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();
	if (NumKeys == 0)
	{
		Notification(LOCTEXT("Animation frame is zero", "Animation frame is zero"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (NextAnimSequence->GetSkeleton() != Skeleton)
	{
		Notification(LOCTEXT("Next Animation skeleton is not same", "Next Animation skeleton is not same"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (NextAnimSequence->GetDataModel()->GetNumberOfKeys() == 0)
	{
		Notification(LOCTEXT("Next Animation frame is zero", "Next Animation frame is zero"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	TArray< FTransform > NextAnimationPose;
	NextAnimationPose.Empty(NumBones);
	NextAnimationPose.AddZeroed(NumBones);

	TArray< bool > NextAnimationPoseHasTrack;
	NextAnimationPoseHasTrack.Empty(NumBones);
	NextAnimationPoseHasTrack.AddZeroed(NumBones);

	for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
		
		if (NextAnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
		{
			FTransform AnimatedLocalKey = NextAnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, 0, NextAnimSequence->Interpolation);

			NextAnimationPose[BoneIndex] = AnimatedLocalKey;
			NextAnimationPoseHasTrack[BoneIndex] = true;
		}
		else
		{
			FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

			NextAnimationPose[BoneIndex] = LocalTransform;
			NextAnimationPoseHasTrack[BoneIndex] = false;
		}
	}

	UAnimSequence* AnimOrig = NewObject<UAnimSequence>();
	AnimOrig->SetSkeleton(AnimSequence->GetSkeleton());
	AnimOrig->CreateAnimation(AnimSequence);

	{
		FScopedTransaction Transation(LOCTEXT("BlendAnimationLastPose", "BlendAnimationLastPose"));
		AnimSequence->Modify();
		AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("BlendAnimationLastPose")));

		BlendStartFrame = FMath::Clamp(BlendStartFrame, 0, NumKeys - 1);
		BlendEndFrame = FMath::Clamp(BlendEndFrame, BlendStartFrame, NumKeys - 1);
		
		int32 NumFrames = BlendEndFrame + 1;

		AnimSequence->GetController().SetNumberOfFrames(NumFrames - 1);

		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			
			if ((AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false) && NextAnimationPoseHasTrack[BoneIndex])
			{
				FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];
				FRawAnimSequenceTrack AnimTrack;
				AnimTrack.PosKeys.Add(FVector3f(LocalTransform.GetLocation()));
				AnimTrack.RotKeys.Add(FQuat4f(LocalTransform.GetRotation()));
				AnimTrack.ScaleKeys.Add(FVector3f(LocalTransform.GetScale3D()));

				AnimSequence->GetController().AddBoneCurve(BoneName);
				AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrack.PosKeys, AnimTrack.RotKeys, AnimTrack.ScaleKeys);

			}

			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				TArray< FVector > AnimTrackPosKeys;
				TArray< FQuat > AnimTrackRotKeys;
				TArray< FVector > AnimTrackScaleKeys;
				AnimTrackPosKeys.Empty(NumKeys);
				AnimTrackRotKeys.Empty(NumKeys);
				AnimTrackScaleKeys.Empty(NumKeys);

				for (int32 key = 0; key < NumKeys; key++)
				{
					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimOrig->Interpolation);
					
					if (BoneIndex != 0 && key >= BlendStartFrame && key <= BlendEndFrame)
					{
						float Alpha = (BlendStartFrame == BlendEndFrame) ? 1.0f : (float)(key - BlendStartFrame) / (float)(BlendEndFrame - BlendStartFrame);

						Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

						AnimatedLocalKey.BlendWith(NextAnimationPose[BoneIndex], Alpha);
					}

					if (key <= BlendEndFrame)
					{
						AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
						AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
						AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());
					}
				}

				AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
			}
		}

		//double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();
		

		AnimSequence->GetController().CloseBracket();

		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}






#undef LOCTEXT_NAMESPACE