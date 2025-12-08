// Copyright 2017 Lee Ju Sik

#include "RootMotionGuideTool.h"
//#include "SlateBasics.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSpacer.h"
#include "RootMotionGuide.h"
//#include "Engine.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "ScopedTransaction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AnimDataController.h"
#include "AnimationUtils.h"


#define LOCTEXT_NAMESPACE "RootMotionGuideTool"

TSharedRef<IDetailCustomization> FRootMotionGuideTool::MakeInstance()
{
	return MakeShareable(new FRootMotionGuideTool);
}

void FRootMotionGuideTool::CustomizeDetails(IDetailLayoutBuilder & DetailBuilder)
{
	// for 4.18 ~
	//if (DetailBuilder.GetDetailsView() == nullptr)
	//{
	//	return;
	//}

	TSharedPtr<IDetailsView> DetailsView = DetailBuilder.GetDetailsViewSharedPtr();
	if (!DetailsView.IsValid())
	{
		return;
	}


	//const TArray< TWeakObjectPtr< AActor > >& SelectedActors = DetailBuilder.GetDetailsView()->GetSelectedActors();
	const TArray< TWeakObjectPtr< AActor > >& SelectedActors = DetailsView->GetSelectedActors();
	if (SelectedActors.Num() != 1)
	{
		return;
	}

	if (ARootMotionGuide* TestActor = Cast<ARootMotionGuide>(SelectedActors[0].Get()))
	{
		RootMotionGuide = TestActor;
	}

	if (RootMotionGuide.IsValid() == false)
	{
		return;
	}

	TSharedPtr< IPropertyHandle > AnimSequencePropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARootMotionGuide, AnimSequence));
	if (DetailBuilder.IsPropertyVisible(AnimSequencePropertyHandle.ToSharedRef()) == false)
	{
		return;
	}


	IDetailCategoryBuilder& RootMotionGuideCategory = DetailBuilder.EditCategory("RootMotionGuide", FText::GetEmpty(), ECategoryPriority::Important);

	FDetailWidgetRow& RefreshAnimationDataRow = RootMotionGuideCategory.AddCustomRow(FText::GetEmpty());
	RefreshAnimationDataRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("refresh animation data", "Refresh Animation Data"))
	]
	.ValueContent()
	[
		SNew(SButton)
		.Text(LOCTEXT("refresh", "Refresh"))
		.OnClicked(this, &FRootMotionGuideTool::RefreshAnimationData)
	];

	FDetailWidgetRow& HideOtherArrowsRow = RootMotionGuideCategory.AddCustomRow(FText::GetEmpty());
	HideOtherArrowsRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("hide other arrows", "Hide Other Arrows"))
	]
	.ValueContent()
	[
		SNew(SButton)
		.Text(LOCTEXT("hide other arrows", "Hide Other Arrows"))
		.ToolTipText(LOCTEXT("hide other arrows tooltip", "Set ShowArrows of all other RootMotionGuides to false. (Used when the performance of the editor is slow.)"))
		.OnClicked(this, &FRootMotionGuideTool::HideOtherArrows)
	];
}

