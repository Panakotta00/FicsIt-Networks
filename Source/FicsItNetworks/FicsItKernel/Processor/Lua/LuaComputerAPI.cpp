#include "LuaComputerAPI.h"

#include "FGTimeSubsystem.h"
#include "FINStateEEPROMLua.h"
#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "FicsItNetworks/Network/FINDynamicStructHolder.h"
#include "FicsItNetworks/Network/FINNetworkUtils.h"
#include "FicsItNetworks/Reflection/FINClass.h"

#define LuaFunc(funcName) \
int funcName(lua_State* L) { \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	FLuaSyncCall SyncCall(L);

namespace FicsItKernel {
	namespace Lua {
		LuaFunc(luaComputerGetInstance)
			newInstance(L, UFINNetworkUtils::RedirectIfPossible(FFINNetworkTrace(kernel->GetNetwork()->GetComponent().GetObject())));
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}

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

		int luaComputerPromote(lua_State* L) {
			UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
			processor->GetTickHelper().shouldPromote();
			return UFINLuaProcessor::luaAPIReturn(L, 0);
		}
		
		int luaComputerDemote(lua_State* L) {
			FLuaSyncCall Sync(L);
			return UFINLuaProcessor::luaAPIReturn(L, 0);
		}

		int luaComputerState(lua_State* L) {
			UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
			int state = 0;
			if (processor->GetTickHelper().getState() & LUA_ASYNC) {
				state = 1;
			}
			lua_pushinteger(L, state);
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}
		
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
			size_t len;
			const char* str = luaL_checklstring(L, 1, &len);
			FUTF8ToTCHAR Conv(str, len);
			eeprom->SetCode(FString(Conv.Length(), Conv.Get()));
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

		LuaFunc(luaComputerMagicTime)
			FDateTime Now = FDateTime::UtcNow();
			lua_pushinteger(L, Now.ToUnixTimestamp());
			FTCHARToUTF8 ConvertStr(*Now.ToString());
			lua_pushlstring(L, ConvertStr.Get(), ConvertStr.Length());
			FTCHARToUTF8 ConvertIso(*Now.ToIso8601());
			lua_pushlstring(L, ConvertIso.Get(), ConvertIso.Length());
			return 3;
		}
		
		LuaFunc(luaComputerPCIDevices)
			lua_newtable(L);
			int args = lua_gettop(L);
			UFINClass* Type = nullptr;
			if (args > 0) {
				FFINNetworkTrace Obj = getObjInstance(L, 1, UFINClass::StaticClass());
				Type = Cast<UFINClass>(Obj.Get());
				if (!Type) {
					return 1;
				}
			}
			int i = 1;
			for (TScriptInterface<IFINPciDeviceInterface> Device : kernel->GetPCIDevices()) {
				if (!Device || (Type && !Device.GetObject()->IsA(Cast<UClass>(Type->GetOuter())))) continue;
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
			{"promote", luaComputerPromote},
			{"demote", luaComputerDemote},
			{"state", luaComputerState},
			{"beep", luaComputerBeep},
			{"setEEPROM", luaComputerSetEEPROM},
			{"getEEPROM", luaComputerGetEEPROM},
			{"time", luaComputerTime},
			{"millis", luaComputerMillis},
			{"magicTime", luaComputerMagicTime},
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