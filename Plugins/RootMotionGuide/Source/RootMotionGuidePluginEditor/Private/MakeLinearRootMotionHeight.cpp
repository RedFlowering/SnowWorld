// Copyright 2017 Lee Ju Sik

#include "MakeLinearRootMotionHeight.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "MakeLinearRootMotionHeight"


FReply UMakeLinearRootMotionHeight::MakeLinearRootMotionHeight()
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
		FName RootBoneName = RefSkeleton.GetBoneName(RootBoneIndex);

		if (AnimSequence->GetDataModel()->IsValidBoneTrackName(RootBoneName) == false)
		{
			Notification(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		// Adjust
		{
			FScopedTransaction Transation(LOCTEXT("MakeLinearRootMotionHeight", "MakeLinearRootMotionHeight"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("MakeLinearRootMotionHeight")));

			TArray<float> HeightOffsetArray;

			LinearRootMotionHeightStartFrame = FMath::Max(LinearRootMotionHeightStartFrame, 0);
			LinearRootMotionHeightEndFrame = FMath::Max(LinearRootMotionHeightStartFrame + 1, LinearRootMotionHeightEndFrame);



			TArray<FVector> RootAnimTrackPosKeys;
			TArray<FQuat> RootAnimTrackRotKeys;
			TArray<FVector> RootAnimTrackScaleKeys;
			RootAnimTrackPosKeys.Empty(NumKeys);
			RootAnimTrackRotKeys.Empty(NumKeys);
			RootAnimTrackScaleKeys.Empty(NumKeys);


			FVector LinearRootMotionStartPos = FVector::ZeroVector;
			FVector LinearRootMotionEndPos = FVector::ZeroVector;

			{
				/*
				* 이전 버전에 키프레임이 아닌 시간으로 애니메이션값을 얻는 버그가 있음
				*/
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, LinearRootMotionHeightStartFrame, AnimSequence->Interpolation);
				LinearRootMotionStartPos = AnimatedLocalKey.GetLocation();


				AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, LinearRootMotionHeightEndFrame, AnimSequence->Interpolation);

				LinearRootMotionEndPos = AnimatedLocalKey.GetLocation();
			}


			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, key, AnimSequence->Interpolation);

				float HeightOffset = 0.0f;
				if (key >= LinearRootMotionHeightStartFrame  && key <= LinearRootMotionHeightEndFrame)
				{
					float Alpha = (LinearRootMotionHeightStartFrame == LinearRootMotionHeightEndFrame) ? 1.0f : (float)(key - LinearRootMotionHeightStartFrame) / (float)(LinearRootMotionHeightEndFrame - LinearRootMotionHeightStartFrame);
					float Height = FMath::Lerp(LinearRootMotionStartPos.Z, LinearRootMotionEndPos.Z, Alpha);

					FVector Pos = AnimatedLocalKey.GetLocation();
					HeightOffset = Pos.Z - Height;
					Pos.Z = Height;

					AnimatedLocalKey.SetLocation(Pos);
				}


				RootAnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
				RootAnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
				RootAnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

				HeightOffsetArray.Add(HeightOffset);
			}

			AnimSequence->GetController().SetBoneTrackKeys(RootBoneName, RootAnimTrackPosKeys, RootAnimTrackRotKeys, RootAnimTrackScaleKeys);

			for (int32 BoneIndex = 1; BoneIndex < NumBones; BoneIndex++)
			{
				if (RefSkeleton.GetParentIndex(BoneIndex) == RootBoneIndex)
				{
					FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
					
					if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						TArray<FVector> AnimTrackPosKeys;
						TArray<FQuat> AnimTrackRotKeys;
						TArray<FVector> AnimTrackScaleKeys;
						AnimTrackPosKeys.Empty(NumKeys);
						AnimTrackRotKeys.Empty(NumKeys);
						AnimTrackScaleKeys.Empty(NumKeys);

						for (int32 key = 0; key < NumKeys; key++)
						{
							FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);
							
							AnimatedLocalKey.AddToTranslation(FVector(0.0f, 0.0f, HeightOffsetArray[key]));

							AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
							AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
							AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());
						}

						AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
					}
				}
			}

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE