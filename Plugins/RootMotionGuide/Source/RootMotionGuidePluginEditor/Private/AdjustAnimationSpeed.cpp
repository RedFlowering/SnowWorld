// Copyright 2017 Lee Ju Sik

#include "AdjustAnimationSpeed.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"
#include "AnimationRuntime.h"

#define LOCTEXT_NAMESPACE "AdjustAnimationSpeed"


FReply UAdjustAnimationSpeed::AdjustAnimationSpeed()
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

	if (Speed <= 0.0f)
	{
		Notification(LOCTEXT("Speed must be greater than zero.", "Speed must be greater than zero."), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

	StartFrame = FMath::Clamp(StartFrame, 0, NumKeys - 1);
	EndFrame = FMath::Clamp(EndFrame, 0, NumKeys - 1);
	EndFrame = FMath::Max(StartFrame, EndFrame);

	float TotalAdjustTime = (EndFrame - StartFrame) * IntervalTime;

	if (TotalAdjustTime <= 0.0f)
	{
		Notification(LOCTEXT("Invalid Start and End Frame.", "Invalid Start and End Frame."), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	int AdjustFrames = -1;

	while (AdjustFrames < 0)
	{
		float BlendInTime = 0.0f;
		for (int32 key = 0; key < BlendInFrames; key++)
		{
			float Alpha = (float)key / (float)BlendInFrames;
			float BlendSpeed = FMath::Lerp(1.0f, Speed, Alpha);
			BlendInTime += IntervalTime * BlendSpeed;
		}

		float BlendOutTime = 0.0f;
		for (int32 key = 0; key < BlendOutFrames; key++)
		{
			float Alpha = (float)key / (float)BlendOutFrames;
			float BlendSpeed = FMath::Lerp(Speed, 1.0f, Alpha);
			BlendOutTime += IntervalTime * BlendSpeed;
		}

		if (TotalAdjustTime < (BlendInTime + BlendOutTime))
		{
			BlendInFrames = FMath::Max(BlendInFrames - 1, 0);
			BlendOutFrames = FMath::Max(BlendOutFrames - 1, 0);
		}
		else
		{
			float AdjustTime = TotalAdjustTime - (BlendInTime + BlendOutTime);
			float AdjustIntervalTime = IntervalTime * Speed;
			AdjustFrames = FMath::FloorToInt(AdjustTime / AdjustIntervalTime);
		}
	}

	int TotalFrames = StartFrame + BlendInFrames + AdjustFrames + BlendOutFrames + (NumKeys - EndFrame);

	UAnimSequence* AnimOrig = NewObject<UAnimSequence>();
	AnimOrig->SetSkeleton(AnimSequence->GetSkeleton());
	AnimOrig->CreateAnimation(AnimSequence);

	{

		

		FScopedTransaction Transation(LOCTEXT("AdjustAnimationSpeed", "AdjustAnimationSpeed"));
		AnimSequence->Modify();
		AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AdjustAnimationSpeed")));


		/*
		* 키 사이의 공간이 프레임 그래서 NumberOfFrames = NumberOfKeys - 1
		* TotalFrames 는 키의 갯수 이기 때문에 TotalFrames - 1로 NumberOfFrames 를 설정
		*/
		//AnimSequence->GetController().SetPlayLength((TotalFrames > 1) ? (TotalFrames - 1) * IntervalTime : MINIMUM_ANIMATION_LENGTH);
		AnimSequence->GetController().SetNumberOfFrames(FFrameNumber(TotalFrames - 1));

		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);

			if (AnimOrig->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				TArray< FVector > AnimTrackPosKeys;
				TArray< FQuat > AnimTrackRotKeys;
				TArray< FVector > AnimTrackScaleKeys;
				AnimTrackPosKeys.Empty(TotalFrames);
				AnimTrackRotKeys.Empty(TotalFrames);
				AnimTrackScaleKeys.Empty(TotalFrames);

				for (int32 Key = 0; Key < StartFrame; Key++)
				{
					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, Key, AnimOrig->Interpolation);


					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());
				}

				double AnimationTime = IntervalTime * StartFrame;

				for (int32 Key = 0; Key < BlendInFrames; Key++)
				{
					int32 KeyIndex1, KeyIndex2;
					float SubFrame;
					FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, AnimationTime, AnimOrig->GetDataModel()->GetFrameRate(), AnimOrig->GetDataModel()->GetNumberOfKeys());
					
					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(KeyIndex1, SubFrame), AnimOrig->Interpolation);

					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

					float Alpha = (float)Key / (float)BlendInFrames;
					float BlendSpeed = FMath::Lerp(1.0f, Speed, Alpha);
					AnimationTime += IntervalTime * BlendSpeed;
				}

				double AdjustIntervalTime = IntervalTime * Speed;

				for (int32 Key = 0; Key < AdjustFrames; Key++)
				{
					int32 KeyIndex1, KeyIndex2;
					float SubFrame;
					FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, AnimationTime, AnimOrig->GetDataModel()->GetFrameRate(), AnimOrig->GetDataModel()->GetNumberOfKeys());

					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(KeyIndex1, SubFrame), AnimOrig->Interpolation);
					
					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

					AnimationTime += AdjustIntervalTime;
				}

				for (int32 Key = 0; Key < BlendOutFrames; Key++)
				{
					int32 KeyIndex1, KeyIndex2;
					float SubFrame;
					FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, AnimationTime, AnimOrig->GetDataModel()->GetFrameRate(), AnimOrig->GetDataModel()->GetNumberOfKeys());

					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(KeyIndex1, SubFrame), AnimOrig->Interpolation);
					
					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

					float Alpha = (float)Key / (float)BlendOutFrames;
					float BlendSpeed = FMath::Lerp(Speed, 1.0f, Alpha);
					AnimationTime += IntervalTime * BlendSpeed;
				}

				for (int32 Key = StartFrame + BlendInFrames + AdjustFrames + BlendOutFrames; Key < TotalFrames; Key++)
				{
					int32 KeyIndex1, KeyIndex2;
					float SubFrame;
					FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, AnimationTime, AnimOrig->GetDataModel()->GetFrameRate(), AnimOrig->GetDataModel()->GetNumberOfKeys());

					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(KeyIndex1, SubFrame), AnimOrig->Interpolation);
					
					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

					AnimationTime += IntervalTime;
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