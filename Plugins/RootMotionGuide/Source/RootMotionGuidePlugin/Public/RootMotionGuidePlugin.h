// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FRootMotionGuidePluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};