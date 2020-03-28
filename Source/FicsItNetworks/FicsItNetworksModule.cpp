#include "FicsItNetworksModule.h"

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"

#include "FGBuildable.h"
#include "FGBuildableHologram.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryLibrary.h"

#include "SML/mod/hooking.h"
#include "SML/util/Utility.h"

#include "FINConfig.h"
#include "FINComponentUtility.h"
#include "Network/Signals/FINSignal.h"
#include "Network/FINNetworkConnector.h"
#include "Network/FINNetworkAdapter.h"

#include "FicsItKernel/Network/SmartSignal.h"
#include "FicsItKernel/Processor/Lua/LuaHooks.h"

IMPLEMENT_GAME_MODULE(FFicsItNetworksModule, FicsItNetworks);

TSharedPtr<FFINSignal> smartAsFINSig(FicsItKernel::Network::SmartSignal* sig) {
	return TSharedPtr<FFINSignal>(new FFINSignal(std::shared_ptr<FicsItKernel::Network::SmartSignal>(sig)));
}

class AFGBuildableHologram_Public : public AFGBuildableHologram {
public:
	USceneComponent* SetupComponentFunc(USceneComponent*, UActorComponent*, const FName&) { return nullptr; }
};

class UFGFactoryConnectionComponent_Public : public UFGFactoryConnectionComponent {
public:
	bool Factory_GrabOutput(FInventoryItem&, float&, TSubclassOf<UFGItemDescriptor>) { return false; }
};

class UFGPowerCircuit_Public : public UFGPowerCircuit {
public:
	void TickCircuit(float) {}
};

class AFGCharacterPlayer_Public : public AFGCharacterPlayer {
public:
	void UpdateBestUsableActor() {}
};

void GetDismantleRefund_Decl(IFGDismantleInterface*, TArray<FInventoryStack>&);
void GetDismantleRefund_Def(CallScope<decltype(&GetDismantleRefund_Decl)>& scope, IFGDismantleInterface* disInt, TArray<FInventoryStack>& refund) {}
#pragma optimize( "", off )
void GetDismantleRefund(CallScope<decltype(&GetDismantleRefund_Decl)>& scope, IFGDismantleInterface* disInt, TArray<FInventoryStack>& refund) {
	AFGBuildable* self = dynamic_cast<AFGBuildable*>(disInt);
	if (!self->IsA<AFINNetworkCable>()) {
		TInlineComponentArray<UFINNetworkConnector*> components;
		self->GetComponents(components);
		TInlineComponentArray<UFINNetworkAdapterReference*> adapters;
		self->GetComponents(adapters);
 		for (auto& adapter_ref : adapters) {
			if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
				components.Add(adapter->Connector);
			}
		}
		for (auto& connector : components) {
			for (AFINNetworkCable* cable : connector->Cables) {
				cable->Execute_GetDismantleRefund(cable, refund);
			}
		}
	}
}
#pragma optimize( "", on )

