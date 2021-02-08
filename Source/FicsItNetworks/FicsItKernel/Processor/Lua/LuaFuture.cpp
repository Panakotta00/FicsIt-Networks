#include "LuaFuture.h"

#include "LuaProcessor.h"
#include "LuaProcessorStateStorage.h"

namespace FicsItKernel {
	namespace Lua {
		typedef TSharedPtr<TFINDynamicStruct<FFINFuture>> LuaFuture;

		int luaFutureAwaitContinue(lua_State* L, int code, lua_KContext ctx) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			if ((*future)->IsDone()) {
				TArray<FFINAnyNetworkValue> Data = future->Get<FFINFuture>().GetOutput();
				for (const FFINAnyNetworkValue& Param : Data) networkValueToLua(L, Param);
				return Data.Num();
			}
			return lua_yieldk(L, LUA_MULTRET, NULL, luaFutureAwaitContinue);
		}
		
		int luaFutureAwait(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			return lua_yieldk(L, LUA_MULTRET, NULL, luaFutureAwaitContinue);
		}

		int luaFutureGet(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			luaL_argcheck(L, (*future)->IsDone(), 1, "Future is not ready");
			const TArray<FFINAnyNetworkValue>& Data = future->Get<FFINFutureReflection>().Output;
			for (const FFINAnyNetworkValue& Param : Data) networkValueToLua(L, Param);
			return Data.Num();
		}

		int luaFutureCanGet(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			lua_pushboolean(L, (*future)->IsDone());
			return 1;
		}

		int luaFutureNewIndex(lua_State* L) {
			return 0;
		}

		int luaFutureGC(lua_State* L) {
			LuaFuture* future = static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			future->~LuaFuture();
			return 0;
		}

		int luaFutureUnpersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			LuaFuture* future = static_cast<LuaFuture*>(lua_newuserdata(L, sizeof(LuaFuture)));
			new (future) LuaFuture(new TFINDynamicStruct<FFINFuture>(*storage->GetStruct(lua_tointeger(L, lua_upvalueindex(1)))));
			if (!(**future)->IsDone()) {
				KernelSystem* kernel = LuaProcessor::luaGetProcessor(L)->getKernel();
				kernel->pushFuture(*future);
			}
			return 1;
		}

		int luaFuturePersist(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));
			
			lua_pushinteger(L, storage->Add(future));

			lua_pushcclosure(L, luaFutureUnpersist, 1);
			
			return 1;
		}

		static const luaL_Reg luaFutureLib[] = {
			{"await", luaFutureAwait},
			{"get", luaFutureGet},
			{"canGet", luaFutureCanGet},
			{NULL,NULL}
		};

		static const luaL_Reg luaFutureMetaLib[] = {
			{"__newindex", luaFutureNewIndex},
			{"__gc", luaFutureGC},
			{"__persist", luaFuturePersist},
			{NULL,NULL}
		};

		void luaFuture(lua_State* L, const TFINDynamicStruct<FFINFuture>& Future) {
			LuaFuture* future = static_cast<LuaFuture*>(lua_newuserdata(L, sizeof(LuaFuture)));
			new (future) LuaFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(Future));
			KernelSystem* kernel = LuaProcessor::luaGetProcessor(L)->getKernel();
			kernel->pushFuture(*future);
			luaL_setmetatable(L, "Future");
		}

		void setupFutureAPI(lua_State* L) {
			PersistSetup("FutureAPI", -2);

			luaL_newmetatable(L, "Future");
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "__metatable");
			luaL_setfuncs(L, luaFutureMetaLib, 0);
			PersistTable("Future", -1);
			lua_newtable(L);
			luaL_setfuncs(L, luaFutureLib, 0);
			PersistTable("FutureLib", -1);
			lua_setfield(L, -2, "__index");
			lua_pop(L, 1);
			lua_pushcfunction(L, luaFutureUnpersist);
			PersistValue("FutureUnpersist");
		}
	}
}
