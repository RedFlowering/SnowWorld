// Copyright 2017 Lee Ju Sik

#include "RootMotionGuidePluginEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "RootMotionGuide.h"
#include "RootMotionGuideTool.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDetailsView.h"
#include "RootMotionGuideCommands.h"
#include "LevelEditor.h"
#include "RootMotionExtractor.h"
//#include "SlateBasics.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSpacer.h"
#include "AdjustAnimationTrack.h"
#include "RootMotionAdjustScale.h"
#include "IAnimationEditorModule.h"
#include "MakeLinearRootMotionHeight.h"
#include "RootMotionApplyFloorHeight.h"
#include "AdjustHoldRootMotion.h"
#include "AddRootMotion.h"
#include "BlendAnimationFirstPose.h"
#include "BlendAnimationLastPose.h"
#include "AppendLandingAnimation.h"
#include "TrimAnimation.h"
#include "AppendAnimation.h"
#include "AdjustAnimationSpeed.h"
#include "MakeTwoBoneIKAnimation.h"
#include "MakeFABRIKAnimation.h"
#include "MakeLookAtAnimation.h"
#include "MakeMirrorAnimation.h"
#include "SyncBoneLocation.h"
#include "RootMotionReExtractor.h"
#include "RotateAnimationTrack.h"
#include "RemoveAnimationTrack.h"
#include "ReverseAnimation.h"
#include "ResetRootBoneTransform.h"


const FName FRootMotionGuidePluginEditorModule::RootMotionExtractorTabName("RootMotionExtractor");
const FName FRootMotionGuidePluginEditorModule::AdjustAnimationTrackTabName("AdjustAnimationTrack");
const FName FRootMotionGuidePluginEditorModule::AdjustRootMotionScaleTabName("AdjustRootMotionScale");
const FName FRootMotionGuidePluginEditorModule::MakeLinearRootMotionHeightTabName("MakeLinearRootMotionHeight");
const FName FRootMotionGuidePluginEditorModule::ApplyFloorHeightTabName("ApplyFloorHeight");
const FName FRootMotionGuidePluginEditorModule::AdjustHoldRootMotionTabName("AdjustHoldRootMotion");
const FName FRootMotionGuidePluginEditorModule::AddRootMotionTabName("AddRootMotion");
const FName FRootMotionGuidePluginEditorModule::BlendAnimationFirstPoseTabName("BlendAnimationFirstPose");
const FName FRootMotionGuidePluginEditorModule::BlendAnimationLastPoseTabName("BlendAnimationLastPose");
const FName FRootMotionGuidePluginEditorModule::AppendLandingAnimationTabName("AppendLandingAnimation");
const FName FRootMotionGuidePluginEditorModule::TrimAnimationTabName("TrimAnimationTabName");
const FName FRootMotionGuidePluginEditorModule::AppendAnimationTabName("AppendAnimationTabName");
const FName FRootMotionGuidePluginEditorModule::AdjustAnimationSpeedTabName("AdjustAnimationSpeedTabName");
const FName FRootMotionGuidePluginEditorModule::MakeTwoBoneIKAnimationTabName("MakeTwoBoneIKAnimationTabName");
const FName FRootMotionGuidePluginEditorModule::MakeFABRIKAnimationTabName("MakeFABRIKAnimationTabName");
const FName FRootMotionGuidePluginEditorModule::MakeLookAtAnimationTabName("MakeLookAtAnimationTabName");
const FName FRootMotionGuidePluginEditorModule::MakeMirrorAnimationTabName("MakeMirrorAnimationTabName");
const FName FRootMotionGuidePluginEditorModule::SyncBoneLocationTabName("SyncBoneLocationTabName");
const FName FRootMotionGuidePluginEditorModule::RootMotionReExtractorTabName("RootMotionReExtractor");
const FName FRootMotionGuidePluginEditorModule::RotateAnimationTrackTabName("RotateAnimationTrackTabName");
const FName FRootMotionGuidePluginEditorModule::RemoveAnimationTrackTabName("RemoveAnimationTrackTabName");
const FName FRootMotionGuidePluginEditorModule::ReverseAnimationTabName("ReverseAnimationTabName");
const FName FRootMotionGuidePluginEditorModule::ResetRootBoneTransformTabName("ResetRootBoneTransformTabName");

