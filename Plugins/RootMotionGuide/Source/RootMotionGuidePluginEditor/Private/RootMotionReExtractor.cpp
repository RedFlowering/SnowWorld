// Copyright 2017 Lee Ju Sik

#include "RootMotionReExtractor.h"
#include "RootMotionGuidePluginEditor.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "EditorStyleSet.h"
#include "ReferenceSkeleton.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"
#include "AssetRegistry/AssetData.h"

#define LOCTEXT_NAMESPACE "RootMotionReExtractor"



FDelegateHandle URootMotionReExtractor::ContentBrowserExtenderDelegateHandle;  // the static handle

URootMotionReExtractor::URootMotionReExtractor()
{
	RootMotionSourceBoneName = FName(TEXT("pelvis"));

	//RootMotionScale = 1.0f;

	RootMotionSourceBoneForwardAxis = ERootMotionReExtractorAxisType::Y;
}


void URootMotionReExtractor::AddContextMenuItem()  // this allows you to select multiple anim sequences and add root motion to them all
{
	// Add Extract Root Motion menu item to Anim Sequences
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateLambda([](const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender = MakeShared<FExtender>();

		if (SelectedAssets.ContainsByPredicate([](const FAssetData& AssetData) { return AssetData.GetClass() == UAnimSequence::StaticClass(); }))
		{
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda([SelectedAssets](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
					NSLOCTEXT("AssetTypeActions_AnimSequence", "ObjectContext_ReExtractRootMotion", "ReExtract Root Motion"),
					NSLOCTEXT("AssetTypeActions_AnimSequence", "ObjectContext_ReExtractRootMotionTooltip", "Extracts the root motion for this sequence."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DestructibleComponent"),
					FUIAction(FExecuteAction::CreateStatic(&URootMotionReExtractor::ExecuteExtractRootMotion, SelectedAssets), FCanExecuteAction()));
			}
			));
		}

		return Extender;
	}));

	ContentBrowserExtenderDelegateHandle = ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Last().GetHandle();
}

void URootMotionReExtractor::RemoveContextMenuItem()
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
		ContentBrowserExtenderDelegateHandle.Reset();
	}
}

void URootMotionReExtractor::ExecuteExtractRootMotion(TArray<FAssetData> AssetDataArray)
{
	// find the plugin module
	FRootMotionGuidePluginEditorModule& RootMotionGuidePluginEditorModule = FModuleManager::LoadModuleChecked<FRootMotionGuidePluginEditorModule>(TEXT("RootMotionGuidePluginEditor"));

	// find the extractor object
	URootMotionReExtractor* RootMotionReExtractor = RootMotionGuidePluginEditorModule.GetRootMotionReExtractor();
	if (!RootMotionReExtractor)
	{
		RootMotionGuidePluginEditorModule.OpenRootMotionReExtractor();  // open it if it's not already
		RootMotionReExtractor = RootMotionGuidePluginEditorModule.GetRootMotionReExtractor();
		if (!RootMotionReExtractor)
			return;
	}

	ERootMotionReExtractorTargetRotationType SaveTargetRotationType = RootMotionReExtractor->TargetRotationType;
	bool SaveForceRootMotionExtraction = RootMotionReExtractor->bForceRootMotionExtraction;

	// for doing multiple, need to use these default settings
	RootMotionReExtractor->TargetRotationType = ERootMotionReExtractorTargetRotationType::SourceRotation;
	RootMotionReExtractor->bForceRootMotionExtraction = true;

	// extract each selected anim sequence
	for (const FAssetData& AssetData : AssetDataArray)
	{
		UAnimSequence* AnimSequence = Cast<UAnimSequence>(AssetData.GetAsset());
		if (!AnimSequence)
			continue;

		RootMotionReExtractor->AnimSequence = AnimSequence;
		RootMotionReExtractor->ExtractRootMotionFromSource();
	}

	// put stuff back
	RootMotionReExtractor->AnimSequence = nullptr;
	RootMotionReExtractor->TargetRotationType = SaveTargetRotationType;
	RootMotionReExtractor->bForceRootMotionExtraction = SaveForceRootMotionExtraction;
}

