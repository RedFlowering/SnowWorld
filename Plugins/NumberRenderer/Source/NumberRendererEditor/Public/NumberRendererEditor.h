// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FNumberRendererEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Override this to set whether your module is allowed to be unloaded on the fly
	 *
	 * @return	Whether the module supports shutdown separate from the rest of the engine.
	 */
	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

private:
	void RegisterObjectCustomizations();
	/**
	 * Registers a custom class
	 *
	 * @param ClassName				The class name to register for property customization
	 * @param DetailLayoutDelegate	The delegate to call to get the custom detail layout instance
	 */
	void RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate);

	/**
	* Registers a custom struct
	*
	* @param StructName				The name of the struct to register for property customization
	* @param StructLayoutDelegate	The delegate to call to get the custom detail layout instance
	*/
	void RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate);
private:
	/** List of registered class that we must unregister when the module shuts down */
	TSet< FName > RegisteredClassNames;
	TSet< FName > RegisteredPropertyTypes;
};
