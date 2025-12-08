// Copyright 2017 Lee Ju Sik

#include "RootMotionExtractor.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "RootMotionExtractor"

URootMotionExtractor::URootMotionExtractor()
{
	RootMotionSourceBoneName = FName(TEXT("pelvis"));

	//RootMotionScale = 1.0f;

	RootMotionSourceBoneForwardAxis = ERootMotionExtractorAxisType::Y;
}


bool URootMotionExtractor::NeedToAddRootBoneAnimationTrack() const
{
	if (AnimSequence)
	{
		USkeleton* Skeleton = AnimSequence->GetSkeleton();
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();

		int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();
		if (NumKeys == 0)
		{
			return false;
		}

		int32 RootBoneIndex = 0;
		FName BoneName = RefSkeleton.GetBoneName(RootBoneIndex);

		if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false)
		{
			return true;
		}
	}

	return false;
}

EVisibility URootMotionExtractor::GetVisibilityAddRootBoneAnimationTrackButton() const
{
	return NeedToAddRootBoneAnimationTrack() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility URootMotionExtractor::GetVisibilityExtractRootMotionButton() const
{
	return NeedToAddRootBoneAnimationTrack() ? EVisibility::Collapsed : EVisibility::Visible;
}

FReply URootMotionExtractor::AddRootBoneAnimationTrack()
{
	if (AnimSequence)
	{
		USkeleton* Skeleton = AnimSequence->GetSkeleton();
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();

		int32 RootBoneIndex = 0;
		FName BoneName = RefSkeleton.GetBoneName(RootBoneIndex);

		if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false)
		{
			FScopedTransaction Transation(LOCTEXT("AddRootBoneAnimationTrack", "AddRootBoneAnimationTrack"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AddRootBoneAnimationTrack")));

			//Notification(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"), 8.0f, SNotificationItem::CS_Fail);

			//TArray<FName> TrackNames;// = AnimSequence->GetAnimationTrackNames();
			//TrackNames.Empty();
			//AnimSequence->GetDataModel()->GetBoneTrackNames(TrackNames);

			//TArray<FRawAnimSequenceTrack> RawAnimationData;// = AnimSequence->GetRawAnimationData();
			//RawAnimationData.Empty();
			//Algo::Transform(AnimSequence->GetDataModel()->GetBoneAnimationTracks(), RawAnimationData, [](FBoneAnimationTrack Track) { return Track.InternalTrackData; });


			//AnimSequence->GetController().RemoveAllBoneTracks();

			FRawAnimSequenceTrack RootTrack;
			RootTrack.PosKeys.Add(FVector3f::ZeroVector);
			RootTrack.RotKeys.Add(FQuat4f::Identity);
			RootTrack.ScaleKeys.Add(FVector3f::OneVector);
			AnimSequence->GetController().AddBoneCurve(BoneName);
			AnimSequence->GetController().SetBoneTrackKeys(BoneName, RootTrack.PosKeys, RootTrack.RotKeys, RootTrack.ScaleKeys);


			//for (int32 i = 0; i < RawAnimationData.Num(); i++)
			//{
			//	AnimSequence->GetController().AddBoneTrack(TrackNames[i]);
			//	AnimSequence->GetController().SetBoneTrackKeys(TrackNames[i], RawAnimationData[i].PosKeys, RawAnimationData[i].RotKeys, RawAnimationData[i].ScaleKeys);
			//}


			AnimSequence->GetController().CloseBracket();

			Notification(LOCTEXT("Success!!", "Success!!"), 8.0f, SNotificationItem::CS_Success);
			return FReply::Handled();
		}
	}

	return FReply::Handled();
}


FReply URootMotionExtractor::ExtractRootMotionFromSource()
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

		int32 RootMotionSourceBoneIndex = RefSkeleton.FindBoneIndex(RootMotionSourceBoneName);
		if (RootMotionSourceBoneIndex == INDEX_NONE)
		{
			Notification(LOCTEXT("Can't find root motion source bone", "Can't find root motion source bone"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		if (AnimSequence->GetDataModel()->IsValidBoneTrackName(RootMotionSourceBoneName) == false)
		{
			Notification(LOCTEXT("Can't find root motion source track", "Can't find root motion source track"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		if (RefSkeleton.GetParentIndex(RootMotionSourceBoneIndex) != 0)
		{
			Notification(LOCTEXT("Root motion source must be child of root bone", "Root motion source must be child of root bone"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		// Check root bone has root motion
		{
			int32 RootBoneIndex = 0;
			FName BoneName = RefSkeleton.GetBoneName(RootBoneIndex);
			
			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				if (bForceRootMotionExtraction == false)
				{
					for (int32 key = 0; key < NumKeys; key++)
					{
						FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);

						if (AnimatedLocalKey.Equals(FTransform::Identity) == false)
						{
							Notification(LOCTEXT("Root bone already has root motion", "Root bone already has root motion"), 8.0f, SNotificationItem::CS_Fail);
							return FReply::Handled();
						}
					}
				}
			}
			else
			{
				FScopedTransaction Transation(LOCTEXT("AddRootBoneAnimationTrack", "AddRootBoneAnimationTrack"));
				AnimSequence->Modify();
				AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AddRootBoneAnimationTrack")));

				//Notification(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"), 8.0f, SNotificationItem::CS_Fail);

				//TArray<FName> TrackNames;// = AnimSequence->GetAnimationTrackNames();
				//TrackNames.Empty();
				//AnimSequence->GetDataModel()->GetBoneTrackNames(TrackNames);

				//TArray<FRawAnimSequenceTrack> RawAnimationData;// = AnimSequence->GetRawAnimationData();
				//RawAnimationData.Empty();
				//Algo::Transform(AnimSequence->GetDataModel()->GetBoneAnimationTracks(), RawAnimationData, [](FBoneAnimationTrack Track) { return Track.InternalTrackData; });

				//AnimSequence->GetController().RemoveAllBoneTracks();

				FRawAnimSequenceTrack RootTrack;
				RootTrack.PosKeys.Add(FVector3f::ZeroVector);
				RootTrack.RotKeys.Add(FQuat4f::Identity);
				RootTrack.ScaleKeys.Add(FVector3f::OneVector);
				AnimSequence->GetController().AddBoneCurve(BoneName);
				AnimSequence->GetController().SetBoneTrackKeys(BoneName, RootTrack.PosKeys, RootTrack.RotKeys, RootTrack.ScaleKeys);


				//for (int32 i = 0; i < RawAnimationData.Num(); i++)
				//{
				//	AnimSequence->GetController().AddBoneTrack(TrackNames[i]);
				//	AnimSequence->GetController().SetBoneTrackKeys(TrackNames[i], RawAnimationData[i].PosKeys, RawAnimationData[i].RotKeys, RawAnimationData[i].ScaleKeys);
				//}

				AnimSequence->GetController().CloseBracket();

				Notification(LOCTEXT("Add root bone animation track. Please try again.", "Add root bone animation track. Please try again."), 8.0f, SNotificationItem::CS_Success);
				return FReply::Handled();
			}
		}


		// Extract
		{
			FScopedTransaction Transation(LOCTEXT("ExtractRootMotionFromSource", "ExtractRootMotionFromSource"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("ExtractRootMotionFromSource")));


			int32 RootBoneIndex = 0;
			FName RootBoneName = RefSkeleton.GetBoneName(RootBoneIndex);
			
			TArray<FVector> RootTrackPosKeys;
			TArray<FQuat> RootTrackRotKeys;
			TArray<FVector> RootTrackScaleKeys;
			RootTrackPosKeys.Empty(NumKeys);
			RootTrackRotKeys.Empty(NumKeys);
			RootTrackScaleKeys.Empty(NumKeys);



			TArray<FVector> RootMotionSourceTrackPosKeys;
			TArray<FQuat> RootMotionSourceTrackRotKeys;
			TArray<FVector> RootMotionSourceTrackScaleKeys;
			RootMotionSourceTrackPosKeys.Empty(NumKeys);
			RootMotionSourceTrackRotKeys.Empty(NumKeys);
			RootMotionSourceTrackScaleKeys.Empty(NumKeys);

			FVector RootMotionSourceStartPos = FVector::ZeroVector;
			FVector RootMotionSourceEndPos = FVector::ZeroVector;

			FRotator RootMotionSourceStartRot = FRotator::ZeroRotator;

			FRotator RootMotionStartRot = FRotator::ZeroRotator;
			FRotator RootMotionEndRot = FRotator::ZeroRotator;


			{
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, 0, AnimSequence->Interpolation);
				
				RootMotionSourceStartPos = AnimatedLocalKey.GetTranslation();
				RootMotionSourceStartRot = GetRootRotation(AnimatedLocalKey.GetRotation(), RootMotionSourceBoneForwardAxis);

				RootMotionStartRot = FRotator::ZeroRotator;

				AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, AnimSequence->GetDataModel()->GetNumberOfKeys() - 1, AnimSequence->Interpolation);
				RootMotionSourceEndPos = AnimatedLocalKey.GetTranslation();
				RootMotionEndRot = GetRootRotation(AnimatedLocalKey.GetRotation(), RootMotionSourceBoneForwardAxis) - RootMotionSourceStartRot;
			}



			int32 RotSampleInterval = RootMotionRotationSamplingInterval;


			for (int32 key = 0; key < NumKeys; key++)
			{
				float RootMotionRotAlpha = (float)key / (float)NumKeys;
				if (RotSampleInterval != 0 && key % RotSampleInterval == 0)
				{
					FTransform NextAnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, key + RotSampleInterval, AnimSequence->Interpolation);
					
					if (key > 0)
					{
						RootMotionStartRot = RootMotionEndRot;
					}

					RootMotionEndRot = GetRootRotation(NextAnimatedLocalKey.GetRotation(), RootMotionSourceBoneForwardAxis) - RootMotionSourceStartRot;
				}

				if (RotSampleInterval != 0)
				{
					RootMotionRotAlpha = (float)(key % RotSampleInterval) / (float)RotSampleInterval;
				}

				
				FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, key, AnimSequence->Interpolation);
				
				float KeyFrameAlpha = 0.0f;
				if (NumKeys > 1)
				{
					KeyFrameAlpha = (float)key / (float)(NumKeys - 1);
				}



				float RootMotionSourceHeight = FMath::InterpEaseInOut(RootMotionSourceHeightAtStart, RootMotionSourceHeightAtEnd, KeyFrameAlpha, 2.0f);
				float RootMotionRotationOffset = FMath::InterpEaseInOut(0.0f, RootMotionTargetRotationAtEnd, KeyFrameAlpha, 2.0f);

				FVector RootPos = AnimatedLocalKey.GetTranslation() - RootMotionSourceStartPos;
				RootPos.Z -= (RootMotionSourceHeight - RootMotionSourceHeightAtStart);

				FRotator RootRot = FMath::Lerp(RootMotionStartRot, RootMotionEndRot, RootMotionRotAlpha);

				if (TargetRotationType == ERootMotionExtractorTargetRotationType::Offset)
				{
					RootRot.Yaw += RootMotionRotationOffset;
				}
				else if (TargetRotationType == ERootMotionExtractorTargetRotationType::Absolute)
				{
					RootRot.Yaw = FMath::InterpEaseInOut(RootRot.Yaw, (double)RootMotionTargetRotationAtEnd, KeyFrameAlpha, 2.0f);
				}
				else if (TargetRotationType == ERootMotionExtractorTargetRotationType::Override)
				{
					RootRot.Yaw = FMath::Lerp(0.0f, (double)RootMotionTargetRotationAtEnd, KeyFrameAlpha);
				}
				

				RootTrackPosKeys.Add(RootPos);
				RootTrackRotKeys.Add(RootRot.Quaternion());
				RootTrackScaleKeys.Add(FVector::OneVector);

				FVector RootSourcePos = RootMotionSourceStartPos;
				RootSourcePos.Z = RootMotionSourceHeight;
				FQuat RootSourceRot = RootRot.Quaternion().Inverse() * AnimatedLocalKey.GetRotation();

				RootMotionSourceTrackPosKeys.Add(RootSourcePos);
				RootMotionSourceTrackRotKeys.Add(RootSourceRot);
				RootMotionSourceTrackScaleKeys.Add(AnimatedLocalKey.GetScale3D());

			}

			AnimSequence->GetController().SetBoneTrackKeys(RootBoneName, RootTrackPosKeys, RootTrackRotKeys, RootTrackScaleKeys);

			AnimSequence->GetController().SetBoneTrackKeys(RootMotionSourceBoneName, RootMotionSourceTrackPosKeys, RootMotionSourceTrackRotKeys, RootMotionSourceTrackScaleKeys);

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}

bool URootMotionExtractor::CanExtractRootMotion() const
{
	if (IsValid(AnimSequence))
	{
		return true;
	}

	return false;
}

FRotator URootMotionExtractor::GetRootRotation(const FQuat & RootSourceRot, ERootMotionExtractorAxisType ForwardAxis)
{
	FVector Forward = RootSourceRot.GetAxisY();
	switch (ForwardAxis)
	{
	case ERootMotionExtractorAxisType::X:
		Forward = RootSourceRot.GetAxisX();
		break;
	case ERootMotionExtractorAxisType::Y:
		Forward = RootSourceRot.GetAxisY();
		break;
	case ERootMotionExtractorAxisType::Z:
		Forward = RootSourceRot.GetAxisZ();
		break;
	default:
		break;
	}


	Forward.Z = 0.0f;
	Forward.Normalize();
	return Forward.ToOrientationRotator();
}



#undef LOCTEXT_NAMESPACE