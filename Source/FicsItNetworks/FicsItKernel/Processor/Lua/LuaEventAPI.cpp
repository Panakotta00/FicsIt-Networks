#include "LuaEventAPI.h"

#include "Network/Signals/FINSignalSender.h"
#include "Network/FINNetworkComponent.h"
#include "FicsItKernel/FicsItKernel.h"

#include "FGPowerCircuit.h"

#include "LuaProcessor.h"
#include "LuaInstance.h"
#include "LuaHooks.h"

namespace FicsItKernel {
	namespace Lua {
		void luaListen(lua_State* L, Network::NetworkTrace o) {
			auto net = LuaProcessor::luaGetProcessor(L)->getKernel()->getNetwork();
			UObject* obj = *o;
			if (!IsValid(obj)) luaL_error(L, "object is not valid");
			if (obj->Implements<UFINSignalSender>()) {
				IFINSignalSender::Execute_AddListener(obj, o.reverse());
				UFINSignalUtility::SetupSender(obj->GetClass());
				// TODO: add sender as signal source to net
			}
			if (obj->Implements<UFINNetworkComponent>()) {
				TSet<UObject*> merged = IFINNetworkComponent::Execute_GetMerged(obj);
				for (auto m : merged) {
					luaListen(L, o(m));
				}
			}

			// PowerCircuit
			if (auto circuit = Cast<UFGPowerCircuit>(*o)) {
				luaListenCircuit(o);
			}
		}

		int luaListen(lua_State* L) {
			int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				Network::NetworkTrace trace;
				auto o = (UObject*)getObjInstance<UObject>(L, i, &trace);
				luaListen(L, trace / o);
			}
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		int luaPullContinue(lua_State* L, int status, lua_KContext ctx) {
			int args = lua_gettop(L);

			return args - ctx;
		}

		int luaPull(lua_State* L) {
			int args = lua_gettop(L);
			double t = 0.0;
			if (args > 0) t = lua_tonumber(L, 1);

			auto luaProc = LuaProcessor::luaGetProcessor(L);
			check(luaProc);
			int a = luaProc->doSignal(L);
			if (!a && !(args > 0 && lua_isinteger(L, 1) && lua_tointeger(L, 1) == 0)) {
				luaProc->timeout = t;
				luaProc->pullStart = std::chrono::high_resolution_clock::now();
				luaProc->pullState = (args > 0) ? 1 : 2;
				
				lua_yieldk(L, 0, args, luaPullContinue);
				return LuaProcessor::luaAPIReturn(L, 0);
			}
			luaProc->pullState = 0;
			return LuaProcessor::luaAPIReturn(L, a);
		}

		static const luaL_Reg luaEventLib[] = {
			{"listen", luaListen},
			{"pull", luaPull},
			{NULL,NULL}
		};

		void setupEventAPI(lua_State* L) {
			PersistSetup("Event", -2);
			lua_newtable(L);
			luaL_setfuncs(L, luaEventLib, 0);
			PersistTable("Lib", -1);
			lua_setglobal(L, "event");
		}
	}
}
