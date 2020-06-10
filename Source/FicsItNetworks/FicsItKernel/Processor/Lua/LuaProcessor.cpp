#include "LuaProcessor.h"

#include "../../FicsItKernel.h"

#include "LuaInstance.h"
#include "LuaStructs.h"
#include "LuaHooks.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "LuaFileSystemAPI.h"
#include "LuaComputerAPI.h"
#include "LuaProcessorStateStorage.h"

#include "FINStateEEPROMLua.h"

#include "SML/util/Logging.h"

namespace FicsItKernel {
	namespace Lua {
		LuaSignalReader::LuaSignalReader(lua_State* L) : L(L) {}

		void LuaSignalReader::operator<<(const std::string& str) {
			lua_pushstring(L, str.c_str());
		}

		void LuaSignalReader::operator<<(double num) {
			lua_pushnumber(L, num);
		}

		void LuaSignalReader::operator<<(int num) {
			lua_pushinteger(L, num);
		}

		void LuaSignalReader::operator<<(bool b) {
			lua_pushboolean(L, b);
		}

		void LuaSignalReader::operator<<(UObject* obj) {
			newInstance(L, Network::NetworkTrace(obj));
		}

		void LuaSignalReader::operator<<(const Network::NetworkTrace& obj) {
			newInstance(L, obj);
		}

		void LuaSignalReader::WriteAbstract(const void* obj, const std::string& id) {
			if (id == "InventoryItem") {
				luaStruct(L, *(const FInventoryItem*)obj);
			} else if (id == "ItemAmount") {
				luaStruct(L, *(const FItemAmount*)obj);
			} else if (id == "InventoryStack") {
				luaStruct(L, *(const FInventoryStack*)obj);
			}
		}

		LuaProcessor* LuaProcessor::luaGetProcessor(lua_State* L) {
			lua_getfield(L, LUA_REGISTRYINDEX, "LuaProcessorPtr");
			LuaProcessor* p = *(LuaProcessor**) luaL_checkudata(L, -1, "LuaProcessor");
			lua_pop(L, 1);
			return p;
		}

		LuaProcessor::LuaProcessor(int speed) : speed(speed) {
			
		}

		void LuaProcessor::tick(float delta) {
			if (!luaState || !luaThread) return;

			// reset out of time
			endOfTick = false;
			lua_sethook(luaThread, luaHook, LUA_MASKCOUNT, speed);
			
			int status = 0;
			if (timeout > -1) {
				// Runtime is pulling a signal
				if (getKernel()->getNetwork()->getSignalCount() > 0) {
					// Signal available -> reset timout and pull signal from network
					timeout = -1;
					auto sigArgs = doSignal(luaThread);
					if (sigArgs < 1) {
						// no signals poped -> crash system
						status = LUA_ERRRUN;
					} else {
						// signal poped -> resume yield with signal as parameters (passing signals parameters back to pull yield)
						status = lua_resume(luaThread, nullptr, sigArgs);
					}
				} else if (timeout == 0 || timeout > std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - pullStart).count()) {
					// no signal available & not timeout reached -> skip tick
					return;
				} else {
					// no signal available & timout reached -> resume yield with  no parameters
					status = lua_resume(luaThread, nullptr, 0);
				}
			} else {
				// resume runtime normally
				status = lua_resume(luaThread, nullptr, 0);
			}
			
			if (status == LUA_YIELD) {
				// system yielded and waits for next tick
				lua_gc(luaState, LUA_GCCOLLECT, 0);
				kernel->recalculateResources(KernelSystem::PROCESSOR);
			} else if (status == LUA_OK) {
				// runtime finished execution -> stop system normally
				kernel->stop();
			} else {
				// runtimed crashed -> crash system with runtime error message
				kernel->crash({ lua_tostring(luaThread, -1) });
			}

			// clear some data
			clearFileStreams();
		}

