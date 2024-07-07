#include "FINLua/Reflection/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "Network/FINNetworkUtils.h"
#include "Network/Signals/FINSignalSubsystem.h"

namespace FINLua {
#define LOCTEXT_NAMESPACE "EventModule"
	BeginLuaModule(Event, LOCTEXT("DisplayName", "Event Module"), LOCTEXT("Description", ""))
#define LOCTEXT_NAMESPACE "EventLibrary"
	BeginLibrary(event, LOCTEXT("DisplayName", "Event"), LOCTEXT("Description", ""))

	void luaListen(lua_State* L, FFINNetworkTrace o) {
		const UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
		UObject* obj = *o;
		if (!IsValid(obj)) luaL_error(L, "object is not valid");
		AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
		SigSubSys->Listen(obj, o.Reverse() / net->GetComponent().GetObject());
	}

	FieldFunction(listen, LOCTEXT("listen_DisplayName", "Listen"), LOCTEXT("listen_Description", "")) {
		// ReSharper disable once CppDeclaratorNeverUsed
		FLuaSyncCall SyncCall(L);
		const int args = lua_gettop(L);

		for (int i = 1; i <= args; ++i) {
			FFINNetworkTrace trace = luaFIN_checkObject(L, i, nullptr);
			luaListen(L, trace);
		}
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(listening, LOCTEXT("listening_DisplayName", "Listening"), LOCTEXT("listening_Description", "")) {
		// ReSharper disable once CppDeclaratorNeverUsed
		FLuaSyncCall SyncCall(L);

		UObject* netComp = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->GetComponent().GetObject();
		
		TArray<UObject*> Listening = AFINSignalSubsystem::GetSignalSubsystem(netComp)->GetListening(netComp);
		int i = 0;
		lua_newtable(L);
		for (UObject* Obj : Listening) {
			luaFIN_pushObject(L, FFINNetworkTrace(netComp) / Obj);
			lua_seti(L, -2, ++i);
		}
		return 1;
	}

	// ReSharper disable once CppParameterNeverUsed
	int luaPullContinue(lua_State* L, int status, lua_KContext ctx) {
		const int args = lua_gettop(L);

		return args - ctx;
	}

	FieldFunction(pull, LOCTEXT("pull_DisplayName", "Pull"), LOCTEXT("pull_Description", "")) {
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

			luaProc->GetTickHelper().shouldWaitForSignal();
			
			return lua_yieldk(L, 0, args, luaPullContinue);
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

	FieldFunction(ignore, LOCTEXT("ignore_DisplayName", "Ignore"), LOCTEXT("ignore_Description", "")) {
		// ReSharper disable once CppDeclaratorNeverUsed
		FLuaSyncCall SyncCall(L);
		const int args = lua_gettop(L);

		for (int i = 1; i <= args; ++i) {
			FFINNetworkTrace Trace = luaFIN_checkObject(L, i, nullptr);
			luaIgnore(L, Trace);
		}
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(ignoreAll, LOCTEXT("ignoreAll_DisplayName", "Ignore All"), LOCTEXT("ignoreAll_Description", "")) {
		// ReSharper disable once CppDeclaratorNeverUsed
		FLuaSyncCall SyncCall(L);
		UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
		AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(net->GetComponent().GetObject());
		SigSubSys->IgnoreAll(net->GetComponent().GetObject());
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	int luaClear(lua_State* L) {
		UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->ClearSignals();
		return 0;
	}

	EndLibrary()

	ModulePostSetup() {
		lua_pushcfunction(L, (int(*)(lua_State*))eventLibrary::luaPullContinue);
		luaFIN_persistValue(L, -1, "Event-luaPullContinue");
		lua_pop(L, 1);
	}

	EndLuaModule()
}
