#include <stdafx.h>
#include <string>
#include <game/Global.h>
#include <game/Input.h>
#include <game/SDKHooks.h>
#include <mod/Mod.h>
#include <HookLoaderInternal.h>
#include <mod/ModFunctions.h>
#include <mod/MathFunctions.h>
#include <assets/AssetFunctions.h>
#include <assets/FObjectSpawnParameters.h>
#include <detours.h>
#include <memory/MemoryObject.h>
#include <memory/MemoryFunctions.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <tchar.h>
#include <sstream>

#include <map>
#include <vector>
#include <memory>
#include <limits>

#include <util/Objects/UObject.h>
#include <util/Objects/FName.h>
#include <util/Objects/UFunction.h>
#include <util/Objects/UProperty.h>
#include <util/Objects/UClass.h>
#include <assets/BPInterface.h>
#include <util/Objects/FFrame.h>

#include "Config.h"

#include "LuaLib.h"
#include "LUAContext.h"
#include "NetworkCable_Holo.h"
#include "NetworkConnector.h"
#include "NetworkAdapter.h"
#include "ComponentUtility.h"
#include "NetworkComponent.h"
#include "FileSystem.h"
#include "LuaImplementation.h"
#include "ModuleSystemPanel.h"
#include "ModuleSystemHolo.h"
#include "Equip_FileSystem.h"
#include "NetworkCircuit.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

using namespace SML;
using namespace SML::Mod;
using namespace SML::Objects;

#define LOG(msg) SML::Utility::infoMod(MOD_NAME, msg)
#define INFO(msg) LOG(msg)
#define WARN(msg) SML::Utility::warningMod(MOD_NAME, msg)
#define ERR(msg) SML::Utility::errorMod(MOD_NAME, msg)

SML::Mod::Mod::Info modInfo {
	SML_VERSION,
	MOD_NAME,
	"0.0.1",
	"FicsIt-Networks is a mod for Satisfactory written in >= SML 1.1 which allows you to control, mointor, manage and automate each process of your factory by providing a network system and programmable computers and other I/O.",
	"Panakotta00, CoderDE, MassiveBytes",
	{}
};

bool idk = true;

void bitFunc(void* b) {
	*((bool*)b) = 1;
}

