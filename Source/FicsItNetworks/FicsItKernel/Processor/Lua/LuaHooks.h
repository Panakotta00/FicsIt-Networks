#pragma once

#include "CoreMinimal.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPowerCircuit.h"

#include "LuaInstance.h"
#include "Network/FINNetworkTrace.h"

#include <set>
#include <deque>

#include "LuaProcessorStateStorage.h"

namespace FicsItKernel {
	namespace Lua {
		void luaHook(lua_State* L, Network::NetworkTrace hook);
		void luaListenCircuit(Network::NetworkTrace circuit);
		void setupHooks(lua_State* L);
	}
}
