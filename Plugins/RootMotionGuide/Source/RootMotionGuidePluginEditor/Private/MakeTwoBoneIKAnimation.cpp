// Copyright 2017 Lee Ju Sik

#include "MakeTwoBoneIKAnimation.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimationRuntime.h"
#include "TwoBoneIK.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"


#define LOCTEXT_NAMESPACE "MakeTwoBoneIKAnimation"



FReply UMakeTwoBoneIKAnimation::MakeTwoBoneIKAnimation()
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

		if (EffectorLocationSpace == EBoneControlSpace::BCS_WorldSpace || JointTargetLocationSpace == EBoneControlSpace::BCS_WorldSpace)
		{
			Notification(LOCTEXT("EffectorLocationSpaceOrJointTargetLocationSpaceIsWorldSpace", "This tool does not support WorldSpace for EffectorLocationSpace and JointTargetLocationSpace."), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		StartFrame = FMath::Clamp(StartFrame, 0, NumKeys - 1);
		EndFrame = FMath::Clamp(EndFrame, 0, NumKeys - 1);
		EndFrame = FMath::Max(StartFrame, EndFrame);

		BlendInFrames = FMath::Max(BlendInFrames, 0);
		BlendOutFrames = FMath::Max(BlendOutFrames, 0);

		bool bInvalidLimb = false;

		int32 IKBoneIndex = RefSkeleton.FindBoneIndex(IKBoneName);
		if (IKBoneIndex == INDEX_NONE)
		{
			Notification(LOCTEXT("Invalid IK Bone", "Invalid IK Bone"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		int32 UpperLimbIndex = INDEX_NONE;
		int32 LowerLimbIndex = RefSkeleton.GetParentIndex(IKBoneIndex);
		if (LowerLimbIndex == INDEX_NONE)
		{
			bInvalidLimb = true;
		}
		else
		{
			UpperLimbIndex = RefSkeleton.GetParentIndex(LowerLimbIndex);
			if (UpperLimbIndex == INDEX_NONE)
			{
				bInvalidLimb = true;
			}
		}


		const bool bInBoneSpace = (EffectorLocationSpace == EBoneControlSpace::BCS_ParentBoneSpace) || (EffectorLocationSpace == EBoneControlSpace::BCS_BoneSpace);
		const int32 EffectorBoneIndex = bInBoneSpace ? RefSkeleton.FindBoneIndex(EffectorSpaceBoneName) : INDEX_NONE;

		if (bInBoneSpace && (EffectorBoneIndex == INDEX_NONE))
		{
			bInvalidLimb = true;
		}

		if (bInvalidLimb)
		{
			Notification(LOCTEXT("Invalid Limb", "Invalid Limb"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		TArray< TArray< FTransform > > AnimationDataInCS;
		AnimationDataInCS.Empty(NumBones);
		AnimationDataInCS.AddZeroed(NumBones);

		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			AnimationDataInCS[BoneIndex].Empty(NumKeys);
			AnimationDataInCS[BoneIndex].AddUninitialized(NumKeys);

			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);

			EBoneTranslationRetargetingMode::Type RetargetMode = Skeleton->GetBoneTranslationRetargetingMode(BoneIndex);

			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, Key, AnimSequence->Interpolation);

					if (RetargetMode == EBoneTranslationRetargetingMode::Skeleton)
					{
						AnimatedLocalKey.SetLocation(RefSkeleton.GetRefBonePose()[BoneIndex].GetLocation());
					}

					if (ParentBoneIndex != INDEX_NONE)
					{
						AnimationDataInCS[BoneIndex][Key] = AnimatedLocalKey * AnimationDataInCS[ParentBoneIndex][Key];
					}
					else
					{
						AnimationDataInCS[BoneIndex][Key] = AnimatedLocalKey;
					}
				}
			}
			else
			{
				FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					if (ParentBoneIndex != INDEX_NONE)
					{
						AnimationDataInCS[BoneIndex][Key] = LocalTransform * AnimationDataInCS[ParentBoneIndex][Key];
					}
					else
					{
						AnimationDataInCS[BoneIndex][Key] = LocalTransform;
					}
				}
			}
		}

		{
			FScopedTransaction Transation(LOCTEXT("MakeTwoBoneIKAnimation", "MakeTwoBoneIKAnimation"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("MakeTwoBoneIKAnimation")));

			FName UpperLimbBoneName = RefSkeleton.GetBoneName(UpperLimbIndex);
			FName LowerLimbBoneName = RefSkeleton.GetBoneName(LowerLimbIndex);

			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(UpperLimbBoneName) == false)
			{
				FTransform LocalTransform = RefSkeleton.GetRefBonePose()[UpperLimbIndex];
				FRawAnimSequenceTrack AnimTrack;
				AnimTrack.PosKeys.Add(FVector3f(LocalTransform.GetLocation()));
				AnimTrack.RotKeys.Add(FQuat4f(LocalTransform.GetRotation()));
				AnimTrack.ScaleKeys.Add(FVector3f(LocalTransform.GetScale3D()));

				AnimSequence->GetController().AddBoneCurve(UpperLimbBoneName);
				AnimSequence->GetController().SetBoneTrackKeys(UpperLimbBoneName, AnimTrack.PosKeys, AnimTrack.RotKeys, AnimTrack.ScaleKeys);

			}


			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(LowerLimbBoneName) == false)
			{
				FTransform LocalTransform = RefSkeleton.GetRefBonePose()[LowerLimbIndex];
				FRawAnimSequenceTrack AnimTrack;
				AnimTrack.PosKeys.Add(FVector3f(LocalTransform.GetLocation()));
				AnimTrack.RotKeys.Add(FQuat4f(LocalTransform.GetRotation()));
				AnimTrack.ScaleKeys.Add(FVector3f(LocalTransform.GetScale3D()));

				AnimSequence->GetController().AddBoneCurve(LowerLimbBoneName);
				AnimSequence->GetController().SetBoneTrackKeys(LowerLimbBoneName, AnimTrack.PosKeys, AnimTrack.RotKeys, AnimTrack.ScaleKeys);

			}

			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(IKBoneName) == false)
			{
				FTransform LocalTransform = RefSkeleton.GetRefBonePose()[IKBoneIndex];
				FRawAnimSequenceTrack AnimTrack;
				AnimTrack.PosKeys.Add(FVector3f(LocalTransform.GetLocation()));
				AnimTrack.RotKeys.Add(FQuat4f(LocalTransform.GetRotation()));
				AnimTrack.ScaleKeys.Add(FVector3f(LocalTransform.GetScale3D()));

				AnimSequence->GetController().AddBoneCurve(IKBoneName);
				AnimSequence->GetController().SetBoneTrackKeys(IKBoneName, AnimTrack.PosKeys, AnimTrack.RotKeys, AnimTrack.ScaleKeys);

			}



			TArray< FVector > UpperLimbAnimTrackPosKeys;
			TArray< FQuat > UpperLimbAnimTrackRotKeys;
			TArray< FVector > UpperLimbAnimTrackScaleKeys;
			UpperLimbAnimTrackPosKeys.Empty(NumKeys);
			UpperLimbAnimTrackRotKeys.Empty(NumKeys);
			UpperLimbAnimTrackScaleKeys.Empty(NumKeys);

			TArray< FVector > LowerLimbAnimTrackPosKeys;
			TArray< FQuat > LowerLimbAnimTrackRotKeys;
			TArray< FVector > LowerLimbAnimTrackScaleKeys;
			LowerLimbAnimTrackPosKeys.Empty(NumKeys);
			LowerLimbAnimTrackRotKeys.Empty(NumKeys);
			LowerLimbAnimTrackScaleKeys.Empty(NumKeys);

			TArray< FVector > EndBoneAnimTrackPosKeys;
			TArray< FQuat > EndBoneAnimTrackRotKeys;
			TArray< FVector > EndBoneAnimTrackScaleKeys;
			EndBoneAnimTrackPosKeys.Empty(NumKeys);
			EndBoneAnimTrackRotKeys.Empty(NumKeys);
			EndBoneAnimTrackScaleKeys.Empty(NumKeys);


			int32 JointTargetBoneIndex = INDEX_NONE;
			if (JointTargetLocationSpace == EBoneControlSpace::BCS_ParentBoneSpace || JointTargetLocationSpace == EBoneControlSpace::BCS_BoneSpace)
			{
				JointTargetBoneIndex = RefSkeleton.FindBoneIndex(JointTargetSpaceBoneName);
			}

			for (int32 Key = 0; Key < NumKeys; Key++)
			{
				FTransform UpperLimbAnimatedKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(UpperLimbBoneName, Key, AnimSequence->Interpolation);
				
				if (Skeleton->GetBoneTranslationRetargetingMode(UpperLimbIndex) == EBoneTranslationRetargetingMode::Skeleton)
				{
					UpperLimbAnimatedKey.SetLocation(RefSkeleton.GetRefBonePose()[UpperLimbIndex].GetLocation());
				}

				FTransform LowerLimbAnimatedKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(LowerLimbBoneName, Key, AnimSequence->Interpolation);
				
				if (Skeleton->GetBoneTranslationRetargetingMode(LowerLimbIndex) == EBoneTranslationRetargetingMode::Skeleton)
				{
					LowerLimbAnimatedKey.SetLocation(RefSkeleton.GetRefBonePose()[LowerLimbIndex].GetLocation());
				}

				FTransform EndBoneAnimatedKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(IKBoneName, Key, AnimSequence->Interpolation);
				
				if (Skeleton->GetBoneTranslationRetargetingMode(IKBoneIndex) == EBoneTranslationRetargetingMode::Skeleton)
				{
					EndBoneAnimatedKey.SetLocation(RefSkeleton.GetRefBonePose()[IKBoneIndex].GetLocation());
				}


				if (Key >= StartFrame && Key <= (EndFrame + BlendOutFrames))
				{
					const FTransform EndBoneLocalTransform = EndBoneAnimatedKey;
					const FTransform LowerLimbLocalTransform = LowerLimbAnimatedKey;
					const FTransform UpperLimbLocalTransform = UpperLimbAnimatedKey;

					FTransform UpperLimbCSTransform = AnimationDataInCS[UpperLimbIndex][Key];
					FTransform LowerLimbCSTransform = AnimationDataInCS[LowerLimbIndex][Key];
					FTransform EndBoneCSTransform = AnimationDataInCS[IKBoneIndex][Key];


					const FVector RootPos = UpperLimbCSTransform.GetTranslation();
					const FVector InitialJointPos = LowerLimbCSTransform.GetTranslation();
					const FVector InitialEndPos = EndBoneCSTransform.GetTranslation();

					FTransform EffectorTransform(EffectorLocation);
					ConvertBoneSpaceTransformToCS(RefSkeleton, AnimationDataInCS, EffectorTransform, EffectorBoneIndex, Key, EffectorLocationSpace);

					FTransform JointTargetTransform(JointTargetLocation);
					ConvertBoneSpaceTransformToCS(RefSkeleton, AnimationDataInCS, JointTargetTransform, JointTargetBoneIndex, Key, JointTargetLocationSpace);

					FVector JointTargetPos = JointTargetTransform.GetTranslation();

					FVector DesiredPos = EffectorTransform.GetTranslation();

					UpperLimbCSTransform.SetLocation(RootPos);
					LowerLimbCSTransform.SetLocation(InitialJointPos);
					EndBoneCSTransform.SetLocation(InitialEndPos);


					AnimationCore::SolveTwoBoneIK(UpperLimbCSTransform, LowerLimbCSTransform, EndBoneCSTransform, JointTargetPos, DesiredPos, bAllowStretching, StartStretchRatio, MaxStretchScale);

					if (bAllowTwist == false)
					{
						int32 UpperLimbParentIndex = RefSkeleton.GetParentIndex(UpperLimbIndex);
						FVector AlignDir = TwistAxis.GetTransformedAxis(FTransform::Identity);
						if (UpperLimbParentIndex != INDEX_NONE)
						{
							FTransform UpperLimbParentTransform = AnimationDataInCS[UpperLimbParentIndex][Key];
							RemoveTwist(UpperLimbParentTransform, UpperLimbCSTransform, UpperLimbLocalTransform, AlignDir);
						}

						RemoveTwist(UpperLimbCSTransform, LowerLimbCSTransform, LowerLimbLocalTransform, AlignDir);
					}


					if (bInBoneSpace && bTakeRotationFromEffectorSpace)
					{
						EndBoneCSTransform.SetRotation(EffectorTransform.GetRotation());
					}
					else if (bMaintainEffectorRelRot)
					{
						EndBoneCSTransform = EndBoneLocalTransform * LowerLimbCSTransform;
					}

					FTransform IKUpperLimbLocalTransform = FTransform::Identity;
					FTransform IKLowerLimbLocalTransform = FTransform::Identity;
					FTransform IKEndBoneLocalTransform = FTransform::Identity;

					{
						IKUpperLimbLocalTransform = UpperLimbCSTransform;
						int32 ParentBoneIndex = RefSkeleton.GetParentIndex(UpperLimbIndex);
						if (ParentBoneIndex != INDEX_NONE)
						{
							IKUpperLimbLocalTransform.SetToRelativeTransform(AnimationDataInCS[ParentBoneIndex][Key]);
						}

						IKLowerLimbLocalTransform = LowerLimbCSTransform;
						IKLowerLimbLocalTransform.SetToRelativeTransform(UpperLimbCSTransform);

						IKEndBoneLocalTransform = EndBoneCSTransform;
						IKEndBoneLocalTransform.SetToRelativeTransform(LowerLimbCSTransform);
					}


					float IKAlpha = (BlendInFrames > 0) ? (float)(Key - StartFrame) / (float)BlendInFrames : 1.0f;
					if (Key >= EndFrame)
					{
						IKAlpha = (BlendOutFrames > 0) ? 1.0f - (float)(Key - EndFrame) / (float)BlendOutFrames : 0.0f;
					}

					IKAlpha = FMath::Clamp(IKAlpha, 0.0f, 1.0f);

					UpperLimbAnimatedKey.BlendWith(IKUpperLimbLocalTransform, IKAlpha);
					UpperLimbAnimatedKey.NormalizeRotation();

					LowerLimbAnimatedKey.BlendWith(IKLowerLimbLocalTransform, IKAlpha);
					LowerLimbAnimatedKey.NormalizeRotation();

					EndBoneAnimatedKey.BlendWith(IKEndBoneLocalTransform, IKAlpha);
					EndBoneAnimatedKey.NormalizeRotation();
				}

				UpperLimbAnimTrackPosKeys.Add(UpperLimbAnimatedKey.GetLocation());
				UpperLimbAnimTrackRotKeys.Add(UpperLimbAnimatedKey.GetRotation());
				UpperLimbAnimTrackScaleKeys.Add(UpperLimbAnimatedKey.GetScale3D());

				LowerLimbAnimTrackPosKeys.Add(LowerLimbAnimatedKey.GetLocation());
				LowerLimbAnimTrackRotKeys.Add(LowerLimbAnimatedKey.GetRotation());
				LowerLimbAnimTrackScaleKeys.Add(LowerLimbAnimatedKey.GetScale3D());

				EndBoneAnimTrackPosKeys.Add(EndBoneAnimatedKey.GetLocation());
				EndBoneAnimTrackRotKeys.Add(EndBoneAnimatedKey.GetRotation());
				EndBoneAnimTrackScaleKeys.Add(EndBoneAnimatedKey.GetScale3D());
			}

			AnimSequence->GetController().SetBoneTrackKeys(UpperLimbBoneName, UpperLimbAnimTrackPosKeys, UpperLimbAnimTrackRotKeys, UpperLimbAnimTrackScaleKeys);

			AnimSequence->GetController().SetBoneTrackKeys(LowerLimbBoneName, LowerLimbAnimTrackPosKeys, LowerLimbAnimTrackRotKeys, LowerLimbAnimTrackScaleKeys);

			AnimSequence->GetController().SetBoneTrackKeys(IKBoneName, EndBoneAnimTrackPosKeys, EndBoneAnimTrackRotKeys, EndBoneAnimTrackScaleKeys);


			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}

