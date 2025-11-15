// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#include "NumberRenderer.h"
#include "NumberRendererSettings.h"
#include "WorldNumberManager.h"


#define LOCTEXT_NAMESPACE "FNumberRendererModule"

DEFINE_LOG_CATEGORY(LogNumberRenderer);

TMap<const UWorld*, TObjectPtr<UWorldNumberManager>> FNumberRendererModule::WorldNumberManagers;

void FNumberRendererModule::StartupModule()
{
#if !UE_SERVER
	PreWorldInitializationHandle = FWorldDelegates::OnPreWorldInitialization.AddRaw(this, &FNumberRendererModule::HandleWorldPreInit);
	PostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FNumberRendererModule::HandleWorldPostInit);
	PostWorldCleanupHandle = FWorldDelegates::OnPostWorldCleanup.AddRaw(this, &FNumberRendererModule::HandleWorldCleanup);
#endif
}

void FNumberRendererModule::ShutdownModule()
{
#if !UE_SERVER
	FWorldDelegates::OnPreWorldInitialization.Remove(PreWorldInitializationHandle);
	FWorldDelegates::OnPostWorldCleanup.Remove(PostWorldCleanupHandle);
	FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationHandle);
#endif
}

void FNumberRendererModule::AddReferencedObjects(FReferenceCollector& Collector)
{
#if !UE_SERVER
	for (TPair<const UWorld*, TObjectPtr<UWorldNumberManager>>& WorldAnimSharingManagerPair : WorldNumberManagers)
	{
		Collector.AddReferencedObject(WorldAnimSharingManagerPair.Value, WorldAnimSharingManagerPair.Key);
	}
#endif
}

FString FNumberRendererModule::GetReferencerName() const
{
	return TEXT("FNumberRendererModule");
}

void FNumberRendererModule::HandleWorldPreInit(UWorld* World, const UWorld::InitializationValues IVS)
{

}

void FNumberRendererModule::HandleWorldPostInit(UWorld* World, const UWorld::InitializationValues IVS)
{
#if !UE_SERVER
	if (World && World->IsGameWorld())
	{
		UWorldNumberManager* NewManger = NewObject<UWorldNumberManager>();
		NewManger->Init(World);
		WorldNumberManagers.Add(World, NewManger);
	}
#endif
}

void FNumberRendererModule::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
#if !UE_SERVER
	WorldNumberManagers.Remove(World);
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNumberRendererModule, NumberRenderer)