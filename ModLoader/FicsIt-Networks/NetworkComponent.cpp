#include "stdafx.h"
#include "NetworkComponent.h"

#include <assets/AssetFunctions.h>

using namespace SML;
using namespace SML::Objects;

void UNetworkComponent::getID_exec(UNetworkComponent * c, FFrame& stack, void * params) {
	stack.code += !!stack.code;

	struct Params {
		SDK::FGuid g;
	};

	((Params*)params)->g = ((INetworkComponent*)c)->getID();
}

void UNetworkComponent::getMerged_exec(UNetworkComponent* c, FFrame& stack, void* params) {
	stack.code += !!stack.code;
	*((Objects::TArray<Objects::UObject*>*)params) = ((INetworkComponent*)c)->getMerged();
}

void UNetworkComponent::getConnected_exec(UNetworkComponent* c, FFrame& stack, void* params) {
	stack.code += !!stack.code;

	*((Objects::TArray<Objects::UObject*>*)params) = ((INetworkComponent*)c)->getConnected();
}

void UNetworkComponent::findComponent_exec(UNetworkComponent * c, SML::Objects::FFrame & stack, void * params) {
	FGuid guid;
	stack.stepCompIn(&guid);

	stack.code += !!stack.code;

	*((Objects::UObject**)params) = ((INetworkComponent*)c)->findComponent(guid);
}

void UNetworkComponent::getCircuit_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params) {
	stack.code += !!stack.code;

	*((UNetworkCircuit**)params) = ((INetworkComponent*)c)->getCircuit();
}

void UNetworkComponent::setCircuit_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params) {
	UNetworkCircuit* circuit;
	stack.stepCompIn(&circuit);

	stack.code += !!stack.code;

	((INetworkComponent*)c)->setCircuit(circuit);
}

UClass * UNetworkComponent::staticClass() {
	return SML::Paks::ClassBuilder<UNetworkComponent>::staticClass();
}

FGuid INetworkComponent::getID() const {
	return FGuid();
}

TArray<UObject*> INetworkComponent::getMerged() const {
	return TArray<UObject*>();
}

TArray<UObject*> INetworkComponent::getConnected() const {
	return TArray<UObject*>();
}

void INetworkComponent::notifyNetworkUpdate(int type, std::set<FWeakObjectPtr> nodes) {}

UObject* INetworkComponent::findComponent(FGuid guid, std::set<UObject*>& searched, UObject* self) const {
	if (searched.find(self) != searched.end()) return nullptr;

	if (getID() == guid) return self;

	searched.insert(self);

	for (auto c : getConnected()) {
		auto f = ((INetworkComponent*)((size_t)c + c->clazz->getImplementation(UNetworkComponent::staticClass()).off))->findComponent(guid, searched, c);
		if (f) return f;
	}

	return nullptr;
}

/*UNetworkCircuit * INetworkComponent::getCircuit() const {
	return circuit;
}

void INetworkComponent::setCircuit(UNetworkCircuit * circuit) {
	this->circuit = circuit;
}*/