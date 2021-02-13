#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworks, Log, Log);

class FFicsItNetworksModule : public FDefaultGameModuleImpl
{
public:
	/**
	* Called when the module is loaded into memory
	**/
	virtual void StartupModule() override;

	/**
	* Called when the module is unloaded from memory
	**/
	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }
};