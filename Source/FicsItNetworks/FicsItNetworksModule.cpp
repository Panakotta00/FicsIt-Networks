#include "FicsItNetworksModule.h"

#include "CoreMinimal.h"
#include "CoreRedirects.h"
#include "Components/StaticMeshComponent.h"

#include "FGBuildable.h"
#include "FGBuildableHologram.h"
#include "FGCharacterPlayer.h"
#include "FGFactoryConnectionComponent.h"

#include "SML/mod/hooking.h"

#include "FINComponentUtility.h"
#include "FINSubsystemHolder.h"
#include "Computer/FINComputerProcessor.h"
#include "Network/FINNetworkConnectionComponent.h"
#include "Network/FINNetworkAdapter.h"
#include "Network/FINNetworkCable.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Runtime/CoreUObject/Public/Misc/RedirectCollector.h"

#include "FicsItKernel/Processor/Lua/LuaLib.h"

IMPLEMENT_GAME_MODULE(FFicsItNetworksModule, FicsItNetworks);

class AFGBuildableHologram_Public : public AFGBuildableHologram {
public:
	USceneComponent* SetupComponentFunc(USceneComponent*, UActorComponent*, const FName&) { return nullptr; }
};

class UFGFactoryConnectionComponent_Public : public UFGFactoryConnectionComponent {
public:
	bool Factory_GrabOutput(FInventoryItem&, float&, TSubclassOf<UFGItemDescriptor>) { return false; }
};

class AFGCharacterPlayer_Public : public AFGCharacterPlayer {
public:
	void UpdateBestUsableActor() {}
};

USceneComponent* Holo_SetupComponentDecl(AFGBuildableHologram_Public* self, USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName);
void Holo_SetupComponent(CallScope<decltype(&Holo_SetupComponentDecl)>& scope, AFGBuildableHologram_Public* self, USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName) {
	UStaticMesh* networkConnectorHoloMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsItNetworks/Network/Mesh_NetworkConnector.Mesh_NetworkConnector"), NULL, LOAD_None, NULL);
	if (componentTemplate->IsA<UFINNetworkConnectionComponent>()) {
		auto comp = NewObject<UStaticMeshComponent>(attachParent);
		comp->RegisterComponent();
		comp->SetMobility(EComponentMobility::Movable);
		comp->SetStaticMesh(networkConnectorHoloMesh);
		comp->AttachToComponent(attachParent, FAttachmentTransformRules::KeepRelativeTransform);
		comp->SetRelativeTransform(Cast<USceneComponent>(componentTemplate)->GetRelativeTransform());
			
		scope.Override(comp);
	}
}

void GetDismantleRefund_Decl(IFGDismantleInterface*, TArray<FInventoryStack>&);
void GetDismantleRefund(CallScope<decltype(&GetDismantleRefund_Decl)>& scope, IFGDismantleInterface* disInt, TArray<FInventoryStack>& refund) {
	AFGBuildable* self = reinterpret_cast<AFGBuildable*>(disInt);
	if (!self->IsA<AFINNetworkCable>()) {
		TInlineComponentArray<UFINNetworkConnectionComponent*> components;
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
		for (UFINNetworkConnectionComponent* connector : components) {
			for (AFINNetworkCable* cable : connector->ConnectedCables) {
				cable->Execute_GetDismantleRefund(cable, refund);
			}
		}
		for (UFINModuleSystemPanel* panel : panels) {
			panel->GetDismantleRefund(refund);
		}
	}
}

void FFicsItNetworksModule::StartupModule(){
	FSubsystemInfoHolder::RegisterSubsystemHolder(UFINSubsystemHolder::StaticClass());

	TArray<FCoreRedirect> redirects;
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINNetworkConnector"), TEXT("/Script/FicsItNetworks.FINAdvancedNetworkConnectionComponent")});
	FCoreRedirects::AddRedirectList(redirects, "FIN-Code");
	
	#ifndef WITH_EDITOR
	finConfig->SetNumberField("SignalQueueSize", 32);
	finConfig = SML::readModConfig(MOD_NAME, finConfig);
	#endif
	
	SUBSCRIBE_METHOD_MANUAL("?SetupComponent@AFGBuildableHologram@@MEAAPEAVUSceneComponent@@PEAV2@PEAVUActorComponent@@AEBVFName@@@Z", AFGBuildableHologram_Public::SetupComponentFunc, &Holo_SetupComponent);

	SUBSCRIBE_METHOD_MANUAL("?UpdateBestUsableActor@AFGCharacterPlayer@@IEAAXXZ", AFGCharacterPlayer_Public::UpdateBestUsableActor, [](auto& scope, AFGCharacterPlayer_Public* self) {
		if (!UFINComponentUtility::bAllowUsing) scope.Cancel();
	});

	SUBSCRIBE_METHOD(AFGBuildable::Dismantle_Implementation, [](auto& scope, AFGBuildable* self_r) {
		IFGDismantleInterface* disInt = reinterpret_cast<IFGDismantleInterface*>(self_r);
		AFGBuildable* self = dynamic_cast<AFGBuildable*>(disInt);
		TInlineComponentArray<UFINNetworkConnectionComponent*> connectors;
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
		for (UFINModuleSystemPanel* panel : panels) {
			TArray<AActor*> modules;
			panel->GetModules(modules);
			for (AActor* module : modules) {
				module->Destroy();
			}
		}
	})

	SUBSCRIBE_METHOD_MANUAL("?GetDismantleBlueprintReturns@AFGBuildable@@IEBAXAEAV?$TArray@UFInventoryStack@@VFDefaultAllocator@@@@@Z", GetDismantleRefund_Decl, &GetDismantleRefund);

	SUBSCRIBE_VIRTUAL_FUNCTION_AFTER(AFGCharacterPlayer, AActor::OnConstruction, [](AActor* self, const FTransform& t) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(self);
        if (character) {
	        AFINComputerSubsystem::GetComputerSubsystem(self->GetWorld())->AttachWidgetInteractionToPlayer(character);
		}
	})
	
	AFINNetworkAdapter::RegisterAdapterSettings();
	FicsItKernel::Lua::LuaLib::get()->registerLib();
}
void FFicsItNetworksModule::ShutdownModule(){ }

extern "C" DLLEXPORT void BootstrapModule(std::ofstream& logFile) {
	
}
