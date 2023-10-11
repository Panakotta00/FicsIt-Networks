#include "FINGameInstanceModule.h"
#include "FINGlobalRegisterHelper.h"
#include "Network/FINNetworkAdapter.h"
#include "Reflection/FINReflection.h"

UFINGameInstanceModule::UFINGameInstanceModule() {}

void UFINGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	if (Phase == ELifecyclePhase::POST_INITIALIZATION) {
		AFINNetworkAdapter::RegisterAdapterSettings();
		FFINGlobalRegisterHelper::Register();
			
		FFINReflection::Get()->PopulateSources();
		FFINReflection::Get()->LoadAllTypes();
	}
}
