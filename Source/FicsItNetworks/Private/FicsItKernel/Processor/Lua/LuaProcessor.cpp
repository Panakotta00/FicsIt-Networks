#include "FicsItKernel/Processor/Lua/LuaProcessor.h"
#include "FicsItKernel/Processor/Lua/LuaInstance.h"
#include "FicsItKernel/Processor/Lua/LuaStructs.h"
#include "FicsItKernel/Processor/Lua/LuaFileSystemAPI.h"
#include "FicsItKernel/Processor/Lua/FINStateEEPROMLua.h"
#include "FicsItKernel/Processor/Lua/LuaComponentAPI.h"
#include "FicsItKernel/Processor/Lua/LuaComputerAPI.h"
#include "FicsItKernel/Processor/Lua/LuaDebugAPI.h"
#include "FicsItKernel/Processor/Lua/LuaEventAPI.h"
#include "FicsItKernel/Processor/Lua/LuaFuture.h"
#include "FicsItKernel/Processor/Lua/LuaRef.h"
#include "Network/FINNetworkTrace.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINSignal.h"


void LuaFileSystemListener::onUnmounted(CodersFileSystem::Path path, CodersFileSystem::SRef<CodersFileSystem::Device> device) {
	for (FicsItKernel::Lua::LuaFile file : Parent->GetFileStreams()) {
		if (file.isValid() && (!Parent->GetKernel()->GetFileSystem() || !Parent->GetKernel()->GetFileSystem()->checkUnpersistPath(file->path))) {
			file->file->close();
		}
	}
}

