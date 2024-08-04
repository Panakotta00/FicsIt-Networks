#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksDocumentation, All, Display);

class FFicsItNetworksDocumentationModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
