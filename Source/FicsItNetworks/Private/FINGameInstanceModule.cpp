#include "FINGameInstanceModule.h"

#include "FicsItLogLibrary.h"
#include "FicsItNetworksCircuit.h"
#include "FicsItNetworksComputer.h"
#include "FicsItNetworksLuaModule.h"
#include "FIRModModule.h"

UFINGameInstanceModule::UFINGameInstanceModule() {}

void UFINGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::CONSTRUCTION:
		SpawnChildModule(TEXT("FicsItReflection"), UFIRGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItLogLibrary"), UFILGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksCircuit"), UFINCircuitGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksComputer"), UFINComputerGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksLua"), UFINLuaGameInstanceModule::StaticClass());
		break;
	default: break;
	}
}
