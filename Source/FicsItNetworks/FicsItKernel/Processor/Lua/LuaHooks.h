#pragma once

#include "CoreMinimal.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPowerCircuit.h"

#include "LuaInstance.h"
#include "FicsItKernel/Network/NetworkTrace.h"

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
		
		extern TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, FactoryHook> factoryHooks;
		extern TMap<TWeakObjectPtr<UFGPowerCircuit>, TSet<Network::NetworkTrace>> powerCircuitListeners;

		void luaHook(lua_State* L, Network::NetworkTrace hook);
		void luaListenCircuit(Network::NetworkTrace circuit);
		void setupHooks(lua_State* L);
	}
}