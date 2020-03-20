#include "FicsItNetworksModule.h"

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"

#include "FGBuildable.h"
#include "FGBuildableHologram.h"
#include "FGCharacterPlayer.h"

#include "SML/mod/hooking.h"
#include "SML/util/Utility.h"

#include "FINConfig.h"
#include "FINComponentUtility.h"
#include "Network/Signals/FINSignal.h"
#include "Network/FINNetworkConnector.h"

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

void FFicsItNetworksModule::StartupModule(){
	#ifndef WITH_EDITOR
	finConfig->SetNumberField("SignalQueueSize", 32);
	finConfig = SML::readModConfig(MOD_NAME, finConfig);
	#endif

	SUBSCRIBE_METHOD("?SetupComponent@AFGBuildableHologram@@MEAAPEAVUSceneComponent@@PEAV2@PEAVUActorComponent@@AEBVFName@@@Z", AFGBuildableHologram_Public::SetupComponentFunc, [](auto& scope, AFGBuildableHologram_Public* self, USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName) {
		static ConstructorHelpers::FObjectFinder<UStaticMesh> networkConnectorHoloMesh(TEXT("StaticMesh'/Game/FicsIt-Networks/ComputerNetwork/Mesh_NetworkConnectorHolo.Mesh_NetworkConnectorHolo"));

		if (componentTemplate->IsA<UFINNetworkConnector>()) {
			auto comp = NewObject<UStaticMeshComponent>(attachParent);
			comp->SetMobility(EComponentMobility::Movable);
			comp->SetupAttachment(attachParent);
			comp->SetStaticMesh(networkConnectorHoloMesh.Object);
			comp->SetRelativeTransform(Cast<USceneComponent>(componentTemplate)->GetRelativeTransform());
			self->FinishAndRegisterComponent(comp);
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

	SUBSCRIBE_METHOD("", AFGCharacterPlayer_Public::UpdateBestUsableActor, [](auto& scope, AFGCharacterPlayer_Public* self) {
		if (!UFINComponentUtility::bAllowUsing) scope.Cancel();
	});
}
void FFicsItNetworksModule::ShutdownModule(){ }

extern "C" DLLEXPORT void BootstrapModule(std::ofstream& logFile) {
	
}
