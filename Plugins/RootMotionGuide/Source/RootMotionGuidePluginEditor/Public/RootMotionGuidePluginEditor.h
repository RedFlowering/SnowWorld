// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FRootMotionGuidePluginEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/**
	* Open RootMotionExtractor window.
	*/
	void OpenRootMotionExtractor();

	/**
	* Open AdjustAnimationTrack window
	*/
	void OpenAdjustAnimationTrack();

	/**
	* Open RootMotionAdjustScale window
	*/
	void OpenAdjustRootMotionScale();

	/**
	* Open MakeLinearRootMotionHeight window
	*/
	void OpenMakeLinearRootMotionHeight();

	/**
	* Open ApplyFloorHeight window
	*/
	void OpenApplyFloorHeight();

	/**
	* Open AdjustHoldRootMotion window
	*/
	void OpenAdjustHoldRootMotion();

	/**
	* Open AddRootMotion window
	*/
	void OpenAddRootMotion();

	/**
	* Open BlendAnimationFirstPose window
	*/
	void OpenBlendAnimationFirstPose();

	/**
	* Open BlendAnimationLastPose window
	*/
	void OpenBlendAnimationLastPose();

	/**
	* Open AppendLandingAnimation window
	*/
	void OpenAppendLandingAnimation();

	/**
	* Open TrimAnimation window
	*/
	void OpenTrimAnimation();

	/**
	* Open AppendAnimation window
	*/
	void OpenAppendAnimation();

	/**
	* Open AdjustAnimationSpeed window
	*/
	void OpenAdjustAnimationSpeed();

	/**
	* Open MakeTwoBoneIKAnimation window
	*/
	void OpenMakeTwoBoneIKAnimation();

	/**
	* Open MakeFABRIKAnimation window
	*/
	void OpenMakeFABRIKAnimation();

	/**
	* Open MakeLookAtAnimation window
	*/
	void OpenMakeLookAtAnimation();

	/**
	* Open MakeMirrorAnimation window
	*/
	void OpenMakeMirrorAnimation();

	/**
	* Open SyncBoneLocation window
	*/
	void OpenSyncBoneLocation();

	/**
	* Open RootMotionReExtractor window.
	*/
	void OpenRootMotionReExtractor();

	/**
	* Open RotateAnimationTrack window
	*/
	void OpenRotateAnimationTrack();

	/**
	* Open RemoveAnimationTrack window
	*/
	void OpenRemoveAnimationTrack();

	/**
	* Open ReverseAnimation window
	*/
	void OpenReverseAnimation();

	/**
	* Open ResetRootBoneTransform window
	*/
	void OpenResetRootBoneTransform();

	class URootMotionReExtractor* GetRootMotionReExtractor() const { return RootMotionReExtractor; }
