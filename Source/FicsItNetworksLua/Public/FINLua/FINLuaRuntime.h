#pragma once

#include "CoreMinimal.h"
#include "FIRAnyValue.h"
#include "FIRInstancedStruct.h"
#include "Queue.h"
#include "Union.h"
#include "FINLuaRuntime.generated.h"

struct FFINLuaRuntimePersistenceState;
struct lua_State;
namespace FINLua {
	FFINLuaRuntimePersistenceState& luaFIN_getPersistence(lua_State*);
}

#define LUAFIN_REGISTRYKEY_UNPERSIST "PersistUperm"
#define LUAFIN_REGISTRYKEY_PERSIST "PersistPerm"
#define LUAFIN_REGISTRYKEY_HIDDENGLOBALS "hidden-globals"

struct FFINLuaPanic {
	FString Message;
};

USTRUCT()
struct FFINLuaRuntime {
	GENERATED_BODY()
	friend FFINLuaRuntimePersistenceState& FINLua::luaFIN_getPersistence(lua_State*);
public:
	enum EStatus {
		None,
		Running,
		Finished,
		Crashed,
	};
private:
	// Lua State
	lua_State* LuaState = nullptr;
	lua_State* LuaThread = nullptr;
	int LuaThreadIndex = 0;

	FFINLuaRuntimePersistenceState* PersistenceState = nullptr;

	FFINLuaRuntime(const FFINLuaRuntime&) = delete;
	FFINLuaRuntime(FFINLuaRuntime&&) = delete;
	FFINLuaRuntime& operator=(const FFINLuaRuntime&) = delete;
	FFINLuaRuntime& operator=(FFINLuaRuntime&&) = delete;

public:
	TMulticastDelegate<void()> OnPreReset;
	TMulticastDelegate<void()> OnPreModules;
	TMulticastDelegate<void()> OnPostReset;
	TMulticastDelegate<void()> OnPreDestroy;
	TMulticastDelegate<void(TArray<TSharedPtr<void>>&)> OnPreLuaTick;
	TMulticastDelegate<void(int, int)> OnPostLuaTick;
	TMulticastDelegate<void(lua_State*)> PreLoadState;
	TMulticastDelegate<void(lua_State*, lua_State*)> PostLoadState;
	TMulticastDelegate<void(lua_State*, lua_State*)> PreSaveState;
	TMulticastDelegate<void(lua_State*, lua_State*)> PostSaveState;

	TOptional<int> Hook_Tick;
	TMap<FString, FFIRAnyValue> Modules;

	TQueue<TFunction<void()>> TickActions;

	TMap<FString, FFIRAnyValue> LoadedModules;
	TMap<FString, FFIRAnyValue> GlobalData;
	TMap<FString, void*> GlobalPointers;

	FFINLuaRuntime();
	~FFINLuaRuntime();

	void Reset();
	void Destroy();

	TOptional<FString> LoadCode(const FString& Code);
	TOptional<FString> LoadState(FFINLuaRuntimePersistenceState& InState);
	TUnion<FFINLuaRuntimePersistenceState, FString> SaveState();

	TOptional<TTuple<int, int>> Tick();
	TTuple<int, int> LuaTick();

	EStatus GetStatus() const;
	TOptional<FString> GetError() const;

	lua_State* GetLuaState() const { return LuaState; }
	lua_State* GetLuaThread() const { return LuaThread; }
};

namespace FINLua {
	FFINLuaRuntime& luaFIN_getRuntime(lua_State* L);
}

template<>
struct TStructOpsTypeTraits<FFINLuaRuntime> : TStructOpsTypeTraitsBase2<FFINLuaRuntime> {
	enum {
		WithCopy = false,
	};
};
