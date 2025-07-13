#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FHarmoniaItemSystem : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