void UMakeTwoBoneIKAnimation::ConvertBoneSpaceTransformToCS(const FReferenceSkeleton& RefSkeleton, TArray<TArray<FTransform>> AnimationDataInCS, FTransform & InOutBoneSpaceTM, int32 BoneIndex, int32 Key, EBoneControlSpace Space)
{
	switch (Space)
	{
	case BCS_WorldSpace:
		// Can't support
		break;
	case BCS_ComponentSpace:
		// Component Space, no change.
		break;
	case BCS_ParentBoneSpace:
		if (BoneIndex != INDEX_NONE)
		{
			int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);
			if (ParentBoneIndex != INDEX_NONE)
			{
				const FTransform& ParentTM = AnimationDataInCS[ParentBoneIndex][Key];
				InOutBoneSpaceTM *= ParentTM;
			}
		}
		break;
	case BCS_BoneSpace:
		if (BoneIndex != INDEX_NONE)
		{
			const FTransform& BoneTM = AnimationDataInCS[BoneIndex][Key];
			InOutBoneSpaceTM *= BoneTM;
		}
		break;
	default:
		UE_LOG(LogClass, Warning, TEXT("ConvertBoneSpaceTransformToCS: Unknown BoneSpace %d"), (int32)Space);
		break;
	}
}

void UMakeTwoBoneIKAnimation::RemoveTwist(const FTransform & InParentTransform, FTransform & InOutTransform, const FTransform & OriginalLocalTransform, const FVector & InAlignVector)
{
	FTransform LocalTransform = InOutTransform.GetRelativeTransform(InParentTransform);
	FQuat LocalRotation = LocalTransform.GetRotation();
	FQuat NewTwist, NewSwing;
	LocalRotation.ToSwingTwist(InAlignVector, NewSwing, NewTwist);
	NewSwing.Normalize();

	// get new twist from old local
	LocalRotation = OriginalLocalTransform.GetRotation();
	FQuat OldTwist, OldSwing;
	LocalRotation.ToSwingTwist(InAlignVector, OldSwing, OldTwist);
	OldTwist.Normalize();

	InOutTransform.SetRotation(InParentTransform.GetRotation() * NewSwing * OldTwist);
	InOutTransform.NormalizeRotation();
}



#undef LOCTEXT_NAMESPACE