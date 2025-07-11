#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FHarmoniaLoadManagerEditor : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnAssetSaved(UObject* Object);
};
