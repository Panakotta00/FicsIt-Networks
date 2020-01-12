#pragma once

#include <set>

#include <assets/BPInterface.h>
#include <util/Utility.h>
#include <util/Objects/TArray.h>

#include <SDK.hpp>

#include "NetworkComponent.h"
#include "LuaImplementation.h"

class UNetworkConnector;

inline bool impl(UNetworkConnector* self) {
	return true;
}

inline void noting(UNetworkConnector *self, int, int) {
}

inline void gdeps(UNetworkConnector *self, SML::Objects::TArray<SML::Objects::UObject*> *arr) {
}

struct IFGSaveInterface {

	
	virtual void preSave(int, int) {}
	virtual void postSave(int, int) {}
	virtual void preLoad(int, int) {}
	virtual void postLoad(int, int) {}
	virtual void gatherDeps(SML::Objects::TArray<SML::Objects::UObject*>*) {}
	virtual bool ShouldSave() {
		return true;
	}
	virtual bool NeedTransform() {
		return false;
	}
};

struct IFGSaveInterfaceVtbl {
	bool(__fastcall *NeedTransform_Implementation)(UNetworkConnector *self) = &impl;
	bool(*ShouldSave_Implementation)(UNetworkConnector* self) = &impl;
	void(__fastcall *GatherDependencies_Implementation)(UNetworkConnector *self, SML::Objects::TArray<SML::Objects::UObject*> *) = gdeps;
	void(__fastcall *PostLoadGame_Implementation)(UNetworkConnector *self, int, int) = noting;
	void(__fastcall *PreLoadGame_Implementation)(UNetworkConnector *self, int, int) = noting;
	void(__fastcall *PostSaveGame_Implementation)(UNetworkConnector *self, int, int) = noting;
	void(__fastcall *PreSaveGame_Implementation)(UNetworkConnector *self, int, int) = noting;
	void* uk = (void*)1; // $8F9346723843332C1110F1C522998FC9 ___u6;
	SML::Objects::UObject *(__fastcall *_getUObject)(UNetworkConnector *self);
};

struct ISaveI : public SDK::UFGSaveInterface {
	
};

class INetworkConnectorComponent : public INetworkComponent {
private:
	UNetworkConnector* self() const;

public:
	virtual SML::Objects::FGuid getID() const override;
	virtual SML::Objects::TArray<SML::Objects::UObject*> getMerged() const override;
	virtual SML::Objects::TArray<SML::Objects::UObject*> getConnected() const override;
	virtual SML::Objects::UObject* findComponent(SML::Objects::FGuid guid) const override;
	virtual UNetworkCircuit* getCircuit() const override;
	virtual void setCircuit(UNetworkCircuit * circuit) override;
	virtual void notifyNetworkUpdate(int type, std::set<SML::Objects::FWeakObjectPtr> nodes);
};

class INetworkConnectorLua : public ILuaImplementation {
private:
	UNetworkConnector* self() const;

public:
	virtual void luaAddSignalListener(ULuaContext* ctx) override;
	virtual void luaRemoveSignalListener(ULuaContext* ctx) override;
	virtual SML::Objects::TArray<ULuaContext*> luaGetSignalListeners() override;
	virtual bool luaIsReachableFrom(SML::Objects::UObject* listener) override;
};

class UNetworkConnector : public SDK::USceneComponent{
private:
	bool searchFor(std::set<UNetworkConnector*>& searched, UNetworkConnector* obj);
	void removeConnector(UNetworkConnector* connector);

public:
	SML::Objects::FGuid id;
	bool idCreated;
	int maxCables;
	UNetworkCircuit* circuit;
	ISaveI saveI;
	INetworkConnectorComponent component;
	INetworkConnectorLua luaImpl;

	std::unordered_set<UNetworkConnector*> connections;
	std::unordered_set<SDK::AFGBuildable*> cables;
	std::unordered_set<SML::Objects::UObject*> components;
	std::unordered_set<SML::Objects::UObject*> merged;
	std::set<SML::Objects::FWeakObjectPtr> listeners;

	void construct();
	void destruct();

	void beginPlay();

	void addConnection(UNetworkConnector* connector);
	void removeConnection(UNetworkConnector* connector);

	bool addCable(SDK::AFGBuildable* cable);
	void removeCable(SDK::AFGBuildable* cable);
	bool isConnected(UNetworkConnector* cable);
	bool searchFor(UNetworkConnector* conn);

	void execNetworkUpdate(SML::Objects::FFrame& stack, void* params);
	static void execAddConn(UNetworkConnector* self, SML::Objects::FFrame& stack, void* params);
	static void execRemConn(UNetworkConnector* self, SML::Objects::FFrame& stack, void* params);
	static void execAddCable(UNetworkConnector* self, SML::Objects::FFrame& stack, void* params);
	static void execRemCable(UNetworkConnector* self, SML::Objects::FFrame& stack, void* params);
	void execAddMerged(SML::Objects::FFrame& stack, void* params);
	void execAddComponent(SML::Objects::FFrame& stack, void* params);

	static SML::Objects::UClass* staticClass();
};