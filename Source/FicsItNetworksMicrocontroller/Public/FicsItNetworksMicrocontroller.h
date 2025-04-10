#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksMicrocontroller, Log, All);

class FICSITNETWORKSMICROCONTROLLER_API FFicsItNetworksMicrocontrollerModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
