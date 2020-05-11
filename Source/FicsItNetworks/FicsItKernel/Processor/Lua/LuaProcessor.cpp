#include "LuaProcessor.h"


#include <xkeycheck.h>
#include <xkeycheck.h>


#include "../../FicsItKernel.h"

#include "LuaInstance.h"
#include "LuaLib.h"
#include "LuaStructs.h"
#include "LuaHooks.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "LuaFileSystemAPI.h"
#include "LuaComputerAPI.h"

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
			return *(LuaProcessor**) luaL_checkudata(L, -1, "LuaProcessor");
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
		}

		void LuaProcessor::reset() {
			// can't reset running system state
			if (getKernel()->getState() != RUNNING) return;

			// reset tempdata
			timeout = -1;
			permTableIdx = 1;

			// clear existing lua state
			if (luaState) {
				lua_close(luaState);
			}

			// create new lua state
			luaState = luaL_newstate();

			// setup library and perm tables for persistency
			// add uperm and perm tables for persistency
			lua_newtable(luaState);
			lua_newtable(luaState);
			luaSetup(luaState);

			// register pointer to this Lua Processor in c registry
			LuaProcessor*& luaProcessor = *(LuaProcessor**)lua_newuserdata(luaState, sizeof(LuaProcessor*));
			luaL_setmetatable(luaState, "LuaProcessor");
			luaProcessor = this;
			lua_pushvalue(luaState, -1);
			lua_setfield(luaState, LUA_REGISTRYINDEX, "LuaProcessorPtr");

			lua_pushvalue(luaState, -1);
			lua_seti(luaState, -3, permTableIdx);
			lua_pushinteger(luaState, permTableIdx++);
			lua_settable(luaState, -4);
			
			// finish perm tables
			lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistUperm");
			lua_setfield(luaState, LUA_REGISTRYINDEX, "PersistPerm");
			
			// create new thread for user code chunk
			luaThread = lua_newthread(luaState);
			luaThreadIndex = lua_gettop(luaState);

			// setup thread with code
			luaL_loadstring(luaThread, code.c_str());
			
			// lua_gc(luaState, LUA_GCSETPAUSE, 100);
			// TODO: Check if we actually want to use this or the manual gc call
		}

		std::int64_t LuaProcessor::getMemoryUsage(bool recalc) {
			return lua_gc(luaState, LUA_GCCOUNT, 0) * 1000;
		}

		int luaReYield(lua_State* L) {
			lua_yield(L,0);
			return 0;
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
		
		TSharedPtr<FJsonObject> LuaProcessor::persist() {
			lua_geti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
			lua_getfield(luaState, LUA_REGISTRYINDEX, "PersistPerm");

			eris_persist(luaState, -1, luaThreadIndex);
			eris_persist(luaState, -2, -3);

			size_t thread_l = 0;
			const char* thread_r = lua_tolstring(luaState, -2, &thread_l);
			FString thread = Base64Encode((uint8*)thread_r, thread_l);
			size_t globals_l = 0;
			const char* globals_r = lua_tolstring(luaState, -1, &globals_l);
			FString globals = Base64Encode((uint8*)globals_r, globals_l);

			lua_pop(luaState, 4);
			
			TSharedPtr<FJsonObject> json = MakeShareable(new FJsonObject());
			json->SetStringField("MainThread", thread);
			json->SetStringField("Globals", globals);

			return json;
		}

#pragma optimize("", off)
		bool Base64Decode(const FString& Source, TArray<ANSICHAR>& OutData) {
			uint32 ExpectedLength = FBase64::GetDecodedDataSize(Source);

			TArray<ANSICHAR> TempDest;
			TempDest.AddZeroed(ExpectedLength + 1);
			if(!FBase64::Decode(*Source, Source.Len(), (uint8*)TempDest.GetData()))
			{
				return false;
			}
			OutData = TempDest;
			return true;
		}

		int luaUnpersist(lua_State* L) {
			eris_unpersist(L, 3, 1);
			eris_unpersist(L, 3, 2);
			return 2;
		}

		void LuaProcessor::unpersist(TSharedPtr<FJsonObject> json) {
			if (!json->HasField("MainThread") || !json->HasField("Globals")) return;
			
			reset();
			
			FString thread_encoded = json->GetStringField("MainThread");
			TArray<ANSICHAR> thread;
			Base64Decode(thread_encoded, thread);
			FString globals_encoded = json->GetStringField("Globals");
			TArray<ANSICHAR> globals;
			Base64Decode(globals_encoded, globals);

			if (thread.Num() <= 1 && globals.Num() <= 1) return;
			std::string str1 = std::string(thread.GetData(), thread.Num());
			std::string str2 = std::string(globals.GetData(), globals.Num());

			lua_pushcfunction(luaState, luaUnpersist);

			lua_pushlstring(luaState, str1.c_str(), str1.length());
			lua_pushlstring(luaState, str2.c_str(), str2.length());
			lua_getfield(luaState, LUA_REGISTRYINDEX, "PersistUperm");
			
			lua_call(luaState, 3, 2);
			//int ok = lua_pcall(luaState, 3, 2, 0);
			//if (ok != LUA_OK) throw std::exception("Unable to unpersist");
			
			lua_seti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
			lua_replace(luaState, luaThreadIndex);
			luaThread = lua_tothread(luaState, luaThreadIndex);
		}
#pragma optimize("", on)

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

#define PersistGlobal(Name) \
		lua_getglobal(L, Name); \
		lua_pushvalue(L, -1); \
		lua_seti(L, -3, permTableIdx); \
		lua_pushinteger(L, permTableIdx++); \
		lua_settable(L, -4);

		void LuaProcessor::luaSetup(lua_State* L) {
			luaL_requiref(L, "table", luaopen_table, true);
			PersistGlobal("table")
			luaL_requiref(L, "coroutine", luaopen_coroutine, true);
			PersistGlobal("coroutine")
			luaL_requiref(L, "math", luaopen_math, true);
			PersistGlobal("math")
			luaL_requiref(L, "string", luaopen_string, true);
			PersistGlobal("string")
			luaL_requiref(L, "debug", luaopen_debug, true);
			PersistGlobal("debug")
			
			lua_register(L, "print", luaPrint);
			PersistGlobal("print")
			lua_register(L, "resume", luaResume);
			PersistGlobal("resume")
			lua_register(L, "yield", luaYield);
			PersistGlobal("yield")
			// TODO: move resume and yield to overwrite coroutine
			
//			setupInstanceSystem(L);
//			setupStructs(L);
//			setupHooks(L);
//			setupComponentAPI(L);
//			setupEventAPI(L);
//			setupFileSystemAPI(getKernel(), L);
//			setupComputerAPI(L, getKernel());

			// LuaProcessor metatable
			luaL_newmetatable(L, "LuaProcessor");
			lua_pop(L, 1);
		}

		void LuaProcessor::setCode(const std::string& code) {
			this->code = code;
			reset();
		}

		int LuaProcessor::doSignal(lua_State* L) {
			auto net = getKernel()->getNetwork();
			if (!net || net->getSignalCount() < 1) return 0;
			int props = 2;
			Network::NetworkTrace sender;
			auto signal = net->popSignal(sender);
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
		
		int LuaProcessor::luaAPIReturn(lua_State* L, int args) {
			LuaProcessor* p = LuaProcessor::luaGetProcessor(L);
			if (p->endOfTick) {
				return lua_yieldk(L, 0, args, &luaAPIReturn_Resume);
			} else {
				return args;
			}
		}
	}
}