#define LOCTEXT_NAMESPACE "FRootMotionGuidePluginEditorModule"

void FRootMotionGuidePluginEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FRootMotionGuideCommands::Register();


	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenRootMotionExtractorWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenRootMotionExtractor),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenAdjustAnimationTrackWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenAdjustAnimationTrack),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenAdjustRootMotionScaleWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenAdjustRootMotionScale),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenMakeLinearRootMotionHeightWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenMakeLinearRootMotionHeight),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenApplyFloorHeightWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenApplyFloorHeight),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenAdjustHoldRootMotionWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenAdjustHoldRootMotion),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenAddRootMotionWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenAddRootMotion),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenBlendAnimationFirstPoseWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenBlendAnimationFirstPose),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenBlendAnimationLastPoseWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenBlendAnimationLastPose),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenAppendLandingAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenAppendLandingAnimation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenTrimAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenTrimAnimation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenAppendAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenAppendAnimation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenAdjustAnimationSpeedWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenAdjustAnimationSpeed),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenMakeTwoBoneIKAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenMakeTwoBoneIKAnimation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenMakeFABRIKAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenMakeFABRIKAnimation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenMakeLookAtAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenMakeLookAtAnimation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenMakeMirrorAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenMakeMirrorAnimation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenResetRootBoneTransformWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenResetRootBoneTransform),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenSyncBoneLocationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenSyncBoneLocation),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenRootMotionReExtractorWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenRootMotionReExtractor),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenRotateAnimationTrackWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenRotateAnimationTrack),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenRemoveAnimationTrackWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenRemoveAnimationTrack),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRootMotionGuideCommands::Get().OpenReverseAnimationWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OpenReverseAnimation),
		FCanExecuteAction());


	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	IAnimationEditorModule& AnimationEditorModule = FModuleManager::LoadModuleChecked<IAnimationEditorModule>("AnimationEditor");

	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FRootMotionGuidePluginEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		AnimationEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	//{
	//	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender());
	//	ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FRootMotionGuidePluginEditorModule::AddToolbarExtension));
	//	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FRootMotionGuidePluginEditorModule::AddToolbarExtension));
	//	
	//	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	//	AnimationEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	//}

	

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RootMotionExtractorTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnRootMotionExtractorTab))
		.SetDisplayName(LOCTEXT("RootMotionExtractorTabTitle", "RootMotionExtractor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AdjustAnimationTrackTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnAdjustAnimationTrackTab))
		.SetDisplayName(LOCTEXT("AdjustAnimationTrackTabTitle", "AdjustAnimationTrack"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AdjustRootMotionScaleTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnAdjustRootMotionScaleTab))
		.SetDisplayName(LOCTEXT("AdjustRootMotionScaleTabTitle", "AdjustRootMotionScale"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MakeLinearRootMotionHeightTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnMakeLinearRootMotionHeightTab))
		.SetDisplayName(LOCTEXT("MakeLinearRootMotionHeightTabTitle", "MakeLinearRootMotionHeight"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ApplyFloorHeightTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnApplyFloorHeightTab))
		.SetDisplayName(LOCTEXT("ApplyFloorHeightTabTitle", "ApplyFloorHeight"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AdjustHoldRootMotionTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnAdjustHoldRootMotionTab))
		.SetDisplayName(LOCTEXT("AdjustHoldRootMotionTabTitle", "AdjustHoldRootMotion"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AddRootMotionTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnAddRootMotionTab))
		.SetDisplayName(LOCTEXT("AddRootMotionTabTitle", "AddRootMotion"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(BlendAnimationFirstPoseTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnBlendAnimationFirstPoseTab))
		.SetDisplayName(LOCTEXT("BlendAnimationFirstPoseTabTitle", "BlendAnimationFirstPose"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(BlendAnimationLastPoseTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnBlendAnimationLastPoseTab))
		.SetDisplayName(LOCTEXT("BlendAnimationLastPoseTabTitle", "BlendAnimationLastPose"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AppendLandingAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnAppendLandingAnimationTab))
		.SetDisplayName(LOCTEXT("AppendLandingAnimationTabTitle", "AppendLandingAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TrimAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnTrimAnimationTab))
		.SetDisplayName(LOCTEXT("TrimAnimationTabTitle", "TrimAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AppendAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnAppendAnimationTab))
		.SetDisplayName(LOCTEXT("AppendAnimationTabTitle", "AppendAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AdjustAnimationSpeedTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnAdjustAnimationSpeedTab))
		.SetDisplayName(LOCTEXT("AdjustAnimationSpeedTabTitle", "AdjustAnimationSpeed"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MakeTwoBoneIKAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnMakeTwoBoneIKAnimationTab))
		.SetDisplayName(LOCTEXT("MakeTwoBoneIKAnimationTabTitle", "MakeTwoBoneIKAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MakeFABRIKAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnMakeFABRIKAnimationTab))
		.SetDisplayName(LOCTEXT("MakeFABRIKAnimationTabTitle", "MakeFABRIKAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MakeLookAtAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnMakeLookAtAnimationTab))
		.SetDisplayName(LOCTEXT("MakeLookAtAnimationTabTitle", "MakeLookAtAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MakeMirrorAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnMakeMirrorAnimationTab))
		.SetDisplayName(LOCTEXT("MakeMirrorAnimationTabTitle", "MakeMirrorAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ResetRootBoneTransformTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnResetRootBoneTransformTab))
		.SetDisplayName(LOCTEXT("ResetRootBoneTransformTabTitle", "ResetRootBoneTransform"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SyncBoneLocationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnSyncBoneLocationTab))
		.SetDisplayName(LOCTEXT("SyncBoneLocationTabTitle", "SyncBoneLocation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RootMotionReExtractorTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnRootMotionReExtractorTab))
		.SetDisplayName(LOCTEXT("RootMotionReExtractorTabTitle", "RootMotionReExtractor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RotateAnimationTrackTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnRotateAnimationTrackTab))
		.SetDisplayName(LOCTEXT("RotateAnimationTrackTabTitle", "RotateAnimationTrack"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RemoveAnimationTrackTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnRemoveAnimationTrackTab))
		.SetDisplayName(LOCTEXT("RemoveAnimationTrackTabTitle", "RemoveAnimationTrack"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ReverseAnimationTabName, FOnSpawnTab::CreateRaw(this, &FRootMotionGuidePluginEditorModule::OnSpawnReverseAnimationTab))
		.SetDisplayName(LOCTEXT("ReverseAnimationTabTitle", "ReverseAnimation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(ARootMotionGuide::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FRootMotionGuideTool::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	// Add context menu handler for Anim Sequence->Extract Root Motion
	URootMotionReExtractor::AddContextMenuItem();
}

void FRootMotionGuidePluginEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.


	FRootMotionGuideCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RootMotionExtractorTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AdjustAnimationTrackTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AdjustRootMotionScaleTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MakeLinearRootMotionHeightTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ApplyFloorHeightTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AdjustHoldRootMotionTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AddRootMotionTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BlendAnimationFirstPoseTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BlendAnimationLastPoseTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AppendLandingAnimationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TrimAnimationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AppendAnimationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AdjustAnimationSpeedTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MakeTwoBoneIKAnimationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MakeFABRIKAnimationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MakeLookAtAnimationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MakeMirrorAnimationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ResetRootBoneTransformTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SyncBoneLocationTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RootMotionReExtractorTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RotateAnimationTrackTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RemoveAnimationTrackTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ReverseAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenRootMotionExtractor()
{
	FGlobalTabmanager::Get()->TryInvokeTab(RootMotionExtractorTabName);
}

void FRootMotionGuidePluginEditorModule::OpenAdjustAnimationTrack()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AdjustAnimationTrackTabName);
}

void FRootMotionGuidePluginEditorModule::OpenAdjustRootMotionScale()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AdjustRootMotionScaleTabName);
}

void FRootMotionGuidePluginEditorModule::OpenMakeLinearRootMotionHeight()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MakeLinearRootMotionHeightTabName);
}

void FRootMotionGuidePluginEditorModule::OpenApplyFloorHeight()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ApplyFloorHeightTabName);
}

void FRootMotionGuidePluginEditorModule::OpenAdjustHoldRootMotion()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AdjustHoldRootMotionTabName);
}

void FRootMotionGuidePluginEditorModule::OpenAddRootMotion()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AddRootMotionTabName);
}

void FRootMotionGuidePluginEditorModule::OpenBlendAnimationFirstPose()
{
	FGlobalTabmanager::Get()->TryInvokeTab(BlendAnimationFirstPoseTabName);
}

void FRootMotionGuidePluginEditorModule::OpenBlendAnimationLastPose()
{
	FGlobalTabmanager::Get()->TryInvokeTab(BlendAnimationLastPoseTabName);
}

void FRootMotionGuidePluginEditorModule::OpenAppendLandingAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AppendLandingAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenTrimAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(TrimAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenAppendAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AppendAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenAdjustAnimationSpeed()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AdjustAnimationSpeedTabName);
}

void FRootMotionGuidePluginEditorModule::OpenMakeTwoBoneIKAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MakeTwoBoneIKAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenMakeFABRIKAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MakeFABRIKAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenMakeLookAtAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MakeLookAtAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenMakeMirrorAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MakeMirrorAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenSyncBoneLocation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SyncBoneLocationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenRootMotionReExtractor()
{
	FGlobalTabmanager::Get()->TryInvokeTab(RootMotionReExtractorTabName);
}

void FRootMotionGuidePluginEditorModule::OpenRotateAnimationTrack()
{
	FGlobalTabmanager::Get()->TryInvokeTab(RotateAnimationTrackTabName);
}

void FRootMotionGuidePluginEditorModule::OpenRemoveAnimationTrack()
{
	FGlobalTabmanager::Get()->TryInvokeTab(RemoveAnimationTrackTabName);
}

void FRootMotionGuidePluginEditorModule::OpenReverseAnimation()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ReverseAnimationTabName);
}

void FRootMotionGuidePluginEditorModule::OpenResetRootBoneTransform()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ResetRootBoneTransformTabName);
}

void FRootMotionGuidePluginEditorModule::AddToolbarExtension(FToolBarBuilder & Builder)
{
	//Builder.AddSeparator();
	Builder.BeginSection(FName(TEXT("RootMotionGuide")));
	Builder.AddToolBarButton(FRootMotionGuideCommands::Get().OpenRootMotionExtractorWindow);
	Builder.AddToolBarButton(FRootMotionGuideCommands::Get().OpenAdjustAnimationTrackWindow);
	Builder.AddToolBarButton(FRootMotionGuideCommands::Get().OpenAdjustRootMotionScaleWindow);
	Builder.EndSection();
}

void FRootMotionGuidePluginEditorModule::AddMenuExtension(FMenuBuilder & Builder)
{
	Builder.BeginSection(FName(), LOCTEXT("RootMotionGuide", "RootMotionGuide"));
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenRootMotionExtractorWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustAnimationTrackWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustRootMotionScaleWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeLinearRootMotionHeightWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustHoldRootMotionWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAddRootMotionWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenBlendAnimationLastPoseWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAppendLandingAnimationWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenTrimAnimationWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAppendAnimationWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustAnimationSpeedWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeTwoBoneIKAnimationWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeFABRIKAnimationWindow);
	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeLookAtAnimationWindow);

	//Builder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenResetRootBoneTransformWindow);
	
	Builder.AddSubMenu(
		LOCTEXT("RootMotionGuide Tools", "RootMotionGuide Tools"), 
		LOCTEXT("RootMotionGuide Tools", "RootMotionGuide Tools"), 
		FNewMenuDelegate::CreateRaw(this, &FRootMotionGuidePluginEditorModule::FillSubMenu), 
		false);
	
	Builder.EndSection();
}

void FRootMotionGuidePluginEditorModule::FillSubMenu(FMenuBuilder& MenuBuilder) const
{
	MenuBuilder.BeginSection("Animation Tool", LOCTEXT("Animation Tool", "Animation Tool"));

	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenRootMotionExtractorWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustAnimationTrackWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustRootMotionScaleWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeLinearRootMotionHeightWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenApplyFloorHeightWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustHoldRootMotionWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAddRootMotionWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenBlendAnimationFirstPoseWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenBlendAnimationLastPoseWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAppendLandingAnimationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenTrimAnimationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAppendAnimationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenAdjustAnimationSpeedWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeTwoBoneIKAnimationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeFABRIKAnimationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeLookAtAnimationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenMakeMirrorAnimationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenSyncBoneLocationWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenRootMotionReExtractorWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenRotateAnimationTrackWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenRemoveAnimationTrackWindow);
	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenReverseAnimationWindow);

	MenuBuilder.EndSection();


	MenuBuilder.BeginSection("Skeleton Tool", LOCTEXT("Skeleton Tool", "Skeleton Tool"));

	MenuBuilder.AddMenuEntry(FRootMotionGuideCommands::Get().OpenResetRootBoneTransformWindow);

	MenuBuilder.EndSection();
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnRootMotionExtractorTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (RootMotionExtractor == nullptr)
	{
		RootMotionExtractor = NewObject<URootMotionExtractor>();
		RootMotionExtractor->AddToRoot();
	}

	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(RootMotionExtractor);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SVerticalBox)
				.Visibility_UObject(RootMotionExtractor.Get(), &URootMotionExtractor::GetVisibilityAddRootBoneAnimationTrackButton)
				+ SVerticalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"))
					.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(RootMotionExtractor.Get(), &URootMotionExtractor::AddRootBoneAnimationTrack)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("Add Root Bone Animation Track", "Add Root Bone Animation Track"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				//.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Visibility_UObject(RootMotionExtractor.Get(), &URootMotionExtractor::GetVisibilityExtractRootMotionButton)
				.IsEnabled_UObject(RootMotionExtractor.Get(), &URootMotionExtractor::CanExtractRootMotion)
				.OnClicked_UObject(RootMotionExtractor.Get(), &URootMotionExtractor::ExtractRootMotionFromSource)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("Extract", "Extract"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnAdjustAnimationTrackTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (AdjustAnimationTrack == nullptr)
	{
		AdjustAnimationTrack = NewObject<UAdjustAnimationTrack>();
		AdjustAnimationTrack->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(AdjustAnimationTrack);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(AdjustAnimationTrack.Get(), &UAdjustAnimationTrack::AdjustAnimationTrack)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("AdjustAnimationTrack", "Adjust Animation Track"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnAdjustRootMotionScaleTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (AdjustRootMotionScale == nullptr)
	{
		AdjustRootMotionScale = NewObject<URootMotionAdjustScale>();
		AdjustRootMotionScale->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(AdjustRootMotionScale);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(AdjustRootMotionScale.Get(), &URootMotionAdjustScale::AdjustRootMotionScale)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("AdjustRootMotionScale", "Adjust RootMotion Scale"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnMakeLinearRootMotionHeightTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (MakeLinearRootMotionHeight == nullptr)
	{
		MakeLinearRootMotionHeight = NewObject<UMakeLinearRootMotionHeight>();
		MakeLinearRootMotionHeight->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(MakeLinearRootMotionHeight);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(MakeLinearRootMotionHeight.Get(), &UMakeLinearRootMotionHeight::MakeLinearRootMotionHeight)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("MakeLinearRootMotionHeight", "Make Linear Root Motion Height"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnApplyFloorHeightTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if (ApplyFloorHeight == nullptr)
	{
		ApplyFloorHeight = NewObject<URootMotionApplyFloorHeight>();
		ApplyFloorHeight->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(ApplyFloorHeight);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
						.VAlign(EVerticalAlignment::VAlign_Center)
						.OnClicked_UObject(ApplyFloorHeight.Get(), &URootMotionApplyFloorHeight::ApplyFloorHeight)
						.DesiredSizeScale(FVector2D(1.0f, 2.0f))
						.ContentPadding(FMargin(4.0f, 0.0f))
						.Content()
						[
							SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.Text(LOCTEXT("ApplyFloorHeight", "Apply Floor Height"))
						]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnAdjustHoldRootMotionTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (AdjustHoldRootMotion == nullptr)
	{
		AdjustHoldRootMotion = NewObject<UAdjustHoldRootMotion>();
		AdjustHoldRootMotion->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(AdjustHoldRootMotion);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(AdjustHoldRootMotion.Get(), &UAdjustHoldRootMotion::AdjustHoldRootMotion)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("AdjustHoldRootMotion", "Adjust Hold Root Motion"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnAddRootMotionTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (AddRootMotion == nullptr)
	{
		AddRootMotion = NewObject<UAddRootMotion>();
		AddRootMotion->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(AddRootMotion);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(AddRootMotion.Get(), &UAddRootMotion::AddRootMotion)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("AddRootMotion", "Add Root Motion"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnBlendAnimationFirstPoseTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (BlendAnimationFirstPose == nullptr)
	{
		BlendAnimationFirstPose = NewObject<UBlendAnimationFirstPose>();
		BlendAnimationFirstPose->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(BlendAnimationFirstPose);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(BlendAnimationFirstPose.Get(), &UBlendAnimationFirstPose::BlendAnimationFirstPose)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("BlendAnimationFirstPose", "Blend Animation First Pose"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnBlendAnimationLastPoseTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (BlendAnimationLastPose == nullptr)
	{
		BlendAnimationLastPose = NewObject<UBlendAnimationLastPose>();
		BlendAnimationLastPose->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(BlendAnimationLastPose);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(BlendAnimationLastPose.Get(), &UBlendAnimationLastPose::BlendAnimationLastPose)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("BlendAnimationLastPose", "Blend Animation Last Pose"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnAppendLandingAnimationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (AppendLandingAnimation == nullptr)
	{
		AppendLandingAnimation = NewObject<UAppendLandingAnimation>();
		AppendLandingAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(AppendLandingAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(AppendLandingAnimation.Get(), &UAppendLandingAnimation::AppendLandingAnimation)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("AppendLandingAnimation", "Append Landing Animation"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnTrimAnimationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (TrimAnimation == nullptr)
	{
		TrimAnimation = NewObject<UTrimAnimation>();
		TrimAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(TrimAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(TrimAnimation.Get(), &UTrimAnimation::TrimAnimation)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("TrimAnimation", "Trim Animation"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnAppendAnimationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (AppendAnimation == nullptr)
	{
		AppendAnimation = NewObject<UAppendAnimation>();
		AppendAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(AppendAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(AppendAnimation.Get(), &UAppendAnimation::AppendAnimation)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("AppendAnimation", "Append Animation"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnAdjustAnimationSpeedTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (AdjustAnimationSpeed == nullptr)
	{
		AdjustAnimationSpeed = NewObject<UAdjustAnimationSpeed>();
		AdjustAnimationSpeed->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(AdjustAnimationSpeed);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(AdjustAnimationSpeed.Get(), &UAdjustAnimationSpeed::AdjustAnimationSpeed)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("AdjustAnimationSpeed", "Adjust Animation Speed"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnMakeTwoBoneIKAnimationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (MakeTwoBoneIKAnimation == nullptr)
	{
		MakeTwoBoneIKAnimation = NewObject<UMakeTwoBoneIKAnimation>();
		MakeTwoBoneIKAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(MakeTwoBoneIKAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(MakeTwoBoneIKAnimation.Get(), &UMakeTwoBoneIKAnimation::MakeTwoBoneIKAnimation)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("MakeTwoBoneIKAnimation", "Make Two Bone IK Animation"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
			]	
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnMakeFABRIKAnimationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (MakeFABRIKAnimation == nullptr)
	{
		MakeFABRIKAnimation = NewObject<UMakeFABRIKAnimation>();
		MakeFABRIKAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(MakeFABRIKAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(MakeFABRIKAnimation.Get(), &UMakeFABRIKAnimation::MakeFABRIKAnimation)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("MakeFABRIKAnimation", "Make FABRIK Animation"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnMakeLookAtAnimationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (MakeLookAtAnimation == nullptr)
	{
		MakeLookAtAnimation = NewObject<UMakeLookAtAnimation>();
		MakeLookAtAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(MakeLookAtAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(MakeLookAtAnimation.Get(), &UMakeLookAtAnimation::MakeLookAtAnimation)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("MakeLookAtAnimation", "Make LookAt Animation"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnMakeMirrorAnimationTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if (MakeMirrorAnimation == nullptr)
	{
		MakeMirrorAnimation = NewObject<UMakeMirrorAnimation>();
		MakeMirrorAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(MakeMirrorAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
				+ SScrollBox::Slot().Padding(5)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							DetailsView->AsShared()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SButton)
								.VAlign(EVerticalAlignment::VAlign_Center)
								.OnClicked_UObject(MakeMirrorAnimation.Get(), &UMakeMirrorAnimation::MakeMirrorAnimation)
								.DesiredSizeScale(FVector2D(1.0f, 2.0f))
								.ContentPadding(FMargin(4.0f, 0.0f))
								.Content()
								[
									SNew(STextBlock)
										.Justification(ETextJustify::Center)
										.Text(LOCTEXT("MakeMirrorAnimation", "Make Mirror Animation"))
								]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SSpacer)
						]
				]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnSyncBoneLocationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (SyncBoneLocation == nullptr)
	{
		SyncBoneLocation = NewObject<USyncBoneLocation>();
		SyncBoneLocation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(SyncBoneLocation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(SyncBoneLocation.Get(), &USyncBoneLocation::SyncBoneLocation)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("SyncBoneLocation", "Sync Bone Location"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnRootMotionReExtractorTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (RootMotionReExtractor == nullptr)
	{
		RootMotionReExtractor = NewObject<URootMotionReExtractor>();
		RootMotionReExtractor->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(RootMotionReExtractor);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SVerticalBox)
				.Visibility_UObject(RootMotionReExtractor.Get(), &URootMotionReExtractor::GetVisibilityAddRootBoneAnimationTrackButton)
				+ SVerticalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("Can't find root bone animation track", "Can't find root bone animation track"))
					.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(RootMotionReExtractor.Get(), &URootMotionReExtractor::AddRootBoneAnimationTrack)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("Add Root Bone Animation Track", "Add Root Bone Animation Track"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				//.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Visibility_UObject(RootMotionReExtractor.Get(), &URootMotionReExtractor::GetVisibilityExtractRootMotionButton)
				.IsEnabled_UObject(RootMotionReExtractor.Get(), &URootMotionReExtractor::CanExtractRootMotion)
				.OnClicked_UObject(RootMotionReExtractor.Get(), &URootMotionReExtractor::ExtractRootMotionFromSource)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("Extract", "Extract"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnRemoveAnimationTrackTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (RemoveAnimationTrack == nullptr)
	{
		RemoveAnimationTrack = NewObject<URemoveAnimationTrack>();
		RemoveAnimationTrack->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(RemoveAnimationTrack);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(RemoveAnimationTrack.Get(), &URemoveAnimationTrack::RemoveAnimationTrack)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("RemoveAnimationTrack", "Remove Animation Track"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnReverseAnimationTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (ReverseAnimation == nullptr)
	{
		ReverseAnimation = NewObject<UReverseAnimation>();
		ReverseAnimation->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(ReverseAnimation);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(ReverseAnimation.Get(), &UReverseAnimation::ReverseAnimation)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("ReverseAnimation", "Reverse Animation"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnRotateAnimationTrackTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (RotateAnimationTrack == nullptr)
	{
		RotateAnimationTrack = NewObject<URotateAnimationTrack>();
		RotateAnimationTrack->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(RotateAnimationTrack);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->AsShared()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.OnClicked_UObject(RotateAnimationTrack.Get(), &URotateAnimationTrack::RotateAnimationTrack)
					.DesiredSizeScale(FVector2D(1.0f, 2.0f))
					.ContentPadding(FMargin(4.0f, 0.0f))
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("RotateAnimationTrack", "Rotate Animation Track"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
				]
			]
		];
}

TSharedRef<class SDockTab> FRootMotionGuidePluginEditorModule::OnSpawnResetRootBoneTransformTab(const FSpawnTabArgs & SpawnTabArgs)
{
	if (ResetRootBoneTransform == nullptr)
	{
		ResetRootBoneTransform = NewObject<UResetRootBoneTransform>();
		ResetRootBoneTransform->AddToRoot();
	}


	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->SetObject(ResetRootBoneTransform);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView->AsShared()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked_UObject(ResetRootBoneTransform.Get(), &UResetRootBoneTransform::ResetRootBoneTransform)
				.DesiredSizeScale(FVector2D(1.0f, 2.0f))
				.ContentPadding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("ResetRootBoneTransform", "Reset Root Bone Transform"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
			]
		];
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRootMotionGuidePluginEditorModule, RootMotionGuidePluginEditor)