#include "FINLua/LuaFuture.h"

#include "FicsItNetworksLuaModule.h"
#include "FINLuaProcessor.h"
#include "LuaExtraSpace.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"

namespace FINLua {
	int lua_futureStruct(lua_State* L);
	int lua_futureStructContinue(lua_State* L, int, lua_KContext);
	int awaitContinue(lua_State* L, int, lua_KContext);

	LuaModule(R"(/**
	 * @LuaModule		FutureModule
	 * @DisplayName		Future Module
	 *
	 * This Module provides the Future type and all its necessary functionallity.
	 */)", FutureModule) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	FutureStruct
		 * @DisplayName		Future Struct
		 */)", FutureStruct) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		Garbage Collect
			 */)", __gc) {
				const FLuaFuture& future = luaFIN_checkFutureStruct(L, 1);
				future.~FLuaFuture();
				return 0;
			}

			int unpersist(lua_State* L) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);

				// get persist storage
				FFINLuaProcessorStateStorage& storage = processor->StateStorage;

				luaFIN_pushFuture(L, *storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1))));

				return 1;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				FLuaFuture future = luaFIN_checkFutureStruct(L, 1);

				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				FFINLuaProcessorStateStorage& storage = processor->StateStorage;
				lua_pushinteger(L, storage.Add(future));
				lua_pushcclosure(L, unpersist, 1);

				return 1;
			}
		}

		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	Future
		 * @DisplayName		Future
		 */)", Future) {
			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	__index
			 * @DisplayName		Index
			 */)", __index) {
				lua_pushnil(L);
			}

			int getInternal(lua_State* L, int index) {
				luaL_checkudata(L, index, _Name);
				lua_getiuservalue(L, index, 1);
				luaL_checktype(L, -1, LUA_TTHREAD);
				lua_State* thread = lua_tothread(L, -1);
				lua_pop(L, 1);
				int status = lua_status(thread);
				switch (status) {
					default: // error
						return luaL_error(L, "Tried to get results of failed future");
					case LUA_OK: // done or start
						if (lua_gettop(thread) == 0) { // done
							lua_getiuservalue(L, index, 2);
							int len = luaL_len(L, -1);
							for (int i = 1; i < len+1; ++i) {
								lua_geti(L, -i, i);
							}
							return len;
						}
				}
				return luaL_error(L, "Tried to get results of a future that is not ready");
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		get
			 * @DisplayName		Get
			 */)", get) {
				return
				getInternal(L, 1);
			}

			int pollInternal(lua_State* L, int idx, lua_State*& thread) {
				idx = lua_absindex(L, idx);
				luaL_checkudata(L, idx, _Name);
				lua_getiuservalue(L, idx, 1);
				thread = lua_tothread(L, -1);
				lua_pop(L, 1);
				int status = lua_status(thread);
				int results = 0;
				switch (status) {
					default: // error
						thread = nullptr;
						return -1;
					case LUA_OK: // done or start
						results = lua_gettop(thread);
						if (results == 0) {
							// done
							return LUA_OK;
						} else {
							results -= 1;
						}
					case LUA_YIELD:
						status = lua_resume(thread, L, results, &results);
				}
				// resumed
				switch (status) {
					case LUA_OK: {
						lua_createtable(L, results, 0);
						int table = lua_gettop(L);
						lua_xmove(thread, L, results);
						for (int i = results; i > 0; --i) {
							lua_seti(L, table, i);
						}
						lua_setiuservalue(L, idx, 2);
						break;
					} case LUA_YIELD:
						lua_pop(thread, results);
						break;
					default: ;
				}
				return status;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	poll()
			 * @DisplayName		Poll
			 */)", poll) {
				lua_State* thread;
				int status = pollInternal(L, 1, thread);
				lua_pushboolean(L, status == LUA_OK);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		await
			 * @DisplayName		Await
			 */)", await) {
				return luaFIN_await(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		canGet
			 * @DisplayName		Can Get
			 */)", canGet) {
				luaL_checkudata(L, 1, _Name);
				lua_getiuservalue(L, 1, 1);
				lua_State* thread = lua_tothread(L, 1);
				int status = lua_status(thread);
				lua_pushboolean(L, status == LUA_OK && lua_gettop(thread) == 0);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__newindex
			 * @DisplayName		New Index
			 */)", __newindex) {
				return 0;
			}

			int unpersist(lua_State* L) {
				lua_newuserdatauv(L, 0, 2);
				luaL_setmetatable(L, _Name);
				lua_setiuservalue(L, lua_upvalueindex(1), 1);
				lua_setiuservalue(L, lua_upvalueindex(2), 2);
				return 1;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				lua_getiuservalue(L, 1, 1);
				lua_getiuservalue(L, 1, 2);
				lua_pushcclosure(L, unpersist, 2);
				return 1;
			}
		}

		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		future
		 * @DisplayName		Future Library
		 *
		 * The global Future Library provides functions to work more easily with futures.
		 */)", future) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Future	async(thread)
			 * @DisplayName		Async
			 *
			 * Wraps the given thread/coroutine in a Lua-Future
			 *
			 * @param	thread	thread	The thread you want to wrap in a future.
			 * @return	future	Future	The Future that wraps the given thread.
			 */)", async) {
				luaFIN_pushLuaFuture(L, 1);
				return 1;
			}

			int joinContinue(lua_State* L, int status, lua_KContext) {
				bool done = true;
				int num = lua_gettop(L);
				for (int i = 1; i <= num; ++i) {
					if (luaL_testudata(L, i, Future::_Name) == nullptr) continue;
					lua_State* thread;
					status = Future::pollInternal(L, i, thread);
					if (status == LUA_OK) {
						lua_getiuservalue(L, i, 2);
						lua_replace(L, i);
					} else {
						done = false;
					}
				}
				if (!done) {
					return lua_yieldk(L, 0, NULL, joinContinue);
				}
				return lua_gettop(L);
			}

			int luaJoin(lua_State* L) {
				return joinContinue(L, 0, NULL);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Future	join(Future...)
			 * @DisplayName		Join
			 *
			 * Creates a new Future that will only finish once all futures passed as parameters have finished.
			 *
			 * @param	futures	Future...	Futures		The futures you want to join.
			 * @return	future	Future	The Future that will finish once all other futures finished.
			 */)", join) {
				luaFIN_pushLuaFutureCFunction(L, luaJoin, lua_gettop(L));
				return 1;
			}

			int luaSleepContinue(lua_State* L, int, lua_KContext) {
				double timeout = lua_tonumber(L, 1);
				double start = lua_tonumber(L, 2);
				double millis = luaFIN_getExtraSpace(L).Processor->GetKernel()->GetTimeSinceStart();
				if (millis - start < timeout*1000) {
					return lua_yieldk(L, 0, NULL, luaSleepContinue);
				}
				return 0;
			}
			int luaSleep(lua_State* L) {
				double millis = luaFIN_getExtraSpace(L).Processor->GetKernel()->GetTimeSinceStart();
				lua_pushnumber(L, millis);
				return luaSleepContinue(L, 0, NULL);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Future	sleep(seconds : number)
			 * @DisplayName		Sleep
			 *
			 * Creates a future that returns after the given amount of seconds.
			 */)", sleep) {
				luaL_checktype(L, 1, LUA_TNUMBER);
				lua_pop(L, lua_gettop(L)-1);
				luaFIN_pushLuaFutureCFunction(L, luaSleep, 1);
				return 1;
			}
		}

		int luaAsync(lua_State* L) {
			luaL_checktype(L, 1, LUA_TFUNCTION);
			lua_State* thread = lua_newthread(L);
			lua_pushvalue(L, 1);
			lua_xmove(L, thread, 1);
			luaFIN_pushLuaFuture(L, -1);
			return 1;
		}
		LuaModuleGlobalBareValue(R"(/**
		 * @LuaBareValue	async	function(function) -> LuaFuture
		 * @DisplayName		Async
		 */)", async) {
			lua_pushcfunction(L, luaAsync);
			luaFIN_persistValue(L, -1, PersistName);
		}

		int luaSleep(lua_State* L) {
			luaL_checktype(L, 1, LUA_TNUMBER);
			lua_pop(L, lua_gettop(L)-1);
			luaFIN_pushLuaFutureCFunction(L, future::luaSleep, 1);
			return luaFIN_await(L, 1);
		}
		LuaModuleGlobalBareValue(R"(/**
		 * @LuaFunction		sleep(seconds : number)
		 * @DisplayName		Sleep
		 *
		 * Blocks the current thread/future until the given amount of time passed
		 */)", sleep) {
			lua_pushcfunction(L, luaSleep);
			luaFIN_persistValue(L, -1, PersistName);
		}

		LuaModulePostSetup() {
			PersistenceNamespace("FutureModule");

			lua_pushcfunction(L, FutureStruct::unpersist);
			PersistValue("FutureStructUnpersist");
			lua_pushcfunction(L, Future::unpersist);
			PersistValue("FutureUnpersist");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(awaitContinue)));
			PersistValue("FutureAwaitContinue");

			luaL_getmetatable(L, Future::_Name);
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			lua_pop(L, 1);

			lua_pushcfunction(L, future::luaJoin);
			PersistValue("LuaJoin");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(future::joinContinue)));
			PersistValue("JoinContinue");

			lua_pushcfunction(L, lua_futureStruct);
			PersistValue("LuaFutureStruct");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(lua_futureStructContinue)));
			PersistValue("LuaFutureStructContinue");

			lua_pushcfunction(L, future::luaSleep);
			PersistValue("FutureSleep");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(future::luaSleepContinue)));
			PersistValue("FutureSleepContinue");
		}
	}

	void luaFIN_pushFutureStruct(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future) {
		FLuaFuture* future = static_cast<FLuaFuture*>(lua_newuserdata(L, sizeof(FLuaFuture)));
		new (future) FLuaFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(Future));
		luaL_setmetatable(L, FutureModule::FutureStruct::_Name);
		if (!(**future)->IsDone()) {
			UFINKernelSystem* kernel = UFINLuaProcessor::luaGetProcessor(L)->GetKernel();
			kernel->PushFuture(*future);
		}
	}

	int lua_futureStructContinue(lua_State* L, int, lua_KContext) {
		const FLuaFuture& future = luaFIN_checkFutureStruct(L, 1);
		if (!(*future)->IsDone()) {
			return lua_yieldk(L, 0, NULL, &lua_futureStructContinue);
		}
		TArray<FFIRAnyValue> Data = (*future)->GetOutput();
		FFIRTrace Trace;
		if (future->GetStruct() == FFINFutureReflection::StaticStruct()) {
			FFINFutureReflection& refFuture = future->Get<FFINFutureReflection>();
			FScopeLock Lock(&refFuture.Mutex);
			Trace = refFuture.Context.GetTrace();
		}
		for (const FFIRAnyValue& Param : Data) luaFIN_pushNetworkValue(L, Param, Trace);
		return Data.Num();
	}
	int lua_futureStruct(lua_State* L) {
		return lua_futureStructContinue(L, 0, NULL);
	}
	void luaFIN_pushFuture(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future) {
		luaFIN_pushFutureStruct(L, Future);
		luaFIN_pushLuaFutureCFunction(L, lua_futureStruct, 1);
	}

	const FLuaFuture& luaFIN_checkFutureStruct(lua_State* L, int Index) {
		FLuaFuture& future = *static_cast<FLuaFuture*>(luaL_checkudata(L, Index, FutureModule::FutureStruct::_Name));
		return future;
	}

	void luaFIN_pushLuaFuture(lua_State* L, int Thread) {
		Thread = lua_absindex(L, Thread);
		luaL_checktype(L, Thread, LUA_TTHREAD);
		lua_newuserdatauv(L, 0, 2);
		luaL_setmetatable(L, FutureModule::Future::_Name);
		lua_pushvalue(L, Thread);
		lua_setiuservalue(L, -2, 1);
	}

	void luaFIN_pushLuaFutureCFunction(lua_State* L, lua_CFunction Func, int args) {
		args = FMath::Min(args, lua_gettop(L));
		lua_State* thread = lua_newthread(L); // ..., args, thread
		lua_pushcfunction(thread, Func);
		lua_insert(L, -args-1);
		lua_xmove(L, thread, args);
		luaFIN_pushLuaFuture(L, -1);
		lua_remove(L, -2);
	}

	int awaitContinue(lua_State* L, int, lua_KContext index) {
		return luaFIN_await(L, reinterpret_cast<uint64>(reinterpret_cast<void*>(index)));
	}
	int luaFIN_await(lua_State* L, int index) {
		uint64 idx = lua_absindex(L, index);
		lua_State* thread;
		int status = FutureModule::Future::pollInternal(L, idx, thread);
		if (thread == nullptr) {
			return luaL_error(L, "Tried to await on failed future");
		}
		switch (status) {
			case LUA_OK:
				return FutureModule::Future::getInternal(L, idx);
			case LUA_YIELD:
				return luaFIN_yield(L, 0, reinterpret_cast<lua_KContext>(reinterpret_cast<void*>(idx)), &awaitContinue);
			default: // error
				lua_xmove(thread, L, 1);
			return -1;
		}
	}
}
