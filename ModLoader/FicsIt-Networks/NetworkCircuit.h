#pragma once

#include <util/Objects/SmartPointer.h>
#include <util/Objects/FGuid.h>
#include <SDK.hpp>
#include <assets/BPInterface.h>

class UNetworkConnector;

class UNetworkCircuit : public SML::Objects::UObject {
	friend UNetworkConnector;

private:
	std::set<SML::Objects::FWeakObjectPtr> nodes;

	void addNodeRecursive(std::set<SML::Objects::UObject*>& added, SML::Objects::UObject* add);

public:
	void construct();

	void destruct();

	UNetworkCircuit* operator+(UNetworkCircuit* circuit);
	
	void recalculate(SML::Objects::UObject* component);
	bool hasNode(SML::Objects::UObject* node);
	SML::Objects::UObject* findComponent(SML::Objects::FGuid addr);

	void getComponents_exec(SML::Objects::FFrame& stack, void* retVals);

	static SML::Objects::UClass* staticClass();
};