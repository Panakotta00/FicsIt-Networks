#include "LuaEventAPI.h"

#include "FicsItNetworks/FicsItKernel/FicsItKernel.h"
#include "FGPowerCircuit.h"
#include "LuaProcessor.h"
#include "LuaInstance.h"
#include "FicsItNetworks/Network/FINNetworkTrace.h"
#include "FicsItNetworks/Network/Signals/FINSignalSubsystem.h"

namespace FicsItKernel {
	namespace Lua {
		void luaListen(lua_State* L, FFINNetworkTrace o) {
			const UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
			UObject* obj = *o;
			if (!IsValid(obj)) luaL_error(L, "object is not valid");
			AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
			SigSubSys->Listen(obj, o.Reverse() / net->GetComponent().GetObject());
		}

		int luaListen(lua_State* L) {
			// ReSharper disable once CppDeclaratorNeverUsed
			FLuaSyncCall SyncCall(L);
			const int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				FFINNetworkTrace trace;
				UObject* o = getObjInstance<UObject>(L, i, &trace);
				luaListen(L, trace / o);
			}
			return UFINLuaProcessor::luaAPIReturn(L, 0);
		}

		int luaListening(lua_State* L) {
			// ReSharper disable once CppDeclaratorNeverUsed
			FLuaSyncCall SyncCall(L);

			UObject* netComp = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->GetComponent().GetObject();
			
			TArray<UObject*> Listening = AFINSignalSubsystem::GetSignalSubsystem(netComp)->GetListening(netComp);
			int i = 0;
			lua_newtable(L);
			for (UObject* Obj : Listening) {
				newInstance(L, FFINNetworkTrace(netComp) / Obj);
				lua_seti(L, -2, ++i);
			}
			return 1;
		}

		// ReSharper disable once CppParameterNeverUsed
		int luaPullContinue(lua_State* L, int status, lua_KContext ctx) {
			const int args = lua_gettop(L);

			return args - ctx;
		}

		int luaPull(lua_State* L) {
			// ReSharper disable once CppDeclaratorNeverUsed
			FLuaSyncCall SyncCall(L);
			const int args = lua_gettop(L);
			double t = 0.0;
			if (args > 0) t = lua_tonumber(L, 1);

			UFINLuaProcessor* luaProc = UFINLuaProcessor::luaGetProcessor(L);
			check(luaProc);
			const int a = luaProc->DoSignal(L);
			if (!a && !(args > 0 && lua_isinteger(L, 1) && lua_tointeger(L, 1) == 0)) {
				luaProc->Timeout = t;
				luaProc->PullStart =  (FDateTime::Now() - FFicsItNetworksModule::GameStart).GetTotalMilliseconds();
				luaProc->PullState = (args > 0) ? 1 : 2;
				
				lua_yieldk(L, 0, args, luaPullContinue);
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}
			luaProc->PullState = 0;
			return UFINLuaProcessor::luaAPIReturn(L, a);
		}

		void luaIgnore(lua_State* L, FFINNetworkTrace o) {
			UObject* obj = *o;
			if (!IsValid(obj)) luaL_error(L, "object is not valid");
			AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
			SigSubSys->Ignore(obj, *o.Reverse());
		}

		int luaIgnore(lua_State* L) {
			// ReSharper disable once CppDeclaratorNeverUsed
			FLuaSyncCall SyncCall(L);
			const int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				FFINNetworkTrace trace;
				UObject* o = getObjInstance<UObject>(L, i, &trace);
				luaIgnore(L, trace / o);
			}
			return UFINLuaProcessor::luaAPIReturn(L, 0);
		}

#pragma optimize("", off)
		int luaIgnoreAll(lua_State* L) {
			// ReSharper disable once CppDeclaratorNeverUsed
			FLuaSyncCall SyncCall(L);
			UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
			AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(net->GetComponent().GetObject());
			SigSubSys->IgnoreAll(net->GetComponent().GetObject());
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}
#pragma optimize("", on)

		int luaClear(lua_State* L) {
			UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->ClearSignals();
			return 0;
		}

		static const luaL_Reg luaEventLib[] = {
			{"listen", luaListen},
			{"listening", luaListening},
			{"pull", luaPull},
			{"ignore", luaIgnore},
			{"ignoreAll", luaIgnoreAll},
			{"clear", luaClear},
			{nullptr, nullptr}
		};

		void setupEventAPI(lua_State* L) {
			PersistSetup("Event", -2);
			lua_newtable(L);
			luaL_setfuncs(L, luaEventLib, 0);
			PersistTable("Lib", -1);
			lua_setglobal(L, "event");
			lua_pushcfunction(L, (int(*)(lua_State*))luaPullContinue);
			PersistValue("pullContinue");
		}
	}
}
