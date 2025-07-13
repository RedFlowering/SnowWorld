#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FHarmoniaSaveGameSystem : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