SDK::USceneComponent*(*FGBuildable_SetupComponent_f)(SDK::AFGBuildable*, SDK::USceneComponent*, SDK::UActorComponent*, const FName&) = nullptr;
SDK::USceneComponent* FGBuildable_SetupComponent(SDK::AFGBuildable* self, SDK::USceneComponent* attachParent, SDK::UActorComponent* componentTemplate, const FName& componentName) {
	static SDK::UStaticMesh* mesh = nullptr;
	if (!mesh) mesh = (SDK::UStaticMesh*)Functions::loadObjectFromPak(SDK::UStaticMesh::StaticClass(), L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/Mesh_NetworkConnectorHolo.Mesh_NetworkConnectorHolo");
	static void(*setupAttach)(SDK::USceneComponent*, SDK::USceneComponent*, FName) = nullptr;
	if (!setupAttach) setupAttach = (void(*)(SDK::USceneComponent*, SDK::USceneComponent*, FName))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "USceneComponent::SetupAttachment");
	static void(*regComp)(SDK::AActor*, SDK::UActorComponent*);
	if (!regComp) regComp = (void(*)(SDK::AActor*, SDK::UActorComponent*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AActor::FinishAndRegisterComponent");

	if (componentTemplate->IsA((SDK::UClass*)UNetworkConnector::staticClass())) {
		auto comp = (SDK::UStaticMeshComponent*) ((UClass*)SDK::UStaticMeshComponent::StaticClass())->newObj((UObject*)self);
		comp->SetMobility(SDK::EComponentMobility::Movable);
		setupAttach(comp, attachParent, FName());
		comp->SetStaticMesh(mesh);
		comp->K2_SetRelativeTransform(((SDK::USceneComponent*)componentTemplate)->GetRelativeTransform(), false, true, nullptr);
		regComp(self, comp);
		return comp;
	} else {
		return FGBuildable_SetupComponent_f(self, attachParent, componentTemplate, componentName);
	}
}

void(*FGBuildable_Dismantle_f)(SDK::AFGBuildable*) = nullptr;
void FGBuildable_Dismantle(SDK::AFGBuildable* self_i) {
	static SDK::UClass* cable = nullptr;
	if (!cable) cable = (SDK::UClass*)Functions::loadObjectFromPak(L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkCable/NetworkCable.NetworkCable_C");
	auto self = (SDK::AFGBuildable*)((size_t)self_i - (size_t)0x328); // sizeof(SDK::AActor);
	
	if (!self->IsA(cable)) {
		TArray<SDK::UActorComponent*> comps = self->GetComponentsByClass(SDK::UActorComponent::StaticClass());
		for (auto comp : comps) {
			if (!comp) continue;
			if (comp->IsA((SDK::UClass*) UNetworkConnector::staticClass())) {
				for (auto c : ((UNetworkConnector*)comp)->cables) {
					if (!c) continue;
					auto d = (UObject*)c;
					auto f = d->findFunction(L"Dismantle");
					f->invoke(d);
					c->K2_DestroyActor();
				}
			} else if (comp->IsA((SDK::UClass*) UNetworkAdapterReference::staticClass())) {
				auto a = ((UNetworkAdapterReference*)comp);
				for (auto c : a->ref->connector->cables) {
					if (!c) continue;
					auto d = (UObject*)c;
					auto f = d->findFunction(L"Dismantle");
					f->invoke(d);
					c->K2_DestroyActor();
				}
				a->ref->K2_DestroyActor();
			}
		}
	}

	FGBuildable_Dismantle_f(self_i);
}

void(*FGBuildable_GetDismantleRefund_f)(SDK::AFGBuildable* self, TArray<SDK::FInventoryStack>& refund) = nullptr;
void FGBuildable_GetDismantleRefund(SDK::AFGBuildable* self_i, TArray<SDK::FInventoryStack>& refund) {
	static SDK::UClass* cable = nullptr;
	if (!cable) cable = (SDK::UClass*)Functions::loadObjectFromPak(L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkCable/NetworkCable.NetworkCable_C");
	auto self = (SDK::AFGBuildable*)((size_t)self_i - (size_t)0x328); // sizeof(SDK::AActor);
	
	if (!self->IsA(cable)) {
		auto f = (UProperty*)((UObject*)self)->clazz->childs;
		while (f) {
			if (f->clazz->castFlags & EClassCastFlags::CAST_UObjectProperty) {
				auto o = *f->getValue<UNetworkConnector*>(self);
				if (o && o->IsA((SDK::UClass*) UNetworkConnector::staticClass())) {
					for (auto c : o->cables) {
						auto f = ((UObject*)c)->findFunction(L"GetDismantleRefund");
						auto* r = &refund;
						f->invoke((UObject*)c, (SDK::TArray<SDK::FInventoryStack>*) r);
					}
				}
			}
			f = (UProperty*)f->next;
		}
	}
	FGBuildable_GetDismantleRefund_f(self_i, refund);
}

bool(*FGFactoryConnectionCompinent_Factory_GrabOutput_f)(SDK::UFGFactoryConnectionComponent*, SDK::FInventoryItem&, float&, UClass*) = nullptr;
bool FGFactoryConnectionCompinent_Factory_GrabOutput(SDK::UFGFactoryConnectionComponent* c, SDK::FInventoryItem& item, float& offset, UClass* type) {
	bool r = FGFactoryConnectionCompinent_Factory_GrabOutput_f(c, item, offset, type);
	if (r) {
		FactoryHook* hook = nullptr;
		try {
			hook = &hooks.at((UObject*)c);
		} catch (...) {
			hook = nullptr;
		}
		if (hook) {
			struct SignalParams {
				FString str;
				SDK::UClass* item;
			};
			
			hook->update();
			hook->iperm.push(std::chrono::high_resolution_clock::now());
			SignalParams params{"ItemTransfer", item.ItemClass};
			for (auto c : hook->deleg) {
				c->pushSignal(new SignalFactoryHook((UClass*)item.ItemClass));
			}
		}
	}
	return r;
}

void(*FGPowerCircuit_TickCircuit_f)(SDK::UFGPowerCircuit*,float) = nullptr;
void FGPowerCircuit_TickCircuit(SDK::UFGPowerCircuit* circuit, float dt) {
	bool fused = circuit->mIsFuseTriggered;
	FGPowerCircuit_TickCircuit_f(circuit, dt);
	if (!fused && circuit->mIsFuseTriggered) try {
		auto& listeners = powerCircuitListeners.at((SML::Objects::UObject*)circuit);
		for (auto& listener : listeners) {
			auto l = (ULuaContext*)*listener;
			if (l) l->pushSignal(new SignalPowerFused());
		}
	} catch (...) {}
}

void(*FGCharacterPlayer_UpdateBestUsableActor_f)(SDK::AFGCharacterPlayer*) = nullptr;
void FGCharacterPlayer_UpdateBestUsableActor(SDK::AFGCharacterPlayer* self) {
	if (UComponentUtility::allowUsing) FGCharacterPlayer_UpdateBestUsableActor_f(self);
}

UClass* saveI() {
	return UObject::findClass("FactoryGame.FGSaveInterface");
}

struct BoolRetVal {
	bool retVal;
};

void loadClasses() {
	Utility::warning("Load custom classes...");

	auto fguid_c = (void*(*)()) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "Z_Construct_UScriptStruct_FGuid");
	auto fhitresult_c = (void*(*)()) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "Z_Construct_UScriptStruct_FHitResult");
	auto fistack_c = (void*(*)()) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "Z_Construct_UScriptStruct_FInventoryStack");
	auto fiitem_c = (void*(*)()) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "Z_Construct_UScriptStruct_FInventoryItem");

	Paks::ClassBuilder<UModuleSystemPanel>::Basic()
		.extendSDK<SDK::USceneComponent>()
		.construct(&UModuleSystemPanel::construct)
		.destruct(&UModuleSystemPanel::destruct)
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Int, "modulePanelWidth").addParamFlags(Prop_DisableEditOnInstance | Prop_Edit).remParamFlags((EPropertyFlags)(Prop_SkipSerialization | Prop_BlueprintVisible)))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Int, "modulePanelHeight").addParamFlags(Prop_DisableEditOnInstance | Prop_Edit).remParamFlags((EPropertyFlags)(Prop_SkipSerialization | Prop_BlueprintVisible)))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Array, "allowedModules").addParamFlags(Prop_DisableEditOnInstance | Prop_Edit).remParamFlags((EPropertyFlags)(Prop_SkipSerialization | Prop_BlueprintVisible)).off(offsetof(UModuleSystemPanel, allowedModules))).prop(Paks::PropertyBuilder::attrib(EPropertyClass::Class, "allowedModules").off(offsetof(UModuleSystemPanel, allowedModules)))
		.func(Paks::FunctionBuilder::method("addModule").native(&UModuleSystemPanel::execAddModule).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "module").classFunc(SDK::AActor::StaticClass)).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "x")).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "y")).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "rot")))
		.func(Paks::FunctionBuilder::method("removeModule").native(&UModuleSystemPanel::execRemoveModule).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "module").classFunc(SDK::AActor::StaticClass)))
		.func(Paks::FunctionBuilder::method("getModule").native(&UModuleSystemPanel::execGetModule).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "x")).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "y")).param(Paks::PropertyBuilder::retVal(EPropertyClass::Object, "retVal").classFunc(SDK::AActor::StaticClass)))
		.func(Paks::FunctionBuilder::method("getModules").native(&UModuleSystemPanel::execGetModules).param(Paks::PropertyBuilder::outParam(EPropertyClass::Array, "retVal").off(0)).param(Paks::PropertyBuilder::outParam(EPropertyClass::Object, "retVal").off(0).classFunc(SDK::AActor::StaticClass)))
		.func(Paks::FunctionBuilder::method("GetPanelDismantleRefund").native(&UModuleSystemPanel::execGetDismantleRefund).param(Paks::PropertyBuilder::outParam(EPropertyClass::Array, "retVal").off(0)).param(Paks::PropertyBuilder::outParam(EPropertyClass::Struct, "retVal").off(0).structFunc(fistack_c)))
		.build();

	Paks::ClassBuilder<UNetworkCircuit>::Basic()
		.extend<SML::Objects::UObject>()
		.construct(&UNetworkCircuit::construct)
		.destruct(&UNetworkCircuit::destruct)
		.func(Paks::FunctionBuilder::method("getComponents").native(&UNetworkCircuit::getComponents_exec).param(Paks::PropertyBuilder::retVal(EPropertyClass::Array, "retVal")).param(Paks::PropertyBuilder::retVal(EPropertyClass::Object, "retVal").off(0)))
		.build();

	Paks::EnumBuilder<ELuaState>::get()
		.param("HALTED", 0)
		.param("RUNNING", 1)
		.param("FINISHED", 2)
		.param("CRASHED", 3)
		.build();

	Paks::ClassBuilder<UNetworkComponent>::Interface()
		.func(Paks::FunctionBuilder::method("getID").native(&UNetworkComponent::getID_exec).addFuncFlags(EFunctionFlags::FUNC_Event | EFunctionFlags::FUNC_BlueprintEvent | EFunctionFlags::FUNC_Const).param(Paks::PropertyBuilder::retVal(EPropertyClass::Struct, "retVal").off(0).structFunc(fguid_c)))
		.func(Paks::FunctionBuilder::method("getMerged").native(&UNetworkComponent::getMerged_exec).addFuncFlags(EFunctionFlags::FUNC_Event | EFunctionFlags::FUNC_BlueprintEvent | EFunctionFlags::FUNC_Const).param(Paks::PropertyBuilder::retVal(EPropertyClass::Array, "retVal").off(0)).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "retVal").off(0)))
		.func(Paks::FunctionBuilder::method("getConnected").native(&UNetworkComponent::getConnected_exec).addFuncFlags(EFunctionFlags::FUNC_Event | EFunctionFlags::FUNC_BlueprintEvent | EFunctionFlags::FUNC_Const).param(Paks::PropertyBuilder::retVal(EPropertyClass::Array, "retVal").off(0)).param(Paks::PropertyBuilder::retVal(EPropertyClass::Object, "retVal").off(0)))
		.func(Paks::FunctionBuilder::method("findComponent").native(&UNetworkComponent::findComponent_exec).addFuncFlags(EFunctionFlags::FUNC_Event | EFunctionFlags::FUNC_BlueprintEvent | EFunctionFlags::FUNC_Const).param(Paks::PropertyBuilder::retVal(EPropertyClass::Object, "retVal").classFunc(UObject::staticClass)).param(Paks::PropertyBuilder::param(EPropertyClass::Struct, "id").structFunc(fguid_c)))
		.func(Paks::FunctionBuilder::method("getCircuit").native(&UNetworkComponent::getCircuit_exec).addFuncFlags(EFunctionFlags::FUNC_Event | EFunctionFlags::FUNC_BlueprintEvent | EFunctionFlags::FUNC_Const).param(Paks::PropertyBuilder::retVal(EPropertyClass::Object, "retVal").classFunc(UNetworkCircuit::staticClass)))
		.func(Paks::FunctionBuilder::method("setCircuit").native(&UNetworkComponent::setCircuit_exec).addFuncFlags(EFunctionFlags::FUNC_Event | EFunctionFlags::FUNC_BlueprintEvent).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "circuit").classFunc(UNetworkCircuit::staticClass)))
		.build();

	Paks::ClassBuilder<UModuleSystemModule>::Interface()
		.func(Paks::FunctionBuilder::method("getModuleSize").native(&UModuleSystemModule::execGetModuleSize).addFuncFlags(EFunctionFlags::FUNC_Event | EFunctionFlags::FUNC_BlueprintEvent | EFunctionFlags::FUNC_Const).param(Paks::PropertyBuilder::outParam(EPropertyClass::Int, "width")).param(Paks::PropertyBuilder::outParam(EPropertyClass::Int, "height")))
		.func(Paks::FunctionBuilder::method("setPanel").native(&UModuleSystemModule::execSetPanel).addFuncFlags(FUNC_Event | FUNC_BlueprintEvent).param(Paks::PropertyBuilder::outParam(EPropertyClass::Object, "panel").classFunc(UModuleSystemPanel::staticClass)).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "x")).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "y")).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "rot")))
		.func(Paks::FunctionBuilder::method("getName").native(&UModuleSystemModule::execGetName).addFuncFlags(FUNC_Event | FUNC_BlueprintEvent).param(Paks::PropertyBuilder::retVal(EPropertyClass::Name, "retVal")))
		.build();

	Paks::ClassBuilder<ULuaImplementation>::Interface()
		.func(Paks::FunctionBuilder::method("luaSetup").native(&ULuaImplementation::execLuaSetup).addFuncFlags(FUNC_Event | FUNC_BlueprintEvent).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "ctx").classFunc(ULuaContext::staticClass)))
		.func(Paks::FunctionBuilder::method("luaAddSignalListener").native(&ULuaImplementation::execAddSignalListener).addFuncFlags(FUNC_Event | FUNC_BlueprintEvent).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "ctx").classFunc(ULuaContext::staticClass)))
		.func(Paks::FunctionBuilder::method("luaRemoveSignalListener").native(&ULuaImplementation::execRemoveSignalListener).addFuncFlags(FUNC_Event | FUNC_BlueprintEvent).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "ctx").classFunc(ULuaContext::staticClass)))
		.func(Paks::FunctionBuilder::method("luaGetSignalListeners").native(&ULuaImplementation::execGetSignalListeners).addFuncFlags(FUNC_Event | FUNC_BlueprintEvent).param(Paks::PropertyBuilder::retVal(EPropertyClass::Array, "retVal").off(0)).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "retVal").off(0).classFunc(ULuaContext::staticClass)))
		.func(Paks::FunctionBuilder::method("luaIsReachableFrom").native(&ULuaImplementation::execIsReachableFrom).addFuncFlags(FUNC_Event | FUNC_BlueprintEvent).param(Paks::PropertyBuilder::retVal(EPropertyClass::Bool, "retVal").helpBool<LuaIsReachableFromParams, &LuaIsReachableFromParams::is>()).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "listener").classFunc(SML::Objects::UObject::staticClass).off(offsetof(LuaIsReachableFromParams, listener))))
		.build();

	Paks::ClassBuilder<ULuaContext>::Basic()
		.extend<UObject>()
		.construct(&ULuaContext::construct)
		.destruct(&ULuaContext::destructor)
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Int, "memory"))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Str, "code").off(offsetof(ULuaContext, code)))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Str, "log"))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Str, "exception"))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Byte, "state").enumFunc(Paks::EnumBuilder<ELuaState>::staticEnum))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "component").classFunc(UObject::staticClass).off(offsetof(ULuaContext, component)))
		.func(Paks::FunctionBuilder::method("Reset").native(&ULuaContext::execReset))
		.func(Paks::FunctionBuilder::method("ExecSteps").native(&ULuaContext::execExecSteps).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "count")))
		.func(Paks::FunctionBuilder::method("SetCode").native(&ULuaContext::execSetCode).param(Paks::PropertyBuilder::param(EPropertyClass::Str, "newCode")))
		.func(Paks::FunctionBuilder::method("signalSlot").native(&ULuaContext::execSignalSlot).param(Paks::PropertyBuilder::param(EPropertyClass::Str, "event")))
		.build();

	Paks::ClassBuilder<ANetworkCable_Holo>::Basic()
		.extendSDK<SDK::AFGBuildableHologram>()
		.construct(&ANetworkCable_Holo::construct)
		.destruct(&ANetworkCable_Holo::destruct)
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "cable").classFunc(&SDK::USplineMeshComponent::StaticClass))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "con").classFunc(&SDK::UStaticMeshComponent::StaticClass))
		.build();

	Paks::ClassBuilder<UNetworkConnector>::Basic()
		.extendSDK<SDK::USceneComponent>()
		.construct(&UNetworkConnector::construct)
		.destruct(&UNetworkConnector::destruct)
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Struct, "id").structFunc(fguid_c).addParamFlags(EPropertyFlags::Prop_SaveGame).remParamFlags(EPropertyFlags::Prop_SkipSerialization))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Bool, "idCreated").addParamFlags(EPropertyFlags::Prop_SaveGame | EPropertyFlags::Prop_Net).remParamFlags(EPropertyFlags::Prop_SkipSerialization).helpBool<UNetworkConnector, &UNetworkConnector::idCreated>())
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Int, "maxCables").off((size_t)&((UNetworkConnector*)nullptr)->maxCables).addParamFlags(EPropertyFlags::Prop_BlueprintVisible | EPropertyFlags::Prop_Edit).remParamFlags(EPropertyFlags::Prop_SkipSerialization))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "circuit").off(offsetof(UNetworkConnector, circuit)))
		.func(Paks::FunctionBuilder::method("addConnection").native(&UNetworkConnector::execAddConn).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "connector").classFunc(Paks::ClassBuilder<UNetworkConnector>::staticClass)))
		.func(Paks::FunctionBuilder::method("removeConnection").native(&UNetworkConnector::execRemConn).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "connector").classFunc(Paks::ClassBuilder<UNetworkConnector>::staticClass)))
		.func(Paks::FunctionBuilder::method("addCable").native(&UNetworkConnector::execAddCable).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "cable").classFunc(SDK::AFGBuildable::StaticClass)).param(Paks::PropertyBuilder::retVal(EPropertyClass::Bool, "retVal").helpBool<BoolRetVal, &BoolRetVal::retVal>()))
		.func(Paks::FunctionBuilder::method("removeCable").native(&UNetworkConnector::execRemCable).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "cable").classFunc(SDK::AFGBuildable::StaticClass)))
		.func(Paks::FunctionBuilder::method("addMerged").native(&UNetworkConnector::execAddMerged).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "merged").classFunc(UObject::staticClass)))
		.func(Paks::FunctionBuilder::method("addComponent").native(&UNetworkConnector::execAddMerged).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "component").classFunc(UObject::staticClass)))
		.func(Paks::FunctionBuilder::method("luaSig_NetworkUpdate").native(&UNetworkConnector::execNetworkUpdate).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "type").off(0)).param(Paks::PropertyBuilder::param(EPropertyClass::Str, "component").off(8)))
		.interfaceImpl(Paks::FImplementedInterfaceParams(saveI, (size_t)&((UNetworkConnector*)nullptr)->saveI))
		.interfaceImpl(Paks::FImplementedInterfaceParams(UNetworkComponent::staticClass, offsetof(UNetworkConnector, component)))
		.interfaceImpl(Paks::FImplementedInterfaceParams(ULuaImplementation::staticClass, offsetof(UNetworkConnector, luaImpl)))
		.build();

	Paks::ClassBuilder<UComponentUtility>::Basic()
		.extendSDK<SDK::UBlueprintFunctionLibrary>()
		.func(Paks::FunctionBuilder::staticFunc("connectPower").native(UComponentUtility::connectPower).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "comp1").classFunc(SDK::UFGPowerConnectionComponent::StaticClass)))
		.func(Paks::FunctionBuilder::staticFunc("disconnectPower").native(UComponentUtility::disconnectPower).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "comp1").classFunc(SDK::UFGPowerConnectionComponent::StaticClass)))
		.func(Paks::FunctionBuilder::staticFunc("getNetworkConnectorFromHit").native(UComponentUtility::getNetworkConnectorFromHit_exec).addFuncFlags(EFunctionFlags::FUNC_BlueprintPure).param(Paks::PropertyBuilder::param(EPropertyClass::Struct, "hit").structFunc(fhitresult_c)).param(Paks::PropertyBuilder::retVal(EPropertyClass::Object, "retVal").classFunc(UNetworkConnector::staticClass)))
		.func(Paks::FunctionBuilder::staticFunc("clipboardCopy").native(UComponentUtility::clipboardCopy).param(Paks::PropertyBuilder::param(EPropertyClass::Str, "str")))
		.func(Paks::FunctionBuilder::staticFunc("setAllowUsing").native(UComponentUtility::setAllowUsing).param(Paks::PropertyBuilder::param(EPropertyClass::Bool, "newUsing").helpBool<BoolRetVal, &BoolRetVal::retVal>()))
		.func(Paks::FunctionBuilder::staticFunc("loadSoundFromFile").native(&UComponentUtility::loadSoundFromFile).param(Paks::PropertyBuilder::param(EPropertyClass::Object, "retVal").classFunc(SDK::USoundWave::StaticClass)).param(Paks::PropertyBuilder::param(EPropertyClass::Str, "file")))
		.build();

	Paks::ClassBuilder<ANetworkAdapter>::Basic()
		.extendSDK<SDK::AActor>()
		.construct(&ANetworkAdapter::constructor)
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "parent").classFunc(SDK::AFGBuildableFactory::StaticClass).saveGame())
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "connector").classFunc(UNetworkConnector::staticClass))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "attachment").classFunc(SDK::UChildActorComponent::StaticClass))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "connectorMesh").classFunc(SDK::UStaticMeshComponent::StaticClass))
		.interfaceImpl(Paks::FImplementedInterfaceParams(saveI, (size_t)&((ANetworkAdapter*)nullptr)->saveI))
		.build();

	Paks::ClassBuilder<UNetworkAdapterReference>::Basic()
		.extendSDK<SDK::UActorComponent>()
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "ref").classFunc(ANetworkAdapter::staticClass))
		.construct(&UNetworkAdapterReference::constructor)
		.build();

	Paks::ClassBuilder<UFileSystem>::Basic()
		.extendSDK<SDK::UActorComponent>()
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Struct, "id").structFunc(fguid_c).saveGame())
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Bool, "idCreated").helpBool<UFileSystem, &UFileSystem::idCreated>().saveGame())
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Int, "capacity"))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Array, "listeners").off(offsetof(UFileSystem, listeners))).prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "listeners").classFunc(ULuaContext::staticClass).off(offsetof(UFileSystem, listeners)))
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "circuit").off(offsetof(UFileSystem, circuit)))
		.func(Paks::FunctionBuilder::method("luaSig_FileSystemChange").native(&UFileSystem::execSigFSC).param(Paks::PropertyBuilder::param(EPropertyClass::Int, "type")).param(Paks::PropertyBuilder::param(EPropertyClass::Str, "oldPath")).param(Paks::PropertyBuilder::param(EPropertyClass::Str, "newPath")))
		.construct(&UFileSystem::constructor)
		.destruct(&UFileSystem::destruct)
		.interfaceImpl({saveI, offsetof(UFileSystem, save)})
		.interfaceImpl({UNetworkComponent::staticClass, offsetof(UFileSystem, component)})
		.interfaceImpl({ULuaImplementation::staticClass, offsetof(UFileSystem, luaImpl)})
		.build();

	Paks::ClassBuilder<AEquip_FileSystem>::Basic()
		.extendSDK<SDK::AFGEquipment>()
		.construct(&AEquip_FileSystem::construct)
		.destruct(&AEquip_FileSystem::destruct)
		.prop(Paks::PropertyBuilder::attrib(EPropertyClass::Object, "FileSystem").classFunc(UFileSystem::staticClass).saveGame())
		.func(Paks::FunctionBuilder::method("MoveSelfToItem").native(&AEquip_FileSystem::moveSelfToItem).param(Paks::PropertyBuilder::param(EPropertyClass::Struct, "retVal").structFunc(fiitem_c)))
		.build()->debug();
	
	Paks::ClassBuilder<AModuleSystemHolo>::Basic()
		.extendSDK<SDK::AFGBuildableHologram>()
		.construct(&AModuleSystemHolo::construct)
		.destruct(&AModuleSystemHolo::destruct)
		.build();

	luaInit();

	// init adapter settings
	ANetworkAdapter::addSetting(L"Buildable/Factory/StorageContainerMk1", L"Build_StorageContainerMk1", {FVector{290,0,400}, SDK::FRotator{0,-90,0}, true, 1});
	ANetworkAdapter::settings.push_back({SDK::AFGBuildableStorage::StaticClass(), AdapterSettings{FVector{0,0,0}, SDK::FRotator{0,0,0}, true, 1}});
}