void LuaFileSystemListener::onNodeRemoved(CodersFileSystem::Path path, CodersFileSystem::NodeType type) {
	for (FicsItKernel::Lua::LuaFile file : Parent->GetFileStreams()) {
		if (file.isValid() && file->path.length() > 0 && (!Parent->GetKernel()->GetFileSystem() || Parent->GetKernel()->GetFileSystem()->unpersistPath(file->path) == path)) {
			file->file->close();
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FLuaTickRunnable::DoWork() {
	while (true) {
		if (!Tick->asyncTick()) {
			break;
		}
	}
}

FFINLuaProcessorTick::FFINLuaProcessorTick() : Runnable(this) {}

FFINLuaProcessorTick::FFINLuaProcessorTick(UFINLuaProcessor* Processor): Processor(Processor), Runnable(this) {
	reset();
}

FFINLuaProcessorTick::~FFINLuaProcessorTick() {
	stop();
	if (asyncTask) asyncTask->EnsureCompletion();
}

void FFINLuaProcessorTick::reset() {
	stop();

	AsyncSync = TPromise<void>();
	SyncAsync = TPromise<void>();
	AsyncSync.EmplaceValue();
	SyncAsync.EmplaceValue();
	bShouldPromote = false;
	bShouldDemote = false;
	bShouldStop = false;
	bShouldReset = false;
	bShouldCrash = false;
	bDoSync = false;
	State = LUA_SYNC;
}

void FFINLuaProcessorTick::stop() {
	if (!(State & LUA_ASYNC)) return;
	demote();
	
	if (asyncTask.IsValid() && !asyncTask->IsIdle()) {
		asyncTask->EnsureCompletion();
		asyncTask->Cancel();
		asyncTask.Reset();
	}
}

void FFINLuaProcessorTick::promote() {
	if (State & LUA_ASYNC) return;
	if (bShouldStop || bShouldCrash || bShouldReset) return;
	if (asyncTask.IsValid()) {
		if (!asyncTask->IsDone()) asyncTask->EnsureCompletion();
		asyncTask->Cancel();
		asyncTask = nullptr;
	}
	asyncTask = MakeShared<FAsyncTask<FLuaTickRunnable>>(this);
	TickMutex.Lock();
	asyncTask->StartBackgroundTask();
	State = LUA_ASYNC;
	TickMutex.Unlock();
}

void FFINLuaProcessorTick::demote() {
	TickMutex.Lock();
	if (State & LUA_SYNC) {
		TickMutex.Unlock();
		return;
	}
	State = LUA_SYNC;
	TickMutex.Unlock();
	if (bDoSync) {
		SyncAsync = TPromise<void>();
		AsyncSync.EmplaceValue();
	}
	asyncTask->EnsureCompletion();
	asyncTask->Cancel();
	TickMutex.Unlock();
}

void FFINLuaProcessorTick::demoteInAsync() {
	if (State & LUA_SYNC) return;
	AsyncSyncMutex.Lock();
	AsyncSync = TPromise<void>();
	SyncAsync = TPromise<void>();
	const TFuture<void> Future = AsyncSync.GetFuture();
	bDoSync = true;
	AsyncSyncMutex.Unlock();
	TickMutex.Unlock();
	Future.Wait(); // wait for sync to continue this async
	TickMutex.Lock();
}

void FFINLuaProcessorTick::shouldStop() {
	bShouldStop = true;
}

void FFINLuaProcessorTick::shouldReset() {
	bShouldReset = true;
}

void FFINLuaProcessorTick::shouldPromote() {
	if (bShouldStop || bShouldCrash || bShouldReset) return;
	bShouldPromote = true;
}

void FFINLuaProcessorTick::shouldDemote() {
	bShouldDemote = true;
}

void FFINLuaProcessorTick::shouldWaitForSignal() {
	if (State & LUA_ASYNC) {
		bWaitForSignal = true;
	}
}

void FFINLuaProcessorTick::signalFound() {
	bWaitForSignal = false;
}

void FFINLuaProcessorTick::shouldCrash(const TSharedRef<FFINKernelCrash>& Crash) {
	bShouldCrash = true;
	ToCrash = Crash;
}

void FFINLuaProcessorTick::syncTick() {
	if (postTick()) return;
	if (State & LUA_SYNC) {
		if (asyncTask.IsValid() && !asyncTask->IsIdle()) {
			asyncTask->EnsureCompletion();
			asyncTask->Cancel();
			asyncTask.Reset();
		}
		TickMutex.Lock();
		Processor->LuaTick();
		TickMutex.Unlock();
		if (bShouldPromote) {
			promote();
		}
	} else if (State & LUA_ASYNC) {
		AsyncSyncMutex.Lock();
		const bool DoSync = bDoSync;
		const bool WaitForSignal = bWaitForSignal;
		AsyncSyncMutex.Unlock();
		if (DoSync) {
			// async tick is waiting for sync
			AsyncSyncMutex.Lock();
			SyncAsync = TPromise<void>();
			const TFuture<void> Future = SyncAsync.GetFuture();
			shouldDemote();
			State = LUA_SYNC;
			AsyncSync.EmplaceValue(); // continue async tick in sync with factory tick
			AsyncSyncMutex.Unlock();
			Future.Wait(); // wait for async tick to finish
			AsyncSyncMutex.Lock();
			bDoSync = false;
			AsyncSyncMutex.Unlock();
		} else {
			if (asyncTask->IsDone() && (!WaitForSignal || Processor->GetKernel()->GetNetwork()->GetSignalCount() > 0 || Processor->PullTimeoutReached())) {
				AsyncSyncMutex.Lock();
				bWaitForSignal = false;
				AsyncSyncMutex.Unlock();
				asyncTask->StartBackgroundTask();
			}
		}
	}
	if (postTick()) return;
}

bool FFINLuaProcessorTick::asyncTick() {
	if (State & LUA_ASYNC) {
		TickMutex.Lock();
		Processor->LuaTick();
		TickMutex.Unlock();
		AsyncSyncMutex.Lock();
		if (bDoSync) {
			SyncAsync.EmplaceValue();
			bDoSync = false;
		}
		AsyncSyncMutex.Unlock();
		if (bShouldDemote) {
			TickMutex.Lock();
			State = LUA_SYNC;
			bShouldDemote = false;
			TickMutex.Unlock();
			return false;
		}
		if (bShouldCrash || bShouldStop || bShouldReset) {
			TickMutex.Lock();
			State = LUA_SYNC;
			TickMutex.Unlock();
			return false;
		}
		return !bWaitForSignal;
	}
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FFINLuaProcessorTick::postTick() {
	if (bShouldCrash) {
		Processor->GetKernel()->Crash(ToCrash.ToSharedRef());
		return true;
	}
	if (bShouldStop) {
		Processor->GetKernel()->Stop();
		return true;
	}
	if (bShouldReset) {
		Processor->GetKernel()->Reset();
		return true;
	}
	return false;
}

void FFINLuaProcessorTick::tickHook(lua_State* L) {
	switch (State) {
	case LUA_SYNC:
	case LUA_ASYNC:
		State |= LUA_ERROR;
		lua_sethook(Processor->luaThread, UFINLuaProcessor::luaHook, LUA_MASKCOUNT, steps());
		break;
	case LUA_SYNC_ERROR:
	case LUA_ASYNC_ERROR:
		State |= LUA_END;
		lua_sethook(Processor->luaThread, UFINLuaProcessor::luaHook, LUA_MASKCOUNT, steps());
		luaL_error(L, "out of time");
		break;
	case LUA_SYNC_END:
    case LUA_ASYNC_END:
        throw FFINKernelCrash("out of time");
	default: ;
	}
}

int luaAPIReturn_Resume(lua_State* L, int status, lua_KContext ctx) {
	return static_cast<int>(ctx);
}

int FFINLuaProcessorTick::apiReturn(lua_State* L, int args) {
	if (State != LUA_SYNC && State != LUA_ASYNC) { // tick state in error or crash
		if (State & LUA_SYNC) State = LUA_SYNC;
		else if (State & LUA_ASYNC) State = LUA_ASYNC;
		return lua_yieldk(L, 0, args, &luaAPIReturn_Resume);
	}
	return args;
}

int FFINLuaProcessorTick::steps() const {
	switch (State) {
	case LUA_SYNC:
		return SyncLen;
	case LUA_SYNC_ERROR:
		return SyncErrorLen;
	case LUA_SYNC_END:
		return SyncEndLen;
	case LUA_ASYNC_BEGIN:
		return AsyncLen;
	case LUA_ASYNC:
		return AsyncLen;
	case LUA_ASYNC_ERROR:
		return AsyncErrorLen;
	case LUA_ASYNC_END:
		return AsyncEndLen;
	default:
		return LUA_SYNC;
	}
}

UFINLuaProcessor* UFINLuaProcessor::luaGetProcessor(lua_State* L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "LuaProcessorPtr");
	UFINLuaProcessor* p = *static_cast<UFINLuaProcessor**>(luaL_checkudata(L, -1, "LuaProcessor"));
	lua_pop(L, 1);
	return p;
}

UFINLuaProcessor::UFINLuaProcessor() : tickHelper(this), FileSystemListener(new LuaFileSystemListener(this)) {
	
}

UFINLuaProcessor::~UFINLuaProcessor() {}

int luaPersist(lua_State* L) {
	UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Lua Processor Persist"), *p->DebugInfo);
	
	// perm, data
	
	// persist data table
	eris_persist(L, 1, 2); // perm, data, data-str
	
	return 1;
}

void UFINLuaProcessor::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Lua Processor %s"), *DebugInfo, TEXT("PreSerialize"));
	if (!Kernel || Kernel->GetState() != FIN_KERNEL_RUNNING) return;
	
	tickHelper.stop();
	StateStorage.Clear();

	for (FicsItKernel::Lua::LuaFile file : FileStreams) {
		if (file->file) {
			file->transfer = CodersFileSystem::SRef<FicsItKernel::Lua::LuaFilePersistTransfer>(new FicsItKernel::Lua::LuaFilePersistTransfer());
			file->transfer->open = file->file->isOpen();
			if (file->transfer->open) {
				file->transfer->mode = file->file->getMode();
				file->transfer->pos = file->file->seek("cur", 0);
			}
			// TODO: Check if combination of APP & TRUNC need special data transfer &
			// TODO: like reading the file store it and then rewrite it to the file when unpersisting.
			file->file->close();
		} else file->transfer = nullptr;
	}

	UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Lua Processor %s"), *DebugInfo, TEXT("'Serialized'"));

	// check state & thread
	if (luaState && luaThread && lua_status(luaThread) == LUA_YIELD) {
		// prepare state data
		lua_getfield(luaState, LUA_REGISTRYINDEX, "PersistPerm");	// ..., perm
		lua_geti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);	// ..., perm, globals
		lua_pushvalue(luaState, -1);									// ..., perm, globals, globals
		lua_pushnil(luaState);											// ..., perm, globals, globals, nil
		lua_settable(luaState, -4);									// ..., perm, globals
		lua_pushvalue(luaState, -2);									// ..., perm, globals, perm
		lua_newtable(luaState);										// ..., perm, globals, perm, data
		lua_pushvalue(luaState, -3);									// ..., perm, globals, perm, data, globals
		lua_setfield(luaState, -2, "globals");						// ..., perm, globals, perm, data
		lua_pushvalue(luaState, luaThreadIndex);						// ..., perm, globals, perm, data, thread
		lua_setfield(luaState, -2, "thread");						// ..., perm, globals, perm, data
		

		lua_pushcfunction(luaState, luaPersist);					// ..., perm, globals, perm, data, persist-func
		lua_insert(luaState, -3);									// ..., perm, globals, persist-func, perm, data
		const int status = lua_pcall(luaState, 2, 1, 0);			// ..., perm, globals, data-str

		// check unpersist
		if (status == LUA_OK) {
			// encode persisted data
			size_t data_l = 0;
			const char* data_r = lua_tolstring(luaState, -1, &data_l);
			TArray<uint8> data((uint8*)data_r, data_l);
			StateStorage.LuaData = FBase64::Encode(data);
	
			lua_pop(luaState, 1); // ..., perm, globals
		} else {
			// print error
			if (lua_isstring(luaState, -1)) {
				UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Unable to persit! '%s'"), *DebugInfo, *FString(lua_tostring(luaState, -1)));
			}

			lua_pop(luaState, 1); // ..., perm, globals
		}

		// cleanup
		lua_pushnil(luaState); // ..., perm, globals, nil
		lua_settable(luaState, -3); // ..., perm
		lua_pop(luaState, 1); // ...
		lua_pushnil(luaState); // ..., nil
		lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistTraces"); // ...
	}
}

void UFINLuaProcessor::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
}

