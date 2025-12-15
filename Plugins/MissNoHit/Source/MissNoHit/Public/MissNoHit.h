// Copyright 2024 Eren Balatkan. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FMissNoHitModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual bool IsGameModule() const override { return true; }
};
