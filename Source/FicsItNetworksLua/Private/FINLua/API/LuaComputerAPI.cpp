#include "FGAttentionPingActor.h"
#include "FGPlayerController.h"
#include "FINLuaProcessor.h"
#include "FINMediaSubsystem.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/FINLuaModule.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		ComputerModule
	 * @DisplayName		Computer Module
	 *
	 * The Computer Module provides the Computer Library.
	 */)", ComputerModule) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		computer
		 * @DisplayName		Computer Library
		 *
		 * The Computer Library provides functions for interaction with the computer and especially the Lua Runtime.
		 */)", computer) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		skip()
			 * @DisplayName		Skip
			 *
			 * This function can be used to skip the current lua tick prematurely.
			 * Mostly for people who want to optimize their games runtime performance.
			 */)", skip) {
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(int, string, string)		magicTime()
			 * @DisplayName		Magic Time
			 *
			 * Returns some kind of strange/mysterious time data from a unknown place (the real life).
			 *
			 * @return	unix			int		Unix			Unix Timestamp
			 * @return	cultureTime		string	Culture Time	The time as text with the culture format used by the Host
			 * @return	iso8601			string	ISO 8601		The time as a Date-Time-Stamp after ISO 8601
			 */)", magicTime) {
				FDateTime Now = FDateTime::UtcNow();
				lua_pushinteger(L, Now.ToUnixTimestamp());
				FTCHARToUTF8 ConvertStr(*Now.ToString());
				lua_pushlstring(L, ConvertStr.Get(), ConvertStr.Length());
				FTCHARToUTF8 ConvertIso(*Now.ToIso8601());
				lua_pushlstring(L, ConvertIso.Get(), ConvertIso.Length());
				return 3;
			}
		}
	}
}