void UFINLuaProcessor::BeginDestroy() {
	Super::BeginDestroy();
	tickHelper.stop();
}

void UFINLuaProcessor::PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {}

void UFINLuaProcessor::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {}

int luaUnpersist(lua_State* L) {
	UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Lua Processor Unpersist"), *p->DebugInfo);
	
	// data-str, uperm
	
	// unpersist data
	eris_unpersist(L, 2, 1); // data-str, uperm, data
	
	return 1;
}

void UFINLuaProcessor::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Lua Processor %s"), *DebugInfo, TEXT("PostDeserialize"));
	if (!Kernel || Kernel->GetState() != FIN_KERNEL_RUNNING) return;

	Reset();

	// decode & check data from json
	TArray<uint8> data;
	FBase64::Decode(StateStorage.LuaData, data);
	if (data.Num() <= 1) return;

	// get uperm table
	lua_getfield(luaState, LUA_REGISTRYINDEX, "PersistUperm");			// ..., uperm

	// prepare protected unpersist
	lua_pushcfunction(luaState, luaUnpersist);							// ..., uperm, unpersist-func

	// push data for protected unpersist
	lua_pushlstring(luaState, (char*)data.GetData(), data.Num());				// ..., uperm, unpersist-func, data-str
	lua_pushvalue(luaState, -3);											// ..., uperm, unpersist-func, data-str, uperm

	// do unpersist
	const int ok = lua_pcall(luaState, 2, 1, 0); // ...,  uperm, data

	// check unpersist
	if (ok != LUA_OK) {
		// print error
		if (lua_isstring(luaState, -1)) {
			UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Unable to unpersit! '%s'"), *DebugInfo, *FString(lua_tostring(luaState, -1)));
		}
		
		// cleanup
		lua_pushnil(luaState); // ..., uperm, err, nil
		lua_setfield(luaState, -3, "Globals"); // ..., uperm, err
		lua_pop(luaState, 1); // ...
		lua_pushnil(luaState); // ..., nil
		lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistTraces"); // ...
	
		//throw std::exception("Unable to unpersist");
	} else {
		// cleanup
		lua_pushnil(luaState); // ..., uperm, data, nil
		lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistTraces"); // ..., uperm, data
	
		// place persisted data
		lua_getfield(luaState, -1, "thread");
		lua_replace(luaState, luaThreadIndex); // ..., uperm, data
		lua_getfield(luaState, -1, "globals");
		lua_seti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS); // ..., uperm, data
		luaThread = lua_tothread(luaState, luaThreadIndex);
		lua_pop(luaState, 2); // ...
	}
}

