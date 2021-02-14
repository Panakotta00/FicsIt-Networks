#include "LuaProcessor.h"

#include <chrono>

#include "../../FicsItKernel.h"

#include "LuaInstance.h"
#include "LuaStructs.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "LuaFileSystemAPI.h"
#include "LuaComputerAPI.h"
#include "LuaProcessorStateStorage.h"

#include "FINStateEEPROMLua.h"
#include "LuaDebugAPI.h"
#include "LuaFuture.h"
#include "LuaRef.h"
#include "Network/FINNetworkComponent.h"
#include "Network/FINNetworkTrace.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINSignal.h"

namespace FicsItKernel {
	namespace Lua {
		void LuaFileSystemListener::onUnmounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) {
			for (LuaFile file : parent->getFileStreams()) {
				if (file.isValid() && (!parent->getKernel()->getFileSystem() || !parent->getKernel()->getFileSystem()->checkUnpersistPath(file->path))) {
					file->file->close();
				}
			}
		}

		void LuaFileSystemListener::onNodeRemoved(FileSystem::Path path, FileSystem::NodeType type) {
			for (LuaFile file : parent->getFileStreams()) {
				if (file.isValid() && file->path.length() > 0 && (!parent->getKernel()->getFileSystem() || parent->getKernel()->getFileSystem()->unpersistPath(file->path) == path)) {
					file->file->close();
				}
			}
		}

		void FLuaTickRunnable::DoWork() {
			while (true) {
				if (!Tick->asyncTick()) {
					break;
				}
			}
		}

		LuaProcessorTick::LuaProcessorTick(LuaProcessor* Processor): Processor(Processor), Runnable(this) {
			reset();
		}

		LuaProcessorTick::~LuaProcessorTick() {
			stop();
		}

