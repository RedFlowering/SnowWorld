// Copyright 2017 Lee Ju Sik

#include "TrimAnimation.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "TrimAnimation"

FReply UTrimAnimation::TrimAnimation()
{
	if (IsValid(AnimSequence) == false)
	{
		Notification(LOCTEXT("No AnimSequence", "No AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
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

	EndFrame = FMath::Min(EndFrame, NumKeys - 1);
	if (EndFrame <= 0)
	{
		Notification(LOCTEXT("Invalid End Frame", "Invalid End Frame"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	StartFrame = FMath::Min(StartFrame, EndFrame - 1);
	if (StartFrame < 0)
	{
		Notification(LOCTEXT("Invalid Start Frame", "Invalid Start Frame"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	UAnimSequence* AnimOrig = NewObject<UAnimSequence>();
	AnimOrig->SetSkeleton(AnimSequence->GetSkeleton());
	AnimOrig->CreateAnimation(AnimSequence);

	{
		FScopedTransaction Transation(LOCTEXT("TrimAnimation", "TrimAnimation"));
		AnimSequence->Modify();
		AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("TrimAnimation")));

		double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

		int TotalFrames = (EndFrame + 1) - StartFrame;

		AnimSequence->GetController().SetNumberOfFrames(TotalFrames - 1);

		{
			int32 RootBoneIndex = 0;
			FName RootBoneName = RefSkeleton.GetBoneName(RootBoneIndex);
			
			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(RootBoneName))
			{
				TArray< FVector > AnimTrackPosKeys;
				TArray< FQuat > AnimTrackRotKeys;
				TArray< FVector > AnimTrackScaleKeys;
				AnimTrackPosKeys.Empty(TotalFrames);
				AnimTrackRotKeys.Empty(TotalFrames);
				AnimTrackScaleKeys.Empty(TotalFrames);

				FTransform StartKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, StartFrame, AnimOrig->Interpolation);
				
				for (int32 key = StartFrame; key <= EndFrame; key++)
				{
					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, key, AnimOrig->Interpolation);
					
					AnimatedLocalKey = AnimatedLocalKey * StartKey.Inverse();
					AnimatedLocalKey.NormalizeRotation();

					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());
				}

				AnimSequence->GetController().SetBoneTrackKeys(RootBoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
			}
		}

		for (int32 BoneIndex = 1; BoneIndex < NumBones; BoneIndex++)
		{
			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			
			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				TArray< FVector > AnimTrackPosKeys;
				TArray< FQuat > AnimTrackRotKeys;
				TArray< FVector > AnimTrackScaleKeys;
				AnimTrackPosKeys.Empty(TotalFrames);
				AnimTrackRotKeys.Empty(TotalFrames);
				AnimTrackScaleKeys.Empty(TotalFrames);

				for (int32 key = StartFrame; key <= EndFrame; key++)
				{
					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimOrig->Interpolation);

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