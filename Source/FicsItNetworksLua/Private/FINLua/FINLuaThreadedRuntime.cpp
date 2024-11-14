#include "FINLuaThreadedRuntime.h"

#include "FINLuaModule.h"
#include "LuaPersistence.h"

FFINLuaThreadedRuntime::FFINLuaThreadedRuntime() : LuaTask(FAsyncTask<FFINLuaTickRunnable>(this)) {
	Runtime.Modules.Add("ThreadedRuntimeModule");
	OnPreGarbageCollectionHandle = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddLambda([this]() {
		PauseAndWait();
	});
	Runtime.OnPostReset.AddLambda([this]() {
		SetShouldBePromoted(false);
		FINLua::luaFIN_setThreadedRuntime(Runtime.GetLuaState(), *this);
	});
	Runtime.OnPreDestroy.AddLambda([this]() {
		PauseAndWait();
	});
}

FFINLuaThreadedRuntime::~FFINLuaThreadedRuntime() {
	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(OnPreGarbageCollectionHandle);
	PauseAndWait();
}
UE_DISABLE_OPTIMIZATION_SHIP
void FFINLuaTickRunnable::DoWork() {
	Runtime->bIsPromotedTick = true;
	ON_SCOPE_EXIT {
		Runtime->bIsPromotedTick = false;
	};
	while (Runtime->ShouldRunInThread.Load() && Runtime->Runtime.TickActions.IsEmpty()) {
		TOptional<TTuple<int, int>> status = Runtime->Runtime.Tick();

		if (Runtime->WaitForThread) {
			Runtime->ContinueThread = MakeShared<FEventRef>();
			Runtime->WaitForThread->Get()->Trigger();
			Runtime->ContinueThread->Get()->Wait();
		}

		if (status && status->Get<0>() != LUA_YIELD) {
			break;
		}
	}
}
UE_ENABLE_OPTIMIZATION_SHIP

void FFINLuaThreadedRuntime::HandleWaitForGame() {
	if (WaitForGame) {
		TSharedPtr<FEventRef> waitForGame = WaitForGame;
		fgcheck(waitForGame.IsValid());
		WaitForGame.Reset();
		fgcheck(!ContinueGame.IsValid());
		TSharedPtr<FEventRef> continueGame = ContinueGame = MakeShared<FEventRef>();
		waitForGame->Get()->Trigger();
		continueGame->Get()->Wait();
	}
}

TSharedPtr<FEventRef> FFINLuaThreadedRuntime::DoWaitForGame() {
	if (!bIsPromotedTick) return nullptr;
	IsWaitingForCompletionMutex.Lock();
	if (bIsWaitingForCompletion) {
		IsWaitingForCompletionMutex.Unlock();
		return nullptr;
	}
	fgcheck(!WaitForGame.IsValid());
	TSharedPtr<FEventRef> waitForGame = WaitForGame = MakeShared<FEventRef>();
	IsWaitingForCompletionMutex.Unlock();
	waitForGame->Get()->Wait();
	TSharedPtr<FEventRef> continueGame = ContinueGame;
	fgcheck(continueGame.IsValid());
	ContinueGame.Reset();
	return continueGame.ToSharedRef();
}

TOptional<TTuple<int, int>> FFINLuaThreadedRuntime::Run() {
	HandleWaitForGame();
	if (!Runtime.TickActions.IsEmpty()) {
		PauseAndWait();
		TFunction<void()> func;
		while (Runtime.TickActions.Dequeue(func)) {
			func();
		}
	} else if (LuaTask.IsDone() && GetStatus() == FFINLuaRuntime::Running) {
		if (ShouldRunInThread.Load() && Runtime.Hook_Tick.IsSet()) {
			LuaTask.StartBackgroundTask(GThreadPool, EQueuedWorkPriority::Normal, EQueuedWorkFlags::DoNotRunInsideBusyWait, -1, TEXT("FINLuaThreadedRuntime"));
		} else {
			return Runtime.Tick();
		}
	}
	return {};
}

void FFINLuaThreadedRuntime::PauseAndWait() {
	bool bShouldThreadRun = ShouldRunInThread.Load();
	ShouldRunInThread.Store(false);
	IsWaitingForCompletionMutex.Lock();
	bIsWaitingForCompletion = true;
	IsWaitingForCompletionMutex.Unlock();
	HandleWaitForGame();
	LuaTask.EnsureCompletion(false);
	bIsWaitingForCompletion = false;
	ShouldRunInThread.Store(bShouldThreadRun);

	TFunction<void()> func;
	while (Runtime.TickActions.Dequeue(func)) {
		func();
	}
}

FFINLuaRuntime::EStatus FFINLuaThreadedRuntime::GetStatus() {
	if (LuaTask.IsDone()) {
		return Runtime.GetStatus();
	} else {
		return FFINLuaRuntime::Running;
	}
}


namespace FINLua {
	void luaFIN_setThreadedRuntime(lua_State* L, FFINLuaThreadedRuntime& threadedRuntime) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		runtime.GlobalPointers.Add(TEXT("threaded"), &threadedRuntime);
	}

	FFINLuaThreadedRuntime& luaFIN_getThreadedRuntime(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		FFINLuaThreadedRuntime** threaded = reinterpret_cast<FFINLuaThreadedRuntime**>(runtime.GlobalPointers.Find(TEXT("threaded")));
		fgcheck(threaded);
		return **threaded;
	}

	FFINLuaThreadedRuntime* luaFIN_tryGetThreadedRuntime(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		FFINLuaThreadedRuntime** threaded = reinterpret_cast<FFINLuaThreadedRuntime**>(runtime.GlobalPointers.Find(TEXT("threaded")));
		if (!threaded) {
			return nullptr;
		}
		return *threaded;
	}
}