		void LuaProcessorTick::reset() {
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

		void LuaProcessorTick::stop() {
			demote();
			
			if (asyncTask.IsValid() && !asyncTask->IsIdle()) {
				asyncTask->EnsureCompletion();
				asyncTask->Cancel();
				asyncTask.Reset();
			}
		}

		void LuaProcessorTick::promote() {
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

		void LuaProcessorTick::demote() {
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

		void LuaProcessorTick::demoteInAsync() {
			if (State & LUA_SYNC) return;
			AsyncSyncMutex.Lock();
			AsyncSync = TPromise<void>();
			SyncAsync = TPromise<void>();
			TFuture<void> Future = AsyncSync.GetFuture();
			bDoSync = true;
			AsyncSyncMutex.Unlock();
			TickMutex.Unlock();
			Future.Wait(); // wait for sync to continue this async
			TickMutex.Lock();
		}

		void LuaProcessorTick::shouldStop() {
			bShouldStop = true;
		}
		
		void LuaProcessorTick::shouldReset() {
			bShouldReset = true;
		}

		void LuaProcessorTick::shouldPromote() {
			if (bShouldStop || bShouldCrash || bShouldReset) return;
			bShouldPromote = true;
		}

		void LuaProcessorTick::shouldDemote() {
			bShouldDemote = true;
		}

		void LuaProcessorTick::shouldCrash(const KernelCrash& Crash) {
			bShouldCrash = true;
			ToCrash = Crash;
		}

#pragma optimize("", off)
		void LuaProcessorTick::syncTick() {
			if (postTick()) return;
			if (State & LUA_SYNC) {
				if (asyncTask.IsValid() && !asyncTask->IsIdle()) {
					asyncTask->EnsureCompletion();
					asyncTask->Cancel();
					asyncTask.Reset();
				}
				TickMutex.Lock();
				Processor->luaTick();
				TickMutex.Unlock();
				if (bShouldPromote) {
					promote();
				}
			} else if (State & LUA_ASYNC) {
				AsyncSyncMutex.Lock();
				bool DoSync = bDoSync;
				AsyncSyncMutex.Unlock();
				if (DoSync) {
					// async tick is waiting for sync
					AsyncSyncMutex.Lock();
					SyncAsync = TPromise<void>();
					TFuture<void> Future = SyncAsync.GetFuture();
					shouldDemote();
					State = LUA_SYNC;
					AsyncSync.EmplaceValue(); // continue async tick in sync with factory tick
					AsyncSyncMutex.Unlock();
					Future.Wait(); // wait for async tick to finish
					AsyncSyncMutex.Lock();
					bDoSync = false;
					AsyncSyncMutex.Unlock();
				} else {
					if (asyncTask->IsDone()) {
						asyncTask->StartBackgroundTask();
					}
				}
			}
			if (postTick()) return;
		}
		
		bool LuaProcessorTick::asyncTick() {
			if (State & LUA_ASYNC) {
				TickMutex.Lock();
				Processor->luaTick();
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
				return true;
			}
			return false;
		}

		bool LuaProcessorTick::postTick() {
			if (bShouldCrash) {
				Processor->getKernel()->crash(ToCrash);
				return true;
			}
			if (bShouldStop) {
				Processor->getKernel()->stop();
				return true;
			}
			if (bShouldReset) {
				Processor->getKernel()->reset();
				return true;
			}
			return false;
		}

		void LuaProcessorTick::tickHook(lua_State* L) {
			switch (State) {
			case LUA_SYNC:
			case LUA_ASYNC:
				State |= LUA_ERROR;
				lua_sethook(Processor->luaThread, LuaProcessor::luaHook, LUA_MASKCOUNT, steps());
				break;
			case LUA_SYNC_ERROR:
			case LUA_ASYNC_ERROR:
				State |= LUA_END;
				lua_sethook(Processor->luaThread, LuaProcessor::luaHook, LUA_MASKCOUNT, steps());
				luaL_error(L, "out of time");
				break;
			case LUA_SYNC_END:
            case LUA_ASYNC_END:
                throw KernelCrash("out of time");
			default: ;
			}
		}
#pragma optimize("", on)

		int luaAPIReturn_Resume(lua_State* L, int status, lua_KContext ctx) {
			return static_cast<int>(ctx);
		}

#pragma optimize("", off)
		int LuaProcessorTick::apiReturn(lua_State* L, int args) {
			if (State != LUA_SYNC && State != LUA_ASYNC) { // tick state in error or crash
				if (State & LUA_SYNC) State = LUA_SYNC;
				else if (State & LUA_ASYNC) State = LUA_ASYNC;
				return lua_yieldk(L, 0, args, &luaAPIReturn_Resume);
			}
			return args;
		}
#pragma optimize("", on)

		int LuaProcessorTick::steps() {
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
				return 0;
			}
		}
		
		LuaProcessor* LuaProcessor::luaGetProcessor(lua_State* L) {
			lua_getfield(L, LUA_REGISTRYINDEX, "LuaProcessorPtr");
			LuaProcessor* p = *(LuaProcessor**) luaL_checkudata(L, -1, "LuaProcessor");
			lua_pop(L, 1);
			return p;
		}

		LuaProcessor::LuaProcessor(const FString& DebugInfo, int speed) : tickHelper(this), fileSystemListener(new LuaFileSystemListener(this)), DebugInfo(DebugInfo) {
			
		}

		LuaProcessor::~LuaProcessor() {}

		void LuaProcessor::setKernel(KernelSystem* newKernel) {
			if (getKernel() && getKernel()->getFileSystem()) getKernel()->getFileSystem()->removeListener(fileSystemListener);
			Processor::setKernel(newKernel);
		}

		void LuaProcessor::tick(float delta) {
			if (!luaState || !luaThread) return;

			tickHelper.syncTick();
		}

		void LuaProcessor::stop(bool isCrash) {
			UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Lua Processor stop %s"), *DebugInfo, isCrash ? TEXT("due to crash") : TEXT(""));
			tickHelper.stop();
		}

#pragma optimize("", off)
		void LuaProcessor::luaTick() {
			try {
				// reset out of time
				lua_sethook(luaThread, LuaProcessor::luaHook, LUA_MASKCOUNT, tickHelper.steps());
				
				int status = 0;
				if (pullState != 0) {
					// Runtime is pulling a signal
					if (getKernel() && getKernel()->getNetwork() && getKernel()->getNetwork()->getSignalCount() > 0) {
						// Signal available -> reset timout and pull signal from network
						pullState = 0;
						int sigArgs = doSignal(luaThread);
						if (sigArgs < 1) {
							// no signals poped -> crash system
							status = LUA_ERRRUN;
						} else {
							// signal poped -> resume yield with signal as parameters (passing signals parameters back to pull yield)
							status = lua_resume(luaThread, nullptr, sigArgs);
						}
					} else if (pullState == 2 || timeout > (static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - pullStart).count()) / 1000.0)) {
						// no signal available & not timeout reached -> skip tick
						return;
					} else {
						// no signal available & timout reached -> resume yield with  no parameters
						pullState = 0;
						status = lua_resume(luaThread, nullptr, 0);
					}
				} else {
					// resume runtime normally
					status = lua_resume(luaThread, nullptr, 0);
				}
				
				if (status == LUA_YIELD) {
					// system yielded and waits for next tick
					lua_gc(luaState, LUA_GCCOLLECT, 0);
					if (getKernel()) getKernel()->recalculateResources(KernelSystem::PROCESSOR);
				} else if (status == LUA_OK) {
					// runtime finished execution -> stop system normally
					tickHelper.shouldStop();
				} else {
					// runtimed crashed -> crash system with runtime error message
					luaL_traceback(luaThread, luaThread, lua_tostring(luaThread, -1), 0);
					tickHelper.shouldCrash(KernelCrash(std::string(lua_tostring(luaThread, -1))));
				}
			} catch (...) {
				// fatal end of time reached
				tickHelper.shouldCrash(KernelCrash("out of time"));
			}

			// clear some data
			clearFileStreams();
		}
#pragma optimize("", on)

