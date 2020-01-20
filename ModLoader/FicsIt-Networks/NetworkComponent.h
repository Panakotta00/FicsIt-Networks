#pragma once

#include <assets/BPInterface.h>
#include <util/Objects/FGuid.h>
#include "NetworkCircuit.h"

class UNetworkComponent : SDK::UInterface {
public:
	//UNetworkCircuit* circuit;
	//void construct();
	static void getID_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void getNick_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void setNick_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void getMerged_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void getConnected_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void findComponent_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void getCircuit_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void setCircuit_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);

	static SML::Objects::UClass* staticClass();
};

class INetworkComponent {
public:
	//UNetworkCircuit* circuit;
	virtual SML::Objects::FGuid getID() const = 0;
	virtual SML::Objects::FString getNick() const = 0;
	virtual void setNick(const SML::Objects::FString& nick) = 0;
	virtual SML::Objects::TArray<SML::Objects::UObject*> getMerged() const;
	virtual SML::Objects::TArray<SML::Objects::UObject*> getConnected() const;
	virtual SML::Objects::UObject* findComponent(SML::Objects::FGuid guid) const = 0;
	virtual UNetworkCircuit* getCircuit() const = 0;
	virtual void setCircuit(UNetworkCircuit* circuit) = 0;
	virtual void notifyNetworkUpdate(int type, std::set<SML::Objects::FWeakObjectPtr> nodes);

	SML::Objects::UObject* findComponentFromCircuit(SML::Objects::FGuid guid) const;

	bool hasNick(std::string nick);
};