#pragma once

#include <SDK.hpp>
#include <assets/BPInterface.h>
#include "NetworkConnector.h"

class UComponentUtility : public SDK::UBlueprintFunctionLibrary {
public:
	static bool allowUsing;
	static UNetworkConnector* getNetworkConnectorFromHit(SDK::FHitResult hit);

	static void connectPower(void* self, SML::Objects::FFrame& stack, void* ret);
	static void disconnectPower(void* self, SML::Objects::FFrame& stack, void* ret);
	static void getNetworkConnectorFromHit_exec(void* self, SML::Objects::FFrame& stack, void* ret);
	static void clipboardCopy(void* self, SML::Objects::FFrame& stack, void* ret);
	static void setAllowUsing(void* self, SML::Objects::FFrame& stack, void* ret);
	static void dumpObject(void* self, SML::Objects::FFrame& stack, void* ret);
	void loadSoundFromFile(SML::Objects::FFrame& stack, void* ret);
};