#include "LuaComputerAPI.h"

#include "FGTimeSubsystem.h"
#include "FINStateEEPROMLua.h"
#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "LuaStructs.h"
#include "Network/FINDynamicStructHolder.h"

#define LuaFunc(funcName) \
int funcName(lua_State* L) { \
	LuaProcessor* processor = LuaProcessor::luaGetProcessor(L); \
	KernelSystem* kernel = processor->getKernel(); \
	FLuaSyncCall SyncCall(L);


namespace FicsItKernel {
	namespace Lua {
		LuaFunc(luaComputerGetInstance)
			newInstance(L, FFINNetworkTrace(kernel->getNetwork()->component));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

#pragma optimize("", off)
		LuaFunc(luaComputerReset)
			kernel->reset();
			lua_yield(L, 0);
			return 0;
		}

		LuaFunc(luaComputerStop)
			kernel->stop();
			lua_yield(L, 0);
			return 0;
		}

		LuaFunc(luaComputerPanic)
		    kernel->crash(KernelCrash(std::string("PANIC! '") + luaL_checkstring(L, 1) + "'"));
			kernel->pushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel]() {
				kernel->getAudio()->beep();
			})));
			lua_yield(L, 0);
			return 0;
		}

		int luaComputerSkipContinue(lua_State* L, int status, lua_KContext ctx) {
			return 0;
		}

		int luaComputerSkip(lua_State* L) {
			LuaProcessor* processor = LuaProcessor::luaGetProcessor(L);
			processor->tickHelper.shouldPromote();
			return LuaProcessor::luaAPIReturn(L, 0);
		}
#pragma optimize("", on)

		LuaFunc(luaComputerBeep)
			float pitch = 1;
			if (lua_isnumber(L, 1)) pitch = lua_tonumber(L, 1);
			kernel->pushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel, pitch]() {
			    kernel->getAudio()->beep(pitch);
			})));
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		LuaFunc(luaComputerSetEEPROM)
			AFINStateEEPROMLua* eeprom = static_cast<LuaProcessor*>(kernel->getProcessor())->getEEPROM();
			if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
			eeprom->SetCode(luaL_checkstring(L, 1));
			return 0;
		}

		LuaFunc(luaComputerGetEEPROM)
            AFINStateEEPROMLua* eeprom = static_cast<LuaProcessor*>(kernel->getProcessor())->getEEPROM();
			if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
			lua_pushlstring(L, TCHAR_TO_UTF8(*eeprom->GetCode()), eeprom->GetCode().Len());
			return 1;
		}

		LuaFunc(luaComputerTime)
			AFGTimeOfDaySubsystem* subsys = AFGTimeOfDaySubsystem::Get(kernel->getNetwork()->component);
			lua_pushnumber(L, subsys->GetPassedDays() * 86400 + subsys->GetDaySeconds());
			return 1;
		}
		
		LuaFunc(luaComputerMillis)
			lua_pushinteger(L, kernel->getTimeSinceStart());
			return 1;
		}
		
		LuaFunc(luaComputerGPUs)
			lua_newtable(L);
			int i = 1;
			for (UObject* gpu : kernel->getGPUs()) {
				newInstance(L, FFINNetworkTrace(kernel->getNetwork()->component) / gpu);
				lua_seti(L, -2, i++);
			}
			return 1;
		}

		LuaFunc(luaComputerScreens)
		    lua_newtable(L);
			int i = 1;
			for (UObject* screen : kernel->getScreens()) {
				newInstance(L, FFINNetworkTrace(kernel->getNetwork()->component) / screen);
				lua_seti(L, -2, i++);
			}
			return 1;
		}

		static const luaL_Reg luaComputerLib[] = {
			{"getInstance", luaComputerGetInstance},
			{"reset", luaComputerReset},
			{"stop", luaComputerStop},
			{"panic", luaComputerPanic},
			{"skip", luaComputerSkip},
			{"beep", luaComputerBeep},
			{"setEEPROM", luaComputerSetEEPROM},
			{"getEEPROM", luaComputerGetEEPROM},
			{"time", luaComputerTime},
			{"millis", luaComputerMillis},
			{"getGPUs", luaComputerGPUs},
			{"getScreens", luaComputerScreens},
			{NULL,NULL}
		};
		
		void setupComputerAPI(lua_State* L) {
			PersistSetup("Computer", -2);
			luaL_newlibtable(L, luaComputerLib);
			luaL_setfuncs(L, luaComputerLib, 0);
			PersistTable("Lib", -1);
			lua_setglobal(L, "computer");
		}
	}
}