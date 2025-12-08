// Copyright 2017 Lee Ju Sik

#include "AddRootMotion.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"


#define LOCTEXT_NAMESPACE "AddRootMotion"

FReply UAddRootMotion::AddRootMotion()
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
			FScopedTransaction Transation(LOCTEXT("AddRootMotion", "AddRootMotion"));
			AnimSequence->Modify();

			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AddRootMotion")));
			

			AddRootMotionStartFrame = FMath::Max(AddRootMotionStartFrame, 0);
			AddRootMotionEndFrame = FMath::Max(AddRootMotionStartFrame + 1, AddRootMotionEndFrame);


			TArray<FVector> RootAnimTrackPosKeys;
			TArray<FQuat> RootAnimTrackRotKeys;
			TArray<FVector> RootAnimTrackScaleKeys;
			RootAnimTrackPosKeys.Empty(NumKeys);
			RootAnimTrackRotKeys.Empty(NumKeys);
			RootAnimTrackScaleKeys.Empty(NumKeys);

			FVector PrevLocationOrig = FVector::ZeroVector;
			FVector PrevLocation = FVector::ZeroVector;

			double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

			FVector AddMoveDelta = AddRootMotionValue / (float)(AddRootMotionEndFrame - AddRootMotionStartFrame);

			float AdjustVelocityZ = 0.0f;

			for (int32 key = 0; key < NumKeys; key++)
			{
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, FFrameTime(key), AnimSequence->Interpolation);


				FVector MoveDelta = AnimatedLocalKey.GetLocation() - PrevLocationOrig;
				PrevLocationOrig = AnimatedLocalKey.GetLocation();


				if (key > AddRootMotionStartFrame  && key <= AddRootMotionEndFrame)
				{
					MoveDelta += AddMoveDelta;

					if (GravityZ != 0.0f)
					{
						AdjustVelocityZ += GravityZ * IntervalTime;

						float VelocityZ = MoveDelta.Z / IntervalTime;
						VelocityZ += AdjustVelocityZ;

						MoveDelta.Z = VelocityZ * IntervalTime;
					}
				}

				AnimatedLocalKey.SetLocation(PrevLocation + MoveDelta);

				RootAnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
				RootAnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
				RootAnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

				PrevLocation = AnimatedLocalKey.GetLocation();
			}


			AnimSequence->GetController().SetBoneTrackKeys(RootBoneName, RootAnimTrackPosKeys, RootAnimTrackRotKeys, RootAnimTrackScaleKeys);


			if (bApplyOnlyRootBone)
			{
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

							{
								FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, 0, AnimSequence->Interpolation);

								PrevLocationOrig = AnimatedLocalKey.GetLocation();
								PrevLocation = AnimatedLocalKey.GetLocation();
							}
							

							for (int32 key = 0; key < NumKeys; key++)
							{
								FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);

								FVector MoveDelta = AnimatedLocalKey.GetLocation() - PrevLocationOrig;
								PrevLocationOrig = AnimatedLocalKey.GetLocation();


								if (key > AddRootMotionStartFrame  && key <= AddRootMotionEndFrame)
								{
									MoveDelta -= AddMoveDelta;
								}

								AnimatedLocalKey.SetLocation(PrevLocation + MoveDelta);

								AnimTrackPosKeys.Add(AnimatedLocalKey.GetLocation());
								AnimTrackRotKeys.Add(AnimatedLocalKey.GetRotation());
								AnimTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

								PrevLocation = AnimatedLocalKey.GetLocation();
							}

							AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
						}
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