bool URootMotionReExtractor::NeedToAddRootBoneAnimationTrack() const
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

EVisibility URootMotionReExtractor::GetVisibilityAddRootBoneAnimationTrackButton() const
{
	return NeedToAddRootBoneAnimationTrack() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility URootMotionReExtractor::GetVisibilityExtractRootMotionButton() const
{
	return NeedToAddRootBoneAnimationTrack() ? EVisibility::Collapsed : EVisibility::Visible;
}

FReply URootMotionReExtractor::AddRootBoneAnimationTrack()
{
	if (AnimSequence)
	{
		USkeleton* Skeleton = AnimSequence->GetSkeleton();
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();

		int32 RootBoneIndex = 0;
		FName BoneName = RefSkeleton.GetBoneName(RootBoneIndex);

		if (AnimSequence->GetDataModel()->IsValidBoneTrackName(BoneName) == false )
		{
			FScopedTransaction Transation(LOCTEXT("AddRootBoneAnimationTrack", "AddRootBoneAnimationTrack"));
			AnimSequence->Modify();
			AnimSequence->GetController().OpenBracket(FText::FromString(TEXT("AddRootBoneAnimationTrack")));

			//Notification(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"), 8.0f, SNotificationItem::CS_Fail);

			TArray<FName> TrackNames;// = AnimSequence->GetAnimationTrackNames();
			TrackNames.Empty();
			AnimSequence->GetDataModel()->GetBoneTrackNames(TrackNames);

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


FReply URootMotionReExtractor::ExtractRootMotionFromSource()
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
				
				if (bForceRootMotionExtraction)
				{
					// to fix earlier bad root extraction, read root too, pelvis world is correct, but root and pelvis local are both wrong
					FTransform AnimatedRootKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, 0, AnimSequence->Interpolation);
					AnimatedLocalKey = AnimatedLocalKey * AnimatedRootKey;  // add the parent bone on too
				}

				RootMotionSourceStartPos = AnimatedLocalKey.GetTranslation();
				RootMotionSourceStartRot = GetRootRotation(AnimatedLocalKey.GetRotation(), RootMotionSourceBoneForwardAxis);

				RootMotionStartRot = FRotator::ZeroRotator;

				AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, NumKeys - 1, AnimSequence->Interpolation);
				RootMotionSourceEndPos = AnimatedLocalKey.GetTranslation();
				RootMotionEndRot = GetRootRotation(AnimatedLocalKey.GetRotation(), RootMotionSourceBoneForwardAxis) - RootMotionSourceStartRot;
			}

			int32 RotSampleInterval = RootMotionSamplingInterval;

			for (int32 key = 0; key < NumKeys; key++)
			{
				float RootMotionRotAlpha = (float)key / (float)NumKeys;
				FTransform AnimatedLocalKey = FTransform::Identity;

				if (RotSampleInterval != 0)
				{
					int32 before = RotSampleInterval / 2;
					int32 after = RotSampleInterval - before;

					int32 start = FMath::Max(key - before, 0);
					int32 end = FMath::Min(key + after, NumKeys);

					FTransform TotalTransform(FQuat(0.0f, 0.0f, 0.0f, 0.0f), FVector(0.0f), FVector(0.0f));
					float subAlpha = 1.0f / (float)(end - start);

					for (int32 subKey = start; subKey < end; subKey++)
					{
						FTransform SubAnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, subKey, AnimSequence->Interpolation);
						if (bForceRootMotionExtraction)
						{
							FTransform AnimatedRootKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, subKey, AnimSequence->Interpolation);
							SubAnimatedLocalKey = SubAnimatedLocalKey * AnimatedRootKey;  // add the parent bone on too
						}
						TotalTransform += SubAnimatedLocalKey;
					}
					TotalTransform *= ScalarRegister(subAlpha);
					FVector RootMotionPos = TotalTransform.GetTranslation();
					FQuat RootMotionQuat = TotalTransform.GetRotation();
					RootMotionQuat.Normalize();
					AnimatedLocalKey.SetTranslation(RootMotionPos);
					AnimatedLocalKey.SetRotation(RootMotionQuat);
				}
				else 
				{
					AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, key, AnimSequence->Interpolation);

					if (bForceRootMotionExtraction)
					{
						// to fix earlier bad root extraction, read root too, pelvis world is correct, but root and pelvis local are both wrong
						FTransform AnimatedRootKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootBoneName, key, AnimSequence->Interpolation);
						AnimatedLocalKey = AnimatedLocalKey * AnimatedRootKey;  // add the parent bone on too
					}
				}


				float KeyFrameAlpha = 0.0f;
				if (NumKeys > 1)
				{
					KeyFrameAlpha = (float)key / (float)(NumKeys - 1);
				}

				float RootMotionSourceHeight = FMath::InterpEaseInOut(RootMotionSourceHeightAtStart, RootMotionSourceHeightAtEnd, KeyFrameAlpha, 2.0f);
				float RootMotionRotationOffset = FMath::InterpEaseInOut(0.0f, RootMotionTargetRotationAtEnd, KeyFrameAlpha, 2.0f);

				// The RootMotionSourceHeight from the dialog is only used as an offset, use the starting transform to determine the base height.
				FVector RootPos = AnimatedLocalKey.GetTranslation();
				RootPos.Z += RootMotionSourceHeight - RootMotionSourceStartPos.Z;

				FRotator RootRot = FMath::Lerp(RootMotionStartRot, RootMotionEndRot, RootMotionRotAlpha);

				if (TargetRotationType == ERootMotionReExtractorTargetRotationType::Offset)
				{
					RootRot.Yaw += RootMotionRotationOffset;
				}
				else if (TargetRotationType == ERootMotionReExtractorTargetRotationType::Absolute)
				{
					RootRot.Yaw = FMath::InterpEaseInOut(RootRot.Yaw, (double)RootMotionTargetRotationAtEnd, KeyFrameAlpha, 2.0f);
				}
				else if (TargetRotationType == ERootMotionReExtractorTargetRotationType::Override)
				{
					RootRot.Yaw = FMath::Lerp(0.0f, (double)RootMotionTargetRotationAtEnd, KeyFrameAlpha);
				}
				else if (TargetRotationType == ERootMotionReExtractorTargetRotationType::SourceRotation)
				{
					RootRot = GetRootRotation(AnimatedLocalKey.GetRotation(), RootMotionSourceBoneForwardAxis);
					RootRot.Yaw += RootMotionTargetRotationAtEnd;
				}

				// set the root bone to sit under the source bone of the model
				RootTrackPosKeys.Add(RootPos);
				RootTrackRotKeys.Add(RootRot.Quaternion());
				RootTrackScaleKeys.Add(FVector::OneVector);
				FTransform RootTransform(RootRot, RootPos, FVector(1.0f));


				// set the source bone with the difference that's left
				FTransform RootSourceTransform = AnimatedLocalKey * RootTransform.Inverse();  // we should have the same total transformation as when we started, even if we changed existing root motion
				FVector RootSourcePos = RootSourceTransform.GetTranslation();
				FQuat RootSourceRot = RootSourceTransform.GetRotation();

				RootMotionSourceTrackPosKeys.Add(RootSourcePos);
				RootMotionSourceTrackRotKeys.Add(RootSourceRot);
				AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(RootMotionSourceBoneName, key, AnimSequence->Interpolation);
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

bool URootMotionReExtractor::CanExtractRootMotion() const
{
	if (IsValid(AnimSequence))
	{
		return true;
	}

	return false;
}

FRotator URootMotionReExtractor::GetRootRotation(const FQuat & RootSourceRot, ERootMotionReExtractorAxisType ForwardAxis)
{
	FVector Forward = RootSourceRot.GetAxisY();
	switch (ForwardAxis)
	{
	case ERootMotionReExtractorAxisType::X:
		Forward = RootSourceRot.GetAxisX();
		break;
	case ERootMotionReExtractorAxisType::Y:
		Forward = RootSourceRot.GetAxisY();
		break;
	case ERootMotionReExtractorAxisType::Z:
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