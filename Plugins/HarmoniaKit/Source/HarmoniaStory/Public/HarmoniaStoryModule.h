// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FHarmoniaStoryModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
