#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksLua, Verbose, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksLuaReflection, Fatal, All);

class FFicsItNetworksLuaModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }
};