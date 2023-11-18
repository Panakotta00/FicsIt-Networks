#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksEd, Verbose, All);

class FFicsItNetworksEdModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return false; }
};