#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "Module/GameWorldModule.h"
#include "Modules/ModuleManager.h"
#include "FicsItNetworksComputer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksComputer, Log, All);

class FICSITNETWORKSCOMPUTER_API FFicsItNetworksComputerModule : public IModuleInterface {
public:
    static FDateTime GameStart;

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

UCLASS()
class FICSITNETWORKSCOMPUTER_API UFINComputerGameWorldModule : public UGameWorldModule {
    GENERATED_BODY()

    UFINComputerGameWorldModule();
};

UCLASS()
class FICSITNETWORKSCOMPUTER_API UFINComputerGameInstanceModule : public UGameInstanceModule {
    GENERATED_BODY()

    UFINComputerGameInstanceModule();
};