		size_t luaLen(lua_State* L, int idx) {
			size_t len = 0;
			idx = lua_absindex(L, idx);
			lua_pushnil(L);
			while (lua_next(L, idx) != 0) {
				const char* str = lua_tostring(L, -2);
				str = lua_tostring(L, -1);
				len++;
				lua_pop(L, 1);
			}
			return len;
		}

		void LuaProcessor::clearFileStreams() {
			for (auto fs = fileStreams.begin(); fs != fileStreams.end(); ++fs) {
				if (!fs->isValid()) fileStreams.erase(fs--);
			}
		}

		std::set<LuaFile> LuaProcessor::getFileStreams() const {
			return fileStreams;
		}

		void LuaProcessor::reset() {
			UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Lua Processor Reset"), *DebugInfo);
			tickHelper.stop();
			
			// can't reset running system state
			if (getKernel()->getState() != RUNNING) return;

			// reset tempdata
			timeout = -1;
			pullState = 0;
			getKernel()->getFileSystem()->addListener(fileSystemListener);

			// clear existing lua state
			if (luaState) {
				lua_close(luaState);
			}

			// create new lua state
			luaState = luaL_newstate();

			// setup library and perm tables for persistency
			lua_newtable(luaState); // perm
			lua_newtable(luaState); // perm, uperm 

			// register pointer to this Lua Processor in c registry, filestream list & perm table
			luaL_newmetatable(luaState, "LuaProcessor"); // perm, uperm, mt-LuaProcessor
			lua_pop(luaState, 1); // perm, uperm
			LuaProcessor*& luaProcessor = *(LuaProcessor**)lua_newuserdata(luaState, sizeof(LuaProcessor*)); // perm, uperm, proc
			luaL_setmetatable(luaState, "LuaProcessor");
			luaProcessor = this;
			lua_pushvalue(luaState, -1); // perm, uperm, proc, proc
			lua_setfield(luaState, LUA_REGISTRYINDEX, "LuaProcessorPtr"); // perm, uperm, proc
			lua_pushvalue(luaState, -1); // perm, uperm, proc, proc
			lua_setfield(luaState, -3, "LuaProcessor"); // perm, uperm, proc
			lua_pushstring(luaState, "LuaProcessor"); // perm, uperm, proc, "LuaProcessorPtr"
			lua_settable(luaState, -4); // perm, uperm
			luaSetup(luaState); // perm, uperm
			lua_pushlightuserdata(luaState, &fileStreams);
			lua_setfield(luaState, LUA_REGISTRYINDEX, "FileStreamStorage");
			
			// finish perm tables
			lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistUperm"); // perm
			lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistPerm"); //
			
			// create new thread for user code chunk
			luaThread = lua_newthread(luaState);
			luaThreadIndex = lua_gettop(luaState);

			// setup thread with code
			if (!eeprom.IsValid()) {
				kernel->crash(KernelCrash("No Valid EEPROM set"));
				return;
			}
			FTCHARToUTF8 CodeConv(*eeprom->GetCode(), eeprom->GetCode().Len());
			std::string code = std::string(CodeConv.Get(), CodeConv.Length());
			luaL_loadbuffer(luaThread, code.c_str(), code.size(), "=EEPROM");
			if (lua_isstring(luaThread, -1)) {
				// Syntax error
				kernel->crash(KernelCrash(lua_tostring(luaThread, -1)));
				return;
			}

			// reset tick state
			tickHelper.reset();

			// lua_gc(luaState, LUA_GCSETPAUSE, 100);
			// TODO: Check if we actually want to use this or the manual gc call
		}

