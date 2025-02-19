// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.

#include "UEGoreSystem.h"

#define LOCTEXT_NAMESPACE "FUEGoreSystemModule"

DEFINE_LOG_CATEGORY(LogUEGoreSystem);

void FUEGoreSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	// Nothing needed
}

void FUEGoreSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	// Nothing needed
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUEGoreSystemModule, UEGoreSystem)