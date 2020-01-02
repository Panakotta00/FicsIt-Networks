#include "stdafx.h"
#include "NetworkAdapter.h"

#include <assets/AssetFunctions.h>

#include "LuaLib.h"

using namespace SML;
using namespace SML::Objects;
using namespace SML::Mod;

void(ANetworkAdapter::*beginPlay_f)() = nullptr;
std::vector<std::pair<SDK::UClass*, AdapterSettings>> ANetworkAdapter::settings = std::vector<std::pair<SDK::UClass*, AdapterSettings>>();

struct IFGSaveInterfaceAdapter {
	virtual void preSave(int, int) {}
	virtual void postSave(int, int) {}
	virtual void preLoad(int, int) {}
	virtual void postLoad(int, int) {}
	virtual void gatherDeps(SML::Objects::TArray<SML::Objects::UObject*>*) {}
	virtual bool ShouldSave() {
		return true;
	}
	virtual bool NeedTransform() {
		return true;
	}
};

void ANetworkAdapter::addSetting(std::wstring clazz, AdapterSettings setting) {
	auto c = (SDK::UClass*)Functions::loadObjectFromPak((std::wstring(L"/Game/FactoryGame/") + clazz).c_str());
	settings.push_back({c, setting});
}

void ANetworkAdapter::addSetting(std::wstring path, std::wstring clazz, AdapterSettings setting) {
	addSetting(path + L"/" + clazz + L"." + clazz + L"_C", setting);
}

void ANetworkAdapter::constructor() {
	static SDK::UStaticMesh* mesh = nullptr;
	if (!mesh) mesh = (SDK::UStaticMesh*)Functions::loadObjectFromPak(SDK::UStaticMesh::StaticClass(), L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/Mesh_NetworkConnector.Mesh_NetworkConnector");
	static void(*setupAttach)(SDK::USceneComponent*, SDK::USceneComponent*, FName) = nullptr;
	if (!setupAttach) setupAttach = (void(*)(SDK::USceneComponent*, SDK::USceneComponent*, FName))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "USceneComponent::SetupAttachment");

	// actor vtable hook
	if (!beginPlay_f) {
		auto& f = ((void(ANetworkAdapter::**)())*(void**)this)[0x60];
		beginPlay_f = f;
		f = &ANetworkAdapter::beginPlay;
	}

	// savegame interface
	*((void**)&saveI) = *((void**)new IFGSaveInterfaceAdapter());

	parent = nullptr;
	attachment = nullptr;
	
	auto self = (Objects::UObject*)this;
	
	RootComponent = self->createDefaultSubobjectSDK<SDK::USceneComponent>(L"Root");

	connector = self->createDefaultSubobject<UNetworkConnector>(L"Connector");
	setupAttach(connector, RootComponent, FName());
	
	connectorMesh = self->createDefaultSubobjectSDK<SDK::UStaticMeshComponent>(L"StaticMesh");
	connectorMesh->SetHiddenInGame(true, true);
	setupAttach(connectorMesh, RootComponent, FName());
	connectorMesh->SetStaticMesh(mesh);

	connector->maxCables = 1;
}

void ANetworkAdapter::beginPlay() {
	static void(*regComp)(SDK::UActorComponent*);
	if (!regComp) regComp = (void(*)(SDK::UActorComponent*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UActorComponent::RegisterComponent");
	static void(*setupAttach)(SDK::USceneComponent*, SDK::USceneComponent*, FName) = nullptr;
	if (!setupAttach) setupAttach = (void(*)(SDK::USceneComponent*, SDK::USceneComponent*, FName))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "USceneComponent::SetupAttachment");
	
	K2_SetActorLocationAndRotation(parent->K2_GetActorLocation(), parent->K2_GetActorRotation(), false, true, nullptr);

	(this->*beginPlay_f)();

	connector->merged.insert((Objects::UObject*)parent);

	auto self = (Objects::UObject*)parent;
	attachment = (UNetworkAdapterReference*) UNetworkAdapterReference::staticClass()->newObj(self);
	attachment->ref = this;
	regComp(attachment);

	for (auto setting_entry : settings) {
		auto clazz = setting_entry.first;
		auto setting = setting_entry.second;
		if (!parent->IsA(clazz)) continue;
		K2_AddActorLocalOffset(setting.loc, false, true, nullptr);
		connectorMesh->K2_AddRelativeRotation(setting.rot, false, true, nullptr);
		connectorMesh->SetHiddenInGame(!setting.mesh, true);
		connector->maxCables = setting.maxCables;
		break;
	}
}

UClass * ANetworkAdapter::staticClass() {
	return Paks::ClassBuilder<ANetworkAdapter>::staticClass();
}

void UNetworkAdapterReference::constructor() {
	ref = nullptr;
}

UClass * UNetworkAdapterReference::staticClass() {
	return Paks::ClassBuilder<UNetworkAdapterReference>::staticClass();
}
