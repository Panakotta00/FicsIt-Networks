#include "LuaHooks.h"

#include "FicsItKernel/FicsItKernel.h"
#include "LuaProcessor.h"
#include "Network/FINNetworkTrace.h"
#include "SML/util/Logging.h"

#define LuaFactoryFuncName(funcName) luaFactoryHook ## funcName
#define LuaFactoryFunc(funcName) \
int LuaFactoryFuncName(funcName) (lua_State* L) { \
	const Network::NetworkTrace& trace = *(Network::NetworkTrace*)luaL_checkudata(L, 1, "FactoryHook"); \
	UFGFactoryConnectionComponent* self = Cast<UFGFactoryConnectionComponent>(*trace); \
	if (!self) { SML::Logging::error("Oh noe"); return 0; } \
	FactoryHook* hook_r = factoryHooks.Find(self); \
	if (!hook_r) return luaL_error(L, "Invalid Hook"); \
	FactoryHook& hook = *hook_r;
#define LuaEndFunc \
	\
}

namespace FicsItKernel {
	namespace Lua {
		FCriticalSection MutexFactoryHooks;
		FCriticalSection MutexPowerCircuitListeners;
		TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, FactoryHook> factoryHooks;
		TMap<TWeakObjectPtr<UFGPowerCircuit>, TSet<FFINNetworkTrace>> powerCircuitListeners;

		void FactoryHook::update() {
			auto now = std::chrono::high_resolution_clock::now() - std::chrono::minutes(1);
			while (iperm.size() > 0 && iperm.front() < now) {
				iperm.pop();
			}
		}

		LuaFactoryFunc(GC)
			--hook.refs;
			return 0;
		LuaEndFunc

		LuaFactoryFunc(GetIperM)
			lua_pushinteger(L, hook.iperm.size());
			return 1;
		LuaEndFunc

		LuaFactoryFunc(Listen)
			hook.deleg.insert(trace.reverse());
			return 0;
		LuaEndFunc

		static const luaL_Reg luaFactoryHookLib[] = {
			{"getIlastM", luaFactoryHookGetIperM},
			{"listen", luaFactoryHookListen},
			{"__gc", luaFactoryHookGC},
			{NULL, NULL}
		};

		void luaHook(lua_State* L, Network::NetworkTrace con) {
			auto connector = Cast<UFGFactoryConnectionComponent>(*con);
			if (!connector) throw std::exception("Object is not FactoryConnector");
			MutexFactoryHooks.Lock();
			++factoryHooks.FindOrAdd(connector).refs;
			MutexFactoryHooks.Unlock();
			auto p = (Network::NetworkTrace*)lua_newuserdata(L, sizeof(Network::NetworkTrace));
			luaL_setmetatable(L, "FactoryHook");
			*p = con;
		}

		void luaListenCircuit(Network::NetworkTrace circuit) {
			MutexPowerCircuitListeners.Lock();
			powerCircuitListeners.FindOrAdd(Cast<UFGPowerCircuit>(*circuit)).Add(circuit.reverse());
			MutexPowerCircuitListeners.Unlock();
		}

		void setupHooks(lua_State* L) {
			PersistSetup("Hook", -2);
			luaL_newmetatable(L, "FactoryHook");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			luaL_setfuncs(L, luaFactoryHookLib, 0);
			PersistTable("Factory", -1);
			lua_pop(L, 1);
		}
	}
}
