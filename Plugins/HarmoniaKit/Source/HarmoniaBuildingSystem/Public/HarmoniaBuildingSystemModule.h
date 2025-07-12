// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FHarmoniaBuildingSystemModule : public IModuleInterface
{
public:
	//~IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~End of IModuleInterface
};
