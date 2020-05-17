#include "LuaHooks.h"

#include "FicsItKernel/FicsItKernel.h"
#include "LuaProcessorStateStorage.h"
#include "Computer/FINComputerSubsystem.h"
#include "Network/FINNetworkTrace.h"
#include "SML/util/Logging.h"

#define LuaFactoryFuncName(funcName) luaFactoryHook ## funcName
#define LuaFactoryFunc(funcName) \
int LuaFactoryFuncName(funcName) (lua_State* L) { \
	const Network::NetworkTrace& trace = *(Network::NetworkTrace*)luaL_checkudata(L, 1, "FactoryHook"); \
	UFGFactoryConnectionComponent* self = Cast<UFGFactoryConnectionComponent>(*trace); \
	AFINComputerSubsystem* CompSS = AFINComputerSubsystem::GetComputerSubsystem(self); \
	if (!self) { return 0; } \
	FFINFactoryHook* hook_r = CompSS->FactoryHooks.Find(self); \
	if (!hook_r) return luaL_error(L, "Invalid Hook"); \
	FFINFactoryHook& hook = *hook_r;
#define LuaEndFunc \
	\
}

namespace FicsItKernel {
	namespace Lua {
		LuaFactoryFunc(GC)
			CompSS->RemoveHook(trace);
			return 0;
		LuaEndFunc

		LuaFactoryFunc(GetIperM)
			CompSS->MutexFactoryHooks.Lock();
			int32 Sum = hook.GetSampleSum();
			CompSS->MutexFactoryHooks.Unlock();
			lua_pushinteger(L, Sum);
			return 1;
		LuaEndFunc

		LuaFactoryFunc(Listen)
			hook.Listeners.Add(trace.reverse());
			return 0;
		LuaEndFunc

		int luaFactoryHookUnpersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = (ULuaProcessorStateStorage*)lua_touserdata(L, -1);

			// get trace to connector & create hook with it
			FFINNetworkTrace trace = storage->GetTrace(lua_tointeger(L, lua_upvalueindex(1)));
			luaHook(L, trace);
			
			return 1;
		}

		LuaFactoryFunc(Persist)
			// get persist storage
	        lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = (ULuaProcessorStateStorage*)lua_touserdata(L, -1);

			// persist ref to connector
			lua_pushnumber(L, storage->Add(trace));
			
			lua_pushcclosure(L, luaFactoryHookUnpersist, 1);
		return 1;
		LuaEndFunc

		static const luaL_Reg luaFactoryHookLib[] = {
			{"getIlastM", luaFactoryHookGetIperM},
			{"listen", luaFactoryHookListen},
			{"__persist", luaFactoryHookPersist},
			{"__gc", luaFactoryHookGC},
			{NULL, NULL}
		};

		void luaHook(lua_State* L, Network::NetworkTrace con) {
			auto connector = Cast<UFGFactoryConnectionComponent>(*con);
			if (!connector) throw std::exception("Object is not FactoryConnector");
			AFINComputerSubsystem* CompSS = AFINComputerSubsystem::GetComputerSubsystem(connector);
			CompSS->MutexFactoryHooks.Lock();
			++CompSS->FactoryHooks.FindOrAdd(connector).CountOfReferences;
			CompSS->MutexFactoryHooks.Unlock();
			auto p = (Network::NetworkTrace*)lua_newuserdata(L, sizeof(Network::NetworkTrace));
			luaL_setmetatable(L, "FactoryHook");
			*p = con;
		}

		void luaListenCircuit(Network::NetworkTrace circuit) {
			AFINComputerSubsystem* CompSS = AFINComputerSubsystem::GetComputerSubsystem(*circuit);
			CompSS->MutexPowerCircuitListeners.Lock();
			CompSS->PowerCircuitListeners.FindOrAdd(Cast<UFGPowerCircuit>(*circuit)).Add(circuit.reverse());
			CompSS->MutexPowerCircuitListeners.Unlock();
		}

		void setupHooks(lua_State* L) {
			PersistSetup("Hook", -2);
			luaL_newmetatable(L, "FactoryHook");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			luaL_setfuncs(L, luaFactoryHookLib, 0);
			PersistTable("Factory", -1);
			lua_pop(L, 1);
			lua_pushcfunction(L, luaFactoryHookUnpersist);
			PersistValue("FactoryUnpersist");
		}
	}
}
