#include "FINLuaProcessor.h"

#include "AsyncWork.h"
#include "Base64.h"
#include "FGInventoryComponent.h"
#include "FicsItNetworksComputer.h"
#include "FicsItNetworksLuaModule.h"
#include "FILLogContainer.h"
#include "FINComputerEEPROMDesc.h"
#include "FINNetworkUtils.h"
#include "FINItemStateEEPROMText.h"
#include "FINMediaSubsystem.h"
#include "FINLua/LuaExtraSpace.h"
#include "FINLua/LuaGlobalLib.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FIRTrace.h"
#include "Engine/Engine.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/LuaUtil.h"
#include "Signals/FINSignalData.h"
#include "tracy/Tracy.hpp"

void LuaFileSystemListener::onUnmounted(CodersFileSystem::Path path, TSharedRef<CodersFileSystem::Device> device) {
	for (FINLua::LuaFile file : Parent->GetFileStreams()) {
		if (!Parent->GetKernel()->GetFileSystem()) {
			file->file->close();
		}
	}
}

void LuaFileSystemListener::onNodeRemoved(CodersFileSystem::Path path, CodersFileSystem::NodeType type) {
	for (FINLua::LuaFile file : Parent->GetFileStreams()) {
		if (file->path.length() > 0 && (!Parent->GetKernel()->GetFileSystem())) {
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
		{
			ZoneScoped;
			Processor->LuaTick();
		}
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
		{
			ZoneScoped;
			Processor->LuaTick();
		}
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
	return FINLua::luaFIN_getExtraSpace(L).Processor;
}

void UFINLuaProcessor::OnPreGarbageCollection() {
	bWasPriorToGCPromoted = (bool)(tickHelper.getState() & LUA_ASYNC);
	if (bWasPriorToGCPromoted) {
		tickHelper.demote();
	}
}

void UFINLuaProcessor::OnPostGarbageCollection() {
	if (bWasPriorToGCPromoted) {
		tickHelper.promote();
	}
}


UFINLuaProcessor::UFINLuaProcessor() : tickHelper(this), FileSystemListener(new LuaFileSystemListener(this)) {
	OnPreGarbageCollectionHandle = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddUObject(this, &UFINLuaProcessor::OnPreGarbageCollection);
	OnPostGarbageCollectionHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &UFINLuaProcessor::OnPostGarbageCollection);
}

UFINLuaProcessor::~UFINLuaProcessor() {
	tickHelper.demote();
	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(OnPreGarbageCollectionHandle);
	FCoreUObjectDelegates::GetPostGarbageCollect().Remove(OnPostGarbageCollectionHandle);
}

int luaPersist(lua_State* L) {
	UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor Persist"), *p->DebugInfo);
	
	// perm, data
	
	// persist data table
	eris_persist(L, 1, 2); // perm, data, data-str
	
	return 1;
}

void UFINLuaProcessor::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor %s"), *DebugInfo, TEXT("PreSerialize"));
	if (!Kernel || Kernel->GetState() != FIN_KERNEL_RUNNING) return;
	
	tickHelper.stop();
	StateStorage.Clear();

	for (FINLua::LuaFile file : FileStreams) {
		if (file->file) {
			file->transfer = MakeShared<FINLua::LuaFilePersistTransfer>();
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

	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor %s"), *DebugInfo, TEXT("'Serialized'"));

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
				UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Unable to persit! '%s'"), *DebugInfo, *FINLua::luaFIN_toFString(luaState, -1));
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
	GEngine->ForceGarbageCollection(true);
}

void UFINLuaProcessor::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(Kernel);
	out_dependentObjects.Add(AFINMediaSubsystem::GetMediaSubsystem(this));
}

void UFINLuaProcessor::PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {}

void UFINLuaProcessor::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {}

int luaUnpersist(lua_State* L) {
	UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor Unpersist"), *p->DebugInfo);
	
	// data-str, uperm
	
	// unpersist data
	eris_unpersist(L, 2, 1); // data-str, uperm, data
	
	return 1;
}

void UFINLuaProcessor::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor %s"), *DebugInfo, TEXT("PostDeserialize"));
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
			UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Unable to unpersit! '%s'"), *DebugInfo, *FINLua::luaFIN_toFString(luaState, -1));
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

//#include "FGPlayerController.h"
//#include "UI/FGGameUI.h"
//#include "UI/FINNotificationMessage.h"
void UFINLuaProcessor::Tick(float InDelta) {
	if (!luaState || !luaThread) return;

	tickHelper.syncTick();

	//AFGPlayerController* PlayerController = GetWorld()->GetFirstPlayerController<AFGPlayerController>();
	//UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Pending Messages? %i"), PlayerController->GetGameUI()->CanReceiveMessageQueue());
	//UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Pending Message? %i"), PlayerController->GetGameUI()->CanReceiveMessage(UFINNotificationMessage::StaticClass()));
}

void UFINLuaProcessor::Stop(bool bIsCrash) {
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor stop %s"), *DebugInfo, bIsCrash ? TEXT("due to crash") : TEXT(""));
	tickHelper.stop();
}

void UFINLuaProcessor::LuaTick() {
	ZoneScoped;
	FFILLogScope LogScope(GetKernel()->GetLog());
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
			if (GetKernel()) {
				TSharedPtr<FFINKernelCrash> Crash = GetKernel()->RecalculateResources(UFINKernelSystem::PROCESSOR);
				if (Crash) {
					tickHelper.shouldCrash(Crash.ToSharedRef());
				}
			}
		} else if (Status == LUA_OK) {
			// runtime finished execution -> stop system normally
			tickHelper.shouldStop();
		} else {
			// runtime crashed -> crash system with runtime error message
			const char* message = lua_tostring(luaThread, -1);
			try {
				luaL_traceback(luaThread, luaThread, message, 0);
				tickHelper.shouldCrash(MakeShared<FFINKernelCrash>(FINLua::luaFIN_toFString(luaThread, -1)));
			} catch (FString error) {
				tickHelper.shouldCrash(MakeShared<FFINKernelCrash>(UTF8_TO_TCHAR(message)));
			}
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
	TArray<TSharedRef<FINLua::LuaFileContainer>> ToRemove;
	for (const TSharedRef<FINLua::LuaFileContainer>& fs : FileStreams) {
		ToRemove.Add(fs);
	}
	for (const TSharedRef<FINLua::LuaFileContainer>& fs : ToRemove) {
		FileStreams.Remove(fs);
	}
}

TArray<FINLua::LuaFile> UFINLuaProcessor::GetFileStreams() const {
	return FileStreams;
}

void luaWarnF(void* ud, const char* msg, int tocont) {
	UFINLuaProcessor* Processor = static_cast<UFINLuaProcessor*>(ud);

	Processor->GetKernel()->GetLog()->PushLogEntry(FIL_Verbosity_Warning, UTF8_TO_TCHAR(msg));
}

int luaPanicF(lua_State* L) {
	throw FINLua::luaFIN_toFString(L, 1);
	return 0;
}

void UFINLuaProcessor::Reset() {
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor Reset"), *DebugInfo);
	tickHelper.stop();
	
	// can't reset running system state
	if (GetKernel()->GetState() != FIN_KERNEL_RUNNING) return;

	// reset temp-data
	Timeout = -1;
	PullState = 0;
	GetKernel()->GetFileSystem()->addListener(FileSystemListener);

	// clear existing lua state
	if (luaState) {
		FINLua::luaFIN_destroyExtraSpace(luaState);

		lua_close(luaState);
	}

	// create new lua state
	luaState = luaL_newstate();

	FINLua::luaFIN_createExtraSpace(luaState, {
		.Processor = this,
		.FileStreams = FileStreams,
	});

	// setup error/warning function
	lua_setwarnf(luaState, luaWarnF, this);
	lua_atpanic(luaState, luaPanicF);

	// setup tables for persistence
	lua_newtable(luaState); // perm
	lua_newtable(luaState); // perm, uperm
	lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistUperm"); // perm
	lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistPerm"); //

	FINLua::setupGlobals(luaState);

	// create new thread for user code chunk
	luaThread = lua_newthread(luaState);
	luaThreadIndex = lua_gettop(luaState);

	// setup thread with code
	TOptional<FString> eepromCode = GetEEPROM();
	if (eepromCode.IsSet() == false) {
		Kernel->Crash(MakeShared<FFINKernelCrash>("No Valid EEPROM set"));
		return;
	}
	const FTCHARToUTF8 CodeConv(**eepromCode, eepromCode->Len());
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

TOptional<FString> UFINLuaProcessor::GetEEPROM() const {
	FInventoryItem eeprom = Kernel->GetEEPROM();
	if (const FFINItemStateEEPROMText* state = eeprom.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		return state->Code;
	}
	return {};
}

bool UFINLuaProcessor::SetEEPROM(const FString& Code) {
	FInventoryItem eeprom = Kernel->GetEEPROM();
	UFINComputerEEPROMDesc::CreateEEPROMStateInItem(eeprom);

	if (const FFINItemStateEEPROMText* stateLua = eeprom.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		FFINItemStateEEPROMText state = *stateLua;
		state.Code = Code;
		return Kernel->SetEEPROM(FFGDynamicStruct(state));
	}

	return false;
}

FFINLuaProcessorTick& UFINLuaProcessor::GetTickHelper() {
	return tickHelper;
}

bool UFINLuaProcessor::PullTimeoutReached() {
	return Timeout <= (static_cast<double>((FDateTime::Now() - FFicsItNetworksComputerModule::GameStart).GetTotalMilliseconds() - PullStart) / 1000.0);
}

int UFINLuaProcessor::DoSignal(lua_State* L) {
	UFINKernelNetworkController* net = GetKernel()->GetNetwork();
	if (!net || net->GetSignalCount() < 1) return 0;
	FFIRTrace sender;
	FFINSignalData signal = net->PopSignal(sender); 
	int props = 2;
	if (signal.Signal) lua_pushstring(L, TCHAR_TO_UTF8(*signal.Signal->GetInternalName()));
	else lua_pushnil(L);
	FINLua::luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(sender));
	for (const FFIRAnyValue& Value : signal.Data) {
		FINLua::luaFIN_pushNetworkValue(L, Value, sender);
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
