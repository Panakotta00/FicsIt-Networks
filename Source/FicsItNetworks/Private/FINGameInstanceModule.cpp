#include "FINGameInstanceModule.h"

#include "FicsItLogLibrary.h"
#include "FIRModModule.h"
#include "Network/FINNetworkAdapter.h"

UFINGameInstanceModule::UFINGameInstanceModule() {}

void UFINGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::CONSTRUCTION:
		SpawnChildModule(TEXT("FicsItReflection"), UFIRGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItLogLibrary"), UFILGameInstanceModule::StaticClass());
		break;
	case ELifecyclePhase::POST_INITIALIZATION:
		AFINNetworkAdapter::RegisterAdapterSettings();
		break;
	default: break;
	}
}
