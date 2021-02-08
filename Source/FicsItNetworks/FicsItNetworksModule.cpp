#include "FicsItNetworksModule.h"

#include "CoreMinimal.h"
#include "CoreRedirects.h"
#include "Components/StaticMeshComponent.h"

#include "FGBuildable.h"
#include "FGBuildableHologram.h"
#include "FGCharacterPlayer.h"
#include "FGFactoryConnectionComponent.h"
#include "FGGameMode.h"

#include "SML/mod/hooking.h"

#include "FINComponentUtility.h"
#include "FINGlobalRegisterHelper.h"
#include "FINSubsystemHolder.h"
#include "Computer/FINComputerRCO.h"
#include "Network/FINNetworkConnectionComponent.h"
#include "Network/FINNetworkAdapter.h"
#include "Network/FINNetworkCable.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Reflection/FINReflection.h"
#include "UI/FINReflectionStyles.h"

DEFINE_LOG_CATEGORY(LogFicsItNetworks);
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
		comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
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

class AActor_public : public AActor {
	friend void FFicsItNetworksModule::StartupModule();
};

bool FINRefLoaded = false;
#pragma optimize("", off)
void FFicsItNetworksModule::StartupModule(){
	FSubsystemInfoHolder::RegisterSubsystemHolder(UFINSubsystemHolder::StaticClass());

	TArray<FCoreRedirect> redirects;
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINNetworkConnector"), TEXT("/Script/FicsItNetworks.FINAdvancedNetworkConnectionComponent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Game/FicsItNetworks/Components/Splitter/Splitter.Splitter_C"), TEXT("/Game/FicsItNetworks/Components/CodeableSplitter/CodeableSplitter.CodeableSplitter_C")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Game/FicsItNetworks/Components/Merger/Merger.Merger_C"), TEXT("/Game/FicsItNetworks/Components/CodeableMerger/CodeableMerger.CodeableMerger_C")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Property, TEXT("/Game/FicsItNetworks/Components/CodeableSplitter/CodeableSplitter.InputConnector"), TEXT("Input1")});
	FCoreRedirects::AddRedirectList(redirects, "FIN-Code");
	
	SUBSCRIBE_METHOD_MANUAL("?SetupComponent@AFGBuildableHologram@@MEAAPEAVUSceneComponent@@PEAV2@PEAVUActorComponent@@AEBVFName@@@Z", AFGBuildableHologram_Public::SetupComponentFunc, &Holo_SetupComponent);

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

	SUBSCRIBE_VIRTUAL_FUNCTION_AFTER(AFGCharacterPlayer, AActor_public::BeginPlay, [](AActor* self) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(self);
        if (character) {
	        AFINComputerSubsystem* SubSys = AFINComputerSubsystem::GetComputerSubsystem(self->GetWorld());
        	if (SubSys) SubSys->AttachWidgetInteractionToPlayer(character);
		}
	})

	SUBSCRIBE_VIRTUAL_FUNCTION_AFTER(AFGCharacterPlayer, AActor::EndPlay, [](AActor* self, EEndPlayReason::Type Reason) {
        AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(self);
		if (character) {
			AFINComputerSubsystem* SubSys = AFINComputerSubsystem::GetComputerSubsystem(self->GetWorld());
			if (SubSys) SubSys->DetachWidgetInteractionToPlayer(character);
		}
    })

	SUBSCRIBE_METHOD(AFGGameMode::PostLogin, [](auto& scope, AFGGameMode* gm, APlayerController* pc) {
	    if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
		    UClass* ModuleRCO = LoadObject<UClass>(NULL, TEXT("/Game/FicsItNetworks/Components/ModularPanel/Modules/Module_RCO.Module_RCO_C"));
	        gm->RegisterRemoteCallObjectClass(UFINComputerRCO::StaticClass());
	    	gm->RegisterRemoteCallObjectClass(ModuleRCO);

	    }
	});

	SUBSCRIBE_METHOD_AFTER(AGameMode::StartMatch, [](AGameMode* World) {
		if (!FINRefLoaded) {
			FINRefLoaded = true;
		}
	});


	SUBSCRIBE_VIRTUAL_FUNCTION_AFTER(AFGGameState, AFGGameState::Init, [](AFGGameState* state) {
		FSlateStyleRegistry::UnRegisterSlateStyle(FFINReflectionStyles::GetStyleSetName());
		FFINReflectionStyles::Initialize();
		
		AFINNetworkAdapter::RegisterAdapterSettings();
		FFINGlobalRegisterHelper::Register();
		
	    FFINReflection::Get()->PopulateSources();
		FFINReflection::Get()->LoadAllTypes();
		FFINReflection::Get()->PrintReflection();
	})

#if WITH_EDITOR
	AFINNetworkAdapter::RegisterAdapterSettings();
	FFINGlobalRegisterHelper::Register();
		
	FFINReflection::Get()->PopulateSources();
	FFINReflection::Get()->LoadAllTypes();
#endif
}
#pragma optimize("", on)

void FFicsItNetworksModule::ShutdownModule() {
	FFINReflectionStyles::Shutdown();
}

extern "C" DLLEXPORT void BootstrapModule(std::ofstream& logFile) {
	
}
