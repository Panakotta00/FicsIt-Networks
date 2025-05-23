﻿#pragma once

#include "CoreMinimal.h"
#include "AsyncWork.h"
#include "Event.h"
#include "FINLuaRuntime.h"
#include "FINLuaThreadedRuntime.generated.h"

struct FFINLuaThreadedRuntime;
struct FICSITNETWORKSLUA_API FFINLuaTickRunnable : public FNonAbandonableTask {
private:
	FFINLuaThreadedRuntime* Runtime;

public:
	FFINLuaTickRunnable(FFINLuaThreadedRuntime* Runtime) : Runtime(Runtime) {}

	void DoWork();

	// ReSharper disable once CppMemberFunctionMayBeStatic
	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

USTRUCT()
struct FICSITNETWORKSLUA_API FFINLuaThreadedRuntime {
	GENERATED_BODY()
	friend FFINLuaTickRunnable;
private:
	// Misc
	FDelegateHandle OnPreGarbageCollectionHandle;
	FDelegateHandle OnPostGarbageCollectionHandle;

	// Thread Handling
	FAsyncTask<FFINLuaTickRunnable> LuaTask;
	FRWLock ShouldRunInThreadMutex;
	bool ShouldRunInThread = false;
	FCriticalSection IsWaitingForCompletionMutex;
	bool bIsWaitingForCompletion = false;
	TSharedPtr<FEventRef> WaitForGame;
	TSharedPtr<FEventRef> ContinueGame;
	FCriticalSection LuaTickMutex;
	bool bIsPromotedTick = false;

public:
	UPROPERTY()
	FFINLuaRuntime Runtime;

	FFINLuaThreadedRuntime();
	~FFINLuaThreadedRuntime();

	// [Any Thread]
	void SetShouldBePromoted(bool bInShouldBePromoted) {
		FRWScopeLock Lock(ShouldRunInThreadMutex, SLT_Write);
		ShouldRunInThread = bInShouldBePromoted;
	}
	bool ShouldBePromoted() {
		FRWScopeLock Lock(ShouldRunInThreadMutex, SLT_ReadOnly);
		return ShouldRunInThread;
	}
	bool ShouldThreadRun();

	// [Main Thead]
	void HandleWaitForGame();
	// [Lua Tick]
	TSharedPtr<FEventRef> DoWaitForGame();

	// [Main Thread]
	// Resume the Lua Runtime and blocks until yielded or returned. If promoted, but should not be promoted, will not wait.
	// If promoted, may sync.
	// If should be promoted, but is not, will start thread.
	TOptional<TTuple<int, int>> Run();

	// [Main Thread]
	// If promoted, blocks and waits for it to yield/return. Runtime has to be resumed manually (f.e Run()).
	void PauseAndWait();

	bool IsTickPromoted() const { return bIsPromotedTick; }

	FFINLuaRuntime::EStatus GetStatus();
};

template<>
struct TStructOpsTypeTraits<FFINLuaThreadedRuntime> : TStructOpsTypeTraitsBase2<FFINLuaThreadedRuntime> {
	enum {
		WithCopy = false,
	};
};

namespace FINLua {
	FICSITNETWORKSLUA_API void luaFIN_setThreadedRuntime(lua_State* L, FFINLuaThreadedRuntime& threadedRuntime);
	FICSITNETWORKSLUA_API FFINLuaThreadedRuntime& luaFIN_getThreadedRuntime(lua_State* L);
	FICSITNETWORKSLUA_API FFINLuaThreadedRuntime* luaFIN_tryGetThreadedRuntime(lua_State* L);

	struct FLuaSync {
		FFINLuaThreadedRuntime* Runtime = nullptr;
		TSharedPtr<FEventRef> Event;
		FLuaSync(lua_State* L) {
			Runtime = luaFIN_tryGetThreadedRuntime(L);
			if (Runtime && Runtime->IsTickPromoted()) {
				Event = Runtime->DoWaitForGame();
			}
		}
		~FLuaSync() {
			if (Event) {
				Event->Get()->Trigger();
			}
		}
	};
}