void FFicsItNetworksModule::StartupModule(){
	#ifndef WITH_EDITOR
	finConfig->SetNumberField("SignalQueueSize", 32);
	finConfig = SML::readModConfig(MOD_NAME, finConfig);
	#endif

	SUBSCRIBE_METHOD("?SetupComponent@AFGBuildableHologram@@MEAAPEAVUSceneComponent@@PEAV2@PEAVUActorComponent@@AEBVFName@@@Z", AFGBuildableHologram_Public::SetupComponentFunc, [](auto& scope, AFGBuildableHologram_Public* self, USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName) {
		UStaticMesh* networkConnectorHoloMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsIt-Networks/Network/Mesh_NetworkConnector.Mesh_NetworkConnector"), NULL, LOAD_None, NULL);
		if (componentTemplate->IsA<UFINNetworkConnector>()) {
			auto comp = NewObject<UStaticMeshComponent>(attachParent);
			comp->RegisterComponent();
			comp->SetMobility(EComponentMobility::Movable);
			comp->SetStaticMesh(networkConnectorHoloMesh);
			comp->AttachTo(attachParent);
			comp->SetRelativeTransform(Cast<USceneComponent>(componentTemplate)->GetRelativeTransform());
			
			scope.Override(comp);
		}
	});

	SUBSCRIBE_METHOD("?Factory_GrabOutput@UFGFactoryConnectionComponent@@QEAA_NAEAUFInventoryItem@@AEAMV?$TSubclassOf@VUFGItemDescriptor@@@@@Z", UFGFactoryConnectionComponent_Public::Factory_GrabOutput, [](auto& scope, UFGFactoryConnectionComponent_Public* c, FInventoryItem& item, float& offset, TSubclassOf<UFGItemDescriptor> type) {
		scope(c, item, offset, type);
		if (scope.getResult()) {
			auto hook = FicsItKernel::Lua::factoryHooks.Find(c);

			if (hook) {
				hook->update();
				hook->iperm.push(std::chrono::high_resolution_clock::now());
				for (auto& c : hook->deleg) {
					auto listener = Cast<IFINSignalListener>(*c);
					if (listener) listener->HandleSignal(smartAsFINSig(new FicsItKernel::Network::SmartSignal("ItemTransfer", item.ItemClass)), c);
				}
			}
		}
	});

	SUBSCRIBE_METHOD("?TickCircuit@UFGPowerCircuit@@MEAAXM@Z", UFGPowerCircuit_Public::TickCircuit, [](auto& scope, UFGPowerCircuit_Public* circuit, float dt) {
		bool oldFused = circuit->IsFuseTriggered();
		scope(circuit, dt);
		bool fused = circuit->IsFuseTriggered();
		if (oldFused != fused) try {
			auto listeners = FicsItKernel::Lua::powerCircuitListeners.Find(circuit);
			if (listeners) for (auto& listener : *listeners) {
				auto l = Cast<IFINSignalListener>(*listener);
				if (l) l->HandleSignal(smartAsFINSig(new FicsItKernel::Network::SmartSignal("PowerFuseChanged")), listener);
			}
		} catch (...) {}
	});

	SUBSCRIBE_METHOD("?UpdateBestUsableActor@AFGCharacterPlayer@@IEAAXXZ", AFGCharacterPlayer_Public::UpdateBestUsableActor, [](auto& scope, AFGCharacterPlayer_Public* self) {
		if (!UFINComponentUtility::bAllowUsing) scope.Cancel();
	});

	SUBSCRIBE_METHOD("?Dismantle_Implementation@AFGBuildable@@UEAAXXZ", AFGBuildable::Dismantle_Implementation, [](auto& scope, AFGBuildable* self_r) {
		IFGDismantleInterface* disInt = reinterpret_cast<IFGDismantleInterface*>(self_r);
		AFGBuildable* self = dynamic_cast<AFGBuildable*>(disInt);
		TInlineComponentArray<UFINNetworkConnector*> connectors;
		self->GetComponents(connectors);
		TInlineComponentArray<UFINNetworkAdapterReference*> adapters;
		self->GetComponents(adapters);
		for (auto& adapter_ref : adapters) {
			if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
				connectors.Add(adapter->Connector);
			}
		}
		for (auto& connector : connectors) {
			for (AFINNetworkCable* cable : connector->Cables) {
				cable->Execute_Dismantle(cable);
			}
		}
		for (auto& adapter_ref : adapters) {
			if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
				adapter->Destroy();
			}
		}
	});

	SUBSCRIBE_METHOD("?GetDismantleRefund_Implementation@AFGBuildableFactory@@UEBAXAEAV?$TArray@UFInventoryStack@@VFDefaultAllocator@@@@@Z", GetDismantleRefund_Decl, &GetDismantleRefund_Def);
	SUBSCRIBE_METHOD("?GetDismantleRefund_Implementation@AFGBuildableGeneratorFuel@@UEBAXAEAV?$TArray@UFInventoryStack@@VFDefaultAllocator@@@@@Z", GetDismantleRefund_Decl, &GetDismantleRefund_Def);
	SUBSCRIBE_METHOD("?GetDismantleRefund_Implementation@AFGBuildable@@UEBAXAEAV?$TArray@UFInventoryStack@@VFDefaultAllocator@@@@@Z", GetDismantleRefund_Decl, &GetDismantleRefund);

	AFINNetworkAdapter::RegisterAdapterSettings();
}
void FFicsItNetworksModule::ShutdownModule(){ }

extern "C" DLLEXPORT void BootstrapModule(std::ofstream& logFile) {
	
}