void UFINLuaProcessor::SetKernel(UFINKernelSystem* InKernel) {
	if (GetKernel() && GetKernel()->GetFileSystem()) GetKernel()->GetFileSystem()->removeListener(FileSystemListener);
	Kernel = InKernel;
}

void UFINLuaProcessor::Tick(float InDelta) {
	if (!luaState || !luaThread) return;

	tickHelper.syncTick();
}

void UFINLuaProcessor::Stop(bool bIsCrash) {
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Lua Processor stop %s"), *DebugInfo, bIsCrash ? TEXT("due to crash") : TEXT(""));
	tickHelper.stop();
}

void UFINLuaProcessor::LuaTick() {
	try {
		// reset out of time
		lua_sethook(luaThread, UFINLuaProcessor::luaHook, LUA_MASKCOUNT, tickHelper.steps());
		
		int nres = -1;
		int Status;
		if (PullState != 0) {
			// Runtime is pulling a signal
			if (GetKernel() && GetKernel()->GetNetwork() && GetKernel()->GetNetwork()->GetSignalCount() > 0) {
				// Signal available -> reset timeout and pull signal from network
				PullState = 0;
				GetTickHelper().signalFound();
				const int SigArgCount = DoSignal(luaThread);
				if (SigArgCount < 1) {
					// no signals popped -> crash system
					Status = LUA_ERRRUN;
				} else {
					// signal popped -> resume yield with signal as parameters (passing signals parameters back to pull yield)
					Status = lua_resume(luaThread, luaState, SigArgCount, &nres);
				}
			} else if (PullState == 2 || !PullTimeoutReached()) {
				// no signal available & not timeout reached -> skip tick
				return;
			} else {
				// no signal available & timeout reached -> resume yield with  no parameters
				PullState = 0;
				GetTickHelper().signalFound();
				Status = lua_resume(luaThread, luaState, 0, &nres);
			}
		} else {
			// resume runtime normally
			Status = lua_resume(luaThread, luaState, 0, &nres);
		}
		if (Status == LUA_YIELD) {
			// system yielded and waits for next tick
			lua_gc(luaState, LUA_GCCOLLECT, 0);
			if (GetKernel()) GetKernel()->RecalculateResources(UFINKernelSystem::PROCESSOR);
		} else if (Status == LUA_OK) {
			// runtime finished execution -> stop system normally
			tickHelper.shouldStop();
		} else {
			// runtime crashed -> crash system with runtime error message
			luaL_traceback(luaThread, luaThread, lua_tostring(luaThread, -1), 0);
			tickHelper.shouldCrash(MakeShared<FFINKernelCrash>(UTF8_TO_TCHAR(lua_tostring(luaThread, -1))));
		}
		if (nres > -1) {
			lua_pop(luaThread, nres);
		}
	} catch (...) {
		// fatal end of time reached
		tickHelper.shouldCrash(MakeShared<FFINKernelCrash>("out of time"));
	}

	// clear some data
	ClearFileStreams();
}

size_t luaLen(lua_State* L, int idx) {
	size_t len = 0;
	idx = lua_absindex(L, idx);
	lua_pushnil(L);
	while (lua_next(L, idx) != 0) {
		len++;
		lua_pop(L, 1);
	}
	return len;
}

void UFINLuaProcessor::ClearFileStreams() {
	TSet<CodersFileSystem::SRef<FicsItKernel::Lua::LuaFileContainer>> ToRemove;
	for (const CodersFileSystem::SRef<FicsItKernel::Lua::LuaFileContainer>& fs : FileStreams) {
		if (!fs.isValid()) ToRemove.Add(fs);
	}
	for (const CodersFileSystem::SRef<FicsItKernel::Lua::LuaFileContainer>& fs : ToRemove) {
		FileStreams.Remove(fs);
	}
}

TSet<FicsItKernel::Lua::LuaFile> UFINLuaProcessor::GetFileStreams() const {
	return FileStreams;
}

void UFINLuaProcessor::Reset() {
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s: Lua Processor Reset"), *DebugInfo);
	tickHelper.stop();
	
	// can't reset running system state
	if (GetKernel()->GetState() != FIN_KERNEL_RUNNING) return;

	// reset temp-data
	Timeout = -1;
	PullState = 0;
	GetKernel()->GetFileSystem()->addListener(FileSystemListener);

	// clear existing lua state
	if (luaState) {
		lua_close(luaState);
	}

	// create new lua state
	luaState = luaL_newstate();

	// setup library and perm tables for persistence
	lua_newtable(luaState); // perm
	lua_newtable(luaState); // perm, uperm 

	// register pointer to this Lua Processor in c registry, filestream list & perm table
	luaL_newmetatable(luaState, "LuaProcessor"); // perm, uperm, mt-LuaProcessor
	lua_pop(luaState, 1); // perm, uperm
	UFINLuaProcessor*& luaProcessor = *static_cast<UFINLuaProcessor**>(lua_newuserdata(luaState, sizeof(UFINLuaProcessor*))); // perm, uperm, proc
	luaL_setmetatable(luaState, "LuaProcessor");
	luaProcessor = this;
	lua_pushvalue(luaState, -1); // perm, uperm, proc, proc
	lua_setfield(luaState, LUA_REGISTRYINDEX, "LuaProcessorPtr"); // perm, uperm, proc
	lua_pushvalue(luaState, -1); // perm, uperm, proc, proc
	lua_setfield(luaState, -3, "LuaProcessor"); // perm, uperm, proc
	lua_pushstring(luaState, "LuaProcessor"); // perm, uperm, proc, "LuaProcessorPtr"
	lua_settable(luaState, -4); // perm, uperm
	LuaSetup(luaState); // perm, uperm
	lua_pushlightuserdata(luaState, &FileStreams);
	lua_setfield(luaState, LUA_REGISTRYINDEX, "FileStreamStorage");
	
	// finish perm tables
	lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistUperm"); // perm
	lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistPerm"); //
	
	// create new thread for user code chunk
	luaThread = lua_newthread(luaState);
	luaThreadIndex = lua_gettop(luaState);

	// setup thread with code
	if (!EEPROM.IsValid()) {
		Kernel->Crash(MakeShared<FFINKernelCrash>("No Valid EEPROM set"));
		return;
	}
	const FTCHARToUTF8 CodeConv(*EEPROM->GetCode(), EEPROM->GetCode().Len());
	const std::string code = std::string(CodeConv.Get(), CodeConv.Length());
	luaL_loadbuffer(luaThread, code.c_str(), code.size(), "=EEPROM");
	if (lua_isstring(luaThread, -1)) {
		// Syntax error
		Kernel->Crash(MakeShared<FFINKernelCrash>(lua_tostring(luaThread, -1)));
		return;
	}

	// reset tick state
	tickHelper.reset();

	// lua_gc(luaState, LUA_GCSETPAUSE, 100);
	// TODO: Check if we actually want to use this or the manual gc call
}

