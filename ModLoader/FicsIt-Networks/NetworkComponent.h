#pragma once

#include <assets/BPInterface.h>
#include <util/Objects/FGuid.h>

class UNetworkComponent : SDK::UInterface {
public:
	static void getID_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void getMerged_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void getConnected_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);
	static void findComponent_exec(UNetworkComponent* c, SML::Objects::FFrame& stack, void* params);

	static SML::Objects::UClass* staticClass();
};

class INetworkComponent {
public:
	virtual SML::Objects::FGuid getID() const;
	virtual SML::Objects::TArray<SML::Objects::UObject*> getMerged() const;
	virtual SML::Objects::TArray<SML::Objects::UObject*> getConnected() const;
	virtual SML::Objects::UObject* findComponent(SML::Objects::FGuid guid) const = 0;

	SML::Objects::UObject* findComponent(SML::Objects::FGuid guid, std::set<SML::Objects::UObject*>& searched, SML::Objects::UObject* self) const;
};