void(*GameEngine_Start_f)(SDK::UGameEngine*) = nullptr;
void GameEngine_Start(SDK::UGameEngine* self) {
	loadClasses();
	GameEngine_Start_f(self);
}

namespace Test {
	struct UGameEngine {
		void Start() {}
	};
}

DEFINE_METHOD(Test::UGameEngine::Start)

class FicsItNetworks : public Mod {
private:
	void beginPlay(Functions::ModReturns* modReturns, AFGPlayerController* playerIn) {
		const std::wstring bpPath = L"/Game/FactoryGame/FicsIt-Networks/ComputerController.ComputerController_C";
		UClass* clazz = (UClass*)Functions::loadObjectFromPak(bpPath.c_str());



		SDK::FVector position = Functions::makeVector(0, 0, 0);
		SDK::FRotator rotation = Functions::makeRotator(0, 0, 0);
		FActorSpawnParameters spawnParams;
		UObject* controller = (UObject*)::call<&SML::Objects::UWorld::SpawnActor>((SML::Objects::UWorld*)*SDK::UWorld::GWorld, (SDK::UClass*)clazz, &position, &rotation, &spawnParams);
		
		Paks::initBPInterface();
	}

public:
	FicsItNetworks() : Mod(modInfo) {
	}

	void setup() override {
		using namespace std::placeholders;

		SDK::InitSDK();

		::subscribe<&AFGPlayerController::BeginPlay>(std::bind(&FicsItNetworks::beginPlay, this, _1, _2));
		::subscribe<&Objects::AFGGameMode::InitGameState>([](Functions::ModReturns* ret, Objects::AFGGameMode* player) {
			Functions::loadObjectFromPak(L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkManager/ED_NetworkMng.ED_NetworkMng");
		});

		/*::subscribe<&Test::UGameEngine::Start>([](Functions::ModReturns* ret, void* engine) {
			loadClasses();
		});*/

		DetourTransactionBegin();
		GameEngine_Start_f = (void(*)(SDK::UGameEngine*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UGameEngine::Start");
		DetourAttach((void**)&GameEngine_Start_f, &GameEngine_Start);
		DetourTransactionCommit();
		
		DetourTransactionBegin();
		FGBuildable_SetupComponent_f = (SDK::USceneComponent*(*)(SDK::AFGBuildable*, SDK::USceneComponent*, SDK::UActorComponent*, const FName&)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGBuildableHologram::SetupComponent");
		DetourAttach((void**)&FGBuildable_SetupComponent_f, &FGBuildable_SetupComponent);
		DetourTransactionCommit();

		DetourTransactionBegin();
		FGBuildable_Dismantle_f = (void(*)(SDK::AFGBuildable*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGBuildable::Dismantle_Implementation");
		DetourAttach((void**)&FGBuildable_Dismantle_f, &FGBuildable_Dismantle);
		DetourTransactionCommit();

		DetourTransactionBegin();
		FGBuildable_GetDismantleRefund_f = (void(*)(SDK::AFGBuildable*, TArray<SDK::FInventoryStack>&)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGBuildable::GetDismantleRefund_Implementation");
		DetourAttach((void**)&FGBuildable_GetDismantleRefund_f, &FGBuildable_GetDismantleRefund);
		DetourTransactionCommit();

		DetourTransactionBegin();
		FGFactoryConnectionCompinent_Factory_GrabOutput_f = (bool(*)(SDK::UFGFactoryConnectionComponent*, SDK::FInventoryItem&, float&, UClass*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGFactoryConnectionComponent::Factory_GrabOutput");
		DetourAttach((void**)&FGFactoryConnectionCompinent_Factory_GrabOutput_f, &FGFactoryConnectionCompinent_Factory_GrabOutput);
		DetourTransactionCommit();

		DetourTransactionBegin();
		FGCharacterPlayer_UpdateBestUsableActor_f = (void(*)(SDK::AFGCharacterPlayer*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGCharacterPlayer::UpdateBestUsableActor");
		DetourAttach((void**)&FGCharacterPlayer_UpdateBestUsableActor_f, &FGCharacterPlayer_UpdateBestUsableActor);
		DetourTransactionCommit();

		DetourTransactionBegin();
		FGPowerCircuit_TickCircuit_f = (void(*)(SDK::UFGPowerCircuit*,float)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGPowerCircuit::TickCircuit");
		DetourAttach((void**)&FGPowerCircuit_TickCircuit_f, &FGPowerCircuit_TickCircuit);
		DetourTransactionCommit();

		LOG("Finished FicsIt-Networks setup!");
	}

	void postSetup() override {
		// for all men kind
	}

	~FicsItNetworks() {
		LOG("FicsIt-Networks finished cleanup!");
	}
};

MOD_API SML::Mod::Mod* ModCreate() {
	return new FicsItNetworks();
}