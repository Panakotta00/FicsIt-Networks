#pragma once

#include "CoreMinimal.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPowerCircuit.h"

#include "LuaInstance.h"
#include "Network/FINNetworkTrace.h"

#include <set>
#include <map>
#include <queue>
#include <chrono>

namespace FicsItKernel {
	namespace Lua {
		struct FactoryHook {
			size_t refs = 0;
			std::queue<std::chrono::high_resolution_clock::time_point> iperm;
			std::set<Network::NetworkTrace> deleg;
			void update();
		};

		extern FCriticalSection MutexFactoryHooks;
		extern FCriticalSection MutexPowerCircuitListeners;
		extern TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, FactoryHook> factoryHooks;
		extern TMap<TWeakObjectPtr<UFGPowerCircuit>, TSet<FFINNetworkTrace>> powerCircuitListeners;

		void luaHook(lua_State* L, Network::NetworkTrace hook);
		void luaListenCircuit(Network::NetworkTrace circuit);
		void setupHooks(lua_State* L);
	}
}