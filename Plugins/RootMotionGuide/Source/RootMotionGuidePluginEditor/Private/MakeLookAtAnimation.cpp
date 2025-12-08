// Copyright 2017 Lee Ju Sik

#include "MakeLookAtAnimation.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimationRuntime.h"
#include "AnimationCoreLibrary.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "MakeLookAtAnimation"


FReply UMakeLookAtAnimation::MakeLookAtAnimation()
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

		StartFrame = FMath::Clamp(StartFrame, 0, NumKeys - 1);
		EndFrame = FMath::Clamp(EndFrame, 0, NumKeys - 1);
		EndFrame = FMath::Max(StartFrame, EndFrame);

		BlendInFrames = FMath::Max(BlendInFrames, 0);
		BlendOutFrames = FMath::Max(BlendOutFrames, 0);


		int32 BoneIndexToModify = RefSkeleton.FindBoneIndex(BoneNameToModify);
		if (BoneIndexToModify == INDEX_NONE)
		{
			Notification(LOCTEXT("Can't find BoneToModify", "Can't find BoneToModify"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		int32 LookAtBoneIndex = RefSkeleton.FindBoneIndex(LookAtBoneName);
		if (LookAtBoneName != NAME_None && LookAtBoneIndex == INDEX_NONE)
		{
			Notification(LOCTEXT("Can't find LookAtBone", "Can't find LookAtBone"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		LookUp_Axis.Initialize();
		if (LookUp_Axis.Axis.IsZero())
		{
			LookUp_Axis = FVector(1.f, 0.f, 0.f);
		}

		LookAt_Axis.Initialize();
		if (LookAt_Axis.Axis.IsZero())
		{
			LookAt_Axis = FVector(0.f, 1.f, 0.f);
		}

		TArray< TArray< FTransform > > AnimationDataInCS;
		AnimationDataInCS.Empty(NumBones);
		AnimationDataInCS.AddZeroed(NumBones);

		TArray< TArray< FTransform > > AnimationDataInLocal;
		AnimationDataInLocal.Empty(NumBones);
		AnimationDataInLocal.AddZeroed(NumBones);


		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			AnimationDataInCS[BoneIndex].Empty(NumKeys);
			AnimationDataInCS[BoneIndex].AddUninitialized(NumKeys);

			AnimationDataInLocal[BoneIndex].Empty(NumKeys);
			AnimationDataInLocal[BoneIndex].AddUninitialized(NumKeys);

			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);

			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, Key, AnimSequence->Interpolation);

					if (ParentBoneIndex != INDEX_NONE)
					{
						AnimationDataInCS[BoneIndex][Key] = AnimatedLocalKey * AnimationDataInCS[ParentBoneIndex][Key];
					}
					else
					{
						AnimationDataInCS[BoneIndex][Key] = AnimatedLocalKey;
					}

					AnimationDataInLocal[BoneIndex][Key] = AnimatedLocalKey;
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

					AnimationDataInLocal[BoneIndex][Key] = LocalTransform;
				}
			}
		}


		{
			FScopedTransaction Transation(LOCTEXT("MakeLookAtAnimation", "MakeLookAtAnimation"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("MakeLookAtAnimation")));

			double IntervalTime = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

			FVector CurrentTargetLocation = FVector::ZeroVector;
			FVector PreviousTargetLocation = FVector::ZeroVector;

			float AccumulatedInterpolationTime = 0.0f; 



			for (int32 Key = 0; Key < NumKeys; Key++)
			{
				if (Key < StartFrame || Key > (EndFrame + BlendOutFrames))
				{
					continue;
				}


				FTransform ComponentBoneTransform = AnimationDataInCS[BoneIndexToModify][Key];


				FVector TargetLocationInComponentSpace = FVector::ZeroVector;
				FTransform TargetTransform = FTransform::Identity;
				if (LookAtBoneIndex != INDEX_NONE)
				{
					TargetTransform = AnimationDataInCS[LookAtBoneIndex][Key];
					TargetLocationInComponentSpace = TargetTransform.TransformPosition(LookAtLocation);
				}
				else
				{
					TargetLocationInComponentSpace = LookAtLocation;
					TargetTransform.SetLocation(LookAtLocation);
				}

				if (Key == StartFrame)
				{
					PreviousTargetLocation = TargetLocationInComponentSpace;
					CurrentTargetLocation = TargetLocationInComponentSpace;
				}

				FVector OldCurrentTargetLocation = CurrentTargetLocation;
				FVector NewCurrentTargetLocation = TargetLocationInComponentSpace;

				if ((NewCurrentTargetLocation - OldCurrentTargetLocation).SizeSquared() > InterpolationTriggerThreashold * InterpolationTriggerThreashold)
				{
					if (AccumulatedInterpolationTime >= InterpolationTime)
					{
						AccumulatedInterpolationTime = 0.0f;
					}

					PreviousTargetLocation = OldCurrentTargetLocation;
					CurrentTargetLocation = NewCurrentTargetLocation;
				}
				else if (InterpolationTriggerThreashold == 0.0f)
				{
					CurrentTargetLocation = NewCurrentTargetLocation;
				}


				FVector CurrentLookAtLocation = FVector::ZeroVector;

				if (InterpolationTime > 0.0f)
				{
					float CurrentAlpha = AccumulatedInterpolationTime / InterpolationTime;

					if (CurrentAlpha < 1.0f)
					{
						float BlendApha = AlphaToBlendType(CurrentAlpha, InterpolationType);

						CurrentLookAtLocation = FMath::Lerp(PreviousTargetLocation, CurrentTargetLocation, BlendApha);
					}
				}
				else
				{
					CurrentLookAtLocation = CurrentTargetLocation;
				}


				FVector LookAtVector = LookAt_Axis.GetTransformedAxis(ComponentBoneTransform);
				FVector LookUpVector = LookUp_Axis.GetTransformedAxis(ComponentBoneTransform);

				FQuat DeltaRotation = AnimationCore::SolveAim(ComponentBoneTransform, CurrentLookAtLocation, LookAtVector, bUseLookUpAxis, LookUpVector, LookAtClamp);
				ComponentBoneTransform.SetRotation(DeltaRotation * ComponentBoneTransform.GetRotation());


				AnimationDataInCS[BoneIndexToModify][Key] = ComponentBoneTransform;
				AnimationDataInLocal[BoneIndexToModify][Key] = ComponentBoneTransform;

				int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndexToModify);
				if (ParentBoneIndex != INDEX_NONE)
				{
					AnimationDataInLocal[BoneIndexToModify][Key].SetToRelativeTransform(AnimationDataInCS[ParentBoneIndex][Key]);
				}


				AccumulatedInterpolationTime = FMath::Clamp((float)(AccumulatedInterpolationTime + IntervalTime), 0.f, InterpolationTime);
			}


	
			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneNameToModify) == false)
			{
				FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndexToModify];
				FRawAnimSequenceTrack AnimTrack;
				AnimTrack.PosKeys.Add(FVector3f(LocalTransform.GetLocation()));
				AnimTrack.RotKeys.Add(FQuat4f(LocalTransform.GetRotation()));
				AnimTrack.ScaleKeys.Add(FVector3f(LocalTransform.GetScale3D()));

				AnimSequence->GetController().AddBoneCurve(BoneNameToModify);
				AnimSequence->GetController().SetBoneTrackKeys(BoneNameToModify, AnimTrack.PosKeys, AnimTrack.RotKeys, AnimTrack.ScaleKeys);

			}


			TArray< FVector > AnimTrackPosKeys;
			TArray< FQuat > AnimTrackRotKeys;
			TArray< FVector > AnimTrackScaleKeys;
			AnimTrackPosKeys.Empty(NumKeys);
			AnimTrackRotKeys.Empty(NumKeys);
			AnimTrackScaleKeys.Empty(NumKeys);

			for (int32 Key = 0; Key < NumKeys; Key++)
			{
				FTransform AnimatedKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneNameToModify, Key, AnimSequence->Interpolation);

				if (Key >= StartFrame && Key <= (EndFrame + BlendOutFrames))
				{
					float Alpha = (BlendInFrames > 0) ? (float)(Key - StartFrame) / (float)BlendInFrames : 1.0f;
					if (Key >= EndFrame)
					{
						Alpha = (BlendOutFrames > 0) ? 1.0f - (float)(Key - EndFrame) / (float)BlendOutFrames : 0.0f;
					}

					AnimatedKey.BlendWith(AnimationDataInLocal[BoneIndexToModify][Key], Alpha);
					AnimatedKey.NormalizeRotation();
				}

				AnimTrackPosKeys.Add(AnimatedKey.GetLocation());
				AnimTrackRotKeys.Add(AnimatedKey.GetRotation());
				AnimTrackScaleKeys.Add(AnimatedKey.GetScale3D());
			}

			AnimSequence->GetController().SetBoneTrackKeys(BoneNameToModify, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}

float UMakeLookAtAnimation::AlphaToBlendType(float InAlpha, EMakeLookAtAnimationInterpolationBlend BlendType)
{
	switch (BlendType)
	{
	case EMakeLookAtAnimationInterpolationBlend::Cubic:
		return FMath::Clamp<float>(FMath::CubicInterp<float>(0.f, 0.f, 1.f, 0.f, InAlpha), 0.f, 1.f);
	case EMakeLookAtAnimationInterpolationBlend::Sinusoidal:
		return FMath::Clamp<float>((FMath::Sin(InAlpha * PI - HALF_PI) + 1.f) / 2.f, 0.f, 1.f);
	case EMakeLookAtAnimationInterpolationBlend::EaseInOutExponent2:
		return FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 2), 0.f, 1.f);
	case EMakeLookAtAnimationInterpolationBlend::EaseInOutExponent3:
		return FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 3), 0.f, 1.f);
	case EMakeLookAtAnimationInterpolationBlend::EaseInOutExponent4:
		return FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 4), 0.f, 1.f);
	case EMakeLookAtAnimationInterpolationBlend::EaseInOutExponent5:
		return FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 5), 0.f, 1.f);
	}

	return InAlpha;
}



#undef LOCTEXT_NAMESPACE