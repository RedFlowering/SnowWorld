// Copyright 2017 Lee Ju Sik


#include "BlendAnimationFirstPose.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"


#define LOCTEXT_NAMESPACE "BlendAnimationFirstPose"


FReply UBlendAnimationFirstPose::BlendAnimationFirstPose()
{
	if (IsValid(AnimSequence) == false)
	{
		Notification(LOCTEXT("No AnimSequence", "No AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (IsValid(PrevAnimSequence) == false)
	{
		Notification(LOCTEXT("No Prev AnimSequence", "No Prev AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
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

	if (PrevAnimSequence->GetSkeleton() != Skeleton)
	{
		Notification(LOCTEXT("Prev Animation skeleton is not same", "Prev Animation skeleton is not same"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (PrevAnimSequence->GetDataModel()->GetNumberOfKeys() == 0)
	{
		Notification(LOCTEXT("Prev Animation frame is zero", "Prev Animation frame is zero"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	TArray< FTransform > PrevAnimationPose;
	PrevAnimationPose.Empty(NumBones);
	PrevAnimationPose.AddZeroed(NumBones);

	TArray< bool > PrevAnimationPoseHasTrack;
	PrevAnimationPoseHasTrack.Empty(NumBones);
	PrevAnimationPoseHasTrack.AddZeroed(NumBones);

	int32 PrevLastKey = PrevAnimSequence->GetDataModel()->GetNumberOfKeys() - 1;

	for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
		
		if (PrevAnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
		{
			FTransform AnimatedLocalKey = PrevAnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, PrevLastKey, PrevAnimSequence->Interpolation);
			
			PrevAnimationPose[BoneIndex] = AnimatedLocalKey;
			PrevAnimationPoseHasTrack[BoneIndex] = true;
		}
		else
		{
			FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

			PrevAnimationPose[BoneIndex] = LocalTransform;
			PrevAnimationPoseHasTrack[BoneIndex] = false;
		}
	}

	{
		FScopedTransaction Transation(LOCTEXT("BlendAnimationFirstPose", "BlendAnimationFirstPose"));
		AnimSequence->Modify();
		AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("BlendAnimationFirstPose")));

		BlendFrames = FMath::Clamp(BlendFrames, 0, NumKeys - 1);

		for (int32 BoneIndex = 1; BoneIndex < NumBones; BoneIndex++)
		{
			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			
			if ((AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false) && PrevAnimationPoseHasTrack[BoneIndex])
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
					FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);

					if (BoneIndex != 0 && key <= BlendFrames)
					{
						float Alpha = (BlendFrames == 0) ? 0.0f : (float)(BlendFrames - key) / (float)(BlendFrames);

						Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

						AnimatedLocalKey.BlendWith(PrevAnimationPose[BoneIndex], Alpha);
					}

					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());
				}

				AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
			}
		}

		AnimSequence->GetController().CloseBracket();

		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}




#undef LOCTEXT_NAMESPACE