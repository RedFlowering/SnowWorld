// Copyright 2017 Lee Ju Sik

#include "MakeFABRIKAnimation.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "AnimationRuntime.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"


#define LOCTEXT_NAMESPACE "MakeFABRIKAnimation"


FReply UMakeFABRIKAnimation::MakeFABRIKAnimation()
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

		if (EffectorTransformSpace == EBoneControlSpace::BCS_WorldSpace)
		{
			Notification(LOCTEXT("EffectorTransformSpaceIsWorldSpace", "This tool does not support WorldSpace for EffectorTransformSpace."), 8.0f, SNotificationItem::CS_Fail);
			return FReply::Handled();
		}

		StartFrame = FMath::Clamp(StartFrame, 0, NumKeys - 1);
		EndFrame = FMath::Clamp(EndFrame, 0, NumKeys - 1);
		EndFrame = FMath::Max(StartFrame, EndFrame);

		BlendInFrames = FMath::Max(BlendInFrames, 0);
		BlendOutFrames = FMath::Max(BlendOutFrames, 0);

		// Gather all bone indices between root and tip.
		TArray< int32 > BoneIndices;
		
		{
			int32 IKRootBoneIndex = RefSkeleton.FindBoneIndex(IKRootBoneName);
			if (IKRootBoneIndex == INDEX_NONE)
			{
				Notification(LOCTEXT("Can't find IK Root Bone", "Can't find IK Root Bone"), 8.0f, SNotificationItem::CS_Fail);
				return FReply::Handled();
			}

			if (IKRootBoneIndex == 0)
			{
				if (AnimSequence->GetDataModel()->IsValidBoneTrackName(IKRootBoneName) == false)
				{
					FScopedTransaction Transation(LOCTEXT("AddRootBoneAnimationTrack", "AddRootBoneAnimationTrack"));
					AnimSequence->Modify();
					AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AddRootBoneAnimationTrack")));

					//Notification(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"), 8.0f, SNotificationItem::CS_Fail);

					/*
					* 기존에는 트랙을 직접 추가 했기 때문에 루트 본 트랙의 순서를 0으로 맞추기 위해서 다음 방법을 써야 했지만
					* 지금 시스템에서는 루트 본 트랙이 없는 경우가 없고 추가 방식이 바뀌었기 때문에 더이상 필요하지 않을 것으로 예상.
					* 테스트가 필요하지만 더이상 루트 본 트랙이 없는 경우를 재현할 수 없음
					*/
					//TArray<FName> TrackNames;
					//TrackNames.Empty();
					//AnimSequence->GetDataModel()->GetBoneTrackNames(TrackNames);

					//TArray<FRawAnimSequenceTrack> RawAnimationData;
					//RawAnimationData.Empty();
					//Algo::Transform(AnimSequence->GetDataModel()->GetBoneAnimationTracks(), RawAnimationData, [](FBoneAnimationTrack Track) { return Track.InternalTrackData; });

					//AnimSequence->GetController().RemoveAllBoneTracks();

					FRawAnimSequenceTrack RootTrack;
					RootTrack.PosKeys.Add(FVector3f::ZeroVector);
					RootTrack.RotKeys.Add(FQuat4f(FQuat::Identity));
					RootTrack.ScaleKeys.Add(FVector3f::OneVector);
					AnimSequence->GetController().AddBoneCurve(IKRootBoneName);
					AnimSequence->GetController().SetBoneTrackKeys(IKRootBoneName, RootTrack.PosKeys, RootTrack.RotKeys, RootTrack.ScaleKeys);


					//for (int32 i = 0; i < RawAnimationData.Num(); i++)
					//{
					//	//AnimSequence->AddNewRawTrack(TrackNames[i], &(RawAnimationData[i]));
					//	AnimSequence->GetController().AddBoneTrack(TrackNames[i]);
					//	AnimSequence->GetController().SetBoneTrackKeys(TrackNames[i], RawAnimationData[i].PosKeys, RawAnimationData[i].RotKeys, RawAnimationData[i].ScaleKeys);

					//}

					AnimSequence->GetController().CloseBracket();

					Notification(LOCTEXT("Add root bone animation track. Please try again.", "Add root bone animation track. Please try again."), 8.0f, SNotificationItem::CS_Success);
					return FReply::Handled();
				}
			}

			int32 IKBoneIndex = RefSkeleton.FindBoneIndex(IKTipBoneName);
			if (IKBoneIndex == INDEX_NONE)
			{
				Notification(LOCTEXT("Can't find IK Tip Bone", "Can't find IK Tip Bone"), 8.0f, SNotificationItem::CS_Fail);
				return FReply::Handled();
			}

			do
			{
				BoneIndices.Insert(IKBoneIndex, 0);
				IKBoneIndex = RefSkeleton.GetParentIndex(IKBoneIndex);
			} while (IKBoneIndex != IKRootBoneIndex);
			BoneIndices.Insert(IKBoneIndex, 0);
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
			FScopedTransaction Transation(LOCTEXT("MakeFABRIKAnimation", "MakeFABRIKAnimation"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("MakeFABRIKAnimation")));

			struct FBoneTransform
			{
				int32 BoneIndex = INDEX_NONE;

				FTransform Transform = FTransform::Identity;

				FBoneTransform() :BoneIndex(INDEX_NONE) {}
				FBoneTransform(int32 InBoneIndex, const FTransform& InTransform)
					:BoneIndex(InBoneIndex), Transform(InTransform) {}
			};


			for (int32 Key = 0; Key < NumKeys; Key++)
			{
				if (Key < StartFrame || Key > (EndFrame + BlendOutFrames))
				{
					continue;
				}

				FTransform CSEffectorTransform = EffectorTransform;
				ConvertBoneSpaceTransformToCS(RefSkeleton, AnimationDataInCS, CSEffectorTransform, RefSkeleton.FindBoneIndex(EffectorTransformBoneName), Key, EffectorTransformSpace);

				FVector const CSEffectorLocation = CSEffectorTransform.GetLocation();

				float MaximumReach = 0.0f;


				int32 const NumTransforms = BoneIndices.Num();

				TArray< FBoneTransform > BoneTransforms;
				BoneTransforms.AddUninitialized(NumTransforms);


				TArray< FABRIKChainLink > Chain;
				Chain.Reserve(NumTransforms);

				{
					const int32& RootBoneIndex = BoneIndices[0];
					const FTransform& BoneCSTransform = AnimationDataInCS[RootBoneIndex][Key];
					
					BoneTransforms[0] = FBoneTransform(RootBoneIndex, BoneCSTransform);
					Chain.Add(FABRIKChainLink(BoneCSTransform.GetLocation(), 0.0f, RootBoneIndex, 0));
				}


				for (int32 TransformIndex = 1; TransformIndex < NumTransforms; TransformIndex++)
				{
					const int32& BoneIndex = BoneIndices[TransformIndex];

					const FTransform& BoneCSTransform = AnimationDataInCS[BoneIndex][Key];
					FVector const BoneCSPosition = BoneCSTransform.GetLocation();

					BoneTransforms[TransformIndex] = FBoneTransform(BoneIndex, BoneCSTransform);

					float const BoneLength = FVector::Dist(BoneCSPosition, BoneTransforms[TransformIndex - 1].Transform.GetLocation());

					if (!FMath::IsNearlyZero(BoneLength))
					{
						Chain.Add(FABRIKChainLink(BoneCSPosition, BoneLength, BoneIndex, TransformIndex));
						MaximumReach += BoneLength;
					}
					else
					{
						FABRIKChainLink& ParentLink = Chain[Chain.Num() - 1];
						ParentLink.ChildZeroLengthTransformIndices.Add(TransformIndex);
					}
				}

				bool bBoneLocationUpdated = false;
				float const RootToTargetDistSq = FVector::DistSquared(Chain[0].Position, CSEffectorLocation);
				int32 const NumChainLinks = Chain.Num();

				if (RootToTargetDistSq > FMath::Square(MaximumReach))
				{
					for (int32 LinkIndex = 1; LinkIndex < NumChainLinks; LinkIndex++)
					{
						FABRIKChainLink const & ParentLink = Chain[LinkIndex - 1];
						FABRIKChainLink & CurrentLink = Chain[LinkIndex];
						CurrentLink.Position = ParentLink.Position + (CSEffectorLocation - ParentLink.Position).GetUnsafeNormal() * CurrentLink.Length;
					}
					bBoneLocationUpdated = true;
				}
				else
				{
					int const TipBoneLinkIndex = NumChainLinks - 1;

					float Slop = FVector::Dist(Chain[TipBoneLinkIndex].Position, CSEffectorLocation);
					if (Slop > Precision)
					{
						Chain[TipBoneLinkIndex].Position = CSEffectorLocation;

						int32 IterationCount = 0;
						while ((Slop > Precision) && (IterationCount++ < MaxIterations))
						{
							for (int32 LinkIndex = TipBoneLinkIndex - 1; LinkIndex > 0; LinkIndex--)
							{
								FABRIKChainLink & CurrentLink = Chain[LinkIndex];
								FABRIKChainLink const & ChildLink = Chain[LinkIndex + 1];

								CurrentLink.Position = ChildLink.Position + (CurrentLink.Position - ChildLink.Position).GetUnsafeNormal() * ChildLink.Length;
							}

							for (int32 LinkIndex = 1; LinkIndex < TipBoneLinkIndex; LinkIndex++)
							{
								FABRIKChainLink const & ParentLink = Chain[LinkIndex - 1];
								FABRIKChainLink & CurrentLink = Chain[LinkIndex];

								CurrentLink.Position = ParentLink.Position + (CurrentLink.Position - ParentLink.Position).GetUnsafeNormal() * CurrentLink.Length;
							}

							Slop = FMath::Abs(Chain[TipBoneLinkIndex].Length - FVector::Dist(Chain[TipBoneLinkIndex - 1].Position, CSEffectorLocation));
						}

						{
							FABRIKChainLink const & ParentLink = Chain[TipBoneLinkIndex - 1];
							FABRIKChainLink & CurrentLink = Chain[TipBoneLinkIndex];

							CurrentLink.Position = ParentLink.Position + (CurrentLink.Position - ParentLink.Position).GetUnsafeNormal() * CurrentLink.Length;
						}

						bBoneLocationUpdated = true;
					}
				}


				if (bBoneLocationUpdated)
				{
					for (int32 LinkIndex = 0; LinkIndex < NumChainLinks; LinkIndex++)
					{
						FABRIKChainLink const & ChainLink = Chain[LinkIndex];
						BoneTransforms[ChainLink.TransformIndex].Transform.SetTranslation(ChainLink.Position);

						int32 const NumChildren = ChainLink.ChildZeroLengthTransformIndices.Num();
						for (int32 ChildIndex = 0; ChildIndex < NumChildren; ChildIndex++)
						{
							BoneTransforms[ChainLink.ChildZeroLengthTransformIndices[ChildIndex]].Transform.SetTranslation(ChainLink.Position);
						}
					}

					for (int32 LinkIndex = 0; LinkIndex < NumChainLinks - 1; LinkIndex++)
					{
						FABRIKChainLink const & CurrentLink = Chain[LinkIndex];
						FABRIKChainLink const & ChildLink = Chain[LinkIndex + 1];

						FVector const OldDir = (GetCurrentLocation(AnimationDataInCS, ChildLink.BoneIndex, Key) - GetCurrentLocation(AnimationDataInCS, CurrentLink.BoneIndex, Key)).GetUnsafeNormal();
						FVector const NewDir = (ChildLink.Position - CurrentLink.Position).GetUnsafeNormal();

						FVector const RotationAxis = FVector::CrossProduct(OldDir, NewDir).GetSafeNormal();
						float const RotationAngle = FMath::Acos(FVector::DotProduct(OldDir, NewDir));
						FQuat const DeltaRotation = FQuat(RotationAxis, RotationAngle);

						checkSlow(DeltaRotation.IsNormalized());

						FTransform& CurrentBoneTransform = BoneTransforms[CurrentLink.TransformIndex].Transform;
						CurrentBoneTransform.SetRotation(DeltaRotation * CurrentBoneTransform.GetRotation());
						CurrentBoneTransform.NormalizeRotation();

						int32 const NumChildren = CurrentLink.ChildZeroLengthTransformIndices.Num();
						for (int32 ChildIndex = 0; ChildIndex < NumChildren; ChildIndex++)
						{
							FTransform& ChildBoneTransform = BoneTransforms[CurrentLink.ChildZeroLengthTransformIndices[ChildIndex]].Transform;
							ChildBoneTransform.SetRotation(DeltaRotation * ChildBoneTransform.GetRotation());
							ChildBoneTransform.NormalizeRotation();
						}
					}
				}

				int32 const TipBoneTransformIndex = BoneTransforms.Num() - 1;
				switch (EffectorRotationSource)
				{
				case BRS_KeepLocalSpaceRotation:
					BoneTransforms[TipBoneTransformIndex].Transform = AnimationDataInLocal[BoneIndices[TipBoneTransformIndex]][Key] * BoneTransforms[TipBoneTransformIndex - 1].Transform;
					break;
				case BRS_CopyFromTarget:
					BoneTransforms[TipBoneTransformIndex].Transform.SetRotation(CSEffectorTransform.GetRotation());
					break;
				case BRS_KeepComponentSpaceRotation:
					break;
				default:
					break;
				}


				for (int32 TransformIndex = 0; TransformIndex < BoneTransforms.Num(); TransformIndex++)
				{
					const FBoneTransform& BoneTransform = BoneTransforms[TransformIndex];

					AnimationDataInCS[BoneTransform.BoneIndex][Key] = BoneTransform.Transform;
					AnimationDataInLocal[BoneTransform.BoneIndex][Key] = BoneTransform.Transform;

					int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneTransform.BoneIndex);
					if (ParentBoneIndex != INDEX_NONE)
					{
						AnimationDataInLocal[BoneTransform.BoneIndex][Key].SetToRelativeTransform(AnimationDataInCS[ParentBoneIndex][Key]);
					}
				}
			}






			for (int32 BoneIndex : BoneIndices)
			{
				FName BoneName = RefSkeleton.GetBoneName(BoneIndex);

				if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false)
				{
					FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];
					FRawAnimSequenceTrack AnimTrack;
					AnimTrack.PosKeys.Add(FVector3f(LocalTransform.GetLocation()));
					AnimTrack.RotKeys.Add(FQuat4f(LocalTransform.GetRotation()));
					AnimTrack.ScaleKeys.Add(FVector3f(LocalTransform.GetScale3D()));

					AnimSequence->GetController().AddBoneCurve(BoneName);
					AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrack.PosKeys, AnimTrack.RotKeys, AnimTrack.ScaleKeys);

				}


				TArray< FVector > AnimTrackPosKeys;
				TArray< FQuat > AnimTrackRotKeys;
				TArray< FVector > AnimTrackScaleKeys;
				AnimTrackPosKeys.Empty(NumKeys);
				AnimTrackRotKeys.Empty(NumKeys);
				AnimTrackScaleKeys.Empty(NumKeys);

				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					FTransform AnimatedKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, Key, AnimSequence->Interpolation);

					if (Skeleton->GetBoneTranslationRetargetingMode(BoneIndex) == EBoneTranslationRetargetingMode::Skeleton)
					{
						AnimatedKey.SetLocation(RefSkeleton.GetRefBonePose()[BoneIndex].GetLocation());
					}


					if (Key >= StartFrame && Key <= (EndFrame + BlendOutFrames))
					{
						float IKAlpha = (BlendInFrames > 0) ? (float)(Key - StartFrame) / (float)BlendInFrames : 1.0f;
						if (Key >= EndFrame)
						{
							IKAlpha = (BlendOutFrames > 0) ?  1.0f - (float)(Key - EndFrame) / (float)BlendOutFrames : 0.0f;
						}

						AnimatedKey.BlendWith(AnimationDataInLocal[BoneIndex][Key], IKAlpha);
						AnimatedKey.NormalizeRotation();
					}


					AnimTrackPosKeys.Add(AnimatedKey.GetLocation());
					AnimTrackRotKeys.Add(AnimatedKey.GetRotation());
					AnimTrackScaleKeys.Add(AnimatedKey.GetScale3D());
				}

				AnimSequence->GetController().SetBoneTrackKeys(BoneName, AnimTrackPosKeys, AnimTrackRotKeys, AnimTrackScaleKeys);
			}

			AnimSequence->GetController().CloseBracket();
		}


		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}
	return FReply::Handled();
}

void UMakeFABRIKAnimation::ConvertBoneSpaceTransformToCS(const FReferenceSkeleton& RefSkeleton, TArray<TArray<FTransform>> AnimationDataInCS, FTransform & InOutBoneSpaceTM, int32 BoneIndex, int32 Key, EBoneControlSpace Space)
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

FVector UMakeFABRIKAnimation::GetCurrentLocation(const TArray<TArray<FTransform>>& AnimationDataInCS, const int32 & BoneIndex, const int32 & Key)
{
	return AnimationDataInCS[BoneIndex][Key].GetLocation();
}

#undef LOCTEXT_NAMESPACE