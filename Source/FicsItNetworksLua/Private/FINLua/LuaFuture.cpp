#include "FINLua/LuaFuture.h"

#include "FINLuaProcessor.h"
#include "LuaKernelAPI.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"

namespace FINLua {
	// Forward Declarations for Persistence
	int lua_futureStruct(lua_State* L);
	int lua_futureStructContinue(lua_State* L, int, lua_KContext);
	int eventLoopContinueCallbacksContinue(lua_State* L, int, lua_KContext);
	int futureRunTasksContinue(lua_State*L, int, lua_KContext);
	int luaFIN_callbackPollContinue(lua_State*L, int, lua_KContext);

	LuaModule(R"(/**
	 * @LuaModule		FutureModule
	 * @DisplayName		Future Module
	 *
	 * @Dependency		KernelModule
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
				FFINLuaRuntimePersistenceState& persistence = luaFIN_getPersistence(L);

				luaFIN_pushFuture(L, *persistence.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1))));

				return 1;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				FLuaFuture future = luaFIN_checkFutureStruct(L, 1);

				FFINLuaRuntimePersistenceState& storage = luaFIN_getPersistence(L);
				lua_pushinteger(L, storage.Add(future));
				lua_pushcclosure(L, unpersist, 1);

				return 1;
			}
		}

		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	FutureDependents
		 * @DisplayName		Future Struct
		 */)", FutureDependents) {
			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	__mode
			 * @DisplayName		Mode
			 */)", __mode) {
				lua_pushstring(L, "k");
			}
		}

		/**
		 * Additionally to the below documentation.
		 *
		 * Generic Futures have to manage who they have to notify themselves.
		 */
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	Future
		 * @DisplayName		Future
		 *
		 * Represents some action to be executed at some point.
		 * Provides an interface to drive execution forward, check for completion and return retrieval.
		 * Futures are mostly used to retrieve data, usually you just want to use `await` function
		 * to yield the current thread until the values become available.
		 *
		 * If the associated actions get executed is defined by the variation of the future, by the creator of the future.
		 * Some Futures may only require to exist, some can be destroyed but the action still will be executed
		 * and others require you to await/poll for them.
		 *
		 * Polling a future using the `poll` function allows you to drive its execution forward if necessary,
		 * but most importantly, allows you to check if it finished execution
		 * You can always poll a future.
		 *
		 * To retrieve values that may be returned by the Future, use the `get` function which causes an error if the future is not ready.
		 *
		 * Actively Polling a future is in most cases quite inefficient.
		 * Use the `await` function to yield the current thread until the future is ready.
		 * It will then also return the return values of the future.
		 * If the future caused an error, the `await` function will propagate the error further.
		 *
		 * Some functions are aware when the get closed, allowing you to have more control over the cancellation of a future.
		 * Every Future gets cancelled on garbage collection, but only some actually care about getting cancelled.
		 *
		 * A Future essentially wraps a thread/coroutine.
		 * When a future yields, the future can be considered pending,
		 * when it returns, the future is Finished,
		 * when a future fails, the future failed.
		 * A future can be actively polled by the runtime or may wait to be woken up by some other future or external system.
		 * For this, the values a future yields are used to control its runtime.
		 * - <nothing> indicates the future should be actively polled. This practically means it gets added as task.
		 * - future    indicates the future is waiting for the given future. When the future gets polled using await or as task, this will make this future be woken up by the given future and be removed as task.
		 * - number    indicates the future is waiting to be woken up by some external system, but if its a task, allows to indicate the runtime its fine to sleep for the given amount of seconds
		 * - <any>     indicates the future is waiting to be woken up by some external system
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
			 *
			 * Get value from the future if one is available.
			 * Causes error if future is not yet resolved.
			 * 
			 * @param		self		Future
			 * @return		...			any			Value		Future's value
			 */)", get) {
				luaL_checkudata(L, 1, _Name);
				lua_getuservalue(L, 1);
				lua_State* thread = lua_tothread(L, -1);
				EFutureState state = luaFIN_getFutureState(thread);
				switch (state) {
				case Future_Ready: {
					lua_getiuservalue(L, 1, 2);
					int table = lua_absindex(L, -1);
					int len = luaL_len(L, table);
					for (int i = 1; i <= len; ++i) {
						lua_geti(L, table, i);
					}
					return len;
				}
				case Future_Failed:
					return luaL_error(L, "Tried to get results of failed future");
				default:
					return luaL_error(L, "Tried to get results of pending future");
				}
			}

			/**
			 * A Lua Function.
			 * Polls the Lua Future at the index 1.
			 * Returns true if the future failed or finished successfully.
			 * Additionally, the futures yield values are returned.
			 *
			 * Semantic of yield values:
			 * - `<nothing>` indicates the future is not waiting for anything and needs to be actively polled
			 * - `nil`       indicates the future is waiting to be woken up by some other system
			 * - `future`    indicates the future is waiting for the given future to finish
			 *
			 * May yield if the thread yields by hook.
			 */
			int poll_continue(lua_State* L, int, lua_KContext ctx);
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		boolean,number?			poll()
			 * @DisplayName		Poll
			 * 
			 * @param		self		Future
			 * @return		ready		boolean		Ready		Whether the future is ready or not
			 * @return		future		Future?		Future		A future this future is awaiting on
			 */)", poll) {
				luaL_checkudata(L, 1, _Name);
				lua_getuservalue(L, 1);
				lua_State* thread = lua_tothread(L, -1);
				EFutureState futureState = luaFIN_getFutureState(thread);
				switch (futureState) {
					case Future_Pending:
						break;
					default:
						lua_pushboolean(L, true);
						return 1;
				}

				// Is Future Sleeping?
				lua_getiuservalue(L, 1, 3);
				lua_getfield(L, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_HIDDENGLOBALS);
				lua_getfield(L, -1, LUAFIN_HIDDENGLOBAL_FUTUREREGISTRY);
				lua_pushthread(L);
				if (lua_gettable(L, -2) != LUA_TNIL) {
					if (lua_gettable(L, -4) != LUA_TNIL) {
						lua_settop(L, 2);

						lua_pushboolean(L, false);
						lua_pushvalue(L, 1);
						return 2;
					}

				}
				lua_settop(L, 2);

				return poll_continue(L, 0, NULL);
			}
			int poll_continue(lua_State* L, int, lua_KContext ctx) {
				lua_State* thread = lua_tothread(L, -1);
				int narg = lua_status(thread) == LUA_OK ? lua_gettop(thread)-1 : 0;
				int nres;
				int status = lua_resume(thread, L, narg, &nres);
				switch (status) {
					case LUA_YIELD: {
						if (nres < 1) { // Hook Yield
							return lua_yieldk(L, 0, NULL, poll_continue);
						} else { // User Yield
							lua_pushboolean(L, false);
							lua_xmove(thread, L, nres-1);
							lua_pop(thread, 1);
							return 1 + nres-1;
						}
					}
					case LUA_OK: {
						// Copy Future Results
						lua_newtable(L);
						int table = lua_absindex(L, -1);
						lua_xmove(thread, L, nres);
						for (int i = nres; i > 0; --i) {
							lua_seti(L, table, i);
						}
						lua_setiuservalue(L, 1, 2);
					}
					default:
						// Queue dependant Callbacks
						lua_getiuservalue(L, 1, 3);
						lua_pushnil(L);
						while (lua_next(L, -2) != 0) {
							lua_pop(L, 1);
							lua_pushvalue(L, -1);
							lua_pushcclosure(L, luaFIN_callbackPoll, 1);
							luaFIN_pushCallback(L);
						}
						lua_newtable(L);
						lua_setiuservalue(L, 1, 3);

						lua_pushboolean(L, true);
						return 1;
				}
			}

			/**
			 * If self is already finished or failed: returns respective values or propagates error
			 * Otherwise resumes the thread:
			 * 	Finished: copy values, mark self as done, queue callbacks, return values
			 * 	Error: propagate error
			 * 	Hook Yield: propagate hook yield -> continue await
			 * 	User Yield:
			 *		if exactly one future: poll future
			 *			if future finished: similar to success
			 *			if future failure: similar to failure
			 *			else: add self as dependant to future
			 *		else: schedule self deferred
			 */
			int await_continue(lua_State* L, int, lua_KContext);
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		await
			 * @DisplayName		Await
			 *
			 * Wait for the future to complete and return its value.
			 *
			 * @param		self		Future
			 * @return		...			any			Value		Future's value
			 */)", await) {
				lua_settop(L, 1);
				lua_getuservalue(L, 1);
				lua_pushcfunction(L, poll);
				lua_pushvalue(L, 1);
				lua_callk(L, 1, LUA_MULTRET, NULL, await_continue);
				return await_continue(L, 0, NULL);
			}
			int await_continue2(lua_State* L, int, lua_KContext);
			int await_continue(lua_State* L, int, lua_KContext) {
				bool status = static_cast<bool>(lua_toboolean(L, 3));
				lua_State* thread = lua_tothread(L, 2);

				if (status) { // Finished
					EFutureState state = luaFIN_getFutureState(thread);
					switch (state) {
						case Future_Failed:
							lua_xmove(thread, L, 1);
							return lua_error(L);
						case Future_Ready: {
							lua_getiuservalue(L, 1, 2);
							int table = lua_absindex(L, -1);
							int len = luaL_len(L, table);
							for (int i = 1; i <= len; ++i) {
								lua_geti(L, table, i);
							}
							return len;
						}
						default:
							return luaL_error(L, "poll reported finished, but future is not ready");
					}
				}

				// Pending
				if (lua_gettop(L) > 3) {
					if (luaL_testudata(L, 4, _Name)) {
						// Depends on Future
						luaFIN_futureDependsOn(L, 1, 4);
					} else if (lua_isnil(L, 4)) {
						// Wakes Up explicitly
					} else {
						// Needs active polling
						luaFIN_addTask(L, 1);
					}
				} else {
					// Needs active polling
					luaFIN_addTask(L, 1);
				}
				lua_settop(L, 2);
				lua_pushvalue(L, 1);
				return luaFIN_yield(L, 1, NULL, await_continue2);
			}
			int await_continue2(lua_State* L, int, lua_KContext) {
				return await(L);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		canGet
			 * @DisplayName		Can Get
			 *
			 * Check if the future's value is available without performing any additional logic.
			 *
			 * @param		self		Future
			 * @return		canGet		boolean		Can Get		True if future is completed and a value is available
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
				lua_newuserdatauv(L, 0, 3);
				luaL_setmetatable(L, _Name);
				lua_pushvalue(L, lua_upvalueindex(1));
				lua_setiuservalue(L, -2, 1);
				lua_pushvalue(L, lua_upvalueindex(2));
				lua_setiuservalue(L, -2, 2);
				lua_pushvalue(L, -1);
				lua_newtable(L);
				luaL_setmetatable(L, FutureDependents::_Name);
				lua_setiuservalue(L, -2, 3);
				luaFIN_pushPollCallback(L, -1);
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
			 * @parameter	thread		thread		Thread		The thread you want to wrap in a future
			 * @return		future		Future		Future		The Future that wraps the given thread
			 */)", async) {
				luaFIN_pushLuaFuture(L, 1);
				return 1;
			}

			int joinContinue(lua_State* L, int, lua_KContext) {
				bool bNotDone = true;
				int num = lua_gettop(L);
				for (int i = 1; i <= num && bNotDone; ++i) {
					if (luaL_testudata(L, i, Future::_Name) == nullptr) continue;
					lua_pushcfunction(L, &Future::poll);
					lua_callk(L, 0, 0, NULL, &joinContinue);
					lua_getiuservalue(L, i, 1);
					lua_State* thread = lua_tothread(L, -1);
					lua_pop(L, 1);
					EFutureState state = luaFIN_getFutureState(thread);
					switch (state) {
						case Future_Pending:
							bNotDone = false;
							break;
						case Future_Failed:
							lua_pushvalue(thread, -1);
							lua_xmove(thread, L, 1);
							return lua_error(L);
						case Future_Ready:
							lua_getiuservalue(L, i, 2);
							lua_replace(L, i);
							break;
					}
				}
				if (bNotDone) {
					return luaFIN_yield(L, 0, NULL, joinContinue);
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
			 * @parameter	...			Future		Futures		The futures you want to join
			 * @return		future		Future		Future		The Future that will finish once all other futures finished
			 */)", join) {
				luaFIN_pushLuaFutureCFunction(L, luaJoin, lua_gettop(L));
				return 1;
			}

			int luaSleepContinue(lua_State* L, int, lua_KContext) {
				double timeout = lua_tonumber(L, 1);
				double start = lua_tonumber(L, 2);
				double now = FPlatformTime::Seconds();
				if (now - start < timeout) {
					lua_pushnil(L);
					return luaFIN_yield(L, 1, NULL, luaSleepContinue);
				}
				return 0;
			}
			int luaSleep(lua_State* L) {
				double timeout = lua_tonumber(L, 1);
				double now = FPlatformTime::Seconds();
				lua_pushnumber(L, now);
				luaFIN_pushThisFuture(L);
				lua_pushcclosure(L, luaFIN_callbackPoll, 1);
				luaFIN_pushTimeout(L, timeout);
				return luaSleepContinue(L, 0, NULL);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Future	sleep(seconds: number)
			 * @DisplayName		Sleep
			 *
			 * Creates a future that returns after the given amount of seconds.
			 * 
			 * @parameter	seconds		number		Seconds		Number of seconds to wait
			 * @return		future		Future		Future		The future that will finish after the given amount of seconds
			 */)", sleep) {
				luaL_checktype(L, 1, LUA_TNUMBER);
				lua_settop(L, 1);
				luaFIN_pushLuaFutureCFunction(L, luaSleep, 1);
				return 1;
			}


			LuaModuleTableBareField(R"(/**
			 * @LuaBareValue	tasks	Future[]
			 * @DisplayName		Tasks
			 *
			 * A list of futures that are considered "Tasks".
			 * Tasks could be seen as background threads. Effectively getting "joined" together.
			 * Examples for tasks are callback invocations of timers and event listeners.
			 */)", tasks) {
				lua_newtable(L);
			}

			LuaModuleTableBareField(R"(/**
			 * @LuaBareValue	callbacks	function[]
			 * @DisplayName		Callbacks
			 */)", callbacks) {
				lua_newtable(L);
				luaFIN_persistValue(L, -1, "Future_callbacks");
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		addTask(Future...)
			 * @DisplayName		Add Task
			 *
			 * Adds the given futures to the tasks list.
			 * 
			 * @parameter	...			Future		Futures		The futures you want to add
			 */)", addTask) {
				int num = lua_gettop(L);
				if (lua_getfield(L, lua_upvalueindex(2), "tasks") != LUA_TTABLE) {
					return luaL_typeerror(L, -1, "table");
				}
				lua_insert(L, 1);
				for (int i = num; i > 0; --i) {
					luaL_checkudata(L, i+1, Future::_Name);
					lua_pushnumber(L, 0);
					lua_settable(L, 1);
				}
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		run()
			 * @DisplayName		Run
			 *
			 * Runs the default task scheduler once.
			 *
			 * Returns true if there are still pending futures.
			 */)", run) {
				return luaFIN_futureRun(L);
			}

			int luaLoopContinue(lua_State* L, int, lua_KContext);
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		loop()
			 * @DisplayName		Loop
			 *
			 * Runs the default task scheduler indefinitely until no pending futures are left.
			 */)", loop) {
				lua_settop(L, 0);
				lua_pushcfunction(L, &luaFIN_futureRun);
				lua_callk(L, 0, 2, NULL, &luaLoopContinue);
				return luaLoopContinue(L, 0, NULL);
			}
			int luaLoopContinue(lua_State* L, int, lua_KContext) {
				if (static_cast<bool>(lua_toboolean(L, 1))) {
					return luaFIN_yield(L, 1, NULL, reinterpret_cast<lua_KFunction>(&loop));
				}
				return 0;
			}
		}

		int luaAsync(lua_State* L) {
			int top = lua_gettop(L);
			luaFIN_pushLuaFutureLuaFunction(L, top-1);
			return 1;
		}
		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		async	fun(fn, ...): Future
		 * @DisplayName		Async
		 * 
		 * Wraps a function into a future.
		 */)", async) {
			lua_pushcfunction(L, luaAsync);
			luaFIN_persistValue(L, -1, PersistName);
		}

		int luaSleep(lua_State* L) {
			luaL_checktype(L, 1, LUA_TNUMBER);
			lua_settop(L, 1);
			double timeout = lua_tonumber(L, 1);
			luaFIN_pushLuaFutureCFunction(L, future::luaSleep, 1);
			return luaFIN_await(L, 1);
		}
		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		sleep	fun(seconds: number): Future
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

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(Future::poll_continue)));
			PersistValue("FuturePollContinue");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(Future::await_continue)));
			PersistValue("FutureAwaitContinue");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(Future::await_continue2)));
			PersistValue("FutureAwaitContinue2");

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

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(future::luaLoopContinue)));
			PersistValue("LoopContinue");


			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(eventLoopContinueCallbacksContinue)));
			PersistValue("eventLoopContinueCallbacksContinue");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(futureRunTasksContinue)));
			PersistValue("futureRunTasksContinue");

			lua_pushcfunction(L, luaFIN_callbackPoll);
			PersistValue("callbackPoll");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_callbackPollContinue)));
			PersistValue("callbackPollContinue");

			lua_getfield(L, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_HIDDENGLOBALS); // -1
			lua_newtable(L); // -2
			lua_newtable(L); // -3
			lua_newtable(L); // -4
			lua_pushstring(L, "kv"); // -5
			lua_setfield(L, -2, "__mode"); // -4
			//lua_pushvalue(L, -2); // -5
			lua_setmetatable(L, -2); // -3
			lua_setfield(L, -3, LUAFIN_HIDDENGLOBAL_FUTUREREGISTRY); // -2
			//lua_setmetatable(L, -2); // -2
			lua_setfield(L, -2, LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY);
		}
	}

	void luaFIN_pushFutureStruct(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future) {
		FLuaFuture* future = static_cast<FLuaFuture*>(lua_newuserdata(L, sizeof(FLuaFuture)));
		new (future) FLuaFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(Future));
		luaL_setmetatable(L, FutureModule::FutureStruct::_Name);
		if (!(**future)->IsDone()) {
			UFINKernelSystem* kernel = luaFIN_getKernel(L);
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
		lua_newuserdatauv(L, 0, 3);
		luaL_setmetatable(L, FutureModule::Future::_Name);
		lua_pushvalue(L, Thread);
		lua_setiuservalue(L, -2, 1);
		lua_newtable(L);
		luaL_setmetatable(L, FutureModule::FutureDependents::_Name);
		lua_setiuservalue(L, -2, 3);

		lua_getfield(L, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_HIDDENGLOBALS);
		lua_getfield(L, -1, LUAFIN_HIDDENGLOBAL_FUTUREREGISTRY);
		lua_pushvalue(L, Thread);
		lua_pushvalue(L, -4);
		lua_settable(L, -3);
		lua_pop(L, 2);
	}

	void luaFIN_pushLuaFutureLuaFunction(lua_State* L, int args) {
		luaL_checktype(L, -args-1, LUA_TFUNCTION);
		lua_State* thread = lua_newthread(L);
		lua_insert(L, -args-2);
		lua_xmove(L, thread, 1 + args);
		luaFIN_pushLuaFuture(L, -1);
	}

	void luaFIN_pushLuaFutureCFunction(lua_State* L, lua_CFunction Func, int args, int upvals) {
		args = FMath::Min(args, lua_gettop(L));
		lua_State* thread = lua_newthread(L);   // ..., args, upvals, thread
		lua_insert(L, -args-upvals-1);          // ..., thread, args, upvals
		lua_xmove(L, thread, upvals);   // ..., thread, args
		lua_pushcclosure(thread, Func, upvals);
		lua_xmove(L, thread, args);
		luaFIN_pushLuaFuture(L, -1);
		lua_remove(L, -2);
	}

	int luaFIN_await(lua_State* L, int index) {
		lua_settop(L, index);
		lua_insert(L, 1);
		lua_settop(L, 1);
		return FutureModule::Future::await(L);
	}

	void luaFIN_addTask(lua_State* L, int index) {
		index = lua_absindex(L, index);
		luaL_checkudata(L, index, FutureModule::Future::_Name);
		lua_getglobal(L, "future");
		lua_getfield(L, -1, "tasks");
		lua_pushvalue(L, index);
		lua_pushnumber(L, 0);
		lua_settable(L, -3);
		lua_pop(L, 2);
	}

	int eventLoopContinueCallbacksContinue(lua_State* L, int, lua_KContext);
	int eventLoopContinueCallbacks(lua_State* L) {
		while (int len_callbacks = luaL_len(L, 1)) {
			lua_geti(L, 1, len_callbacks);
			lua_pushnil(L);
			lua_seti(L, 1, len_callbacks);
			luaL_checktype(L, -1, LUA_TFUNCTION);
			lua_callk(L, 0, 0, 0, &eventLoopContinueCallbacksContinue);
		}
		return 0;
	}
	int eventLoopContinueCallbacksContinue(lua_State* L, int, lua_KContext) {
		return eventLoopContinueCallbacks(L);
	}

	int futureRunTasks(lua_State*L, int, lua_KContext);
	int luaFIN_futureRun(lua_State* L) {
		lua_getglobal(L, "future");
		if (lua_getfield(L, -1, "callbacks") != LUA_TTABLE) {
			return luaL_typeerror(L, -1, "table");
		}

		int callbacks = lua_gettop(L);

		lua_pushcfunction(L, eventLoopContinueCallbacks);
		lua_pushvalue(L, callbacks);
		lua_callk(L, 1, 0, NULL, futureRunTasks);

		return futureRunTasks(L, 0, NULL);
	}
	int futureRunTasksContinue(lua_State* L, int, lua_KContext);
	int futureRunTasks(lua_State* L, int, lua_KContext) {
		lua_settop(L, 1);
		if (lua_getfield(L, -1, "tasks") != LUA_TTABLE) {
			return luaL_typeerror(L, -1, "table");
		}
		lua_pushnil(L);
		lua_pushnumber(L, TNumericLimits<LUA_NUMBER>::Max());
		lua_replace(L, 1);
		return futureRunTasksContinue(L, 0, NULL);
	}
	void futureRunTasksResult(lua_State* L) {   // timeout, tasks, prev_key, ...
		int top = lua_gettop(L);
		if (top > 3) {
			// task polled
			bool bDelete = false;
			if (lua_toboolean(L, 4)) {
				// Finished
				bDelete = true;
			} else {
				// Pending
				if (top > 4) {
					if (luaL_testudata(L, 5, FutureModule::Future::_Name)) {
						// Waiting for future
						luaFIN_futureDependsOn(L, 3, 5);
						bDelete = true;
					} else if (lua_isnumber(L, 5)) {
						// Active Polling with Timeout
						if (lua_isnumber(L, 1)) {
							double current = lua_tonumber(L, 1);
							double timeout = lua_tonumber(L, 5);
							if (timeout < current) {
								lua_settop(L, 5);
								lua_replace(L, 1);
							}
						}
					} else if (lua_isnil(L, 5)) {
						// Waiting for Wakeup
						bDelete = true;
					} else {
						// Active Polling
						lua_pushnil(L);
						lua_replace(L, 1);
					}
				} else {
					// Active Polling
					lua_pushnil(L);
					lua_replace(L, 1);
				}
			}
			lua_settop(L, 3);
			if (bDelete) {
				lua_pushvalue(L, -1);
			}
		}
	}
	int futureRunTasksContinue(lua_State* L, int, lua_KContext) {
		                                   // timeout, tasks, prev_key, ...
		futureRunTasksResult(L);
		if (lua_gettop(L) > 2) {
			// timeout, tasks, (prev_key,) prev_key
			while (lua_next(L, 2) != 0) {  // timeout, tasks, (prev_key,) next_key, next_value
				lua_pop(L, 1);              // timeout, tasks, (prev_key,) next_key
				if (lua_gettop(L) > 3) {
					lua_rotate(L, -2, -1);   // timeout, tasks, next_key, prev_key

					// remove prev_key
					lua_pushnil(L);                 // timeout, tasks, next_key, prev_key, nil
					lua_settable(L, 2);         // timeout, tasks, next_key
				}

				lua_pushvalue(L, -1);       // timeout, tasks, next_key, next_key
				lua_pushcfunction(L, &FutureModule::Future::poll);
				lua_rotate(L, -2, 1);
				lua_callk(L, 1, LUA_MULTRET, NULL, futureRunTasksContinue);
				futureRunTasksResult(L); // timeout, tasks, next_key
			}
			if (lua_gettop(L) > 2) {
				// remove prev_key
				lua_pushnil(L);                 // timeout, tasks, prev_key, nil
				lua_settable(L, 2);         // timeout, tasks
			}
		}

		TOptional<double> timeout;
		if (lua_isnumber(L, 1)) {
			timeout = lua_tonumber(L, 1);
		}

		lua_settop(L, 0);
		lua_getglobal(L, "future");
		double pollTimeout = luaFIN_pollTimeouts(L);
		if (timeout && *timeout > pollTimeout) {
			timeout = pollTimeout;
		}

		if (lua_getfield(L, -1, "callbacks") != LUA_TTABLE) {
			return luaL_typeerror(L, -1, "table");
		}
		if (luaL_len(L, -1) > 0) {
			timeout.Reset();
		}
		lua_pop(L, 1);

		// TODO: Determine if there is anything left to do
		lua_pushboolean(L, true);
		if (timeout) {
			lua_pushnumber(L, *timeout);
			return 2;
		}
		return 1;
	}

	EFutureState luaFIN_getFutureState(lua_State* thread) {
		int status = lua_status(thread);
		switch (status) {
			default:
				return Future_Failed;
			case LUA_OK:
				if (lua_gettop(thread) == 0) {
					return Future_Ready;
				}
			return Future_Pending;
			case LUA_YIELD:
				return Future_Pending;
		}
	}

	int luaFIN_pushCallback(lua_State* L) {
		luaL_checktype(L, -1, LUA_TFUNCTION);
		lua_getglobal(L, "future");
		if (lua_getfield(L, -1, "callbacks") != LUA_TTABLE) {
			return luaL_typeerror(L, -1, "table");
		}
		lua_rotate(L, -3, -1),
		lua_seti(L, -2, luaL_len(L, -2)+1);
		lua_pop(L, 2);
		return 0;
	}

	int luaFIN_callbackPollContinue(lua_State* L, int, lua_KContext) {
		futureRunTasksResult(L);
		return 0;
	}
	int luaFIN_callbackPoll(lua_State* L) {
		lua_settop(L, 2);
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_pushcfunction(L, &FutureModule::Future::poll);
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_callk(L, 1, LUA_MULTRET, NULL, luaFIN_callbackPollContinue);
		return luaFIN_callbackPollContinue(L, 0, NULL);
	}

	void luaFIN_pushPollCallback(lua_State* L, int future) {
		lua_pushvalue(L, future);
		lua_pushcclosure(L, luaFIN_callbackPoll, 1);
		luaFIN_pushCallback(L);
	}

	bool luaFIN_pushThisFuture(lua_State* L) {
		lua_getfield(L, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_HIDDENGLOBALS);
		lua_getfield(L, -1, LUAFIN_HIDDENGLOBAL_FUTUREREGISTRY);
		lua_pushthread(L);
		int type = lua_gettable(L, -2);
		lua_insert(L, -3);
		lua_pop(L, 2);
		return type != LUA_TNIL;
	}

	void luaFIN_futureDependsOn(lua_State* L, int dependent, int dependency) {
		dependent = lua_absindex(L, dependent);
		dependency = lua_absindex(L, dependency);
		void* t1 = luaL_checkudata(L, dependent, FutureModule::Future::_Name);
		void* t2 = luaL_checkudata(L, dependency, FutureModule::Future::_Name);
		check(t1 != t2);
		lua_getiuservalue(L, dependency, 3);
		lua_pushvalue(L, dependent);
		lua_pushinteger(L, 0);
		lua_settable(L, -3);
		lua_pop(L, 1);
	}

	void luaFIN_futureDependsOn(lua_State* L, int dependency) {
		dependency = lua_absindex(L, dependency);
		if (luaFIN_pushThisFuture(L)) {
			luaL_checkudata(L, dependency, FutureModule::Future::_Name);
			lua_getiuservalue(L, dependency, 3);
			lua_insert(L, -2);
			lua_pushinteger(L, 0);
			lua_settable(L, -3);
		} else {
			luaL_error(L, "calling await within non-async function");
		}
	}
	double luaFIN_pollTimeouts(lua_State* L) {
		int top = lua_gettop(L);
		lua_getfield(L, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_HIDDENGLOBALS);
		lua_getfield(L, -1, LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY);

		float now = FPlatformTime::Seconds();

		double minTimeout = TNumericLimits<double>::Max();
		TArray<int> toDelete;
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_isinteger(L, -1)) {
				lua_pop(L, 1);
				continue;
			}
			int ref = lua_tointeger(L, -2);
			lua_geti(L, -1, 1);
			double timeout = lua_tonumber(L, -1);
			if (now >= timeout) {
				lua_geti(L, -2, 2);
				luaFIN_pushCallback(L);
				toDelete.Add(ref);
			} else {
				minTimeout = FMath::Min(minTimeout, timeout);
			}
			lua_pop(L, 2);
		}

		for (int ref : toDelete) {
			luaL_unref(L, -1, ref);
		}

		lua_pop(L, 2);

		return minTimeout;
	}

	void luaFIN_pushTimeout(lua_State* L, float timeout) {
		int func = lua_absindex(L, -1);
		luaL_checktype(L, func, LUA_TFUNCTION);
		lua_getfield(L, LUA_REGISTRYINDEX, LUAFIN_REGISTRYKEY_HIDDENGLOBALS);
		lua_getfield(L, -1, LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY);
		lua_newtable(L);
		lua_pushnumber(L, FPlatformTime::Seconds() + timeout);
		lua_seti(L, -2, 1);
		lua_rotate(L, func, -1);
		lua_seti(L, -2, 2);
		luaL_ref(L, -2);
		lua_pop(L, 2);
	}
}