int64 UFINLuaProcessor::GetMemoryUsage(bool bInRecalc) {
	return lua_gc(luaState, LUA_GCCOUNT, 0) * 100;
}

void UFINLuaProcessor::SetEEPROM(AFINStateEEPROM* InEEPROM) {
	EEPROM = Cast<AFINStateEEPROMLua>(InEEPROM);
}

AFINStateEEPROMLua* UFINLuaProcessor::GetEEPROM() const {
	return EEPROM.Get();
}

FFINLuaProcessorTick& UFINLuaProcessor::GetTickHelper() {
	return tickHelper;
}

bool UFINLuaProcessor::PullTimeoutReached() {
	return Timeout <= (static_cast<double>((FDateTime::Now() - FFicsItNetworksModule::GameStart).GetTotalMilliseconds() - PullStart) / 1000.0);
}

int luaReYield(lua_State* L) {
	lua_yield(L,0);
	return 0;
}

int luaPrint(lua_State* L) {
	// ReSharper disable once CppDeclaratorNeverUsed
	FLuaSyncCall SyncCall(L);
	const int args = lua_gettop(L);
	std::string log;
	for (int i = 1; i <= args; ++i) {
		size_t s_len = 0;
		const char* s = luaL_tolstring(L, i, &s_len);
		if (!s) luaL_argerror(L, i, "is not valid type");
		log += std::string(s, s_len) + " ";
	}
	if (log.length() > 0) log = log.erase(log.length()-1);
	
	try {
		UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
		CodersFileSystem::SRef<CodersFileSystem::FileStream> serial = Processor->GetKernel()->GetDevDevice()->getSerial()->open(CodersFileSystem::OUTPUT);
		if (serial) {
			*serial << log << "\r\n";
			serial->close();
		}
	} catch (std::exception ex) {
		luaL_error(L, ex.what());
	}
	
	return UFINLuaProcessor::luaAPIReturn(L, 0);
}

int luaYieldResume(lua_State* L, int status, lua_KContext ctx) {
	// don't pass pushed bool for user executed yield identification
	return UFINLuaProcessor::luaAPIReturn(L, lua_gettop(L)-1);
}

int luaYield(lua_State* L) {
	const int args = lua_gettop(L);

	// insert a boolean to indicate a user executed yield
	lua_pushboolean(L, true);
	lua_insert(L, 1);
	
	return lua_yieldk(L, args+1, NULL, &luaYieldResume);
}

