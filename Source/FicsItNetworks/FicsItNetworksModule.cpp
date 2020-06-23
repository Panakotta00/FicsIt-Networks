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
#include "FINSubsystemHolder.h"
#include "Network/Signals/FINSignal.h"
#include "Network/FINNetworkConnector.h"
#include "Network/FINNetworkAdapter.h"
#include "ModuleSystem/FINModuleSystemPanel.h"

#include "FicsItKernel/Network/SmartSignal.h"
#include "FicsItKernel/Processor/Lua/LuaHooks.h"
#include "FicsItKernel/Processor/Lua/LuaLib.h"

IMPLEMENT_GAME_MODULE(FFicsItNetworksModule, FicsItNetworks);

FFINSignal smartAsFINSig(FicsItKernel::Network::SmartSignal* sig) {
	return FFINSignal(std::shared_ptr<FicsItKernel::Network::SmartSignal>(sig));
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

USceneComponent* Holo_SetupComponentDecl(AFGBuildableHologram_Public* self, USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName);
void Holo_SetupComponent(CallScope<decltype(&Holo_SetupComponentDecl)>& scope, AFGBuildableHologram_Public* self, USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName) {
	UStaticMesh* networkConnectorHoloMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsItNetworks/Network/Mesh_NetworkConnector.Mesh_NetworkConnector"), NULL, LOAD_None, NULL);
	if (componentTemplate->IsA<UFINNetworkConnector>()) {
		auto comp = NewObject<UStaticMeshComponent>(attachParent);
		comp->RegisterComponent();
		comp->SetMobility(EComponentMobility::Movable);
		comp->SetStaticMesh(networkConnectorHoloMesh);
		comp->AttachTo(attachParent);
		comp->SetRelativeTransform(Cast<USceneComponent>(componentTemplate)->GetRelativeTransform());
			
		scope.Override(comp);
	}
}

void GetDismantleRefund_Decl(IFGDismantleInterface*, TArray<FInventoryStack>&);
void GetDismantleRefund(CallScope<decltype(&GetDismantleRefund_Decl)>& scope, IFGDismantleInterface* disInt, TArray<FInventoryStack>& refund) {
	AFGBuildable* self = reinterpret_cast<AFGBuildable*>(disInt);
	if (!self->IsA<AFINNetworkCable>()) {
		TInlineComponentArray<UFINNetworkConnector*> components;
		self->GetComponents(components);
		TInlineComponentArray<UFINNetworkAdapterReference*> adapters;
		self->GetComponents(adapters);
		TInlineComponentArray<UFINModuleSystemPanel*> panels;
		self->GetComponents(panels);
 		for (UFINNetworkAdapterReference* adapter_ref : adapters) {
			if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
				components.Add(adapter->Connector);
			}
		}
		for (UFINNetworkConnector* connector : components) {
			for (AFINNetworkCable* cable : connector->Cables) {
				cable->Execute_GetDismantleRefund(cable, refund);
			}
		}
		for (UFINModuleSystemPanel* panel : panels) {
			panel->GetDismantleRefund(refund);
		}
	}
}

FCriticalSection MutexFactoryGrab;
TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> FactoryGrabsRunning;

void LockFactoryGrab(UFGFactoryConnectionComponent* comp) {
	MutexFactoryGrab.Lock();
	++FactoryGrabsRunning.FindOrAdd(comp);
	MutexFactoryGrab.Unlock();
}

bool UnlockFactoryGrab(UFGFactoryConnectionComponent* comp) {
	MutexFactoryGrab.Lock();
	int8* i = FactoryGrabsRunning.Find(comp);
	bool valid = false;
	if (i) {
		--*i;
		valid = (*i <= 0);
		if (valid) FactoryGrabsRunning.Remove(comp);
	}
	MutexFactoryGrab.Unlock();
	return valid;
}

void DoFactoryGrab(UFGFactoryConnectionComponent* c, FInventoryItem& item) {
	AFINComputerSubsystem* CompSS = AFINComputerSubsystem::GetComputerSubsystem(c);
	CompSS->MutexFactoryHooks.Lock();
	FFINFactoryHook* hook = CompSS->FactoryHooks.Find(c);

	if (hook) {
		hook->CurrentSample += 1;
		for (auto& c : hook->Listeners) {
			UObject* obj = *c;
			if (obj->Implements<UFINSignalListener>()) IFINSignalListener::Execute_HandleSignal(obj, smartAsFINSig(new FicsItKernel::Network::SmartSignal("ItemTransfer", {item})), c);
		}
	}
	CompSS->MutexFactoryHooks.Unlock();
}

bool FactoryGrabHook_Decl(UFGFactoryConnectionComponent_Public*, FInventoryItem&, float&, TSubclassOf<UFGItemDescriptor>);
void FactoryGrabHook(CallScope<decltype(&FactoryGrabHook_Decl)>& scope, UFGFactoryConnectionComponent_Public* c, FInventoryItem& item, float& offset, TSubclassOf<UFGItemDescriptor> type) {
	LockFactoryGrab(c);
	scope(c, item, offset, type);
	if (UnlockFactoryGrab(c) && scope.getResult()) {
		DoFactoryGrab(c, item);
	}
}

