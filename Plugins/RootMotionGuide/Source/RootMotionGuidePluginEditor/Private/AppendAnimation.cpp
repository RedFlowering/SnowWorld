// Copyright 2017 Lee Ju Sik

#include "AppendAnimation.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"
#include "AnimationRuntime.h"

#define LOCTEXT_NAMESPACE "AppendAnimation"

FReply UAppendAnimation::AppendAnimation()
{
	if (IsValid(AnimSequence) == false)
	{
		Notification(LOCTEXT("No AnimSequence", "No AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (IsValid(NextAnimSequence) == false)
	{
		Notification(LOCTEXT("No NextAnimSequence", "No NextAnimSequence"), 8.0f, SNotificationItem::CS_Fail);
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

	int32 RootBoneIndex = 0;
	FName RootBoneName = RefSkeleton.GetBoneName(RootBoneIndex);
	if (AnimSequence->GetDataModel()->IsValidBoneTrackName(RootBoneName) == false)
	{
		Notification(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"), 8.0f, SNotificationItem::CS_Fail);
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

	double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

	int32 TotalFrames = NumKeys;
	NextStartTime = FMath::Min(NextStartTime, NextAnimSequence->GetDataModel()->GetPlayLength());
	int32 NextFrames = FMath::FloorToInt((NextAnimSequence->GetDataModel()->GetPlayLength() - NextStartTime) / IntervalTime) + 1;
	TotalFrames += NextFrames;

	

	UAnimSequence* AnimOrig = NewObject<UAnimSequence>();
	AnimOrig->SetSkeleton(AnimSequence->GetSkeleton());
	AnimOrig->CreateAnimation(AnimSequence);

	UAnimSequence* NextOrig = NewObject<UAnimSequence>();
	NextOrig->SetSkeleton(NextAnimSequence->GetSkeleton());
	NextOrig->CreateAnimation(NextAnimSequence);

	{
		

		FScopedTransaction Transation(LOCTEXT("AppendAnimation", "AppendAnimation"));
		AnimSequence->Modify();
		AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AppendAnimation")));

		AnimSequence->GetController().SetNumberOfFrames(FFrameNumber(TotalFrames - 1));

		{
			TArray< FVector > AnimTrackPosKeys;
			TArray< FQuat > AnimTrackRotKeys;
			TArray< FVector > AnimTrackScaleKeys;
			AnimTrackPosKeys.Empty(TotalFrames);
			AnimTrackRotKeys.Empty(TotalFrames);
			AnimTrackScaleKeys.Empty(TotalFrames);

			FTransform LastKey = FTransform::Identity;

			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, key, AnimOrig->Interpolation);


				AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
				AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
				AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

				LastKey = AnimatedLocalKey;
			}


			{
				if (NextOrig->GetDataModel()->IsValidBoneTrackName(RootBoneName))
				{
					int32 KeyIndex1, KeyIndex2;
					float SubFrame;
					FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, NextStartTime, NextOrig->GetDataModel()->GetFrameRate(), NextOrig->GetDataModel()->GetNumberOfKeys());

					FTransform NextStartKey = NextOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, FFrameTime(KeyIndex1, SubFrame), NextOrig->Interpolation);
					

					for (int32 key = 0; key < NextFrames; key++)
					{
						FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, (key* IntervalTime) + NextStartTime, NextOrig->GetDataModel()->GetFrameRate(), NextOrig->GetDataModel()->GetNumberOfKeys());

						FTransform AnimatedLocalKey = NextOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, FFrameTime(KeyIndex1, SubFrame), NextOrig->Interpolation);
						
						AnimatedLocalKey = AnimatedLocalKey * NextStartKey.Inverse();
						AnimatedLocalKey.SetLocation(LastKey.GetRotation().RotateVector(AnimatedLocalKey.GetLocation()));


						FTransform NextLocalKey = LastKey;
						NextLocalKey.Accumulate(AnimatedLocalKey);
						NextLocalKey.NormalizeRotation();

						AnimTrackPosKeys.Add(NextLocalKey.GetLocation());
						AnimTrackRotKeys.Add(NextLocalKey.GetRotation());
						AnimTrackScaleKeys.Add(NextLocalKey.GetScale3D());
					}
				}
				else
				{
					for (int32 key = 0; key < NextFrames; key++)
					{
						AnimTrackPosKeys.Add(LastKey.GetLocation());
						AnimTrackRotKeys.Add(LastKey.GetRotation());
						AnimTrackScaleKeys.Add(LastKey.GetScale3D());
					}
				}
			}

			AnimSequence->GetController().SetBoneTrackKeys(RootBoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
		}

		for (int32 BoneIndex = 1; BoneIndex < NumBones; BoneIndex++)
		{
			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			
			if (AnimOrig->GetDataModel()->IsValidBoneTrackName(BoneName) == false)
			{
				bool bAddNewTrack = false;

				if (NextOrig->GetDataModel()->IsValidBoneTrackName(BoneName))
				{
					bAddNewTrack = true;
				}

				if (bAddNewTrack)
				{
					FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];
					FRawAnimSequenceTrack AnimTrack;
					AnimTrack.PosKeys.Add(FVector3f(LocalTransform.GetLocation()));
					AnimTrack.RotKeys.Add(FQuat4f(LocalTransform.GetRotation()));
					AnimTrack.ScaleKeys.Add(FVector3f(LocalTransform.GetScale3D()));

					AnimSequence->GetController().AddBoneCurve(BoneName);
					AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrack.PosKeys, AnimTrack.RotKeys, AnimTrack.ScaleKeys);
				}
			}

			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				TArray< FVector > AnimTrackPosKeys;
				TArray< FQuat > AnimTrackRotKeys;
				TArray< FVector > AnimTrackScaleKeys;
				AnimTrackPosKeys.Empty(TotalFrames);
				AnimTrackRotKeys.Empty(TotalFrames);
				AnimTrackScaleKeys.Empty(TotalFrames);

				FTransform LastKey = FTransform::Identity;

				for (int32 key = 0; key < NumKeys; key++)
				{
					FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimOrig->Interpolation);


					AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

					if (key == (NumKeys - 1))
					{
						LastKey = AnimatedLocalKey;
					}
				}


				{
					if (NextOrig->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						for (int32 key = 0; key < NextFrames; key++)
						{
							int32 KeyIndex1, KeyIndex2;
							float SubFrame;
							FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, (key* IntervalTime) + NextStartTime, NextOrig->GetDataModel()->GetFrameRate(), NextOrig->GetDataModel()->GetNumberOfKeys());

							FTransform AnimatedLocalKey = NextOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(KeyIndex1, SubFrame), NextOrig->Interpolation);
							
							float Alpha = FMath::Clamp((float)(NextBlendFrames - key) / (float)(NextBlendFrames + 1), 0.0f, 1.0f);
							if (Alpha > 0.0f)
							{
								AnimatedLocalKey.BlendWith(LastKey, Alpha);
								AnimatedLocalKey.NormalizeRotation();
							}

							AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
							AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
							AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());
						}
					}
					else
					{
						for (int32 key = 0; key < NextFrames; key++)
						{
							AnimTrackPosKeys.Add(LastKey.GetLocation());
							AnimTrackRotKeys.Add(LastKey.GetRotation());
							AnimTrackScaleKeys.Add(LastKey.GetScale3D());
						}
					}
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