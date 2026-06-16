#include "FINLuaThreadedRuntime.h"

#include "FINLuaModule.h"
#include "LuaPersistence.h"

FFINLuaThreadedRuntime::FFINLuaThreadedRuntime() : LuaTask(FAsyncTask<FFINLuaTickRunnable>(this)) {
	Runtime.Modules.Add("ThreadedRuntimeModule");
	OnPreGarbageCollectionHandle = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddLambda([this]() {
		PauseAndWait();
	});
	Runtime.OnPostReset.AddLambda([this]() {
		if (Runtime.GetLuaState() == nullptr) return;
		SetShouldBePromoted(false);
		FINLua::luaFIN_setThreadedRuntime(Runtime.GetLuaState(), *this);
	});
	Runtime.OnPreDestroy.AddLambda([this]() {
		PauseAndWait();
	});
	Runtime.OnTickHook.AddLambda([this](bool& bShouldYield) {
		if (bIsPromotedTick && ShouldThreadRun()) {
			WaitForGame.Reset();
			if (TSharedPtr<FEventRef> continueGame = ContinueGame) {
				continueGame->Get()->Trigger();
			}
			bShouldYield = false;
		}
	});
}

FFINLuaThreadedRuntime::~FFINLuaThreadedRuntime() {
	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(OnPreGarbageCollectionHandle);
	PauseAndWait();
}

void FFINLuaTickRunnable::DoWork() {
	Runtime->bIsPromotedTick = true;
	while (Runtime->bIsPromotedTick) {
		Runtime->WaitForGame.Reset();
		Runtime->ContinueGame = MakeShared<FEventRef>(EEventMode::ManualReset);

		TOptional<TTuple<int, int>> status = Runtime->Runtime.Tick();

		if (!Runtime->ShouldThreadRun() || (status && status->Get<0>() != LUA_YIELD)) {
			Runtime->bIsPromotedTick = false;
		}

		Runtime->ContinueGame->Get()->Trigger();
	}
}

bool FFINLuaThreadedRuntime::ShouldThreadRun() {
	FRWScopeLock Lock(ShouldRunInThreadMutex, SLT_ReadOnly);
	return ShouldRunInThread && Runtime.TickActions.IsEmpty() && !Runtime.Timeout.IsSet() && !bIsWaitingForCompletion;
}

void FFINLuaThreadedRuntime::HandleWaitForGame() {
	IsWaitingForCompletionMutex.Lock();
	if (TSharedPtr<FEventRef> waitForGame = WaitForGame) {
		waitForGame->Get()->Trigger();
		IsWaitingForCompletionMutex.Unlock();
		ContinueGame->Get()->Wait();
	} else {
		IsWaitingForCompletionMutex.Unlock();
	}
}

TSharedPtr<FEventRef> FFINLuaThreadedRuntime::DoWaitForGame() {
	if (!bIsPromotedTick) return nullptr;

	IsWaitingForCompletionMutex.Lock();
	if (bIsWaitingForCompletion) {
		IsWaitingForCompletionMutex.Unlock();
		return nullptr;
	}

	if (WaitForGame) {
		IsWaitingForCompletionMutex.Unlock();
		return nullptr;
	}

	ContinueGame->Get()->Reset();
	WaitForGame = MakeShared<FEventRef>(EEventMode::ManualReset);
	IsWaitingForCompletionMutex.Unlock();

	WaitForGame->Get()->Wait();

	return nullptr;
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
		if (Runtime.Tick().IsSet()) {
			// FIN-1.2-PORT (Port-Schuld #Threading): Promotion vorsorglich deaktiviert.
			// HINWEIS: Die eigentliche 0xc0000374-Crash-Ursache war NICHT diese Threading-Race,
			// sondern eine ungefangene FFIRException durch die Lua-C-Frames (Fix in LuaRef.cpp,
			// luaFIN_callReflectionFunctionDirectly: catch(const FFIRException&)). Der frueher
			// gesehene Worker-Thread-close_state-Callstack war nur die Stelle, an der die bereits
			// vorhandene Heap-Corruption beim Threaded-Teardown detektiert wurde.
			// Promotion bleibt vorerst aus, weil die Event-Handshake-Sync (FLuaSync/WaitForGame/
			// ContinueGame) unter UE5.6 (neuer LowLevelTasks-Scheduler, inkrementelle GC, FEvent-
			// Semantik) NICHT verifiziert ist. Lua laeuft synchron auf dem Game-Thread (Runtime.Tick()
			// oben), per Tick-Budget zeitgescheibt - FINs Vor-Threading-Modell. promote() bleibt
			// aufrufbar (ohne Worker-Effekt), bestehende Skripte funktionieren weiter.
			// TODO (Reaktivierung): Promotion wieder einschalten + Threaded-Runtime unter UE5.6 mit
			// promote()-Workload verifizieren; falls instabil, Sync sauber fuer UE5.6 nachziehen.
			if (false && ShouldBePromoted() && Runtime.Hook_Tick.IsSet() && !Runtime.Timeout.IsSet()) {
				LuaTask.StartBackgroundTask(GThreadPool, EQueuedWorkPriority::Normal, EQueuedWorkFlags::DoNotRunInsideBusyWait, -1, TEXT("FINLuaThreadedRuntime"));
			}
		}
	}
	return {};
}

void FFINLuaThreadedRuntime::PauseAndWait() {
	IsWaitingForCompletionMutex.Lock();
	bIsWaitingForCompletion = true;
	IsWaitingForCompletionMutex.Unlock();
	HandleWaitForGame();
	LuaTask.EnsureCompletion(false);
	IsWaitingForCompletionMutex.Lock();
	bIsWaitingForCompletion = false;
	IsWaitingForCompletionMutex.Unlock();

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