		std::int64_t LuaProcessor::getMemoryUsage(bool recalc) {
			return lua_gc(luaState, LUA_GCCOUNT, 0)* 100;
		}

		static constexpr uint32 Base64GetEncodedDataSize(uint32 NumBytes) {
			return ((NumBytes + 2) / 3) * 4;
		}
		
		FString Base64Encode(const uint8* Source, uint32 Length) {
			uint32 ExpectedLength = Base64GetEncodedDataSize(Length);

			FString OutBuffer;

			TArray<TCHAR>& OutCharArray = OutBuffer.GetCharArray();
			OutCharArray.SetNum(ExpectedLength + 1);
			int64 EncodedLength = FBase64::Encode(Source, Length, OutCharArray.GetData());
			verify(EncodedLength == OutBuffer.Len());

			return OutBuffer;
		}

		void LuaProcessor::PreSerialize(UProcessorStateStorage* storage, bool bLoading) {
			UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Lua Processor %s"), *DebugInfo, bLoading ? TEXT("PreDeserialize") : TEXT("PreSerialize"));
			tickHelper.stop();
			
			for (LuaFile file : fileStreams) {
				if (file->file) {
					file->transfer = FileSystem::SRef<LuaFilePersistTransfer>(new LuaFilePersistTransfer());
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
		}

#pragma optimize("", off)
		int luaPersist(lua_State* L) {
			LuaProcessor* p = LuaProcessor::luaGetProcessor(L);
			UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Lua Processor Persist"), *p->DebugInfo);
			
			// perm, globals, thread
			
			// persist globals table
			eris_persist(L, 1, 2); // perm, globals, thread, str-globals

			// add global table to perm table
			lua_pushvalue(L, 2); // perm, globals, thread, str-globals, globals
			lua_pushstring(L, "Globals"); // perm, globals, thread, str-globals, globals, "globals"
			lua_settable(L, 1); // perm, globals, thread, str-globals

			// persist thread
			eris_persist(L, 1, 3); // perm, globals, thread, str-globals, str-thread

			return 2;
		}
#pragma optimize("", on)
		
		void LuaProcessor::Serialize(UProcessorStateStorage* storage, bool bLoading) {
			UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Lua Processor %s"), *DebugInfo, bLoading ? TEXT("Deserialize") : TEXT("Serialize"));
			if (!bLoading) {
				// check state & thread
				if (!luaState || !luaThread || lua_status(luaThread) != LUA_YIELD) return;

				ULuaProcessorStateStorage* Data = Cast<ULuaProcessorStateStorage>(storage);

				// save pull state
				Data->PullState = pullState;
				Data->Timeout = timeout;
				Data->PullStart = static_cast<uint64>(std::chrono::duration_cast<std::chrono::milliseconds>(pullStart.time_since_epoch()).count());
				
				// prepare traces list
				lua_pushlightuserdata(luaState, storage); // ..., storage
				lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistStorage"); // ...
			
				// prepare state data
				lua_getfield(luaState, LUA_REGISTRYINDEX, "PersistPerm"); // ..., perm
				lua_geti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS); // ..., perm, globals
				lua_pushvalue(luaState, -1); // ..., perm, globals, globals
				lua_pushnil(luaState); // ..., perm, globals, globals, nil
				lua_settable(luaState, -4); // ..., perm, globals
				lua_pushvalue(luaState, -2); // ..., perm, globals, perm
				lua_pushvalue(luaState, -2); // ..., perm, globals, perm, globals
				lua_pushvalue(luaState, luaThreadIndex); // ..., perm, globals, perm, globals, thread

				lua_pushcfunction(luaState, luaPersist);  // ..., perm, globals, perm, globals, thread, persist-func
				lua_insert(luaState, -4); // ..., perm, globals, persist-func, perm, globals, thread
				int status = lua_pcall(luaState, 3, 2, 0); // ..., perm, globals, str-globals, str-thread

				// check unpersist
				if (status == LUA_OK) {
					// encode persisted globals
					size_t globals_l = 0;
					const char* globals_r = lua_tolstring(luaState, -2, &globals_l);
					Data->Globals = Base64Encode((uint8*)globals_r, globals_l);

					// encode persisted thread
					size_t thread_l = 0;
					const char* thread_r = lua_tolstring(luaState, -1, &thread_l);
					Data->Thread = Base64Encode((uint8*)thread_r, thread_l);
				
					lua_pop(luaState, 2); // ..., perm, globals
				} else {
					// print error
					if (lua_isstring(luaState, -1)) {
						UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Unable to persit! '%s'"), *DebugInfo, *FString(lua_tostring(luaState, -1)));
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

#pragma optimize("", off)
		bool Base64Decode(const FString& Source, TArray<ANSICHAR>& OutData) {
			uint32 ExpectedLength = FBase64::GetDecodedDataSize(Source);

			TArray<ANSICHAR> TempDest;
			TempDest.AddZeroed(ExpectedLength + 1);
			if(!FBase64::Decode(*Source, Source.Len(), (uint8*)TempDest.GetData())) {
				return false;
			}
			OutData = TempDest;
			return true;
		}

		int luaUnpersist(lua_State* L) {
			LuaProcessor* p = LuaProcessor::luaGetProcessor(L);
			UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Lua Processor Unpersist"), *p->DebugInfo);
			
			// str-thread, str-globals, uperm
			// unpersist globals
			eris_unpersist(L, 3, 2); // str-thread, str-globals, uperm, globals

			// add globals to uperm
			lua_pushvalue(L, -1); // str-thread, str-globals, uperm, globals, globals
			lua_setfield(L, 3, "Globals"); // str-thread, str-globals, uperm, globals
			
			// unpersist thread
			eris_unpersist(L, 3, 1); // str-thread, str-globals, uperm, globals, thread
			
			return 2;
		}

		void LuaProcessor::PostSerialize(UProcessorStateStorage* Storage, bool bLoading) {
			UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Lua Processor %s"), *DebugInfo, bLoading ? TEXT("PostDeserialize") : TEXT("PostSerialize"));
			if (bLoading) {
				if (kernel->getState() != RUNNING) return;

				ULuaProcessorStateStorage* Data = Cast<ULuaProcessorStateStorage>(Storage);
				
				reset();

				// save pull state
				pullState = Data->PullState;
				timeout = Data->Timeout;
				pullStart = std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::milliseconds(Data->PullStart));

				// decode & check data from json
				TArray<ANSICHAR> thread;
				Base64Decode(Data->Thread, thread);
				TArray<ANSICHAR> globals;
				Base64Decode(Data->Globals, globals);
				if (thread.Num() <= 1 && globals.Num() <= 1) return;

				// prepare traces list
				lua_pushlightuserdata(luaState, Storage); // ..., storage
				lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistStorage"); // ...
			
				// get uperm table
				lua_getfield(luaState, LUA_REGISTRYINDEX, "PersistUperm"); // ..., uperm
			
				// prepare protected unpersist
				lua_pushcfunction(luaState, luaUnpersist); // ..., uperm, unpersist

				// push data for protected unpersist
				lua_pushlstring(luaState, thread.GetData(), thread.Num()); // ..., uperm, unpersist, str-thread
				lua_pushlstring(luaState, globals.GetData(), globals.Num()); // ..., uperm, unpersist, str-thread, str-globals
				lua_pushvalue(luaState, -4); // ..., uperm, unpersist, str-thread, str-globals, uperm
			
				// do unpersist
				int ok = lua_pcall(luaState, 3, 2, 0); // ...,  uperm, globals, thread

				// check unpersist
				if (ok != LUA_OK) {
					// print error
					if (lua_isstring(luaState, -1)) {
						UE_LOG(LogFicsItNetworks, Log, TEXT("%s: Unable to unpersit! '%s'"), *DebugInfo, *FString(lua_tostring(luaState, -1)));
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
					lua_pushnil(luaState); // ..., uperm, globals, thread, nil
					lua_setfield(luaState, -4, "Globals"); // ..., uperm, globals, thread
					lua_pushnil(luaState); // ..., uperm, globals, thread, nil
					lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistTraces"); // ..., uperm, globals, thread
				
					// place persisted data
					lua_replace(luaState, luaThreadIndex); // ..., uperm, globals
					lua_seti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS); // ..., uperm
					luaThread = lua_tothread(luaState, luaThreadIndex);
					lua_pop(luaState, 1); // ...
				}
			}
		}
#pragma optimize("", on)

		UProcessorStateStorage* LuaProcessor::CreateSerializationStorage() {
			return NewObject<ULuaProcessorStateStorage>();
		}

		void LuaProcessor::setEEPROM(AFINStateEEPROM* newEeprom) {
			eeprom = Cast<AFINStateEEPROMLua>(newEeprom);
			reset();
		}

		LuaProcessorTick& LuaProcessor::getTickHelper() {
			return tickHelper;
		}

		int luaReYield(lua_State* L) {
			lua_yield(L,0);
			return 0;
		}

		int luaPrint(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			int args = lua_gettop(L);
			std::string log;
			for (int i = 1; i <= args; ++i) {
				size_t s_len = 0;
				const char* s = luaL_tolstring(L, i, &s_len);
				if (!s) luaL_argerror(L, i, "is not valid type");
				log += std::string(s, s_len) + " ";
			}
			if (log.length() > 0) log = log.erase(log.length()-1);
			
			try {
				FileSystem::SRef<FileSystem::FileStream> serial = LuaProcessor::luaGetProcessor(L)->getKernel()->getDevDevice()->getSerial()->open(FileSystem::OUTPUT);
				if (serial) {
					*serial << log << "\r\n";
					serial->close();
				}
			} catch (std::exception ex) {
				luaL_error(L, ex.what());
			}
			
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		int luaYieldResume(lua_State* L, int status, lua_KContext ctx) {
			// dont pass pushed bool for user executed yield identification
			return LuaProcessor::luaAPIReturn(L, lua_gettop(L) - 1);
		}

		int luaYield(lua_State* L) {
			int args = lua_gettop(L);

			// insert a boolean to idicate a user executed yield
			lua_pushboolean(L, true);
			lua_insert(L, 1);
			
			return lua_yieldk(L, args+1, NULL, &luaYieldResume);
		}

		int luaResume(lua_State* L); // pre-declare

		int luaResumeResume(lua_State* L, int status, lua_KContext ctx) {
			return LuaProcessor::luaAPIReturn(L, luaResume(L));
		}

		int luaResume(lua_State* L) {
			int args = lua_gettop(L);
			int threadIndex = 1;
			if (lua_isboolean(L, 1)) threadIndex = 2;
			if (!lua_isthread(L, threadIndex)) luaL_argerror(L, threadIndex, "is no thread");
			lua_State* thread = lua_tothread(L, threadIndex);

			// attach hook for out-of-time exception if thread got loaded from save and hook is not applied
			lua_sethook(thread, LuaProcessor::luaHook, LUA_MASKCOUNT, LuaProcessor::luaGetProcessor(L)->getTickHelper().steps());

			// copy passed arguments to coroutine so it can return these arguments from the yield function
			// but dont move the passed coroutine and then resume the coroutine
			lua_xmove(L, thread, args - threadIndex);
			int state = lua_resume(thread, L, args - threadIndex);

			int nargs = lua_gettop(thread);
			// no args indicates return or internal yield (count hook)
			if (nargs == 0) {
				// yield self to cascade the yield down and so the lua execution halts
				if (state == LUA_YIELD) {
					// yield from count hook
					if (threadIndex == 2 && lua_toboolean(L, 1)) {
						return lua_yield(L, 0);
					} else {
						return lua_yieldk(L, 0, NULL, &luaResumeResume);
					}
				} else return LuaProcessor::luaAPIReturn(L, 0);
			}
			
			if (state == LUA_YIELD) nargs -= 1; // remove bool added by overwritten yield
			if (state >= LUA_YIELD) {
				lua_pushboolean(L, false);
				nargs += 1;
			} else {
				lua_pushboolean(L, true);
				nargs += 1;
			}

			// copy the parameters passed to yield or returned to our stack so we can return them
			lua_xmove(thread, L, nargs);
			return LuaProcessor::luaAPIReturn(L, nargs);
		}
		
		void LuaProcessor::luaSetup(lua_State* L) {
			PersistSetup("LuaProcessor", -2);

			luaL_requiref(L, "_G", luaopen_base, true);
			lua_pushnil(L);
			lua_setfield(L, -2, "collectgarbage");
			lua_pushnil(L);
			lua_setfield(L, -2, "dofile");
			lua_pushnil(L);
			lua_setfield(L, -2, "loadfile");
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
			PersistTable("coroutine", -1);
			lua_pop(L, 1);
			
			luaL_requiref(L, "math", luaopen_math, true);
			PersistTable("math", -1);
			lua_pop(L, 1);

			luaL_requiref(L, "string", luaopen_string, true);
			PersistTable("string", -1);
			lua_pop(L, 1);

			setupRefUtils(L);
			setupInstanceSystem(L);
			setupStructSystem(L);
			setupComponentAPI(L);
			setupEventAPI(L);
			setupFileSystemAPI(L);
			setupComputerAPI(L);
			setupDebugAPI(L);
			setupFutureAPI(L);
		}

		AFINStateEEPROMLua* LuaProcessor::getEEPROM() {
			return eeprom.Get();
		}
#pragma optimize("", off)
		int LuaProcessor::doSignal(lua_State* L) {
			auto net = getKernel()->getNetwork();
			if (!net || net->getSignalCount() < 1) return 0;
			FFINNetworkTrace sender;
			FFINSignalData signal = net->popSignal(sender);
			int props = 2;
			if (signal.Signal) lua_pushstring(L, TCHAR_TO_UTF8(*signal.Signal->GetInternalName()));
			else lua_pushnil(L);
			newInstance(L, UFINNetworkUtils::RedirectIfPossible(sender));
			for (const FFINAnyNetworkValue& Value : signal.Data) {
				networkValueToLua(L, Value);
				props++;
			}
			return props;
		}
#pragma optimize("", on)

		void LuaProcessor::luaHook(lua_State* L, lua_Debug* ar) {
			LuaProcessor* p = LuaProcessor::luaGetProcessor(L);
			p->tickHelper.tickHook(L);
		}

#pragma optimize("", off)
		int LuaProcessor::luaAPIReturn(lua_State* L, int args) {
			LuaProcessor* p = LuaProcessor::luaGetProcessor(L);
			return p->tickHelper.apiReturn(L, args);
		}
#pragma optimize("", on)

		lua_State* LuaProcessor::getLuaState() const {
			return luaState;
		}
	}
}
