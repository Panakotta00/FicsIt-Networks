#include "FicsItNetworksLuaModule.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"

namespace FINLua {
#define LOCTEXT_NAMESPACE "DebugModule"
	BeginLuaModule(Debug, LOCTEXT("DisplayName", "Debug Module"), LOCTEXT("Description", ""))
#define LOCTEXT_NAMESPACE "DebugLibrary"
	BeginLibrary(debug, LOCTEXT("DisplayName", "Debug Library"), LOCTEXT("Description", ""))

	FieldFunction(log, LOCTEXT("log_DisplayName", "Log"), LOCTEXT("log_Description", "")) {
		int args = lua_gettop(L);
		FString Msg;
		for (int i = 1; i <= args; ++i) {
			Msg.Append(lua_tostring(L, i));
		}
		UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Lua Log \"%s\""), *Msg);
		return 0;
	}

	FieldFunction(getinfo, LOCTEXT("getinfo_DisplayName", "Get Info"), LOCTEXT("getinfo_Description", "")) { return 0; }
	FieldFunction(traceback, LOCTEXT("traceback_DisplayName", "Traceback"), LOCTEXT("traceback_Description", "")) { return 0; }

	EndLibrary()

	ModulePostSetup() {
		lua_getglobal(L, "debug");
		lua_pushcfunction(L, luaopen_debug);
		lua_call(L, 0, 1);
		lua_getfield(L, -1, "getinfo");
		luaFIN_persistValue(L, -1, TEXT("Debug-Global-debug-getinfo"));
		lua_setfield(L, -3, "getinfo");
		lua_getfield(L, -1, "traceback");
		luaFIN_persistValue(L, -1, TEXT("Debug-Global-debug-traceback"));
		lua_setfield(L, -3, "traceback");
		lua_pop(L, 2);
	}

	EndLuaModule()
}
