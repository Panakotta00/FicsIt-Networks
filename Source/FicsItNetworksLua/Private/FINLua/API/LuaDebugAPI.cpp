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
			 * @param		...			string		Messages	A list of log messages that should get printed to the game console.
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

		// TODO: how to properly document xpcall? Uncommenting this code crashes the game on loading a save.
		//LuaModuleGlobalBareValue(R"(/**
		// * @LuaGlobal		xpcall	fun(fn, ...): (ok: boolean, err: { message: any, trace: string }?)
		// * @DisplayName		XPcall
		// *
		// * Ficsit networks ships with a slightly modifier version of `xpcall`. Like `pcall`, it accepts a function
		// * and its arguments, calls it and checks whether an error has occurred.
		// *
		// * It returns a boolean indicating that the function call was successful.
		// * If the call fails, it also returns a table with two fields: `message` is an error message
		// * (or whatever was passed to an `error` call), and `trace` is a string with traceback.
		// *
		// * If the call was successful, our version of xpcall doesn't return its result.
		// * To get it, pass in a closure that will set a local variable:
		// *
		// * [source,lua]
		// * ---
		// * local result
		// * local ok, err = xpcall(function() result = doSomething() end)
		// * ---
		// */)", xpcall) {
		//	// This function is here for documentation purposes only;
		//	// The actual implementation is in LuaBaseModule.cpp
		//}

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
