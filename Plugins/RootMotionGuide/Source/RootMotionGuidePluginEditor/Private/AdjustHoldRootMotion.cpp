// Copyright 2017 Lee Ju Sik

#include "AdjustHoldRootMotion.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "AdjustHoldRootMotion"

FReply UAdjustHoldRootMotion::AdjustHoldRootMotion()
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

		int HoldBoneIndex = RefSkeleton.FindBoneIndex(HoldRootMotionHoldBoneName);
		if (HoldBoneIndex == INDEX_NONE)
		{
			Notification(LOCTEXT("Can't find hold bone index", "Can't find hold bone index"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		{
			FScopedTransaction Transation(LOCTEXT("AdjustHoldRootMotion", "AdjustHoldRootMotion"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AdjustHoldRootMotion")));

			double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

			TArray< TArray<FTransform> > AnimationDataInComponentSpace;
			AnimationDataInComponentSpace.Empty(NumBones);
			AnimationDataInComponentSpace.AddZeroed(NumBones);

			TArray< int32 > RequiredBoneIndexArray;

			{
				int32 BoneIndex = HoldBoneIndex;

				while (BoneIndex != INDEX_NONE)
				{
					AnimationDataInComponentSpace[BoneIndex].Empty(NumKeys);
					AnimationDataInComponentSpace[BoneIndex].AddUninitialized(NumKeys);

					RequiredBoneIndexArray.AddUnique(BoneIndex);

					BoneIndex = RefSkeleton.GetParentIndex(BoneIndex);
				}
			}

			for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
			{
				if (RequiredBoneIndexArray.Contains(BoneIndex))
				{
					FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
					int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);

					EBoneTranslationRetargetingMode::Type RetargetMode = Skeleton->GetBoneTranslationRetargetingMode(BoneIndex);

					if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						for (int32 key = 0; key < NumKeys; key++)
						{
							FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);

							if (RetargetMode == EBoneTranslationRetargetingMode::Skeleton)
							{
								AnimatedLocalKey.SetLocation(RefSkeleton.GetRefBonePose()[BoneIndex].GetLocation());
							}

							if (ParentBoneIndex != INDEX_NONE)
							{
								AnimationDataInComponentSpace[BoneIndex][key] = AnimatedLocalKey * AnimationDataInComponentSpace[ParentBoneIndex][key];
							}
							else
							{
								AnimationDataInComponentSpace[BoneIndex][key] = AnimatedLocalKey;
							}
						}
					}
					else
					{
						FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

						for (int32 key = 0; key < NumKeys; key++)
						{
							if (ParentBoneIndex != INDEX_NONE)
							{
								AnimationDataInComponentSpace[BoneIndex][key] = LocalTransform * AnimationDataInComponentSpace[ParentBoneIndex][key];
							}
							else
							{
								AnimationDataInComponentSpace[BoneIndex][key] = LocalTransform;
							}
						}
					}
				}
			}


			HoldRootMotionStartFrame = FMath::Max(HoldRootMotionStartFrame, 0);
			HoldRootMotionEndFrame = FMath::Max(HoldRootMotionStartFrame, HoldRootMotionEndFrame);

			FVector PrevHoldBonePos = AnimationDataInComponentSpace[HoldBoneIndex][HoldRootMotionStartFrame].GetLocation();


			TArray<FVector> RootAnimTrackPosKeys;
			TArray<FQuat> RootAnimTrackRotKeys;
			TArray<FVector> RootAnimTrackScaleKeys;
			RootAnimTrackPosKeys.Empty(NumKeys);
			RootAnimTrackRotKeys.Empty(NumKeys);
			RootAnimTrackScaleKeys.Empty(NumKeys);

			FVector PrevLocationOrig = FVector::ZeroVector;
			FVector PrevLocation = FVector::ZeroVector;


			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, key, AnimSequence->Interpolation);

				FVector MoveDelta = AnimatedLocalKey.GetLocation() - PrevLocationOrig;
				PrevLocationOrig = AnimatedLocalKey.GetLocation();


				if (key >= HoldRootMotionStartFrame  && key <= HoldRootMotionEndFrame)
				{
					FVector HoldBonePos = AnimationDataInComponentSpace[HoldBoneIndex][key].GetLocation();

					if (bHoldRootMotionLockX)
					{
						MoveDelta.X -= HoldBonePos.X - PrevHoldBonePos.X;
					}

					if (bHoldRootMotionLockY)
					{
						MoveDelta.Y -= HoldBonePos.Y - PrevHoldBonePos.Y;
					}

					if (bHoldRootMotionLockZ)
					{
						MoveDelta.Z -= HoldBonePos.Z - PrevHoldBonePos.Z;
					}

					PrevHoldBonePos = HoldBonePos;
				}

				AnimatedLocalKey.SetLocation(PrevLocation + MoveDelta);

				RootAnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
				RootAnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
				RootAnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

				PrevLocation = AnimatedLocalKey.GetLocation();
			}


			AnimSequence->GetController().SetBoneTrackKeys(RootBoneName, RootAnimTrackPosKeys, RootAnimTrackRotKeys, RootAnimTrackScaleKeys);

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE



