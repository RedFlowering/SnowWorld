// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FHarmoniaSkillSystem : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
