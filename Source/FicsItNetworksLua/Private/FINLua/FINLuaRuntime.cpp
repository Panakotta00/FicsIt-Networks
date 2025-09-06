#include "FINLua/FINLuaRuntime.h"

#include "Base64.h"
#include "FicsItLogLibrary.h"
#include "FicsItNetworksLuaModule.h"
#include "FINLuaModule.h"
#include "FINLuaRuntimePersistence.h"
#include "LuaFuture.h"
#include "Union.h"

void luaWarnF(void* ud, const char* msg, int tocont) {
	UFILogLibrary::Log(FIL_Verbosity_Warning, UTF8_TO_TCHAR(msg));
}

int luaPanicF(lua_State* L) {
	FString message = FINLua::luaFIN_toFString(L, -1);
	UE_LOG(LogFicsItNetworksLua, Warning, TEXT("A Lua Runtime caused Panic with message: %s"), *message);
	throw FFINLuaPanic(message);
}

FFINLuaRuntime::FFINLuaRuntime() {
	Modules.Add(TEXT("BaseModule"), FFIRAnyValue());
}

FFINLuaRuntime::~FFINLuaRuntime() {
	Destroy();
}

void FFINLuaRuntime::Reset() {
	Destroy();

	OnPreReset.Broadcast();

	// create new lua state
	LuaState = luaL_newstate();

	*static_cast<FFINLuaRuntime**>(lua_getextraspace(LuaState)) = this;

	// setup error/warning function
	lua_setwarnf(LuaState, luaWarnF, this);
	lua_atpanic(LuaState, luaPanicF);

	// setup registry
	lua_newtable(LuaState);
	lua_setfield(LuaState, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_UNPERSIST);
	lua_newtable(LuaState);
	lua_setfield(LuaState, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_PERSIST);
	lua_newtable(LuaState);
	lua_seti(LuaState, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS);

	OnPreModules.Broadcast();

	FINLua::luaFIN_loadModules(LuaState, Modules);

	// create new thread for user code chunk
	LuaThread = lua_newthread(LuaState);
	LuaThreadIndex = lua_gettop(LuaState);

	OnPostReset.Broadcast();
}

void FFINLuaRuntime::Destroy() {
	OnPreDestroy.Broadcast();

	if (LuaState) {
		lua_close(LuaState);
	}

	Timeout.Reset(),
	LoadedModules.Empty();
	GlobalData.Empty();
	GlobalPointers.Empty();
	LuaState = nullptr;
	LuaThread = nullptr;
	LuaThreadIndex = 0;

	TickActions.Empty();
}

TOptional<FString> FFINLuaRuntime::LoadCode(const FString& Code) {
	// setup thread with code
	const FTCHARToUTF8 CodeConv(*Code, Code.Len());
	luaL_loadbuffer(LuaThread, CodeConv.Get(), CodeConv.Length(), "=EEPROM");
	if (lua_isstring(LuaThread, -1)) {
		// Syntax error
		return FINLua::luaFIN_toFString(LuaThread, -1);
	}
	return {};
}

int luaUnpersist(lua_State* L) {
	// data-str, uperm
	lua_pushboolean(L, true);
	eris_set_setting(L, "path", -1);
	eris_unpersist(L, 2, 1); // data-str, uperm, data

	return 1;
}

TOptional<FString> FFINLuaRuntime::LoadState(FFINLuaRuntimePersistenceState& InState) {
	if (InState.IsFailure()) {
		return FString::Printf(TEXT("Due to previous computer state save error: %s"), *InState.Failure);
	}

	if (InState.LuaData.Len() < 1) return {};

	if (!LuaState) {
		return {};
	}

	// decode & check data from json
	TArray<uint8> luaData;
	if (!FBase64::Decode(InState.LuaData, luaData)) {
		return {};
	}
	lua_pushlstring(LuaState, (char*)luaData.GetData(), luaData.Num());		// ..., data-str

	// get uperm table
	lua_getfield(LuaState, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_UNPERSIST);			// ..., data-str, uperm

	PersistenceState = &InState;

	try {
		eris_unpersist(LuaState, -1, -2);				// ..., data-str, uperm, data
	} catch(const FFINLuaPanic& e) {
		PersistenceState = nullptr;
		return e.Message;
	}
	PersistenceState = nullptr;

	// place persisted data
	lua_geti(LuaState, -1, 1);                     	                // ..., data-str, uperm, data, hidden-globals
	lua_seti(LuaState, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS);	// ..., data-str, uperm, data
	lua_geti(LuaState, -1, 2);					                	// ..., data-str, uperm, data, globals
	lua_seti(LuaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);  		// ..., data-str, uperm, data
	lua_geti(LuaState, -1, 3);	             						// ..., data-str, uperm, data, thread
	lua_replace(LuaState, LuaThreadIndex);					    	// ..., data-str, uperm, data

	LuaThread = lua_tothread(LuaState, LuaThreadIndex);

	lua_pop(LuaState, 3);									     		// ...

	return {};
}

int luaPersist(lua_State* L) {
	// perm, data
	lua_pushboolean(L, true);
	eris_set_setting(L, "path", -1);
	eris_persist(L, lua_upvalueindex(1), lua_upvalueindex(2));		// ..., perm, data, data-str
	return 1;
}