int luaResume(lua_State* L); // pre-declare

int luaResumeResume(lua_State* L, int status, lua_KContext ctx) {
	return UFINLuaProcessor::luaAPIReturn(L, luaResume(L));
}

int luaResume(lua_State* L) {
	const int args = lua_gettop(L);
	if (!lua_isthread(L, 1)) return luaL_argerror(L, 1, "is no thread");
	lua_State* thread = lua_tothread(L, 1);

	if (!lua_checkstack(thread, args - 1)) {
		lua_pushboolean(L, false);
		lua_pushliteral(L, "too many arguments to resume");
		return -1;
	}

	// attach hook for out-of-time exception if thread got loaded from save and hook is not applied
	lua_sethook(thread, UFINLuaProcessor::luaHook, LUA_MASKCOUNT, UFINLuaProcessor::luaGetProcessor(L)->GetTickHelper().steps());

	// copy passed arguments to coroutine so it can return these arguments from the yield function
	// but don't move the passed coroutine and then resume the coroutine
	lua_xmove(L, thread, args - 1);
	int argCount = 0;
	const int status = lua_resume(thread, L, args - 1, &argCount);

	if (status == LUA_OK || status == LUA_YIELD) {
		if (argCount == 0) {
			// A hook yielded the thread
			return lua_yieldk(L, 0, NULL, &luaResumeResume);
		}

		if (!lua_checkstack(L, argCount + 1)) {
			lua_pop(thread, argCount);
			lua_pushliteral(L, "too many results to resume");
			return -1;
		}
		lua_xmove(thread, L, argCount-1);
		return argCount-1;
	} else {
		lua_xmove(thread, L, 1);
		return -1;
	}
}

int luaRunning(lua_State* L) {
	UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
	int ismain = lua_pushthread(L);
	lua_State* thread = lua_tothread(L, -1);
	if (thread == p->GetLuaThread()) ismain = 1;
	lua_pushboolean(L, ismain);
	return 2;
}

int luaXpcallMsg(lua_State* L) {
	lua_newtable(L);
	lua_insert(L, -2);
	lua_setfield(L, -2, "message");
	luaL_traceback(L, L, NULL, 0);
	lua_setfield(L, -2, "trace");
	return 1;
}

int luaXpcallFinish(lua_State *L, int status, lua_KContext extra) {
	if (status != LUA_OK && status != LUA_YIELD) {
		lua_pushboolean(L, 0);
		lua_pushvalue(L, -2);
		return 2;
	}
	lua_pushboolean(L, 1);
	lua_rotate(L, 1 + extra, 1);
	return lua_gettop(L) - extra;
}

// xpcall-reimplementation adding traceback to error object
int luaXpcall(lua_State *L) {
	int n = lua_gettop(L);
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_pushcfunction(L, &luaXpcallMsg);
	lua_rotate(L, 1, 1);
	int status = lua_pcallk(L, n - 1, LUA_MULTRET, 1, 0, luaXpcallFinish);
	return luaXpcallFinish(L, status, 1);
}

// Theoratical better xpcall-reimplementation for FIN but
// doesn't work because not easy to interact with non LUA_OK or LUA_YIELD coroutines
/*
int luaXpcallMsgContinue(lua_State* L2, int args, lua_KContext ctx) {
	lua_State* L = (lua_State*)ctx;
	lua_pushboolean(L, 0);
	return args+1;
}

int luaXpcallContinue(lua_State* L, int args, lua_KContext ctx) {
	lua_State* L2 = (lua_State*)ctx;
	int res;
	int status = lua_resume(L2, L, 0, &res);
	if (status == LUA_YIELD) {
		return lua_yieldk(L, 0, ctx, &luaXpcallContinue);
	}
	if (status > LUA_YIELD) {
		lua_rotate(L2, 1, -1);
		lua_rotate(L2, -2, -1);
		lua_callk(L2, 1, 1, (lua_KContext)L, &luaXpcallMsgContinue);
	}
	lua_pushboolean(L, 1);
	return res+1;
}
#pragma optimize("", off)
int luaXpcall(lua_State* L) {
	int n = lua_gettop(L);
	luaL_checktype(L, 1, LUA_TFUNCTION); // check actual function
	luaL_checktype(L, 2, LUA_TFUNCTION); // check msg function
	lua_State* L2 = lua_newthread(L);
	lua_rotate(L, 1, -1);
	lua_xmove(L, L2, n - 1);
	int L2n = lua_gettop(L2);
	int res;
	int status = lua_resume(L2, L, n - 2, &res);
	if (status == LUA_YIELD) {
		return lua_yieldk(L, 0, (lua_KContext)L2, &luaXpcallContinue);
	}
	if (status > LUA_YIELD) {
		luaL_checktype(L2, L2n, LUA_TFUNCTION);
		lua_rotate(L2, L2n, -1);
		lua_rotate(L2, -2, -1);
		luaL_checktype(L2, -2, LUA_TFUNCTION);
		lua_pcallk(L2, 1, 1, 0, (lua_KContext)L, &luaXpcallMsgContinue);
	}
	lua_pushboolean(L, 1);
	return res+1;
}
#pragma optimize("", on)
*/