bool FactoryGrabInternalHook_Decl(UFGFactoryConnectionComponent* c, FInventoryItem& item, TSubclassOf< UFGItemDescriptor > type);
void FactoryGrabInternalHook(CallScope<decltype(&FactoryGrabInternalHook_Decl)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, TSubclassOf< UFGItemDescriptor > type) {
	LockFactoryGrab(c);
	scope(c, item, type);
	if (UnlockFactoryGrab(c) && scope.getResult()) {
		DoFactoryGrab(c, item);
	}
}

void TickCircuitHook_Decl(UFGPowerCircuit_Public*, float);
void TickCircuitHook(CallScope<decltype(&TickCircuitHook_Decl)>& scope, UFGPowerCircuit_Public* circuit, float dt) {
	bool oldFused = circuit->IsFuseTriggered();
	scope(circuit, dt);
	bool fused = circuit->IsFuseTriggered();
	if (oldFused != fused) try {
		AFINComputerSubsystem* CompSS = AFINComputerSubsystem::GetComputerSubsystem(circuit);
		CompSS->MutexPowerCircuitListeners.Lock();
		auto listeners = CompSS->PowerCircuitListeners.Find(circuit);
		if (listeners) for (auto& listener : *listeners) {
			UObject* obj = *listener;
			if (obj->Implements<UFINSignalListener>()) IFINSignalListener::Execute_HandleSignal(obj, smartAsFINSig(new FicsItKernel::Network::SmartSignal("PowerFuseChanged")), listener);
		}
		CompSS->MutexPowerCircuitListeners.Unlock();
	} catch (...) {}
}

void FFicsItNetworksModule::StartupModule(){
	FSubsystemInfoHolder::RegisterSubsystemHolder(UFINSubsystemHolder::StaticClass());
	
	#ifndef WITH_EDITOR
	finConfig->SetNumberField("SignalQueueSize", 32);
	finConfig = SML::readModConfig(MOD_NAME, finConfig);
	#endif
	
	SUBSCRIBE_METHOD_MANUAL("?SetupComponent@AFGBuildableHologram@@MEAAPEAVUSceneComponent@@PEAV2@PEAVUActorComponent@@AEBVFName@@@Z", AFGBuildableHologram_Public::SetupComponentFunc, &Holo_SetupComponent);

	SUBSCRIBE_METHOD_MANUAL("?Factory_GrabOutput@UFGFactoryConnectionComponent@@QEAA_NAEAUFInventoryItem@@AEAMV?$TSubclassOf@VUFGItemDescriptor@@@@@Z", UFGFactoryConnectionComponent_Public::Factory_GrabOutput, &FactoryGrabHook);
	SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::Factory_Internal_GrabOutputInventory, &FactoryGrabInternalHook);

	SUBSCRIBE_METHOD_MANUAL("?TickCircuit@UFGPowerCircuit@@MEAAXM@Z", UFGPowerCircuit_Public::TickCircuit, TickCircuitHook);

	SUBSCRIBE_METHOD_MANUAL("?UpdateBestUsableActor@AFGCharacterPlayer@@IEAAXXZ", AFGCharacterPlayer_Public::UpdateBestUsableActor, [](auto& scope, AFGCharacterPlayer_Public* self) {
		if (!UFINComponentUtility::bAllowUsing) scope.Cancel();
	});

	SUBSCRIBE_METHOD(AFGBuildable::Dismantle_Implementation, [](auto& scope, AFGBuildable* self_r) {
		IFGDismantleInterface* disInt = reinterpret_cast<IFGDismantleInterface*>(self_r);
		AFGBuildable* self = dynamic_cast<AFGBuildable*>(disInt);
		TInlineComponentArray<UFINNetworkConnector*> connectors;
		self->GetComponents(connectors);
		TInlineComponentArray<UFINNetworkAdapterReference*> adapters;
		self->GetComponents(adapters);
		TInlineComponentArray<UFINModuleSystemPanel*> panels;
		self->GetComponents(panels);
		for (UFINNetworkAdapterReference* adapter_ref : adapters) {
			if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
				connectors.Add(adapter->Connector);
			}
		}
		for (UFINNetworkConnector* connector : connectors) {
			for (AFINNetworkCable* cable : connector->Cables) {
				cable->Execute_Dismantle(cable);
			}
		}
		for (UFINNetworkAdapterReference* adapter_ref : adapters) {
			if (AFINNetworkAdapter* adapter = adapter_ref->Ref) {
				adapter->Destroy();
			}
		}
		for (UFINModuleSystemPanel* panel : panels) {
			TArray<AActor*> modules;
			panel->GetModules(modules);
			for (AActor* module : modules) {
				module->Destroy();
			}
		}
	})

	SUBSCRIBE_METHOD_MANUAL("?GetDismantleBlueprintReturns@AFGBuildable@@IEBAXAEAV?$TArray@UFInventoryStack@@VFDefaultAllocator@@@@@Z", GetDismantleRefund_Decl, &GetDismantleRefund);
	
	AFINNetworkAdapter::RegisterAdapterSettings();
	FicsItKernel::Lua::LuaLib::get()->registerLib();
}
void FFicsItNetworksModule::ShutdownModule(){ }

extern "C" DLLEXPORT void BootstrapModule(std::ofstream& logFile) {
	
}
