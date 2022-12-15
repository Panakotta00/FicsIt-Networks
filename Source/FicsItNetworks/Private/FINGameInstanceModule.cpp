#include "FINGameInstanceModule.h"
#include "FINGlobalRegisterHelper.h"
#include "Network/FINNetworkAdapter.h"
#include "Reflection/FINReflection.h"
#include "UI/FINReflectionStyles.h"

UFINGameInstanceModule::UFINGameInstanceModule() {
	ModKeyBindings.Add({
		TEXT("FicsItNetworks.CopyUUID"),
		FInputActionKeyMapping(TEXT("FicsItNetworks.CopyUUID"), EKeys::C, false, true, false),
		FText::FromString(TEXT("Copy FIN Comp. ID"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItNetworks.CopyNick"),
		FInputActionKeyMapping(TEXT("FicsItNetworks.CopyNick"), EKeys::C, true, true, false),
		FText::FromString(TEXT("Copy FIN Comp. Nick/Groups"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItNetworks.PasteNick"),
		FInputActionKeyMapping(TEXT("FicsItNetworks.PasteNick"), EKeys::V, true, true, false),
		FText::FromString(TEXT("Paste FIN Comp. Nick/Groups"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItNetworks.CopyScreen"),
		FInputActionKeyMapping(TEXT("FicsItNetworks.CopyScreen"), EKeys::C, false, true, false),
		FText::FromString(TEXT("Copy FIN Screen Content"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItNetworks.PasteScreen"),
		FInputActionKeyMapping(TEXT("FicsItNetworks.PasteScreen"), EKeys::V, false, true, false),
		FText::FromString(TEXT("Paste FIN Screen Content"))
	});
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
