// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"


class ROOTMOTIONGUIDEPLUGINEDITOR_API FRootMotionGuideCommands : public TCommands<FRootMotionGuideCommands>
{
public:
	FRootMotionGuideCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenRootMotionExtractorWindow;
	TSharedPtr< FUICommandInfo > OpenAdjustAnimationTrackWindow;
	TSharedPtr< FUICommandInfo > OpenAdjustRootMotionScaleWindow;
	TSharedPtr< FUICommandInfo > OpenMakeLinearRootMotionHeightWindow;
	TSharedPtr< FUICommandInfo > OpenApplyFloorHeightWindow;
	TSharedPtr< FUICommandInfo > OpenAdjustHoldRootMotionWindow;
	TSharedPtr< FUICommandInfo > OpenAddRootMotionWindow;
	TSharedPtr< FUICommandInfo > OpenBlendAnimationFirstPoseWindow;
	TSharedPtr< FUICommandInfo > OpenBlendAnimationLastPoseWindow;
	TSharedPtr< FUICommandInfo > OpenAppendLandingAnimationWindow;
	TSharedPtr< FUICommandInfo > OpenTrimAnimationWindow;
	TSharedPtr< FUICommandInfo > OpenAppendAnimationWindow;
	TSharedPtr< FUICommandInfo > OpenAdjustAnimationSpeedWindow;
	TSharedPtr< FUICommandInfo > OpenMakeTwoBoneIKAnimationWindow;
	TSharedPtr< FUICommandInfo > OpenMakeFABRIKAnimationWindow;
	TSharedPtr< FUICommandInfo > OpenMakeLookAtAnimationWindow;
	TSharedPtr< FUICommandInfo > OpenMakeMirrorAnimationWindow;
	TSharedPtr< FUICommandInfo > OpenSyncBoneLocationWindow;
	TSharedPtr< FUICommandInfo > OpenRootMotionReExtractorWindow;
	TSharedPtr< FUICommandInfo > OpenRotateAnimationTrackWindow;
	TSharedPtr< FUICommandInfo > OpenRemoveAnimationTrackWindow;
	TSharedPtr< FUICommandInfo > OpenReverseAnimationWindow;

	TSharedPtr< FUICommandInfo > OpenResetRootBoneTransformWindow;
};