FFINLuaRuntimePersistenceState FFINLuaRuntime::SaveState() {
	FFINLuaRuntimePersistenceState State;

	if (!LuaState) {
		return State;
	}

	// prepare state data
	lua_getfield(LuaState, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_PERSIST);		// ..., perm
	lua_newtable(LuaState);															// ..., perm, data
	lua_geti(LuaState, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS);         	// ..., perm, data, hidden-globals
	lua_seti(LuaState, -2, 1);	                								// ..., perm, data
	lua_geti(LuaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);						// ..., perm, data, globals
	lua_seti(LuaState, -2, 2);					              					// ..., perm, data
	lua_pushvalue(LuaState, LuaThreadIndex);											// ..., perm, data, thread
	lua_seti(LuaState, -2, 3);         											// ..., perm, data

	PersistenceState = &State;

	lua_pushcclosure(LuaState, &luaPersist, 2);
	int stateCode = lua_pcall(LuaState, 0, 1, 0);								// ..., data-str|error

	PersistenceState = nullptr;

	if (stateCode != LUA_OK) {
		FString message = FINLua::luaFIN_toFString(LuaState, -1);
		return FFINLuaRuntimePersistenceState(message);
	}

	// encode persisted data
	size_t data_l = 0;
	const char* data_r = lua_tolstring(LuaState, -1, &data_l);
	TArray<uint8> data((uint8*)data_r, data_l);
	State.LuaData = FBase64::Encode(data);

	lua_pop(LuaState, 1);														// ...

	return State;
}

FFINLuaRuntime& FINLua::luaFIN_getRuntime(lua_State* L) {
	return **(FFINLuaRuntime**)lua_getextraspace(L);
}

void luaFIN_lastChanceTickHook(lua_State* L, lua_Debug*);

void luaFIN_tickHook(lua_State* L, lua_Debug*) {
	FFINLuaRuntime& runtime = FINLua::luaFIN_getRuntime(L);
	bool bShouldYield = true;
	runtime.OnTickHook.Broadcast(bShouldYield);
	if (!bShouldYield) return;

	if (lua_isyieldable(L)) {
		lua_yield(L, 0);
	} else {
		if (!runtime.LastChance.IsSet()) {
			runtime.LastChance = {{FPlatformTime::Seconds()+2, *runtime.Hook_Tick}};
		}
		lua_sethook(L, luaFIN_lastChanceTickHook, LUA_MASKCOUNT, 1);
	}
}

void luaFIN_lastChanceTickHook(lua_State* L, lua_Debug*) {
	FFINLuaRuntime& runtime = FINLua::luaFIN_getRuntime(L);
	if (!runtime.LastChance.IsSet()) {
		lua_sethook(L, luaFIN_tickHook, LUA_MASKCOUNT, runtime.Hook_Tick.Get(1));
		return;
	}
	if (lua_isyieldable(L)) {
		runtime.LastChance.Reset();
		lua_yield(L, 0);
	} else {
		double now = FPlatformTime::Seconds();
		if (now >= runtime.LastChance->Key && --runtime.LastChance->Value <= 0) {
			runtime.LastChance.Reset();
			luaL_error(L, "Timeout reached! Code took to long to leave un-yieldable C-Call Boundary.");
		}
	}
}

TOptional<TTuple<int, int>> FFINLuaRuntime::Tick() {
	if (!TickActions.IsEmpty()) {
		TFunction<void()> func;
		while (TickActions.Dequeue(func)) {
			func();
		}
	} else {
		if (Timeout) {
			double now = FPlatformTime::Seconds();
			if (*Timeout <= now) {
				return  {LuaTick()};
			}
		} else {
			return {LuaTick()};
		}
	}
	return {};
}

TTuple<int, int> FFINLuaRuntime::LuaTick() {
	TArray<TSharedPtr<void>> TickScope;
	OnPreLuaTick.Broadcast(TickScope);
	Timeout.Reset();

	if (Hook_Tick) {
		lua_sethook(LuaThread, luaFIN_tickHook, LUA_MASKCOUNT, *Hook_Tick);
	}

	int results;
	int status = lua_resume(LuaThread, LuaState, 0, &results);

	OnPostLuaTick.Broadcast(status, results);

	LastChance.Reset();

	if (status == LUA_YIELD) {
		Timeout.Reset();
		if (results > 1) {
			if (lua_type(LuaThread, -results+1) == LUA_TNUMBER) {
				double timeout = lua_tonumber(LuaThread, -results+1);
				Timeout = timeout;
			} else if (lua_type(LuaThread, -results+1) == LUA_TNIL) {
				Timeout = TNumericLimits<double>::Max();
			}
		}

		lua_pop(LuaThread, results);

		lua_gc(LuaState, LUA_GCCOLLECT, 0);
	}

	return {status, results};
}

FFINLuaRuntime::EStatus FFINLuaRuntime::GetStatus() const {
	if (!LuaState) {
		return None;
	}
	int status = lua_status(LuaThread);
	switch (status) {
	case LUA_YIELD:
		return Running;
	case LUA_OK: {
		int results = lua_gettop(LuaThread);
		if (results == 0) {
			return Finished;
		}
		return Running;
	}
	default:
		return Crashed;
	}
}

TOptional<FString> FFINLuaRuntime::GetError() const {
	if (GetStatus() != Crashed) {
		return {};
	}

	const char* message = lua_tostring(LuaThread, -1);
	try {
		luaL_traceback(LuaThread, LuaThread, message, 0);
	} catch (FString error) {}
	return {FINLua::luaFIN_toFString(LuaThread, -1)};
}