private:
	/**
	* Add menus to the toolbar
	*/
	void AddToolbarExtension(FToolBarBuilder& Builder);

	/**
	* Add menus
	*/
	void AddMenuExtension(FMenuBuilder& Builder);

	/**
	* Return RootMotionExtractorTab
	*/
	TSharedRef< class SDockTab > OnSpawnRootMotionExtractorTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return AdjustAnimationTrackTab
	*/
	TSharedRef< class SDockTab > OnSpawnAdjustAnimationTrackTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return RootMotionAdjustScaleTab
	*/
	TSharedRef< class SDockTab > OnSpawnAdjustRootMotionScaleTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return MakeLinearRootMotionHeightTab
	*/
	TSharedRef< class SDockTab > OnSpawnMakeLinearRootMotionHeightTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return ApplyFloorHeightTab
	*/
	TSharedRef< class SDockTab > OnSpawnApplyFloorHeightTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return AdjustHoldRootMotionTab
	*/
	TSharedRef< class SDockTab > OnSpawnAdjustHoldRootMotionTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return AddRootMotionTab
	*/
	TSharedRef< class SDockTab > OnSpawnAddRootMotionTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return BlendAnimationFirstPoseTab
	*/
	TSharedRef< class SDockTab > OnSpawnBlendAnimationFirstPoseTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return BlendAnimationLastPoseTab
	*/
	TSharedRef< class SDockTab > OnSpawnBlendAnimationLastPoseTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return AppendLandingAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnAppendLandingAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return TrimAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnTrimAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return AppendAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnAppendAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return AdjustAnimationSpeedTab
	*/
	TSharedRef< class SDockTab > OnSpawnAdjustAnimationSpeedTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return MakeTwoBoneIKAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnMakeTwoBoneIKAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return MakeFABRIKAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnMakeFABRIKAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return MakeLookAtAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnMakeLookAtAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return MakeMirrorAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnMakeMirrorAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return SyncBoneLocationTab
	*/
	TSharedRef< class SDockTab > OnSpawnSyncBoneLocationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return RootMotionReExtractorTab
	*/
	TSharedRef< class SDockTab > OnSpawnRootMotionReExtractorTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return RotateAnimationTrackTab
	*/
	TSharedRef< class SDockTab > OnSpawnRotateAnimationTrackTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return RemoveAnimationTrackTab
	*/
	TSharedRef< class SDockTab > OnSpawnRemoveAnimationTrackTab(const class FSpawnTabArgs& SpawnTabArgs);
	
	/**
	* Return ReverseAnimationTab
	*/
	TSharedRef< class SDockTab > OnSpawnReverseAnimationTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	* Return ResetRootBoneTransformTab
	*/
	TSharedRef< class SDockTab > OnSpawnResetRootBoneTransformTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	void FillSubMenu(FMenuBuilder& MenuBuilder) const;

	static const FName RootMotionExtractorTabName;
	static const FName AdjustAnimationTrackTabName;
	static const FName AdjustRootMotionScaleTabName;
	static const FName MakeLinearRootMotionHeightTabName;
	static const FName ApplyFloorHeightTabName;
	static const FName AdjustHoldRootMotionTabName;
	static const FName AddRootMotionTabName;
	static const FName BlendAnimationFirstPoseTabName;
	static const FName BlendAnimationLastPoseTabName;
	static const FName AppendLandingAnimationTabName;
	static const FName TrimAnimationTabName;
	static const FName AppendAnimationTabName;
	static const FName AdjustAnimationSpeedTabName;
	static const FName MakeTwoBoneIKAnimationTabName;
	static const FName MakeFABRIKAnimationTabName;
	static const FName MakeLookAtAnimationTabName;
	static const FName MakeMirrorAnimationTabName;
	static const FName SyncBoneLocationTabName;
	static const FName RootMotionReExtractorTabName;
	static const FName RotateAnimationTrackTabName;
	static const FName RemoveAnimationTrackTabName;
	static const FName ReverseAnimationTabName;
	static const FName ResetRootBoneTransformTabName;

	TSharedPtr< class FUICommandList > PluginCommands;

	TObjectPtr<class URootMotionExtractor> RootMotionExtractor;
	TObjectPtr<class UAdjustAnimationTrack> AdjustAnimationTrack;
	TObjectPtr<class URootMotionAdjustScale> AdjustRootMotionScale;
	TObjectPtr<class UMakeLinearRootMotionHeight> MakeLinearRootMotionHeight;
	TObjectPtr<class URootMotionApplyFloorHeight> ApplyFloorHeight;
	TObjectPtr<class UAdjustHoldRootMotion> AdjustHoldRootMotion;
	TObjectPtr<class UAddRootMotion> AddRootMotion;
	TObjectPtr<class UBlendAnimationFirstPose> BlendAnimationFirstPose;
	TObjectPtr<class UBlendAnimationLastPose> BlendAnimationLastPose;
	TObjectPtr<class UAppendLandingAnimation> AppendLandingAnimation;
	TObjectPtr<class UTrimAnimation> TrimAnimation;
	TObjectPtr<class UAppendAnimation> AppendAnimation;
	TObjectPtr<class UAdjustAnimationSpeed> AdjustAnimationSpeed;
	TObjectPtr<class UMakeTwoBoneIKAnimation> MakeTwoBoneIKAnimation;
	TObjectPtr<class UMakeFABRIKAnimation> MakeFABRIKAnimation;
	TObjectPtr<class UMakeLookAtAnimation> MakeLookAtAnimation;
	TObjectPtr<class UMakeMirrorAnimation> MakeMirrorAnimation;
	TObjectPtr<class USyncBoneLocation> SyncBoneLocation;
	TObjectPtr<class URootMotionReExtractor> RootMotionReExtractor;
	TObjectPtr<class URotateAnimationTrack> RotateAnimationTrack;
	TObjectPtr<class URemoveAnimationTrack> RemoveAnimationTrack;
	TObjectPtr<class UReverseAnimation> ReverseAnimation;
	TObjectPtr<class UResetRootBoneTransform> ResetRootBoneTransform;
};