// Copyright 2017 Lee Ju Sik


#include "RootMotionApplyFloorHeight.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "ApplyFloorHeight"

FReply URootMotionApplyFloorHeight::ApplyFloorHeight()
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
			FScopedTransaction Transation(LOCTEXT("ApplyFloorHeight", "ApplyFloorHeight"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("ApplyFloorHeight")));

			TArray<float> HeightOffsetArray;

			ApplyFloorHeightStartFrame = FMath::Max(ApplyFloorHeightStartFrame, 0);
			ApplyFloorHeightEndFrame = FMath::Max(ApplyFloorHeightStartFrame + 1, ApplyFloorHeightEndFrame);



			TArray<FVector> RootAnimTrackPosKeys;
			TArray<FQuat> RootAnimTrackRotKeys;
			TArray<FVector> RootAnimTrackScaleKeys;
			RootAnimTrackPosKeys.Empty(NumKeys);
			RootAnimTrackRotKeys.Empty(NumKeys);
			RootAnimTrackScaleKeys.Empty(NumKeys);



			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, key, AnimSequence->Interpolation);

				float HeightOffset = 0.0f;
				if (key >= ApplyFloorHeightStartFrame && key <= ApplyFloorHeightEndFrame)
				{
					FVector Pos = AnimatedLocalKey.GetLocation();

					float Height = Pos.Z;
					if (Height < FloorHeight)
					{
						Height = FloorHeight;
					}

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