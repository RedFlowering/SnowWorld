// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"

DECLARE_STATS_GROUP(TEXT("Number Renderer"), STATGROUP_NumberRenderer, STATCAT_Advanced);
DECLARE_LOG_CATEGORY_EXTERN(LogNumberRenderer, Log, All);

class UWorldNumberManager;

class NUMBERRENDERER_API FNumberRendererModule : public FDefaultModuleImpl, public FGCObject
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;

	FORCEINLINE static UWorldNumberManager* Get(const UWorld* World)
	{
		return WorldNumberManagers.FindRef(World);
	}

private:
	void HandleWorldPreInit(UWorld* World, const UWorld::InitializationValues IVS);
	void HandleWorldPostInit(UWorld* World, const UWorld::InitializationValues IVS);
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	FDelegateHandle PreWorldInitializationHandle;
	FDelegateHandle PostWorldInitializationHandle;
	FDelegateHandle PostWorldCleanupHandle;

	static TMap<const UWorld*, TObjectPtr<UWorldNumberManager>> WorldNumberManagers;
};
