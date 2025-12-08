// Copyright 2017 Lee Ju Sik

#include "RootMotionGuideCommands.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FRootMotionGuidePluginEditorModule"

FRootMotionGuideCommands::FRootMotionGuideCommands()
	:TCommands<FRootMotionGuideCommands>(TEXT("RootMotionGuidePlugin"), NSLOCTEXT("Contexts", "RootMotionGuidePlugin", "RootMotionGuidePlugin"), NAME_None, FCoreStyle::Get().GetStyleSetName())
{
}

void FRootMotionGuideCommands::RegisterCommands()
{
	UI_COMMAND(OpenRootMotionExtractorWindow, "Root Motion Extractor", "Open Root Motion Extractor window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenAdjustAnimationTrackWindow, "Adjust Animation Track", "Open Adjust Animation Track window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenAdjustRootMotionScaleWindow, "Adjust Root Motion Scale", "Open Adjust Root Motion Scale window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenMakeLinearRootMotionHeightWindow, "Make Linear Root Motion Height", "Open Make Linear Root Motion Height window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenApplyFloorHeightWindow, "Apply Floor Height", "Open Apply Floor Height window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenAdjustHoldRootMotionWindow, "Adjust Hold Root Motion", "Open Adjust Hold Root Motion window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenAddRootMotionWindow, "Add Root Motion", "Open Add Root Motion window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenBlendAnimationFirstPoseWindow, "Blend Animation First Pose", "Open Blend Animation First Pose window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenBlendAnimationLastPoseWindow, "Blend Animation Last Pose", "Open Blend Animation Last Pose window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenAppendLandingAnimationWindow, "Append Landing Animation", "Open Append Landing Animation window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenTrimAnimationWindow, "Trim Animation", "Open Trim Animation window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenAppendAnimationWindow, "Append Animation", "Open Append Animation window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenAdjustAnimationSpeedWindow, "Adjust Animation Speed", "Open Adjust Animation Speed window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenMakeTwoBoneIKAnimationWindow, "Make Two Bone IK Animation", "Open Make Two Bone IK Animation window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenMakeFABRIKAnimationWindow, "Make FABRIK Animation", "Open Make FABRIK Animation window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenMakeLookAtAnimationWindow, "Make Look At Animation", "Open Make Look At Animation window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenMakeMirrorAnimationWindow, "Make Mirror Animation", "Open Make Mirror Animation window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenSyncBoneLocationWindow, "Sync Bone Location", "Open Sync Bone Location window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenRootMotionReExtractorWindow, "Root Motion ReExtractor", "Open Root Motion ReExtractor window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenRotateAnimationTrackWindow, "Rotate Animation Track", "Open Rotate Animation Track window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenRemoveAnimationTrackWindow, "Remove Animation Track", "Open Remove Animation Track window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenReverseAnimationWindow, "Reverse Animation", "Open Reverse Animation window", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(OpenResetRootBoneTransformWindow, "Reset Root Bone Transform", "Open Reset Root Bone Transform window", EUserInterfaceActionType::Button, FInputChord());
}




#undef LOCTEXT_NAMESPACE