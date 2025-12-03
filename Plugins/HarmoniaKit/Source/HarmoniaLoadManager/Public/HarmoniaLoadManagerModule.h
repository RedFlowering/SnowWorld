// Copyright 2025 Snow Game Studio.

#pragma once

#include "Modules/ModuleManager.h"

class FHarmoniaLoadManagerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};