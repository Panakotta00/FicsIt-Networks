#include "FicsItNetworksCircuit.h"

#include "FGGameMode.h"
#include "Reflection/FIRSignal.h"
#include "Signals/FINSignalData.h"
#include "UObject/CoreRedirects.h"
#include "FicsItReflection.h"
#include "FINNetworkAdapter.h"
#include "FINNetworkCable.h"
#include "FINNetworkConnectionComponent.h"
#include "Buildables/FGBuildableRadarTower.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/NativeHookManager.h"
#include "Signals/FINSignalSubsystem.h"
#include "Wireless/FINWirelessRCO.h"
#include "Wireless/FINWirelessSubsystem.h"

#define LOCTEXT_NAMESPACE "FFicsItNetworksCircuitModule"

DEFINE_LOG_CATEGORY(LogFicsItNetworksCircuit);

void AFGBuildable_Dismantle_Implementation(CallScope<void(*)(IFGDismantleInterface*)>& scope, IFGDismantleInterface* self_r) {
	AFGBuildable* self = dynamic_cast<AFGBuildable*>(self_r);
	TInlineComponentArray<UFINNetworkConnectionComponent*> connectors;
	self->GetComponents(connectors);
	TInlineComponentArray<UFINNetworkAdapterReference*> adapters;
	self->GetComponents(adapters);
	for (UFINNetworkAdapterReference* adapter_ref : adapters) {
		if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
			connectors.Add(adapter->Connector);
		}
	}
	for (UFINNetworkConnectionComponent* connector : connectors) {
		for (AFINNetworkCable* cable : connector->ConnectedCables) {
			cable->Execute_Dismantle(cable);
		}
	}
	for (UFINNetworkAdapterReference* adapter_ref : adapters) {
		if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
			adapter->Destroy();
		}
	}
}

void AFGBuildable_GetDismantleRefund_Implementation(CallScope<void(*)(const IFGDismantleInterface*, TArray<FInventoryStack>&, bool)>& scope, const IFGDismantleInterface* self_r, TArray<FInventoryStack>& refund, bool noCost) {
	const AFGBuildable* self = dynamic_cast<const AFGBuildable*>(self_r);
	if (!self->IsA<AFINNetworkCable>()) {
		TInlineComponentArray<UFINNetworkConnectionComponent*> components;
		self->GetComponents(components);
		TInlineComponentArray<UFINNetworkAdapterReference*> adapters;
		self->GetComponents(adapters);
		for (UFINNetworkAdapterReference* adapter_ref : adapters) {
			if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
				components.Add(adapter->Connector);
			}
		}
		for (UFINNetworkConnectionComponent* connector : components) {
			for (AFINNetworkCable* cable : connector->ConnectedCables) {
				cable->Execute_GetDismantleRefund(cable, refund, noCost);
			}
		}
	}
}