FReply FRootMotionGuideTool::RefreshAnimationData()
{
	if (RootMotionGuide.IsValid() == false)
	{
		return FReply::Handled();
	}

	//FScopedTransaction Transation(LOCTEXT("RefreshAnimationData", "RefreshAnimationData"));

	ARootMotionGuide* RootMotionGuideActor = RootMotionGuide.Get();

	UAnimSequence* AnimSequence = RootMotionGuide.Get()->AnimSequence;

	if (AnimSequence)
	{
		TArray< TArray<FTransform> > AnimationDataInComponentSpace;

		USkeleton* Skeleton = AnimSequence->GetSkeleton();
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();


		AnimationDataInComponentSpace.Empty(NumBones);
		AnimationDataInComponentSpace.AddZeroed(NumBones);

		int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();

		TArray< int32 > RequiredBoneIndexArray;
		RequiredBoneIndexArray.Add(0);

		for (int i = 0; i < RootMotionGuideActor->ExtractBoneArray.Num(); i++)
		{
			FName BoneName = RootMotionGuideActor->ExtractBoneArray[i].BoneName;
			int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);

			while (BoneIndex != INDEX_NONE)
			{
				RequiredBoneIndexArray.AddUnique(BoneIndex);

				BoneIndex = RefSkeleton.GetParentIndex(BoneIndex);
			}
		}


		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			if (RequiredBoneIndexArray.Contains(BoneIndex))
			{
				AnimationDataInComponentSpace[BoneIndex].Empty(NumKeys);
				AnimationDataInComponentSpace[BoneIndex].AddUninitialized(NumKeys);
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
					FTransform AnimatedLocalKey = FTransform::Identity;

					for (int32 key = 0; key < NumKeys; key++)
					{
						AnimatedLocalKey = AnimSequence->GetDataModel()->EvaluateBoneTrackTransform(BoneName, key, AnimSequence->Interpolation);

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

		RootMotionGuideActor->Modify();

		RootMotionGuideActor->AnimationData.Empty(RootMotionGuideActor->ExtractBoneArray.Num() + 1);

		TArray< FName > ExtractBoneNames;
		ExtractBoneNames.Empty(RootMotionGuideActor->ExtractBoneArray.Num());
		for (int i = 0; i < RootMotionGuideActor->ExtractBoneArray.Num(); i++)
		{
			ExtractBoneNames.Add(RootMotionGuideActor->ExtractBoneArray[i].BoneName);
		}

		
		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			if (BoneIndex == 0 || ExtractBoneNames.Contains(RefSkeleton.GetBoneName(BoneIndex)))
			{
				FRootMotionGuideAnimationTrack ExtractAnimationTrack;
				ExtractAnimationTrack.BoneName = RefSkeleton.GetBoneName(BoneIndex);
				ExtractAnimationTrack.ComponentSpaceAnimationTrack = AnimationDataInComponentSpace[BoneIndex];

				RootMotionGuideActor->AnimationData.Add(ExtractAnimationTrack);
			}
		}

		

		RootMotionGuideActor->AnimSequenceForAnimationData = AnimSequence;

		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}
	else
	{
		RootMotionGuideActor->Modify();

		RootMotionGuideActor->AnimationData.Empty(0);

		RootMotionGuideActor->AnimSequenceForAnimationData = nullptr;

		Notification(LOCTEXT("Success!!", "Success!!"), 4.0f, SNotificationItem::CS_Success);
	}


	if (RootMotionGuideActor->bEnableAdjustRootMotionScale)
	{
		RootMotionGuideActor->UpdateRootMotionScale();
	}


	RootMotionGuideActor->UpdateArrow();
	RootMotionGuideActor->UpdateSnapshot();

	return FReply::Handled();
}

FReply FRootMotionGuideTool::HideOtherArrows()
{
	if (RootMotionGuide.IsValid() == false)
	{
		return FReply::Handled();
	}

	ARootMotionGuide* RootMotionGuideActor = RootMotionGuide.Get();
	UWorld* World = RootMotionGuideActor->GetWorld();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, ARootMotionGuide::StaticClass(), FoundActors);

	bool bMyShowArrows = RootMotionGuideActor->bShowArrows;

	for (int32 i = 0; i < FoundActors.Num(); i++)
	{
		ARootMotionGuide* TestRootMotionGuide = Cast<ARootMotionGuide>(FoundActors[i]);
		if (IsValid(TestRootMotionGuide) && TestRootMotionGuide->bShowArrows)
		{
			TestRootMotionGuide->bShowArrows = false;
			TestRootMotionGuide->UpdateArrow();
		}
	}

	if (RootMotionGuideActor->bShowArrows != bMyShowArrows)
	{
		RootMotionGuideActor->bShowArrows = bMyShowArrows;
		RootMotionGuideActor->UpdateArrow();
	}


	return FReply::Handled();
}

void FRootMotionGuideTool::Notification(const FText NotificationText, float ExpireDuration, SNotificationItem::ECompletionState State)
{
	if (GIsEditor)
	{
		FNotificationInfo Info(NotificationText);
		Info.ExpireDuration = ExpireDuration;
		Info.bUseLargeFont = false;
		TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
		if (Notification.IsValid())
		{
			Notification->SetCompletionState(State);
		}
	}
}

#undef LOCTEXT_NAMESPACE