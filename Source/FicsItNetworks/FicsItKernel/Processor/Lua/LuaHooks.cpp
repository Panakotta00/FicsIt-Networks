#include "LuaHooks.h"

#include "FicsItKernel/FicsItKernel.h"
#include "LuaProcessor.h"

#define LuaFactoryFuncName(funcName) luaFactoryHook ## funcName
#define LuaFactoryFunc(funcName) \
int LuaFactoryFuncName(funcName) (lua_State* L) { \
	auto self = ((TWeakObjectPtr<UFGFactoryConnectionComponent>*)luaL_checkudata(L, 1, "FactoryHook"))->Get(); \
	if (!self) return 0; \
	auto hook_r = factoryHooks.Find(self); \
	if (!hook_r) return luaL_error(L, "Invalid Hook"); \
	auto hook = *hook_r;
#define LuaEndFunc \
	\
}

namespace FicsItKernel {
	namespace Lua {
		TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, FactoryHook> factoryHooks;
		TMap<TWeakObjectPtr<UFGPowerCircuit>, TSet<Network::NetworkTrace>> powerCircuitListeners;

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
			hook.deleg.insert(Network::NetworkTrace(LuaProcessor::getCurrentProcessor()->getKernel()->getNetwork()->component));
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
			++factoryHooks[connector].refs;
			auto p = (Network::NetworkTrace*)lua_newuserdata(L, sizeof(Network::NetworkTrace));
			luaL_setmetatable(L, "FactoryHook");
			*p = con.reverse();
		}

		void luaListenCircuit(Network::NetworkTrace circuit) {
			powerCircuitListeners[Cast<UFGPowerCircuit>(*circuit)].Add(circuit.reverse());
		}

		void setupHooks(lua_State* L) {
			luaL_newmetatable(L, "FactoryHook");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			luaL_setfuncs(L, luaFactoryHookLib, 0);
			lua_pop(L, 1);
		}
	}
}
