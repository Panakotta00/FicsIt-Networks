#include "LuaComputerAPI.h"

#include "FGTimeSubsystem.h"
#include "FINStateEEPROMLua.h"
#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "FicsItNetworks/Network/FINDynamicStructHolder.h"
#include "FicsItNetworks/Reflection/FINClass.h"

#define LuaFunc(funcName) \
int funcName(lua_State* L) { \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	FLuaSyncCall SyncCall(L);


namespace FicsItKernel {
	namespace Lua {
		LuaFunc(luaComputerGetInstance)
			newInstance(L, FFINNetworkTrace(kernel->GetNetwork()->GetComponent().GetObject()));
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}

#pragma optimize("", off)
		LuaFunc(luaComputerReset)
			processor->GetTickHelper().shouldReset();
			lua_yield(L, 0);
			return 0;
		}

		LuaFunc(luaComputerStop)
			processor->GetTickHelper().shouldStop();
			lua_yield(L, 0);
			return 0;
		}

		LuaFunc(luaComputerPanic)
		    processor->GetTickHelper().shouldCrash(MakeShared<FFINKernelCrash>(FString("PANIC! '") + luaL_checkstring(L, 1) + "'"));
			kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel]() {
				kernel->GetAudio()->Beep();
			})));
			lua_yield(L, 0);
			return 0;
		}

		int luaComputerSkipContinue(lua_State* L, int status, lua_KContext ctx) {
			return 0;
		}

		int luaComputerSkip(lua_State* L) {
			UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
			processor->GetTickHelper().shouldPromote();
			return UFINLuaProcessor::luaAPIReturn(L, 0);
		}
#pragma optimize("", on)

		LuaFunc(luaComputerBeep)
			float pitch = 1;
			if (lua_isnumber(L, 1)) pitch = lua_tonumber(L, 1);
			kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel, pitch]() {
			    kernel->GetAudio()->Beep(pitch);
			})));
			return UFINLuaProcessor::luaAPIReturn(L, 0);
		}

		LuaFunc(luaComputerSetEEPROM)
			AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
			if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
			eeprom->SetCode(luaL_checkstring(L, 1));
			return 0;
		}

		LuaFunc(luaComputerGetEEPROM)
            const AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
			if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
			lua_pushlstring(L, TCHAR_TO_UTF8(*eeprom->GetCode()), eeprom->GetCode().Len());
			return 1;
		}

		LuaFunc(luaComputerTime)
			const AFGTimeOfDaySubsystem* Subsystem = AFGTimeOfDaySubsystem::Get(kernel);
			lua_pushnumber(L, Subsystem->GetPassedDays() * 86400 + Subsystem->GetDaySeconds());
			return 1;
		}
		
		LuaFunc(luaComputerMillis)
			lua_pushinteger(L, kernel->GetTimeSinceStart());
			return 1;
		}
		
		LuaFunc(luaComputerPCIDevices)
			lua_newtable(L);
			FFINNetworkTrace Obj = getObjInstance(L, 1, UFINClass::StaticClass());
			UFINClass* Type = Cast<UFINClass>(Obj.Get());
			int i = 1;
			for (TScriptInterface<IFINPciDeviceInterface> Device : kernel->GetPCIDevices()) {
				if (Type && !Device.GetObject()->IsA(Cast<UClass>(Type->GetOuter()))) continue;
				newInstance(L, FFINNetworkTrace(kernel->GetNetwork()->GetComponent().GetObject()) / Device.GetObject());
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
			{"getPCIDevices", luaComputerPCIDevices},
			{nullptr, nullptr}
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