void UFINLuaProcessor::LuaSetup(lua_State* L) {
	PersistSetup("LuaProcessor", -2);

	luaL_requiref(L, "_G", luaopen_base, true);
	lua_pushnil(L);
	lua_setfield(L, -2, "collectgarbage");
	lua_pushnil(L);
	lua_setfield(L, -2, "dofile");
	lua_pushnil(L);
	lua_setfield(L, -2, "loadfile");
	lua_pushcfunction(L, luaXpcall);
	lua_setfield(L, -2, "xpcall");
	lua_pushcfunction(L, luaPrint);
	lua_setfield(L, -2, "print");
	PersistTable("global", -1);
	lua_pop(L, 1);
	
	luaL_requiref(L, "table", luaopen_table, true);
	PersistTable("table", -1);
	lua_pop(L, 1);

	luaL_requiref(L, "coroutine", luaopen_coroutine, true);
	lua_pushcfunction(L, luaResume);
	lua_setfield(L, -2, "resume");
	lua_pushcfunction(L, luaYield);
	lua_setfield(L, -2, "yield");
	lua_pushcfunction(L, luaRunning);
	lua_setfield(L, -2, "running");
	PersistTable("coroutine", -1);
	lua_pop(L, 1);

	lua_pushcfunction(L, (int(*)(lua_State*))luaYieldResume);
	PersistValue("coroutineYieldContinue");
	lua_pushcfunction(L, (int(*)(lua_State*))luaResumeResume);
	PersistValue("coroutineResumeContinue");
	
	luaL_requiref(L, "math", luaopen_math, true);
	PersistTable("math", -1);
	lua_pop(L, 1);

	luaL_requiref(L, "string", luaopen_string, true);
	PersistTable("string", -1);
	lua_pop(L, 1);

	FicsItKernel::Lua::setupRefUtils(L);
	FicsItKernel::Lua::setupInstanceSystem(L);
	FicsItKernel::Lua::setupStructSystem(L);
	FicsItKernel::Lua::setupComponentAPI(L);
	FicsItKernel::Lua::setupEventAPI(L);
	FicsItKernel::Lua::setupFileSystemAPI(L);
	FicsItKernel::Lua::setupComputerAPI(L);
	FicsItKernel::Lua::setupDebugAPI(L);
	FicsItKernel::Lua::setupFutureAPI(L);
}

int UFINLuaProcessor::DoSignal(lua_State* L) {
	UFINKernelNetworkController* net = GetKernel()->GetNetwork();
	if (!net || net->GetSignalCount() < 1) return 0;
	FFINNetworkTrace sender;
	FFINSignalData signal = net->PopSignal(sender); 
	int props = 2;
	if (signal.Signal) lua_pushstring(L, TCHAR_TO_UTF8(*signal.Signal->GetInternalName()));
	else lua_pushnil(L);
	FicsItKernel::Lua::newInstance(L, UFINNetworkUtils::RedirectIfPossible(sender));
	for (const FFINAnyNetworkValue& Value : signal.Data) {
		FicsItKernel::Lua::networkValueToLua(L, Value, sender);
		props++;
	}
	return props;
}

void UFINLuaProcessor::luaHook(lua_State* L, lua_Debug* ar) {
	//UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
	//p->tickHelper.tickHook(L);
	lua_yield(L, 0);
}

int UFINLuaProcessor::luaAPIReturn(lua_State* L, int args) {
	//UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
	//return p->tickHelper.apiReturn(L, args);
	return args;
}

lua_State* UFINLuaProcessor::GetLuaState() const {
	return luaState;
}

lua_State* UFINLuaProcessor::GetLuaThread() const {
	return luaThread;
}