		size_t luaLen(lua_State* L, int idx) {
			size_t len = 0;
			idx = lua_absindex(L, idx);
			lua_pushnil(L);
			while (lua_next(L, idx) != 0) {
				const char* str = lua_tostring(L, -2);
				//if (str) SML::Logging::error("   k: ", str);
				str = lua_tostring(L, -1);
				//if (str) SML::Logging::error("   v: ", str);
				//else if (lua_isnil(L, -1)) SML::Logging::error("   v: nil");
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
		
		void LuaProcessor::reset() {
			// can't reset running system state
			if (getKernel()->getState() != RUNNING) return;

			// reset tempdata
			timeout = -1;

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
			luaL_loadstring(luaThread, std::string(TCHAR_TO_UTF8(*eeprom->Code), eeprom->Code.Len()).c_str());
			
			// lua_gc(luaState, LUA_GCSETPAUSE, 100);
			// TODO: Check if we actually want to use this or the manual gc call
		}

		std::int64_t LuaProcessor::getMemoryUsage(bool recalc) {
			return lua_gc(luaState, LUA_GCCOUNT, 0)* 1000;
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
		
		void LuaProcessor::Serialize(UProcessorStateStorage* storage, bool bLoading) {
			if (!bLoading) {
				// check state & thread
				if (!luaState || !luaThread) return;

				ULuaProcessorStateStorage* Data = Cast<ULuaProcessorStateStorage>(storage);

				// prepare traces list
				lua_pushlightuserdata(luaState, storage); // ..., storage
				lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistStorage"); // ...
			
				// prepare state data
				lua_getfield(luaState, LUA_REGISTRYINDEX, "PersistPerm"); // ..., perm
				lua_geti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS); // ..., perm, globals
				lua_pushvalue(luaState, -1); // ..., perm, globals, globals
				lua_pushnil(luaState); // ..., perm, globals, globals, nil
				lua_settable(luaState, -4); // ..., perm, globals
			
				// persist globals table
				eris_persist(luaState, -2, -1); // ..., perm, globals, str-globals

				// encode persisted globals
				size_t globals_l = 0;
				const char* globals_r = lua_tolstring(luaState, -1, &globals_l);
				Data->Globals = Base64Encode((uint8*)globals_r, globals_l);
				lua_pop(luaState, 1); // ..., perm, globals

				// add global table to perm table
				lua_pushvalue(luaState, -1); // ..., perm, globals, globals
				lua_pushstring(luaState, "Globals"); // ..., perm, globals, globals, "globals"
				lua_settable(luaState, -4); // ..., perm, globals
			
				// persist thread
				eris_persist(luaState, -2, luaThreadIndex); // ..., perm, globals, str-thread

				// encode persisted thread
				size_t thread_l = 0;
				const char* thread_r = lua_tolstring(luaState, -1, &thread_l);
				Data->Thread = Base64Encode((uint8*)thread_r, thread_l);

				// cleanup
				lua_pop(luaState, 1); // ..., perm, globals
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
			if (bLoading) {
				if (kernel->getState() != RUNNING) return;

				ULuaProcessorStateStorage* Data = Cast<ULuaProcessorStateStorage>(Storage);

				reset();

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
				lua_call(luaState, 3, 2); int ok = LUA_OK;
				//int ok = lua_pcall(luaState, 3, 2, 0); // ...,  uperm, globals, thread

				// check unpersist
				if (ok != LUA_OK) {
					// cleanup
					lua_pushnil(luaState); // ..., uperm, err, nil
					lua_setfield(luaState, -3, "Globals"); // ..., uperm, err
					lua_pop(luaState, 1); // ...
					lua_pushnil(luaState); // ..., nil
					lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistTraces"); // ...
				
					throw std::exception("Unable to unpersist");
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

		void LuaProcessor::setEEPROM(AFINStateEEPROM* eeprom) {
			this->eeprom = Cast<AFINStateEEPROMLua>(eeprom);
			reset();
		}

		int luaReYield(lua_State* L) {
			lua_yield(L,0);
			return 0;
		}

		int luaPrint(lua_State* L) {
			int args = lua_gettop(L);
			std::string log;
			for (int i = 1; i <= args; ++i) {
				size_t s_len = 0;
				const char* s = luaL_tolstring(L, i, &s_len);
				if (!s) luaL_argerror(L, i, "is not valid type");
				log += std::string(s, s_len);
			}
			
			try {
				auto serial = LuaProcessor::luaGetProcessor(L)->getKernel()->getDevDevice()->getSerial()->open(FileSystem::OUTPUT);
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
			if (!lua_isthread(L, 1)) luaL_argerror(L, 1, "is no thread");
			lua_State* thread = lua_tothread(L, 1);

			// copy passed arguments to coroutine so it can return these arguments from the yield function
			// but dont move the passed coroutine and then resume the coroutine
			lua_xmove(L, thread, args - 1);
			int state = lua_resume(thread, L, args - 1);

			int nargs = lua_gettop(thread);
			// no args indicates return or internal yield (count hook)
			if (nargs == 0) {
				// yield self to cascade the yield down and so the lua execution halts
				if (state == LUA_YIELD) return lua_yieldk(L, 0, NULL, &luaResumeResume);
				else return LuaProcessor::luaAPIReturn(L, 0);
			}
			
			if (state == LUA_YIELD) nargs -= 1; // remove bool added by overwritten yield

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
			lua_pushnil(L);
			lua_setfield(L, -2, "setmetatable");
			PersistTable("global", -1);
			lua_pop(L, 1);
			luaL_requiref(L, "table", luaopen_table, true);
			PersistTable("table", -1);
			lua_pop(L, 1);
			luaL_requiref(L, "coroutine", luaopen_coroutine, true);
			PersistTable("coroutine", -1);
			lua_pop(L, 1);
			luaL_requiref(L, "math", luaopen_math, true);
			PersistTable("math", -1);
			lua_pop(L, 1);
			luaL_requiref(L, "string", luaopen_string, true);
			PersistTable("string", -1);
			lua_pop(L, 1);
			
			lua_register(L, "print", luaPrint);
			PersistGlobal("print");
			lua_register(L, "resume", luaResume);
			PersistGlobal("resume");
			lua_register(L, "yield", luaYield);
			PersistGlobal("yield");
			// TODO: move resume and yield to overwrite coroutine
			
			setupInstanceSystem(L);
			setupStructs(L);
			setupHooks(L);
			setupComponentAPI(L);
			setupEventAPI(L);
			setupFileSystemAPI(L);
			setupComputerAPI(L);
		}

		int LuaProcessor::doSignal(lua_State* L) {
			auto net = getKernel()->getNetwork();
			if (!net || net->getSignalCount() < 1) return 0;
			Network::NetworkTrace sender;
			std::shared_ptr<Network::Signal> signal = net->popSignal(sender);
			if (!signal.get()) return 0;
			int props = 2;
			lua_pushstring(L, signal->getName().c_str());
			newInstance(L, sender);
			LuaSignalReader reader(L);
			props += *signal >> reader;
			return props;
		}

		void LuaProcessor::luaHook(lua_State* L, lua_Debug* ar) {
			LuaProcessor* p = LuaProcessor::luaGetProcessor(L);
			if (p->endOfTick) {
				luaL_error(L, "out of time");
			} else {
				p->endOfTick = true;
				lua_sethook(p->luaThread, luaHook, LUA_MASKCOUNT, p->speed / 2);
			}
		}

		int luaAPIReturn_Resume(lua_State* L, int status, lua_KContext ctx) {
			return static_cast<int>(ctx);
		}

#pragma optimize("", off)
		int LuaProcessor::luaAPIReturn(lua_State* L, int args) {
			LuaProcessor* p = LuaProcessor::luaGetProcessor(L);
			if (p->endOfTick) {
				return lua_yieldk(L, 0, args, &luaAPIReturn_Resume);
			} else {
				return args;
			}
		}
#pragma optimize("", on)
	}
}
       