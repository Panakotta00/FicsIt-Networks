#include "FINGameWorldModule.h"

#include "FicsItNetworksCircuit.h"
#include "FicsItNetworksComputer.h"
#include "FicsItNetworksMisc.h"
#include "FINArrowModuleHolo.h"
#include "FINComputerSubsystem.h"
#include "FINMediaSubsystem.h"
#include "FIRModModule.h"
#include "Utils/FINBlueprintParameterHooks.h"

UFINGameWorldModule::UFINGameWorldModule() {
	ModSubsystems.Add(AFINBuildgunHooks::StaticClass());
	ModSubsystems.Add(AFINComputerSubsystem::StaticClass());
	ModSubsystems.Add(AFINBlueprintParameterHooks::StaticClass());
	ModSubsystems.Add(AFINMediaSubsystem::StaticClass());
}

void UFINGameWorldModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::CONSTRUCTION:
		SpawnChildModule(TEXT("FicsItReflection"), UFIRGameWorldModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksMisc"), UFINMiscGameWorldModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksCircuit"), UFINCircuitGameWorldModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksComputer"), UFINComputerGameWorldModule::StaticClass());
		break;
	default: break;
	}
}
