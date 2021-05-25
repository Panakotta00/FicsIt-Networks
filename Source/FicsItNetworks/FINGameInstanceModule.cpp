#include "FINGameInstanceModule.h"

#include "FicsItNetworksModule.h"
#include "FINGlobalRegisterHelper.h"
#include "Network/FINNetworkAdapter.h"
#include "Reflection/FINReflection.h"
#include "UI/FINReflectionStyles.h"

UFINGameInstanceModule::UFINGameInstanceModule() {
	
}

void UFINGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	if (Phase == ELifecyclePhase::POST_INITIALIZATION) {
		FFINReflectionStyles::Shutdown();
		FFINReflectionStyles::Initialize();
			
		AFINNetworkAdapter::RegisterAdapterSettings();
		FFINGlobalRegisterHelper::Register();
			
		FFINReflection::Get()->PopulateSources();
		FFINReflection::Get()->LoadAllTypes();
	}
}
