#include "FicsItNetworksMicrocontroller.h"

#include "FGBuildable.h"
#include "FGDismantleInterface.h"
#include "FINMicrocontroller.h"
#include "FINMicrocontrollerReference.h"
#include "NativeHookManager.h"

DEFINE_LOG_CATEGORY(LogFicsItNetworksMicrocontroller);

void AFGBuildable_Dismantle_Implementation(CallScope<void(*)(IFGDismantleInterface*)>& scope, IFGDismantleInterface* self_r) {
	AFGBuildable* self = dynamic_cast<AFGBuildable*>(self_r);
	AFINMicrocontroller* controller = UFINMicrocontrollerReference::FindMicrocontroller(self);
	if (controller) {
		IFGDismantleInterface::Execute_Dismantle(controller);
	}
}

void AFGBuildable_GetDismantleRefund_Implementation(CallScope<void(*)(const IFGDismantleInterface*, TArray<FInventoryStack>&, bool)>& scope, const IFGDismantleInterface* self_r, TArray<FInventoryStack>& refund, bool noCost) {
	const AFGBuildable* self = dynamic_cast<const AFGBuildable*>(self_r);
	AFINMicrocontroller* controller = UFINMicrocontrollerReference::FindMicrocontroller(self);
	if (controller) {
		IFGDismantleInterface::Execute_GetDismantleRefund(controller, refund, noCost);
	}
}

void FFicsItNetworksMicrocontrollerModule::StartupModule() {
	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::Dismantle_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_Dismantle_Implementation);
		SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::GetDismantleRefund_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_GetDismantleRefund_Implementation);
#endif
	});
}

void FFicsItNetworksMicrocontrollerModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItNetworksMicrocontrollerModule, FicsItNetworksMicrocontroller)