UE_DISABLE_OPTIMIZATION_SHIP
void FFicsItNetworksCircuitModule::StartupModule() {
	TArray<FCoreRedirect> redirects;

	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkUtils"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkUtils")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkMessageInterface"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkMessageInterface")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkConnectionComponent"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkConnectionComponent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkComponent"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkComponent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkCircuitNode"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkCircuitNode")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkCircuit"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkCircuit")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkCableHologramPlacementStepType"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkCableHologramPlacementStepType")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINCablePlacementStepInfo"), TEXT("/Script/FicsItNetworksCircuit.FINCablePlacementStepInfo")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkCableHologram"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkCableHologram")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkCable"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkCable")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkAdapterHologram"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkAdapterHologram")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINAdapterSettings"), TEXT("/Script/FicsItNetworksCircuit.FINAdapterSettings")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkAdapter"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkAdapter")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkAdapterReference"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkAdapterReference")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINMCPConnector"), TEXT("/Script/FicsItNetworksCircuit.FINMCPConnector")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINMCPAdvConnector"), TEXT("/Script/FicsItNetworksCircuit.FINMCPAdvConnector")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINHandleSignal"), TEXT("/Script/FicsItNetworksCircuit.FINHandleSignal")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINHandleNetworkMessage"), TEXT("/Script/FicsItNetworksCircuit.FINHandleNetworkMessage")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINIsNetworkRouter"), TEXT("/Script/FicsItNetworksCircuit.FINIsNetworkRouter")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINIsNetworkPortOpen"), TEXT("/Script/FicsItNetworksCircuit.FINIsNetworkPortOpen")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINAdvancedNetworkConnectionComponent"), TEXT("/Script/FicsItNetworksCircuit.FINAdvancedNetworkConnectionComponent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessSubsystem"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessSubsystem")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessRCO"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessRCO")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessAccessPointConnectionData"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessAccessPointConnectionData")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessAccessPointConnection"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessAccessPointConnection")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessAccessPointActorRepresentation"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessAccessPointActorRepresentation")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINSignalData"), TEXT("/Script/FicsItNetworksCircuit.FINSignalData")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINSignalListener"), TEXT("/Script/FicsItNetworksCircuit.FINSignalListener")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINSignalSender"), TEXT("/Script/FicsItNetworksCircuit.FINSignalSender")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINSignalListeners"), TEXT("/Script/FicsItNetworksCircuit.FINSignalListeners")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINSignalSubsystem"), TEXT("/Script/FicsItNetworksCircuit.FINSignalSubsystem")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINCDWirelessAccessPointRequiresTower"), TEXT("/Script/FicsItNetworksCircuit.FINCDWirelessAccessPointRequiresTower")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessAccessPointHologram"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessAccessPointHologram")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessDirection"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessDirection")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessAccessPointConnectionsUpdateDelegate"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessAccessPointConnectionsUpdateDelegate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINWirelessAccessPoint"), TEXT("/Script/FicsItNetworksCircuit.FINWirelessAccessPoint")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkRouterLampFlags"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkRouterLampFlags")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINNetworkRouter"), TEXT("/Script/FicsItNetworksCircuit.FINNetworkRouter")});

	FCoreRedirects::AddRedirectList(redirects, "FicsItNetworksCircuit");

	FFicsItReflectionModule::Get().OnSignalTriggered.AddLambda([](UObject* Context, UFIRSignal* Signal, const TArray<FFIRAnyValue>& Data) {
		AFINSignalSubsystem* SubSys = AFINSignalSubsystem::GetSignalSubsystem(Context);
		if (!SubSys) {
			UE_LOG(LogFicsItNetworksCircuit, Error, TEXT("Unable to get signal subsystem for executing signal '%s'"), *Signal->GetInternalName())
			return;
		}
		SubSys->BroadcastSignal(Context, FFINSignalData(Signal, Data));
	});

	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::Dismantle_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_Dismantle_Implementation);
		SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::GetDismantleRefund_Implementation, (void*)static_cast<const IFGDismantleInterface*>(GetDefault<AFGBuildable>()), &AFGBuildable_GetDismantleRefund_Implementation);

		SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::SetupComponent, (void*)GetDefault<AFGBuildableHologram>(), [](auto& scope, AFGBuildableHologram* self, USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName, const FName& socketName) {
			UStaticMesh* networkConnectorHoloMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Buildings/Network/-Shared/SM_NetworkConnector.SM_NetworkConnector"), NULL, LOAD_None, NULL);
			if (!IsValid(componentTemplate)) return;

			if (!componentTemplate->IsA<UFINNetworkConnectionComponent>()) return;

			auto comp = NewObject<UStaticMeshComponent>(attachParent);
			comp->RegisterComponent();
			comp->SetMobility(EComponentMobility::Movable);
			comp->SetStaticMesh(networkConnectorHoloMesh);
			comp->AttachToComponent(attachParent, FAttachmentTransformRules::KeepRelativeTransform);
			comp->SetRelativeTransform(Cast<USceneComponent>(componentTemplate)->GetRelativeTransform());
			comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			scope.Override(comp);
		});

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGameMode::PostLogin, (void*)GetDefault<AFGGameMode>(), [](AFGGameMode* gm, APlayerController* pc) {
			if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
				gm->RegisterRemoteCallObjectClass(UFINWirelessRCO::StaticClass());
			}
		});

		// Wireless - Recalculate network topology when radar tower is created or destroyed
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableRadarTower::BeginPlay, (void*)GetDefault<AFGBuildableRadarTower>(), [](AActor* self) {
			if (self->HasAuthority()) {
				UE_LOG(LogFicsItNetworksCircuit, Display, TEXT("[Wireless] Radar tower Created, recalculating network topology"));
				AFINWirelessSubsystem::Get(self->GetWorld())->RecalculateWirelessConnections();
			}
		});

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableRadarTower::EndPlay, (void*)GetDefault<AFGBuildableRadarTower>(), [](AActor* self, EEndPlayReason::Type Reason) {
			if (Reason == EEndPlayReason::Destroyed && self->HasAuthority()) {
				UE_LOG(LogFicsItNetworksCircuit, Display, TEXT("[Wireless] Radar tower Destroyed, recalculating network topology"));
				AFINWirelessSubsystem::Get(self->GetWorld())->RecalculateWirelessConnections();
			}
		});
#endif
	});
}
UE_ENABLE_OPTIMIZATION_SHIP

void FFicsItNetworksCircuitModule::ShutdownModule() {}
UFINCircuitGameWorldModule::UFINCircuitGameWorldModule() {
	ModSubsystems.Add(AFINSignalSubsystem::StaticClass());
	ModSubsystems.Add(AFINWirelessSubsystem::StaticClass());
}

UFINCircuitGameInstanceModule::UFINCircuitGameInstanceModule() {
	RemoteCallObjects.Add(UFINWirelessRCO::StaticClass());
}

void UFINCircuitGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::POST_INITIALIZATION:
		AFINNetworkAdapter::RegisterAdapterSettings();
		break;
	default: break;
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItNetworksCircuitModule, FicsItNetworksCircuit)