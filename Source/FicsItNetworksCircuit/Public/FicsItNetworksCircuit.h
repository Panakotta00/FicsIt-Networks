#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "Module/GameWorldModule.h"
#include "Modules/ModuleManager.h"
#include  "FicsItNetworksCircuit.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksCircuit, Warning, All)

class FFicsItNetworksCircuitModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

UCLASS()
class FICSITNETWORKSCIRCUIT_API UFINCircuitGameWorldModule : public UGameWorldModule {
    GENERATED_BODY()

    UFINCircuitGameWorldModule();
};

UCLASS()
class FICSITNETWORKSCIRCUIT_API UFINCircuitGameInstanceModule : public UGameInstanceModule {
    GENERATED_BODY()

    UFINCircuitGameInstanceModule();

    virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;
};