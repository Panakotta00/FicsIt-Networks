#include "LuaDebugAPI.h"

namespace FicsItKernel {
	namespace Lua {
		void setupDebugAPI(lua_State* L) {
			PersistSetup("Debug", -2);
			lua_newtable(L);
			lua_pushcfunction(L, luaopen_debug);
			lua_call(L, 0, 1);
			lua_getfield(L, -1, "getinfo");
			lua_setfield(L, -3, "getinfo");
			lua_getfield(L, -1, "traceback");
			lua_setfield(L, -3, "traceback");
			PersistTable("Lib", -1);
			lua_setglobal(L, "debug");
			lua_pop(L, 1);
		}
	}
}
