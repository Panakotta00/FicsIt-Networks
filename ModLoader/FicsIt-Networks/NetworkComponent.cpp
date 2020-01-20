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

void UNetworkComponent::getNick_exec(UNetworkComponent* c, FFrame& stack, void* params) {
	stack.code += !!stack.code;

	struct Params {
		FString nick;
	};

	((Params*)params)->nick = ((INetworkComponent*)c)->getNick();
}

void UNetworkComponent::setNick_exec(UNetworkComponent * c, SML::Objects::FFrame & stack, void * params) {
	FString nick;
	stack.stepCompIn(&nick);

	stack.code += !!stack.code;

	((INetworkComponent*)c)->setNick(nick);
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

TArray<UObject*> INetworkComponent::getMerged() const {
	return TArray<UObject*>();
}

TArray<UObject*> INetworkComponent::getConnected() const {
	return TArray<UObject*>();
}

void INetworkComponent::notifyNetworkUpdate(int type, std::set<FWeakObjectPtr> nodes) {}

UObject* INetworkComponent::findComponentFromCircuit(FGuid guid) const {
	auto circuit = getCircuit();

	return circuit->findComponent(guid);
}

bool INetworkComponent::hasNick(std::string nick) {
	auto has = getNick().toStr();
	std::set<std::string> has_nicks;
	do {
		auto pos = has.find(" ");
		std::string n = "";
		if (pos >= has.length()) {
			n = has;
			has = "";
		} else {
			n = has.substr(0, pos);
			has = has.substr(pos + 1);
		}
		if (n.length() > 0) has_nicks.insert(n);
	} while (has.length() > 0);

	do {
		auto pos = nick.find(" ");
		std::string n = "";
		if (pos >= nick.length()) {
			n = nick;
			nick = "";
		} else {
			n = nick.substr(0, pos);
			nick = nick.substr(pos + 1);
		}
		if (n.length() > 0) if (has_nicks.find(n) == has_nicks.end()) return false;
	} while (nick.length() > 0);

	return true;
}

/*UNetworkCircuit * INetworkComponent::getCircuit() const {
	return circuit;
}

void INetworkComponent::setCircuit(UNetworkCircuit * circuit) {
	this->circuit = circuit;
}*/