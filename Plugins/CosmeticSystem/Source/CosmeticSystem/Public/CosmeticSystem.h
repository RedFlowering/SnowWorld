// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCosmeticSystem, Log, All);

class FCosmeticSystemModule : public IModuleInterface
{
public:	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
