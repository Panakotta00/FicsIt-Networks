#include "LuaDebugAPI.h"
#include "LuaUtil.h"
#include "FicsItNetworks/FicsItNetworksModule.h"

namespace FicsItKernel {
	namespace Lua {
		int luaLog(lua_State* L) {
			int args = lua_gettop(L);
			FString Msg;
			for (int i = 1; i <= args; ++i) {
				Msg.Append(lua_tostring(L, i));
			}
			UE_LOG(LogFicsItNetworks, Warning, TEXT("Lua Log \"%s\""), *Msg);
			return 0;
		}
		
		void setupDebugAPI(lua_State* L) {
			PersistSetup("Debug", -2);
			lua_newtable(L);
			lua_pushcfunction(L, luaopen_debug);
			lua_call(L, 0, 1);
			lua_getfield(L, -1, "getinfo");
			lua_setfield(L, -3, "getinfo");
			lua_getfield(L, -1, "traceback");
			lua_setfield(L, -3, "traceback");
			lua_pushcfunction(L, luaLog);
			lua_setfield(L, -2, "log");
			PersistTable("Lib", -1);
			lua_setglobal(L, "debug");
			lua_pop(L, 1);
		}
	}
}
