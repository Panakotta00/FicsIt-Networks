#include "FicsItNetworksLuaModule.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"

#define LOCTEXT_NAMESPACE "Undefined"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		DebugModule
	 * @DisplayName		Debug Module
	 */)", DebugModule) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		debug
		 * @DisplayName		Debug Library
		 */)", debug) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		log(string...)
			 * @DisplayName		Log
			 *
			 * Allows to log the given strings to the Game Log.
			 *
			 * @param	msgs	string...	A list of log messages that should get printed to the game console.
			 */)", log) {
				int args = lua_gettop(L);
				FString Msg;
				for (int i = 1; i <= args; ++i) {
					Msg.Append(lua_tostring(L, i));
				}
				UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Lua Log \"%s\""), *Msg);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		getinfo
			 * @DisplayName		Get Info
			 *
			 * Check https://www.lua.org/manual/5.4/manual.html#pdf-debug.getinfo[the Lua Manual] for more information.
			 */)", getinfo) { return 0; }
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		traceback
			 * @DisplayName		Traceback
			 *
			 * Check https://www.lua.org/manual/5.4/manual.html#pdf-debug.traceback[the Lua Manual] for more information.
			 */)", traceback) { return 0; }
		}

		LuaModulePostSetup() {
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
	}
}

#undef LOCTEXT_NAMESPACE
