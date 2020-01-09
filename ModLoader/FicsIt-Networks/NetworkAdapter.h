#pragma once

#include <vector>
#include <map>

#include <SDK.hpp>
#include <assets/BPInterface.h>
#include <util/Objects/FTransform.h>
#include <util/Objects/UClass.h>

#include "NetworkConnector.h"
#include "LuaContext.h"
#include "LuaLib.h"

class UNetworkAdapterReference;

struct AdapterSettings {
	SML::Objects::FVector loc;
	SDK::FRotator rot;
	bool mesh;
	int maxCables;
};

class ANetworkAdapter : public SDK::AActor {
public:
	static std::vector<std::pair<SDK::UClass*, AdapterSettings>> settings;
	static void addSetting(std::wstring clazzpath, AdapterSettings setting);
	static void addSetting(std::wstring path, std::wstring clazz, AdapterSettings setting);

	SDK::AFGBuildableFactory* parent;
	UNetworkConnector* connector;
	UNetworkAdapterReference* attachment;
	SDK::UStaticMeshComponent* connectorMesh;
	ISaveI saveI;

	void constructor();
	void destructor();
	void beginPlay();

	static SML::Objects::UClass* staticClass();
};

class UNetworkAdapterReference : public SDK::UActorComponent {
public:
	ANetworkAdapter* ref;

	void constructor();

	static SML::Objects::UClass* staticClass();
};