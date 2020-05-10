#include "LuaProcessor.h"

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

		LuaProcessor* LuaProcessor::currentProcessor = nullptr;

		LuaProcessor* LuaProcessor::getCurrentProcessor() {
			return currentProcessor;
		}

		void luaHook(lua_State *L, lua_Debug* ar) {
			lua_gc(L, LUA_GCCOLLECT, 0);
			auto kernel = LuaProcessor::getCurrentProcessor()->getKernel();
			kernel->recalculateResources(KernelSystem::PROCESSOR);
			lua_yield(L, 0);
		}

		LuaProcessor::LuaProcessor(int speed) : speed(speed) {
			
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

		void LuaProcessor::tick(double delta) {
			currentProcessor = this;

			if (!luaState || !luaThread) return;

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
			} else if (status == LUA_OK) {
				// runtime finished execution -> stop system normally
				kernel->stop();
			} else {
				// runtimed crashed -> crash system with runtime error message
				kernel->crash({ lua_tostring(luaThread, -1) });
			}

			currentProcessor = nullptr;
		}

		void LuaProcessor::reset() {
			// can't reset running system state
			if (getKernel()->getState() != RUNNING) return;

			// reset event pull
			timeout = -1;

			// clear existing lua state
			if (luaState) {
				lua_close(luaState);
			}

			// create and setup new lua state
			luaState = luaL_newstate();
			luaSetup(luaState);

			// create new thread for user code chunk
			luaThread = lua_newthread(luaState);
			luaThreadIndex = lua_gettop(luaState);

			// load user code chunk into user thread
			luaL_loadstring(luaThread, code.c_str());
			
			// lua_gc(luaState, LUA_GCSETPAUSE, 100);
			// TODO: Check if we actually want to use this or the manual gc call
		}

		std::int64_t LuaProcessor::getMemoryUsage(bool recalc) {
			return lua_gc(luaState, LUA_GCCOUNT, 0) * 1000;
		}

#define PersistGlobal(Global) \
	lua_getglobal(luaState, Global); \
	lua_pushnumber(luaState, i++); \
	lua_settable(luaState, -3);

		int luaReYield(lua_State* L) {
			lua_yield(L,0);
			return 0;
		}
		
		TSharedPtr<FJsonObject> LuaProcessor::persist() {
			lua_geti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
			lua_newtable(luaState);
			int i = 1;
			PersistGlobal("print")
			PersistGlobal("yield")
			PersistGlobal("resume")
			PersistGlobal("filesystem")
			PersistGlobal("component")
			PersistGlobal("event")
			PersistGlobal("computer")
			
			PersistGlobal("table")
			PersistGlobal("coroutine")
			PersistGlobal("string")
			PersistGlobal("math")
			PersistGlobal("debug")

			//lua_pushcfunction(luaThread, luaReYield);
			//lua_call(luaThread, 0, 0);
			eris_persist(luaState, -1, luaThreadIndex);
			//eris_persist(luaState, -2, -3);
			size_t thread_l = 0;
			
			const char* thread_r = lua_tolstring(luaState, -2, &thread_l);
			FString thread = FBase64::Encode((uint8*)thread_r, thread_l);
			//size_t globals_l = 0;
			//const char* globals_r = lua_tolstring(luaState, -1, &thread_l);
			//FString globals = FBase64::Encode((uint8*)globals_r, globals_l);

			lua_pop(luaState, 3);
			
			TSharedPtr<FJsonObject> json = MakeShareable(new FJsonObject());
			json->SetStringField("MainThread", thread);
			//json->SetStringField("Globals", globals);
			return json;
		}

#define UnpersistGlobal(Global) \
	lua_pushnumber(luaState, i++); \
	lua_getglobal(luaState, Global); \
	lua_settable(luaState, -3);

		void LuaProcessor::unpersist(TSharedPtr<FJsonObject> json) {
			FString thread;
			FBase64::Decode(json->GetStringField("MainThread"), thread);
			FString globals;
			FBase64::Decode(json->GetStringField("Globals"), globals);
			
			lua_pushlstring(luaState, TCHAR_TO_UTF8(*thread), thread.Len());
			lua_pushlstring(luaState, TCHAR_TO_UTF8(*globals), globals.Len());
			
			lua_newtable(luaState);
			int i = 1;
			UnpersistGlobal("print")
            UnpersistGlobal("yield")
            UnpersistGlobal("resume")
            UnpersistGlobal("filesystem")
            UnpersistGlobal("component")
            UnpersistGlobal("event")
            UnpersistGlobal("computer")
			
            UnpersistGlobal("table")
            UnpersistGlobal("coroutine")
            UnpersistGlobal("string")
            UnpersistGlobal("math")
            UnpersistGlobal("debug")
			
            eris_unpersist(luaState, -1, -3);
			eris_unpersist(luaState, -2, -3);

			lua_seti(luaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
			lua_replace(luaState, luaThreadIndex);

			lua_pop(luaState, 3);
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
				auto serial = LuaProcessor::getCurrentProcessor()->getKernel()->getDevDevice()->getSerial()->open(FileSystem::OUTPUT);
				if (serial) {
					*serial << log << "\r\n";
					serial->close();
				}
			} catch (std::exception ex) {
				luaL_error(L, ex.what());
			}

			return 0;
		}

		int luaYieldResume(lua_State* L, int status, lua_KContext ctx) {
			// dont pass pushed bool for user executed yield identification
			return lua_gettop(L) - 1;
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
			return luaResume(L);
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
				else return 0;
			}
			
			if (state == LUA_YIELD) nargs -= 1; // remove bool added by overwritten yield

			// copy the parameters passed to yield or returned to our stack so we can return them
			lua_xmove(thread, L, nargs);
			return nargs;
		}

		void LuaProcessor::luaSetup(lua_State* L) {
			luaL_requiref(L, "table", luaopen_table, true);
			luaL_requiref(L, "coroutine", luaopen_coroutine, true);
			luaL_requiref(L, "math", luaopen_math, true);
			luaL_requiref(L, "string", luaopen_string, true);
			luaL_requiref(L, "debug", luaopen_debug, true);
			
			lua_register(L, "print", luaPrint);
			lua_register(L, "resume", luaResume);
			lua_register(L, "yield", luaYield);
			// TODO: move resume and yield to overwrite coroutine
			
//			setupInstanceSystem(L);
//			setupStructs(L);
//			setupHooks(L);
			setupComponentAPI(L);
			setupEventAPI(L);
			setupFileSystemAPI(getKernel(), L);
			setupComputerAPI(L, getKernel());
		}

		void LuaProcessor::setCode(const std::string& code) {
			this->code = code;
			reset();
		}
	}
}
