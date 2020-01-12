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

bool UNetworkConnector::searchFor(std::set<UNetworkConnector*>& searched, UNetworkConnector* connector) {
	static UProperty* p1 = nullptr;
	static UProperty* p2 = nullptr;
	if (!p1 || !p2) {
		auto c = (UClass*)Functions::loadObjectFromPak(L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkCable/NetworkCable.NetworkCable_C");
		p1 = c->findField<UProperty>("Connector1");
		p2 = c->findField<UProperty>("Connector2");
	}

	if (searched.find(this) != searched.end()) return false;
	searched.insert(this);

	if (this == connector) return true;
	
	for (auto con : connections) {
		if (con->searchFor(searched, connector)) return true;
	}

	for (auto cable : cables) {
		auto c1 = *p1->getValue<UNetworkConnector*>(cable);
		auto c2 = *p2->getValue<UNetworkConnector*>(cable);
		if (c1->searchFor(searched, connector) || c2->searchFor(searched, connector)) return true;
	}

	return false;
}

void UNetworkConnector::removeConnector(UNetworkConnector * connector) {
	if (!searchFor(connector)) {
		connector->circuit = (UNetworkCircuit*)UNetworkCircuit::staticClass()->constructObject((Objects::UObject*)Functions::getWorld(), L"");
		connector->circuit->recalculate((Objects::UObject*)connector);
		
		for (auto n : circuit->nodes) {
			if (connector->circuit->nodes.find(n) != connector->circuit->nodes.end()) continue;
			INetworkComponent* comp;
			try {
				comp = ((INetworkComponent*)((size_t)n.get() + n->clazz->getImplementation(UNetworkComponent::staticClass()).off));
			} catch (...) {
				continue;
			}
			comp->notifyNetworkUpdate(1, connector->circuit->nodes);
		}
		circuit->recalculate((Objects::UObject*)this);
		for (auto n : connector->circuit->nodes) {
			if (circuit->nodes.find(n) != circuit->nodes.end()) continue;
			INetworkComponent* comp;
			try {
				comp = ((INetworkComponent*)((size_t)n.get() + n->clazz->getImplementation(UNetworkComponent::staticClass()).off));
			} catch (...) {
				continue;
			}
			comp->notifyNetworkUpdate(1, circuit->nodes);
		}
	}
}

void UNetworkConnector::construct() {
	// actor vtable hook
	if (!beginPlay_f) {
		auto& f = ((void(UNetworkConnector::**)())this->Vtable)[0x5F];
		beginPlay_f = f;
		f = &UNetworkConnector::beginPlay;
	}

	// IFGSaveInterface first implementation
	new (&saveI) IFGSaveInterface();

	new (&component) INetworkConnectorComponent();
	new (&luaImpl) INetworkConnectorLua();

	new (&connections) std::unordered_set<UNetworkConnector*>();
	new (&cables) std::unordered_set<SDK::AFGBuildable*>();
	new (&components) std::unordered_set<UObject*>();
	new (&merged) std::unordered_set<UObject*>();
	new (&listeners) std::set<FWeakObjectPtr>();
	new (&id) FGuid();

	idCreated = false;
	maxCables = -1;
	circuit = nullptr;

	merged.insert((Objects::UObject*)this->GetOwner());
}

void UNetworkConnector::destruct() {
	component.~INetworkConnectorComponent();
	luaImpl.~INetworkConnectorLua();
	connections.~unordered_set();
	cables.~unordered_set();
	components.~unordered_set();
	merged.~unordered_set();
	listeners.~set();
	id.~FGuid();
}

void UNetworkConnector::beginPlay() {
	(this->*beginPlay_f)();

	static auto nguid = (FGuid(*)()) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FGuid::NewGuid");
	if (!idCreated) {
		id = nguid();
		idCreated = true;
	}

	// setup circuit
	if (!circuit) {
		circuit = (UNetworkCircuit*) UNetworkCircuit::staticClass()->constructObject((Objects::UObject*)Functions::getWorld(), L"");
		circuit->recalculate((Objects::UObject*)this);
	}
}

void UNetworkConnector::addConnection(UNetworkConnector* connector) {
	if (connections.find(connector) != connections.end()) return;

	connections.insert(connector);
	connector->addConnection(this);

	circuit = connector->circuit = (circuit) ? *circuit + connector->circuit : connector->circuit;
}

void UNetworkConnector::removeConnection(UNetworkConnector* connector) {
	if (connections.find(connector) == connections.end()) return;

	connections.erase(connector);
	connector->connections.erase(this);
	removeConnector(connector);
}

class TestI {
	void** vtable;
};

class Test : public SDK::AActor, public TestI {

};

bool UNetworkConnector::addCable(SDK::AFGBuildable * cable) {
	if (maxCables >= 0 && maxCables <= cables.size()) return false;

	static UProperty* p1 = nullptr;
	static UProperty* p2 = nullptr;
	if (!p1 || !p2) {
		auto c = (UClass*)Functions::loadObjectFromPak(L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkCable/NetworkCable.NetworkCable_C");
		p1 = c->findField<UProperty>("Connector1");
		p2 = c->findField<UProperty>("Connector2");
	}

	cables.insert(cable);

	auto c1 = *p1->getValue<UNetworkConnector*>(cable);
	auto c2 = *p2->getValue<UNetworkConnector*>(cable);
	auto c = (c1 == this) ? ((c2 == this) ? nullptr : c2) : c1;

	if (c && c->circuit) {
		if (this->circuit) {
			circuit = c->circuit = *c1->circuit + c2->circuit;
		} else {
			circuit = c->circuit;
			circuit->recalculate((SML::Objects::UObject*)this);
		}
	}

	return true;
}

void UNetworkConnector::removeCable(SDK::AFGBuildable * cable) {
	static UProperty* p1 = nullptr;
	static UProperty* p2 = nullptr;
	if (!p1 || !p2) {
		auto c = (UClass*)Functions::loadObjectFromPak(L"/Game/FactoryGame/FicsIt-Networks/ComputerNetwork/NetworkCable/NetworkCable.NetworkCable_C");
		p1 = c->findField<UProperty>("Connector1");
		p2 = c->findField<UProperty>("Connector2");
	}
	
	if (cables.find(cable) == cables.end()) return;

	cables.erase(cable);

	auto c1 = *p1->getValue<UNetworkConnector*>(cable);
	auto c2 = *p2->getValue<UNetworkConnector*>(cable);
	auto other = (c1 == this) ? c2 : c1;
	other->cables.erase(cable);

	removeConnector(other);
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

bool UNetworkConnector::searchFor(UNetworkConnector * conn) {
	std::set<UNetworkConnector*> searched;
	return searchFor(searched, conn);
}

UNetworkConnector * INetworkConnectorComponent::self() const {
	return (UNetworkConnector*)((size_t)this - offsetof(UNetworkConnector, component));
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
	return INetworkComponent::findComponentFromCircuit(guid);
}

UNetworkCircuit * INetworkConnectorComponent::getCircuit() const {
	return self()->circuit;
}

void INetworkConnectorComponent::setCircuit(UNetworkCircuit * circuit) {
	self()->circuit = circuit;
}

void INetworkConnectorComponent::notifyNetworkUpdate(int type, std::set<SML::Objects::FWeakObjectPtr> nodes) {
	if (self()->listeners.size() < 1) return;
	auto func = ((UObject*)self())->findFunction(L"luaSig_NetworkUpdate");
	for (auto node : nodes) {
		struct {
			std::int32_t t;
			FString n;
		} params;
		params.t = type;
		INetworkComponent* comp;
		try {
			comp = ((INetworkComponent*)((size_t)node.get() + node->clazz->getImplementation(UNetworkComponent::staticClass()).off));
		} catch (...) {
			continue;
		}
		params.n = comp->getID().toStr().c_str();
		func->invoke((UObject*)self(), &params);
	}
}

void UNetworkConnector::execNetworkUpdate(SML::Objects::FFrame & stack, void * params) {
	FString s1;
	int i;
	stack.stepCompIn(&i);
	stack.stepCompIn(&s1);
	stack.code += !!stack.code;
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

UNetworkConnector * INetworkConnectorLua::self() const {
	return (UNetworkConnector*)((size_t)this - offsetof(UNetworkConnector, luaImpl));;
}

void INetworkConnectorLua::luaAddSignalListener(ULuaContext * ctx) {
	if (self()->listeners.find((UObject*)ctx) != self()->listeners.end()) return;
	self()->listeners.insert((UObject*)ctx);
}

void INetworkConnectorLua::luaRemoveSignalListener(ULuaContext * ctx) {
	self()->listeners.erase((UObject*)ctx);
}

TArray<ULuaContext*> INetworkConnectorLua::luaGetSignalListeners() {
	TArray<ULuaContext*> listeners;
	for (auto& listener : self()->listeners) {
		listeners.add((ULuaContext*)*listener);
	}
	return listeners;
}

bool INetworkConnectorLua::luaIsReachableFrom(SML::Objects::UObject * listener) {
	INetworkComponent * comp;
	try {
		comp = ((INetworkComponent*)((size_t)self() + ((SML::Objects::UObject*)self())->clazz->getImplementation(UNetworkComponent::staticClass()).off));
	} catch (...) {
		return false;
	}
	INetworkComponent* lcomp;
	try {
		lcomp = ((INetworkComponent*)((size_t)listener + (listener)->clazz->getImplementation(UNetworkComponent::staticClass()).off));
	} catch (...) {
		return false;
	}
	return comp->findComponent(lcomp->getID());;
}
