// Fill out your copyright notice in the Description page of Project Settings.


#include "MakeMirrorAnimation.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimationRuntime.h"
#include "AnimationCoreLibrary.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"

#define LOCTEXT_NAMESPACE "MakeMirrorAnimation"

FReply UMakeMirrorAnimation::MakeMirrorAnimation()
{
	if (IsValid(AnimSequence) == false)
	{
		Notification(LOCTEXT("No AnimSequence", "No AnimSequence"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (IsValid(MirrorDataTable) == false)
	{
		Notification(LOCTEXT("No MirrorDataTable", "No MirrorDataTable"), 8.0f, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	if (AnimSequence)
	{
		USkeleton* Skeleton = AnimSequence->GetSkeleton();
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();
		if (NumBones == 0)
		{
			Notification(LOCTEXT("No Bones", "No Bones"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();
		if (NumKeys == 0)
		{
			Notification(LOCTEXT("Animation frame is zero", "Animation frame is zero"), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}


		TMap<FName, FName> NameToMirrorNameBoneMap;
		MirrorDataTable->ForeachRow<FMirrorTableRow>(TEXT("UMakeMirrorAnimation::MakeMirrorAnimation"), [&NameToMirrorNameBoneMap](const FName& Key, const FMirrorTableRow& Value) mutable
			{
				if (Value.MirrorEntryType == EMirrorRowType::Bone)
				{
					NameToMirrorNameBoneMap.Add(Value.Name, Value.MirroredName);
				}
			});

		TArray<int32> MirrorBoneIndexes;
		MirrorBoneIndexes.Init(INDEX_NONE, NumBones);


		if (MirrorDataTable->MirrorAxis != EAxis::None)
		{
			for (int32 BoneIndex = 0; BoneIndex < MirrorBoneIndexes.Num(); ++BoneIndex)
			{
				if (MirrorBoneIndexes[BoneIndex] == INDEX_NONE)
				{
					// Find the candidate mirror partner for this bone (falling back to mirroring to self)
					FName SourceBoneName = RefSkeleton.GetBoneName(BoneIndex);
					int32 MirrorBoneIndex = INDEX_NONE;

					FName* MirroredBoneName = NameToMirrorNameBoneMap.Find(SourceBoneName);
					if (!SourceBoneName.IsNone() && MirroredBoneName)
					{
						MirrorBoneIndex = RefSkeleton.FindBoneIndex(*MirroredBoneName);
					}

					MirrorBoneIndexes[BoneIndex] = MirrorBoneIndex;
					if (MirrorBoneIndex != INDEX_NONE)
					{
						MirrorBoneIndexes[MirrorBoneIndex] = BoneIndex;
					}
				}
			}
		}


		TArray<FTransform> RefTransformInCS;
		RefTransformInCS.Empty(NumBones);
		RefTransformInCS.AddDefaulted(NumBones);

		for(int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
		{
			int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);

			FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

			if (ParentBoneIndex != INDEX_NONE)
			{
				RefTransformInCS[BoneIndex] = LocalTransform * RefTransformInCS[ParentBoneIndex];
			}
			else
			{
				RefTransformInCS[BoneIndex] = LocalTransform;
			}
		}


		TArray< TArray< FTransform > > AnimationDataInLocal;
		AnimationDataInLocal.Empty(NumBones);
		AnimationDataInLocal.AddZeroed(NumBones);


		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			AnimationDataInLocal[BoneIndex].Empty(NumKeys);
			AnimationDataInLocal[BoneIndex].AddUninitialized(NumKeys);

			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);

			if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName))
			{
				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					FTransform AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, Key, AnimSequence->Interpolation);
					AnimationDataInLocal[BoneIndex][Key] = AnimatedLocalKey;
				}
			}
			else
			{
				FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					AnimationDataInLocal[BoneIndex][Key] = LocalTransform;
				}
			}
		}

		if (MirrorBoneIndexes[0] != INDEX_NONE)
		{
			const FQuat RootBoneRefRotation = RefTransformInCS[0].GetRotation();

			for(int32 Key = 0; Key < NumKeys; Key++)
			{
				FVector T = AnimationDataInLocal[0][Key].GetTranslation();
				T = FAnimationRuntime::MirrorVector(T, MirrorDataTable->MirrorAxis);

				FQuat Q = AnimationDataInLocal[0][Key].GetRotation();
				Q = FAnimationRuntime::MirrorQuat(Q, MirrorDataTable->MirrorAxis);
				Q *= FAnimationRuntime::MirrorQuat(RootBoneRefRotation, MirrorDataTable->MirrorAxis).Inverse() * RootBoneRefRotation;

				FVector S = AnimationDataInLocal[0][Key].GetScale3D();

				AnimationDataInLocal[0][Key] = FTransform(Q, T, S);
			}
		}

		// Mirroring is authored in object space and as such we must transform the local space transforms in object space in order
		// to apply the object space mirroring axis. To facilitate this, we use object space transforms for the bind pose which can be cached.
		// We ignore the translation/scale part of the bind pose as they don't impact mirroring.
		// 
		// Rotations, translations, and scales are all treated differently:
		//    Rotation:
		//        We transform the local space rotation into object space
		//        We mirror the rotation axis
		//        We apply a correction: if the source and target bones are different, we must account for the mirrored delta between the two
		//        We transform the result back into local space
		//    Translation:
		//        We rotate the local space translation into object space
		//        We mirror the result
		//        We then rotate it back into local space
		//    Scale:
		//        Mirroring does not modify scale
		// 
		// This sadly doesn't quite work for additive poses because in order to transform it into the bind pose reference frame,
		// we need the base pose it is applied on. Worse still, the base pose might not be static, it could be a time scaled sequence.

		// FAnimationRuntime::MirrorPose() 구현 참고
		// 월드 스페이스 (소스 본) -> 미러 -> 로컬 스페이스 (타겟 본)

		auto MirrorTransform = [&RefTransformInCS, this](const FTransform& SourceTransform, const int32& SourceParentIndex, const int32& SourceBoneIndex, const int32& TargetParentIndex, const int32& TargetBoneIndex) -> FTransform {
			const FQuat TargetParentRefRotation = RefTransformInCS[TargetParentIndex].GetRotation();
			const FQuat TargetBoneRefRotation = RefTransformInCS[TargetBoneIndex].GetRotation();
			const FQuat SourceParentRefRotation = RefTransformInCS[SourceParentIndex].GetRotation();
			const FQuat SourceBoneRefRotation = RefTransformInCS[SourceBoneIndex].GetRotation();

			// Mirror the translation component:  Rotate the translation into the space of the mirror plane,  mirror across the mirror plane, and rotate into the space of its new parent

			FVector T = SourceTransform.GetTranslation();
			T = SourceParentRefRotation.RotateVector(T);
			T = FAnimationRuntime::MirrorVector(T, MirrorDataTable->MirrorAxis);
			T = TargetParentRefRotation.UnrotateVector(T);

			// Mirror the rotation component:- Rotate into the space of the mirror plane, mirror across the plane, apply corrective rotation to align result with target space's rest orientation, 
			// then rotate into the space of its new parent

			FQuat Q = SourceTransform.GetRotation();
			Q = SourceParentRefRotation * Q;
			Q = FAnimationRuntime::MirrorQuat(Q, MirrorDataTable->MirrorAxis);
			
			// 왼쪽과 오른쪽 본이 비대칭일 경우 두 본의 차이만큼을 보정
			Q *= FAnimationRuntime::MirrorQuat(SourceBoneRefRotation, MirrorDataTable->MirrorAxis).Inverse() * TargetBoneRefRotation;
			Q = TargetParentRefRotation.Inverse() * Q;

			FVector S = SourceTransform.GetScale3D();

			return FTransform(Q, T, S);
		};


		for(int32 TargetBoneIndex = 1; TargetBoneIndex < NumBones; TargetBoneIndex++)
		{
			const int32 SourceBoneIndex = MirrorBoneIndexes[TargetBoneIndex];
			if (SourceBoneIndex == TargetBoneIndex)
			{
				const int32 TargetParentBoneIndex = RefSkeleton.GetParentIndex(TargetBoneIndex);

				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					AnimationDataInLocal[TargetBoneIndex][Key] = MirrorTransform(AnimationDataInLocal[TargetBoneIndex][Key], TargetParentBoneIndex, TargetBoneIndex, TargetParentBoneIndex, TargetBoneIndex);
				}
			}
			else if (SourceBoneIndex > TargetBoneIndex)
			{
				const int32 TargetParentBoneIndex = RefSkeleton.GetParentIndex(TargetBoneIndex);
				const int32 SourceParentBoneIndex = RefSkeleton.GetParentIndex(SourceBoneIndex);

				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					const FTransform NewTargetBoneTransform = MirrorTransform(AnimationDataInLocal[SourceBoneIndex][Key], SourceParentBoneIndex, SourceBoneIndex, TargetParentBoneIndex, TargetBoneIndex);
					const FTransform NewSourceBoneTransform = MirrorTransform(AnimationDataInLocal[TargetBoneIndex][Key], TargetParentBoneIndex, TargetBoneIndex, SourceParentBoneIndex, SourceBoneIndex);
					AnimationDataInLocal[TargetBoneIndex][Key] = NewTargetBoneTransform;
					AnimationDataInLocal[SourceBoneIndex][Key] = NewSourceBoneTransform;
				}
			}
		}



		{
			FScopedTransaction Transation(LOCTEXT("MakeMirrorAnimation", "MakeMirrorAnimation"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("MakeMirrorAnimation")));

			for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
			{
				if (MirrorBoneIndexes[BoneIndex] != INDEX_NONE)
				{
					FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
					if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false)
					{
						AnimSequence->GetController().AddBoneCurve(BoneName);
					}

					TArray< FVector > AnimTrackPosKeys;
					TArray< FQuat > AnimTrackRotKeys;
					TArray< FVector > AnimTrackScaleKeys;

					for (int32 Key = 0; Key < NumKeys; Key++)
					{
						FTransform LocalTransform = AnimationDataInLocal[BoneIndex][Key];
						AnimTrackPosKeys.Add(LocalTransform.GetLocation());
						AnimTrackRotKeys.Add(LocalTransform.GetRotation());
						AnimTrackScaleKeys.Add(LocalTransform.GetScale3D());
					}

					AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
				}
			}

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE