#include "..\SatisfactoryModLoader\SatisfactoryModLoader\util\Objects\FGuid.h"
#include "stdafx.h"
#include "NetworkConnector.h"

#include <mod/ModFunctions.h>
#include <assets/AssetFunctions.h>
#include <assets/FObjectSpawnParameters.h>
#include <game/Global.h>

#include <util/Objects/UClass.h>
#include <util/Objects/UProperty.h>

#include <sstream>

using namespace SML;
using namespace SML::Mod;
using namespace SML::Objects;
using namespace SML::Paks;

void(UNetworkConnector::*beginPlay_f)() = nullptr;

void UNetworkConnector::construct() {
	// actor vtable hook
	if (!beginPlay_f) {
		auto& f = ((void(UNetworkConnector::**)())this->Vtable)[0x5F];
		beginPlay_f = f;
		f = &UNetworkConnector::beginPlay;
	}

	// IFGSaveInterface first implementation
	IFGSaveInterface i;
	saveI.Vtable = *((void**)new IFGSaveInterface());

	new (&component) INetworkConnectorComponent();

	new (&connections) std::unordered_set<UNetworkConnector*>();
	new (&cables) std::unordered_set<SDK::AFGBuildable*>();
	new (&components) std::unordered_set<UObject*>();
	new (&merged) std::unordered_set<UObject*>();
	new (&id) FGuid();

	idCreated = false;
	maxCables = -1;

	merged.insert((Objects::UObject*)this->GetOwner());
}

void UNetworkConnector::destruct() {
	connections.~unordered_set();
	cables.~unordered_set();
	components.~unordered_set();
	merged.~unordered_set();
}

void UNetworkConnector::beginPlay() {
	(this->*beginPlay_f)();

	static auto nguid = (FGuid(*)()) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FGuid::NewGuid");
	if (!idCreated) {
		id = nguid();
		idCreated = true;
	}
}

void UNetworkConnector::addConnection(UNetworkConnector* connector) {
	if (connections.find(connector) != connections.end()) return;

	connections.insert(connector);
	connector->addConnection(this);
}

void UNetworkConnector::removeConnection(UNetworkConnector* connector) {
	if (connections.find(connector) == connections.end()) return;

	connections.erase(connector);
	connector->removeConnection(this);
}

class TestI {
	void** vtable;
};

class Test : public SDK::AActor, public TestI {

};

bool UNetworkConnector::addCable(SDK::AFGBuildable * cable) {
	if (maxCables >= 0 && maxCables <= cables.size()) return false;
	cables.insert(cable);
	return true;
}

void UNetworkConnector::removeCable(SDK::AFGBuildable * cable) {
	cables.erase(cable);
}

bool UNetworkConnector::isConnected(UNetworkConnector* con) {
	static UProperty* p1 = nullptr;
	static UProperty* p2 = nullptr;
	if (!p1 || !p2) {
		auto c = (UClass*)Functions::loadObjectFromPak(L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkCable/NetworkCable.NetworkCable_C");
		p1 = c->findField<UProperty>("Connector1");
		p2 = c->findField<UProperty>("Connector2");
	}

	if (connections.find(con) != connections.end()) return true;
	for (auto c : cables) {
		auto c1 = *p1->getValue<UNetworkConnector*>(c);
		auto c2 = *p2->getValue<UNetworkConnector*>(c);
		if (c1 == con || c2 == con) return true;
	}
	return false;
}

UNetworkConnector * INetworkConnectorComponent::self() const {
	return (UNetworkConnector*)((size_t)this - offsetof(UNetworkConnector, component));;
}

FGuid INetworkConnectorComponent::getID() const {
	return self()->id;
}

TArray<UObject*> INetworkConnectorComponent::getMerged() const {
	auto s = self();
	TArray<UObject*> arr;
	for (auto m : s->merged) {
		arr.add((UObject*)m);
	}
	return arr;
}

TArray<UObject*> INetworkConnectorComponent::getConnected() const {
	static UProperty* p1 = nullptr;
	static UProperty* p2 = nullptr;

	auto s = self();
	TArray<UObject*> arr;
	for (auto c : s->connections) {
		if (c && ((UObject*)c)->isValid()) arr.add((UObject*)c);
	}
	for (auto c : s->cables) {
		if (!p1 || !p2) {
			auto cs = (UClass*)c->Class;
			p1 = cs->findField<UProperty>("Connector1");
			p2 = cs->findField<UProperty>("Connector2");
		}
		auto c1 = *p1->getValue<UObject*>(c);
		auto c2 = *p2->getValue<UObject*>(c);
		if (c1 && c1->isValid()) arr.add(c1);
		if (c2 && c2->isValid()) arr.add(c2);
	}
	for (auto c : s->components) {
		if (c || ((UObject*)c)->isValid()) arr.add((UObject*)c);
	}
	return arr;
}

UObject * INetworkConnectorComponent::findComponent(FGuid guid) const {
	std::set<UObject*> searched;
	return INetworkComponent::findComponent(guid, searched, (UObject*)self());
}

void UNetworkConnector::execAddConn(UNetworkConnector* self, FFrame& stack, void* params) {
	UNetworkConnector* con;
	stack.stepCompIn(&con);

	stack.code += !!stack.code;

	self->addConnection(con);
}

void UNetworkConnector::execRemConn(UNetworkConnector* self, FFrame& stack, void* params) {
	UNetworkConnector* con;
	stack.stepCompIn(&con);

	stack.code += !!stack.code;

	self->removeConnection(con);
}


void UNetworkConnector::execAddCable(UNetworkConnector* self, FFrame& stack, void* params) {
	SDK::AFGBuildable* cable;
	stack.stepCompIn(&cable);

	stack.code += !!stack.code;

	*((bool*)params) = self->addCable(cable);
}

void UNetworkConnector::execRemCable(UNetworkConnector* self, FFrame& stack, void* params) {
	SDK::AFGBuildable* cable;
	stack.stepCompIn(&cable);

	stack.code += !!stack.code;

	self->removeCable(cable);
}

void UNetworkConnector::execAddMerged(SML::Objects::FFrame & stack, void * params) {
	SML::Objects::UObject* m;
	stack.stepCompIn(&m);

	stack.code += !!stack.code;

	merged.insert(m);
}

void UNetworkConnector::execAddComponent(SML::Objects::FFrame & stack, void * params) {
	SML::Objects::UObject* c;
	stack.stepCompIn(&c);

	stack.code += !!stack.code;

	components.insert(c);
}

SML::Objects::UClass * UNetworkConnector::staticClass() {
	return Paks::ClassBuilder<UNetworkConnector>::staticClass();
}

