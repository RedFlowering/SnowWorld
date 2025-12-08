// Copyright 2017 Lee Ju Sik

#include "AppendLandingAnimation.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"
#include "AnimationRuntime.h"


#define LOCTEXT_NAMESPACE "AppendLandingAnimation"


FReply UAppendLandingAnimation::AppendLandingAnimation()
{
	if (IsValid(AnimSequence) == false)
	{
		Notification(LOCTEXT("No AnimSequence", "No AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (FallingFrames > 0 && IsValid(FallingAnimSequence) == false)
	{
		Notification(LOCTEXT("No FallingAnimSequence", "No FallingAnimSequence"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (IsValid(LandingAnimSequence) == false)
	{
		Notification(LOCTEXT("No LandingAnimSequence", "No LandingAnimSequence"), 8.0f, SNotificationItem::CS_Fail);
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

	if (IsValid(FallingAnimSequence) && FallingAnimSequence->GetSkeleton() != Skeleton)
	{
		Notification(LOCTEXT("Falling Animation skeleton is not same", "Falling Animation skeleton is not same"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (IsValid(FallingAnimSequence) && FallingAnimSequence->GetDataModel()->GetNumberOfKeys() == 0)
	{
		Notification(LOCTEXT("Falling Animation frame is zero", "Falling Animation frame is zero"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (LandingAnimSequence->GetSkeleton() != Skeleton)
	{
		Notification(LOCTEXT("Landing Animation skeleton is not same", "Landing Animation skeleton is not same"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (LandingAnimSequence->GetDataModel()->GetNumberOfKeys() == 0)
	{
		Notification(LOCTEXT("Landing Animation frame is zero", "Landing Animation frame is zero"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

	int32 TotalFrames = NumKeys;
	if (FallingFrames > 0)
	{
		TotalFrames += FallingFrames;
	}


	LandingStartTime = FMath::Min(LandingStartTime, LandingAnimSequence->GetDataModel()->GetPlayLength());
	int32 LandingFrames = FMath::FloorToInt((LandingAnimSequence->GetDataModel()->GetPlayLength() - LandingStartTime) / IntervalTime) + 1;
	TotalFrames += LandingFrames;


	UAnimSequence* AnimOrig = NewObject<UAnimSequence>();
	AnimOrig->SetSkeleton(AnimSequence->GetSkeleton());
	AnimOrig->CreateAnimation(AnimSequence);

	UAnimSequence* FallingOrig = nullptr;
	if (IsValid(FallingAnimSequence))
	{
		FallingOrig = NewObject<UAnimSequence>();
		FallingOrig->SetSkeleton(FallingAnimSequence->GetSkeleton());
		FallingOrig->CreateAnimation(FallingAnimSequence);
	}

	UAnimSequence* LandingOrig = nullptr;
	if (IsValid(LandingAnimSequence))
	{
		LandingOrig = NewObject<UAnimSequence>();
		LandingOrig->SetSkeleton(LandingAnimSequence->GetSkeleton());
		LandingOrig->CreateAnimation(LandingAnimSequence);
	}
	

	{
		FScopedTransaction Transation(LOCTEXT("AppendLandingAnimation", "AppendLandingAnimation"));
		AnimSequence->Modify();
		AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AppendLandingAnimation")));
		AnimSequence->GetController().SetNumberOfFrames(FFrameNumber(TotalFrames - 1));

		{

			TArray< FVector > AnimTrackPosKeys;
			TArray< FQuat > AnimTrackRotKeys;
			TArray< FVector > AnimTrackScaleKeys;
			AnimTrackPosKeys.Empty(TotalFrames);
			AnimTrackRotKeys.Empty(TotalFrames);
			AnimTrackScaleKeys.Empty(TotalFrames);

			FTransform LastKey = FTransform::Identity;
			FVector LastVelocity = FVector::ZeroVector;

			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, key, AnimOrig->Interpolation);


				AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
				AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
				AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

				if (key == (NumKeys - 1))
				{
					LastVelocity = (AnimatedLocalKey.GetLocation() - LastKey.GetLocation()) / IntervalTime;
				}

				LastKey = AnimatedLocalKey;
			}


			for (int32 key = 0; key < FallingFrames; key++)
			{
				FVector NextVelocity = LastVelocity;
				NextVelocity.Z += GravityZ * IntervalTime;

				FVector MoveDelta = FMath::Lerp(LastVelocity, NextVelocity, 0.5f) * IntervalTime;

				LastKey.AddToTranslation(MoveDelta);
				AnimTrackPosKeys.Add(LastKey.GetLocation());
				AnimTrackRotKeys.Add(LastKey.GetRotation());
				AnimTrackScaleKeys.Add(LastKey.GetScale3D());

				LastVelocity = NextVelocity;
			}

			{
				if (LandingOrig->GetDataModel()->IsValidBoneTrackName(RootBoneName))
				{
					int32 KeyIndex1, KeyIndex2;
					float SubFrame;
					FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, LandingStartTime, LandingOrig->GetDataModel()->GetFrameRate(), LandingOrig->GetDataModel()->GetNumberOfKeys());

					FTransform LandingStartKey = LandingOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, FFrameTime(KeyIndex1, SubFrame), LandingOrig->Interpolation);
					

					for (int32 key = 0; key < LandingFrames; key++)
					{
						FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, (key* IntervalTime) + LandingStartTime, LandingOrig->GetDataModel()->GetFrameRate(), LandingOrig->GetDataModel()->GetNumberOfKeys());

						FTransform AnimatedLocalKey = LandingOrig->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, FFrameTime(KeyIndex1, SubFrame), LandingOrig->Interpolation);
						
						AnimatedLocalKey = AnimatedLocalKey * LandingStartKey.Inverse();
						AnimatedLocalKey.SetLocation(LastKey.GetRotation().RotateVector(AnimatedLocalKey.GetLocation()));
						

						FTransform LandingLocalKey = LastKey;
						LandingLocalKey.Accumulate(AnimatedLocalKey);
						LandingLocalKey.NormalizeRotation();

						AnimTrackPosKeys.Add(LandingLocalKey.GetLocation());
						AnimTrackRotKeys.Add(LandingLocalKey.GetRotation());
						AnimTrackScaleKeys.Add(LandingLocalKey.GetScale3D());
					}
				}
				else
				{
					for (int32 key = 0; key < LandingFrames; key++)
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
				if (FallingFrames > 0)
				{
					if (FallingOrig->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						bAddNewTrack = true;
					}
				}

				if (LandingOrig->GetDataModel()->IsValidBoneTrackName(BoneName))
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

				if (FallingFrames > 0)
				{
					if (FallingOrig->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						for (int32 key = 0; key < FallingFrames; key++)
						{
							int32 KeyIndex1, KeyIndex2;
							float SubFrame;
							FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, key* IntervalTime, FallingOrig->GetDataModel()->GetFrameRate(), FallingOrig->GetDataModel()->GetNumberOfKeys());

							FTransform AnimatedLocalKey = FallingOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(KeyIndex1, SubFrame), FallingOrig->Interpolation);
							

							float Alpha = FMath::Clamp((float)(FallingBlendFrames - key) / (float)(FallingBlendFrames + 1), 0.0f, 1.0f);
							if (Alpha > 0.0f)
							{
								AnimatedLocalKey.BlendWith(LastKey, Alpha);
								AnimatedLocalKey.NormalizeRotation();
							}

							AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
							AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
							AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

							if (key == (FallingFrames - 1))
							{
								LastKey = AnimatedLocalKey;
							}
						}
					}
					else
					{
						for (int32 key = 0; key < FallingFrames; key++)
						{
							AnimTrackPosKeys.Add(LastKey.GetLocation());
							AnimTrackRotKeys.Add(LastKey.GetRotation());
							AnimTrackScaleKeys.Add(LastKey.GetScale3D());
						}
					}
				}

				{
					if (LandingOrig->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						for (int32 key = 0; key < LandingFrames; key++)
						{
							int32 KeyIndex1, KeyIndex2;
							float SubFrame;
							FAnimationRuntime::GetKeyIndicesFromTime(KeyIndex1, KeyIndex2, SubFrame, (key* IntervalTime) + LandingStartTime, LandingOrig->GetDataModel()->GetFrameRate(), LandingOrig->GetDataModel()->GetNumberOfKeys());

							FTransform AnimatedLocalKey = LandingOrig->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(KeyIndex1, SubFrame), LandingOrig->Interpolation);
							

							float Alpha = FMath::Clamp((float)(LandingBlendFrames - key) / (float)(LandingBlendFrames + 1), 0.0f, 1.0f);
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
						for (int32 key = 0; key < LandingFrames; key++)
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