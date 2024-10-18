#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "Modules/ModuleManager.h"
#include "FicsItNetworksMisc.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksMisc, Log, All);

class FFicsItNetworksMiscModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

UCLASS()
class FICSITNETWORKSMISC_API UFINMiscGameWorldModule : public UGameWorldModule {
    GENERATED_BODY()

    UFINMiscGameWorldModule();
};
