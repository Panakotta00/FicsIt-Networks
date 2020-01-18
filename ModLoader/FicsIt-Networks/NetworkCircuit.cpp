#include "stdafx.h"
#include "NetworkCircuit.h"
#include "NetworkComponent.h"

using namespace SML;
using namespace SML::Objects;

void UNetworkCircuit::addNodeRecursive(std::set<UObject*>& added, UObject* add) {
	INetworkComponent* comp;
	try {
		comp = ((INetworkComponent*)((size_t)add + add->clazz->getImplementation(UNetworkComponent::staticClass()).off));
	} catch (...) {
		return;
	}

	if (added.find(add) == added.end()) {
		added.insert(add);
		nodes.insert(add);
		comp->setCircuit(this);
		auto connectors = comp->getConnected();
		for (auto connector : connectors) {
			addNodeRecursive(added, connector);
		}
	}
}

void UNetworkCircuit::construct() {
	new (&nodes) std::set<FWeakObjectPtr>();
}

void UNetworkCircuit::destruct() {
	nodes.~set();
}

UNetworkCircuit* UNetworkCircuit::operator+(UNetworkCircuit* circuit) {
	if (this == circuit || !circuit) return this;

	UNetworkCircuit* from = circuit;
	UNetworkCircuit* to = this;

	if (circuit->nodes.size() > nodes.size()) {
		from = this;
		to = circuit;
	}

	for (auto& node : from->nodes) {
		INetworkComponent* comp;
		try {
			comp = ((INetworkComponent*)((size_t)node.get() + node->clazz->getImplementation(UNetworkComponent::staticClass()).off));
		} catch (...) {
			continue;
		}
		comp->setCircuit(to);
		comp->notifyNetworkUpdate(0, to->nodes);
	}
	for (auto& node : to->nodes) {
		INetworkComponent* comp;
		try {
			comp = ((INetworkComponent*)((size_t)node.get() + node->clazz->getImplementation(UNetworkComponent::staticClass()).off));
		} catch (...) {
			continue;
		}
		comp->notifyNetworkUpdate(0, from->nodes);
	}
	to->nodes.merge(from->nodes);

	return to;
}

void UNetworkCircuit::recalculate(UObject * component) {
	nodes.clear();

	std::set<UObject*> added;
	addNodeRecursive(added, component);
}

bool UNetworkCircuit::hasNode(SML::Objects::UObject* node) {
	return nodes.find(node) != nodes.end();
}

SML::Objects::UObject* UNetworkCircuit::findComponent(SML::Objects::FGuid addr) {
	for (auto node : nodes) {
		INetworkComponent* comp;
		try {
			comp = ((INetworkComponent*)((size_t)node.get() + node->clazz->getImplementation(UNetworkComponent::staticClass()).off));
		} catch (...) {
			continue;
		}

		if (comp->getID() == addr) {
			return *node;
		}
	}

	return nullptr;
}

SML::Objects::UObject * UNetworkCircuit::findComponentByNick(SML::Objects::FString nick) {
	for (auto node : nodes) {
		INetworkComponent* comp;
		try {
			comp = ((INetworkComponent*)((size_t)node.get() + node->clazz->getImplementation(UNetworkComponent::staticClass()).off));
		} catch (...) {
			continue;
		}

		if (comp->getNick().toStr() == nick.toStr()) {
			return *node;
		}
	}

	return nullptr;
}

void UNetworkCircuit::getComponents_exec(SML::Objects::FFrame & stack, void * retVals) {
	stack.code += !!stack.code;

	for (auto o : nodes) {
		auto p = o.get();
		if (p) ((TArray<UObject*>*)retVals)->add(p);
	}
}

UClass* UNetworkCircuit::staticClass() {
	return SML::Paks::ClassBuilder<UNetworkCircuit>::staticClass();
}
