#include "FINGameWorldModule.h"

#include "FicsItNetworksCircuit.h"
#include "FIRModModule.h"
#include "Computer/FINComputerSubsystem.h"
#include "Utils/FINBlueprintParameterHooks.h"
#include "Utils/FINMediaSubsystem.h"

UFINGameWorldModule::UFINGameWorldModule() {
	ModSubsystems.Add(AFINComputerSubsystem::StaticClass());
	ModSubsystems.Add(AFINBlueprintParameterHooks::StaticClass());
	ModSubsystems.Add(AFINMediaSubsystem::StaticClass());
}

void UFINGameWorldModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::CONSTRUCTION:
		SpawnChildModule(TEXT("FicsItReflection"), UFIRGameWorldModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksCircuit"), UFINCircuitGameWorldModule::StaticClass());
		break;
	default: break;
	}
}
