#include "FINLua/LuaFuture.h"

#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"

namespace FINLua {
	/**
	 * I the TSharedPtr is valid, then this future is mapping a Future Struct.
	 * If it is invalid, then it is wrapping a Lua Coroutine as Future.
	 */
	typedef TSharedPtr<TFIRInstancedStruct<FFINFuture>> FLuaFuture;

	FLuaFuture* luaFIN_pushFutureInternal(lua_State* L, const FLuaFuture& futurePtr);

	LuaModule(R"(/**
	 * @LuaModule		FutureModule
	 * @DisplayName		Future Module
	 *
	 * This Module provides the Future type and all its necessary functionallity.
	 */)", FutureModule) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		get
			 * @DisplayName		Get
			 */)", get) {
				const FLuaFuture& future = luaFIN_checkFuture(L, 1);
				if (future) {
					if ((*future)->IsDone()) {
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
				} else {
					lua_getiuservalue(L, 1, 1);
					luaL_checktype(L, -1, LUA_TTHREAD);
					lua_State* thread = lua_tothread(L, -1);
					lua_pop(L, 1);
					int status = lua_status(thread);
					switch (status) {
						default: // error
							return luaL_error(L, "Tried to get results of failed future");
						case LUA_OK: // done or start
							if (lua_gettop(thread) == 0) { // done
								lua_getiuservalue(L, 1, 2);
								int len = luaL_len(L, 2);
								for (int i = 1; i < len+1; ++i) {
									lua_geti(L, 2, i);
								}
								return len;
							}
					}
				}
				return luaL_error(L, "Tried to get results of a future that is not ready");
			}

			int pollInternal(lua_State* L, int idx, lua_State*& thread) {
				const FLuaFuture& future = luaFIN_checkFuture(L, 1);
				if (future) {
					if ((*future)->IsDone()) {
						return LUA_OK;
					} else {
						return LUA_YIELD;
					}
				} else {
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

			int awaitContinue(lua_State* L, int, lua_KContext);
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		await
			 * @DisplayName		Await
			 */)", await) {
				lua_State* thread;
				int status = pollInternal(L, 1, thread);
				if (thread == nullptr) {
					return luaL_error(L, "Tried to await on failed future");
				}
				switch (status) {
					case LUA_OK:
						return get(L);
					case LUA_YIELD:
						return luaFIN_yield(L, 0, NULL, &awaitContinue);
					default: // error
						lua_xmove(thread, L, 1);
					return -1;
				}
			}
			int awaitContinue(lua_State* L, int, lua_KContext) {
				return await(L);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		canGet
			 * @DisplayName		Can Get
			 */)", canGet) {
				if (const FLuaFuture& future = luaFIN_checkFuture(L, 1)) {
					lua_pushboolean(L, (*future)->IsDone());
				} else {
					lua_getiuservalue(L, 1, 1);
					lua_State* thread = lua_tothread(L, 1);
					int status = lua_status(thread);
					lua_pushboolean(L, status == LUA_OK && lua_gettop(thread) == 0);
				}
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__newindex
			 * @DisplayName		New Index
			 */)", __newindex) {
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		Garbage Collect
			 */)", __gc) {
				const FLuaFuture& future = luaFIN_checkFuture(L, 1);
				future.~FLuaFuture();
				return 0;
			}

			int structUnpersist(lua_State* L) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);

				// get persist storage
				FFINLuaProcessorStateStorage& storage = processor->StateStorage;

				luaFIN_pushFuture(L, *storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1))));

				return 1;
			}
			int threadUnpersist(lua_State* L) {
				luaFIN_pushFutureInternal(L, nullptr);
				lua_setiuservalue(L, lua_upvalueindex(1), 1);
				lua_setiuservalue(L, lua_upvalueindex(2), 2);
				return 1;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				FLuaFuture future = luaFIN_checkFuture(L, 1);

				if (future) {
					UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
					FFINLuaProcessorStateStorage& storage = processor->StateStorage;
					lua_pushinteger(L, storage.Add(future));
					lua_pushcclosure(L, structUnpersist, 1);
				} else {
					lua_getiuservalue(L, 1, 1);
					lua_getiuservalue(L, 1, 2);
					lua_pushcclosure(L, threadUnpersist, 2);
				}

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
			 * @LuaFunction		LuaFuture	async(thread)
			 * @DisplayName		Async
			 *
			 * Wraps the given thread/coroutine in a Lua-Future
			 *
			 * @param	thread	thread		The thread you want to wrap in a future.
			 * @return	future	LuaFuture	The Lua-Future that wraps the given thread.
			 */)", async) {
				luaFIN_pushLuaFuture(L, 1);
				return 1;
			}

UE_DISABLE_OPTIMIZATION_SHIP
			int joinContinue(lua_State* L, int status, lua_KContext ctx) {
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
UE_ENABLE_OPTIMIZATION_SHIP

			int luaJoin(lua_State* L) {
				return joinContinue(L, 0, NULL);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		LuaFuture	join(LuaFuture...)
			 * @DisplayName		Join
			 *
			 * Creates a new Lua-Future that will only finish once all futures passed as parameters have finished.
			 *
			 * @param	futures	LuaFuture...	Futures		The futures you want to join.
			 * @return	future	LuaFuture	The Lua-Future that will finish once all other futures finished.
			 */)", join) {
				luaFIN_pushLuaFutureCFunction(L, luaJoin, lua_gettop(L));
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

		LuaModulePostSetup() {
			PersistenceNamespace("FutureModule");

			lua_pushcfunction(L, Future::structUnpersist);
			PersistValue("FutureStructUnpersist");
			lua_pushcfunction(L, Future::threadUnpersist);
			PersistValue("FutureThreadUnpersist");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(Future::awaitContinue)));
			PersistValue("FutureAwaitContinue");

			luaL_getmetatable(L, Future::_Name);
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			lua_pop(L, 1);
		}
	}

	FLuaFuture* luaFIN_pushFutureInternal(lua_State* L, const FLuaFuture& futurePtr) {
		FLuaFuture* future = static_cast<FLuaFuture*>(lua_newuserdatauv(L, sizeof(FLuaFuture), 2));
		new (future) FLuaFuture(futurePtr);
		luaL_setmetatable(L, FutureModule::Future::_Name);
		return future;
	}

	void luaFIN_pushFuture(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future) {
		FLuaFuture* future = luaFIN_pushFutureInternal(L, MakeShared<TFIRInstancedStruct<FFINFuture>>(Future));
		if (!(**future)->IsDone()) {
			UFINKernelSystem* kernel = UFINLuaProcessor::luaGetProcessor(L)->GetKernel();
			kernel->PushFuture(*future);
		}
	}

	const FLuaFuture& luaFIN_checkFuture(lua_State* L, int Index) {
		FLuaFuture& future = *static_cast<FLuaFuture*>(luaL_checkudata(L, Index, FutureModule::Future::_Name));
		return future;
	}

	void luaFIN_pushLuaFuture(lua_State* L, int Thread) {
		Thread = lua_absindex(L, Thread);
		luaL_checktype(L, Thread, LUA_TTHREAD);
		luaFIN_pushFutureInternal(L, nullptr);
		lua_pushvalue(L, Thread);
		lua_setiuservalue(L, -2, 1);
	}

	void luaFIN_pushLuaFutureCFunction(lua_State* L, lua_CFunction Func, int args) {
		lua_State* thread = lua_newthread(L);
		lua_pushcfunction(thread, Func);
		lua_insert(L, -args-1);
		lua_xmove(L, thread, args);
		luaFIN_pushLuaFuture(L, -1);
	}
}
