#include "FicsItKernel.h"
#include "FicsItLogLibrary.h"
#include "FINLuaModule.h"
#include "FINLuaRuntime.h"
#include "FINLuaThreadedRuntime.h"
#include "FINNetworkUtils.h"
#include "LuaFileSystemAPI.h"
#include "LuaObject.h"
#include "LuaPersistence.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		ThreadedRuntimeModule
	 * @DisplayName		Threaded Runtime Module
	 *
	 * This Module provides an interface for other Modules to interact with the Kernel.
	 */)", ThreadedRuntimeModule) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		computer
		 * @DisplayName		Computer Library
		 *
		 * The Computer Library provides functions for interaction with the computer and especially the Lua Runtime.
		 */)", computer) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		promote()
			 * @DisplayName		Promote
			 *
			 * This function is mainly used to allow switching to a higher tick runtime state.
			 * Usually you use this when you want to make your code run faster when using functions that can run in asynchronous environment.
			 */)", promote) {
				FFINLuaThreadedRuntime& runtime = luaFIN_getThreadedRuntime(L);
				runtime.SetShouldBePromoted(true);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		demote()
			 * @DisplayName		Demote
			 *
			 * This function is used to allow switching back to the normal tick rate.
			 */)", demote) {
				FFINLuaThreadedRuntime& runtime = luaFIN_getThreadedRuntime(L);
				runtime.SetShouldBePromoted(false);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	isPromoted()
			 * @DisplayName		Is Promoted
			 *
			 * Returns true if the Lua runtime is currently promoted/elevated.
			 * Which means its running in an seperate game thread allowing for fast bulk calculations.
			 *
			 * @return	isPromoted	bool	True if the currenty runtime is running in promoted/elevated tick state.
			 */)", isPromoted) {
				FFINLuaThreadedRuntime& runtime = luaFIN_getThreadedRuntime(L);
				lua_pushboolean(L, runtime.ShouldBePromoted());
				return 1;
			}
		}
	}

	LuaModule(R"(/**
	 * @LuaModule		LogModule
	 * @DisplayName		Log Module
	 */)", LogModule) {
		int luaPrint(lua_State* L) {
			const int args = lua_gettop(L);
			std::string log;
			for (int i = 1; i <= args; ++i) {
				size_t s_len = 0;
				const char* s = luaL_tolstring(L, i, &s_len);
				if (!s) luaL_argerror(L, i, "is not valid type");
				log += std::string(s, s_len) + " ";
			}
			if (log.length() > 0) log = log.erase(log.length()-1);

			UFILogLibrary::Log(FIL_Verbosity_Info, UTF8_TO_TCHAR(log.c_str()));

			return 0;
		}
		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		print	function(string...)
		 * @DisplayName		Print
		 */)", print) {
			lua_pushcfunction(L, luaPrint);
			luaFIN_persistValue(L, -1, PersistName);
		}

		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		computer
		 * @DisplayName		Computer Library
		 *
		 * The Computer Library provides functions for interaction with the computer and especially the Lua Runtime.
		 */)", computer) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		log(verbosity: int, message: string)
			 * @DisplayName		Log
			 *
			 * Allows you to print a log message to the computers log with the given log verbosity.
			 *
			 * @parameter	verbosity	int		Verbosity	The log-level/verbosity of the message you want to log. 0 = Debug, 1 = Info, 2 = Warning, 3 = Error & 4 = Fatal
			 * @parameter	message		string	Message		The log message you want to print
			 */)", log) {
				FLuaSync sync(L);

				int verbosity = luaL_checknumber(L, 1);
				FString text = FINLua::luaFIN_checkFString(L, 2);
				verbosity = FMath::Clamp(verbosity, 0, FIL_Verbosity_Max);
				UFILogLibrary::Log((EFILLogVerbosity)verbosity, text);
				return 0;
			}
		}
	}
}
