// Copyright 2017 Lee Ju Sik

#include "ReverseAnimation.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "ReverseAnimation"


FReply UReverseAnimation::ReverseAnimation()
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


		{
			FScopedTransaction Transation(LOCTEXT("ReverseAnimation", "ReverseAnimation"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("ReverseAnimation")));

			for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
			{
				if (BoneIndex == 0)
				{
					FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
					
					if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						TArray< FVector > AnimTrackPosKeys;
						TArray< FQuat > AnimTrackRotKeys;
						TArray< FVector > AnimTrackScaleKeys;

						AnimTrackPosKeys.Reserve(AnimSequence->GetDataModel()->GetNumberOfKeys());
						AnimTrackRotKeys.Reserve(AnimSequence->GetDataModel()->GetNumberOfKeys());
						AnimTrackScaleKeys.Reserve(AnimSequence->GetDataModel()->GetNumberOfKeys());

						FVector LastPos = FVector::ZeroVector;
						FQuat LastRot = FQuat::Identity;

						if (AnimSequence->GetDataModel()->GetNumberOfKeys() > 1)
						{
							FTransform AnimKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, AnimSequence->GetDataModel()->GetNumberOfKeys() - 1, AnimSequence->Interpolation);
							LastPos = AnimKey.GetLocation();
							LastRot = AnimKey.GetRotation();
						}


						for (int32 Key = AnimSequence->GetDataModel()->GetNumberOfKeys() - 1; Key >= 0; Key--)
						{
							FTransform AnimKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, Key, AnimSequence->Interpolation);

							AnimTrackPosKeys.Add(AnimKey.GetLocation() - LastPos);
							AnimTrackRotKeys.Add(LastRot.Inverse() * AnimKey.GetRotation());
							AnimTrackScaleKeys.Add(AnimKey.GetScale3D());
						}

						AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
					}
				}
				else
				{
					FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
					
					if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
					{
						TArray< FVector > AnimTrackPosKeys;
						TArray< FQuat > AnimTrackRotKeys;
						TArray< FVector > AnimTrackScaleKeys;

						AnimTrackPosKeys.Reserve(AnimSequence->GetDataModel()->GetNumberOfKeys());
						AnimTrackRotKeys.Reserve(AnimSequence->GetDataModel()->GetNumberOfKeys());
						AnimTrackScaleKeys.Reserve(AnimSequence->GetDataModel()->GetNumberOfKeys());


						for (int32 Key = AnimSequence->GetDataModel()->GetNumberOfKeys() - 1; Key >= 0; Key--)
						{
							FTransform AnimKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, Key, AnimSequence->Interpolation);

							AnimTrackPosKeys.Add(AnimKey.GetLocation());
							AnimTrackRotKeys.Add(AnimKey.GetRotation());
							AnimTrackScaleKeys.Add(AnimKey.GetScale3D());
						}

						AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
					}
				}
			}

			AnimSequence->GetController().CloseBracket();

			Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
		}
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE