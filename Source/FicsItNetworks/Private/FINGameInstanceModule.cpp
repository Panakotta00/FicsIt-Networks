#include "FINGameInstanceModule.h"

#include "FicsItLogLibrary.h"
#include "FicsItNetworksCircuit.h"
#include "FIRModModule.h"

UFINGameInstanceModule::UFINGameInstanceModule() {}

void UFINGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::CONSTRUCTION:
		SpawnChildModule(TEXT("FicsItReflection"), UFIRGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItLogLibrary"), UFILGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksCircuit"), UFINCircuitGameInstanceModule::StaticClass());
		break;
	default: break;
	}
}
