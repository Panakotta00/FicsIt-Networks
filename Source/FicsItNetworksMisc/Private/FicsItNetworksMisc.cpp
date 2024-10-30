#include "FicsItNetworksMisc.h"

#include "FGDismantleInterface.h"
#include "FINMediaSubsystem.h"
#include "Buildables/FGBuildable.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Patching/NativeHookManager.h"
#include "UObject/CoreRedirects.h"

#define LOCTEXT_NAMESPACE "FFicsItNetworksMiscModule"

DEFINE_LOG_CATEGORY(LogFicsItNetworksMisc);

void AFGBuildable_Dismantle_Implementation(CallScope<void(*)(IFGDismantleInterface*)>& scope, IFGDismantleInterface* self_r) {
	AFGBuildable* self = dynamic_cast<AFGBuildable*>(self_r);
	TInlineComponentArray<UFINModuleSystemPanel*> panels;
	self->GetComponents(panels);
	for (UFINModuleSystemPanel* panel : panels) {
		TArray<AActor*> modules;
		panel->GetModules(modules);
		for (AActor* module : modules) {
			module->Destroy();
		}
	}
}

void AFGBuildable_GetDismantleRefund_Implementation(CallScope<void(*)(const IFGDismantleInterface*, TArray<FInventoryStack>&, bool)>& scope, const IFGDismantleInterface* self_r, TArray<FInventoryStack>& refund, bool noCost) {
	const AFGBuildable* self = dynamic_cast<const AFGBuildable*>(self_r);
	TInlineComponentArray<UFINModuleSystemPanel*> panels;
	self->GetComponents(panels);
	for (UFINModuleSystemPanel* panel : panels) {
		panel->GetDismantleRefund(refund, noCost);
	}
}

void FFicsItNetworksMiscModule::StartupModule() {
	TArray<FCoreRedirect> redirects;

	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINFuture"), TEXT("/Script/FicsItNetworksMisc.FINFuture")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINFutureReflection"), TEXT("/Script/FicsItNetworksMisc.FINFutureReflection")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINFunctionFuture"), TEXT("/Script/FicsItNetworksMisc.FINFunctionFuture")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINCopyableItemInterface"), TEXT("/Script/FicsItNetworksMisc.FINCopyableItemInterface")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINModuleDelegate"), TEXT("/Script/FicsItNetworksMisc.FINModuleDelegate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINModuleSystemPanel"), TEXT("/Script/FicsItNetworksMisc.FINModuleSystemPanel")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINModuleSystemModule"), TEXT("/Script/FicsItNetworksMisc.FINModuleSystemModule")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINModuleSystemHolo"), TEXT("/Script/FicsItNetworksMisc.FINModuleSystemHolo")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINCustomVersion"), TEXT("/Script/FicsItNetworksMisc.FINCustomVersion")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINUtils"), TEXT("/Script/FicsItNetworksMisc.FINUtils")});

	FCoreRedirects::AddRedirectList(redirects, "FicsItNetworksMisc");

#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::Dismantle_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_Dismantle_Implementation);
	SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::GetDismantleRefund_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_GetDismantleRefund_Implementation);
#endif
}

void FFicsItNetworksMiscModule::ShutdownModule() {
    
}

UFINMiscGameWorldModule::UFINMiscGameWorldModule() {
	ModSubsystems.Add(AFINMediaSubsystem::StaticClass());
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItNetworksMiscModule, FicsItNetworksMisc)