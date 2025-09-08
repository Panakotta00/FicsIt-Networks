#include "FINLua/LuaFuture.h"

#include "FicsItNetworksLuaModule.h"
#include "FINLuaProcessor.h"
#include "LuaKernelAPI.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"

namespace FINLua {
	// Forward Declarations for Persistence
	int lua_futureStruct(lua_State* L);
	int lua_futureStructContinue(lua_State* L, int, lua_KContext);
	int luaFIN_futureRun_continue1(lua_State*L, int, lua_KContext);
	int luaFIN_futureRun_continue2(lua_State*L, int, lua_KContext);
	int luaFIN_runCallbacks(lua_State* L);
	int luaFIN_runCallbacks_continue(lua_State* L, int, lua_KContext);
	int luaFIN_runTasks(lua_State*L);
	int luaFIN_runTasks_continue(lua_State*L, int, lua_KContext);
	int luaFIN_timeoutTask(lua_State*L, int, lua_KContext);

	LuaModule(R"(/**
	 * @LuaModule		FutureModule
	 * @DisplayName		Future Module
	 * @Dependency KernelModule
	 * This Module provides the Future type and all its necessary functionallity.
	 *
	 * The Future system of FicsIt-Networks combines ideas from Rust Async/Await, libuv and Python 2.0 Async.
	 * The most relevant participant is the Future (check the futures documentation on more insight on how it operates).
	 *
	 *
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

				luaFIN_pushFutureStruct(L, *persistence.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1))));

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
		 * - number    indicates the future is waiting to be woken up by some external system, but if its a task or called in the main thread, allows to indicate the runtime its fine to sleep for the given amount of seconds
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

						lua_closethread(thread, L);

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
				lua_State* thread = lua_tothread(L, 2);
				double timeout;
				switch (luaFIN_handlePollResults(L, 3, 1, 1, 1, &timeout)) {
					case 0: {
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
					default:
						lua_settop(L, 2);
						FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
						if (L == runtime.GetLuaThread()) {
							lua_pushcfunction(L, &luaFIN_futureRun);
							lua_callk(L, 0, LUA_MULTRET, NULL, &await_continue2);
							return await_continue2(L, 0, NULL);
						}
						lua_pushnil(L);
						lua_pushvalue(L, 1);
						return await_continue2(L, 0, NULL);
				}
			}
			int await_continue2(lua_State* L, int, lua_KContext) {
				if (lua_gettop(L) > 2) {
					if (lua_gettop(L) > 3) {
						lua_remove(L, -2);
					}
					return luaFIN_yield(L, 1, NULL, &await_continue2);
				}
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
				lua_State* thread = lua_tothread(L, -1);
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
				luaL_checktype(L, -1, LUA_TTHREAD);
				lua_setiuservalue(L, -2, 1);

				lua_pushvalue(L, lua_upvalueindex(2));
				lua_setiuservalue(L, -2, 2);

				lua_newtable(L);
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
				//lua_getiuservalue(L, 1, 3);
				//lua_getiuservalue(L, 1, 4);
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

			void joinHandleResults(lua_State* L) {
				int num = lua_tointeger(L, lua_upvalueindex(1));
				int top = lua_gettop(L);
				if (top > num) {
					int future = lua_tointeger(L, lua_upvalueindex(2));
					switch (luaFIN_handlePollResults(L, num+1, future, future, future)) {
						case 0: {
							lua_settop(L, num);
							lua_getiuservalue(L, future, 1);
							lua_State* thread = lua_tothread(L, -1);
							EFutureState state = luaFIN_getFutureState(thread);
							switch (state) {
								case Future_Failed:
									lua_xmove(thread, L, 1);
									lua_error(L);
									break;
								case Future_Ready: {
									lua_getiuservalue(L, 1, 2);
									lua_replace(L, future);
									break;
								}
								default:
									luaL_error(L, "poll reported finished, but future is not ready");
							}
							break;
						}
						default:
							lua_pushboolean(L, false);
							lua_replace(L, lua_upvalueindex(3));
							break;
					}
				}
				lua_settop(L, num);
			}
			int joinContinue(lua_State* L, int, lua_KContext) {
				joinHandleResults(L);
				int num = lua_gettop(L);
				for (int i = lua_tointeger(L, lua_upvalueindex(2))+1; i <= num; ++i) {
					if (luaL_testudata(L, i, Future::_Name) != nullptr) {
						lua_pushinteger(L, i);
						lua_replace(L, lua_upvalueindex(2));

						lua_pushcfunction(L, FutureModule::Future::poll);
						lua_pushvalue(L, i);
						lua_callk(L, 1, LUA_MULTRET, NULL, joinContinue);
						joinHandleResults(L);
					}
				}
				if (lua_toboolean(L, lua_upvalueindex(3))) {
					return lua_gettop(L);
				} else {
					lua_pushinteger(L, 0);
					lua_replace(L, lua_upvalueindex(2));
					lua_pushboolean(L, true);
					lua_replace(L, lua_upvalueindex(3));

					int futures = 0;
					for (int i = 1; i <= num; ++i) {
						if (luaL_testudata(L, i, Future::_Name) != nullptr) {
							lua_pushvalue(L, i);
							++futures;
						}
					}
					return luaFIN_yield(L, futures, NULL, joinContinue);
				}
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Future	join(Future...)
			 * @DisplayName		Join
			 *
			 * Creates a new Future that will only finish once all futures passed as parameters have finished.
			 * The return values of all futures will be packed into tables and returned in order.
			 *
			 * @parameter	...			Future		Futures		The futures you want to join
			 * @return		future		Future		Future		The Future that will finish once all other futures finished
			 */)", join) {
				int num = lua_gettop(L);
				lua_pushinteger(L, num);
				lua_pushinteger(L, 0);
				lua_pushboolean(L, true);
				luaFIN_pushLuaFutureCFunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(joinContinue)), num,  lua_gettop(L) - num);
				return 1;
			}

			void anyHandleResults(lua_State* L) {
				int num = lua_tointeger(L, lua_upvalueindex(1));
				int top = lua_gettop(L);
				if (top > num) {
					int future = lua_tointeger(L, lua_upvalueindex(2));
					switch (luaFIN_handlePollResults(L, num+1, future, future, future)) {
						case 0: {
							lua_pushboolean(L, true);
							lua_replace(L, lua_upvalueindex(3));

							lua_settop(L, num);
							lua_getiuservalue(L, future, 1);
							lua_State* thread = lua_tothread(L, -1);
							EFutureState state = luaFIN_getFutureState(thread);
							switch (state) {
								case Future_Failed:
									lua_xmove(thread, L, 1);
									lua_settop(thread, 0);
									lua_error(L);
									break;
								case Future_Ready: {
									lua_getiuservalue(L, 1, 2);
									lua_replace(L, future);
									break;
								}
								default:
									luaL_error(L, "poll reported finished, but future is not ready");
							}
							break;
						}
						default: break;
					}
				}
				lua_settop(L, num);
			}
			int anyContinue(lua_State* L, int, lua_KContext) {
				anyHandleResults(L);
				int num = lua_gettop(L);
				for (int i = lua_tointeger(L, lua_upvalueindex(2))+1; i <= num; ++i) {
					if (luaL_testudata(L, i, Future::_Name) != nullptr) {
						lua_pushinteger(L, i);
						lua_replace(L, lua_upvalueindex(2));

						lua_pushcfunction(L, FutureModule::Future::poll);
						lua_pushvalue(L, i);
						lua_callk(L, 1, LUA_MULTRET, NULL, anyContinue);
						anyHandleResults(L);
					}
				}
				if (lua_toboolean(L, lua_upvalueindex(3))) {
					return lua_gettop(L);
				} else {
					lua_pushinteger(L, 0);
					lua_replace(L, lua_upvalueindex(2));

					int futures = 0;
					for (int i = 1; i <= num; ++i) {
						if (luaL_testudata(L, i, Future::_Name) != nullptr) {
							lua_pushvalue(L, i);
							++futures;
						}
					}
					return luaFIN_yield(L, futures, NULL, anyContinue);
				}
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Future	any(Future...)
			 * @DisplayName		Any
			 *
			 * Creates a new Future that will finish once any of the passed futures has finished.
			 * The other futures will be ignored.
			 * The future will return all futures and the table containing the results of the one future that finished in order.
			 *
			 * @parameter	...			Future		Futures		The futures you want to wait for any of
			 * @return		future		Future		Future		The Future that will finish once any future finished
			 */)", any) {
				int num = lua_gettop(L);
				lua_pushinteger(L, num);
				lua_pushinteger(L, 0);
				lua_pushboolean(L, false);
				luaFIN_pushLuaFutureCFunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(anyContinue)), num, lua_gettop(L) - num);
				return 1;
			}

			int luaSleepContinue(lua_State* L, int, lua_KContext) {
				double timeout = lua_tonumber(L, 1);
				double now = FPlatformTime::Seconds();
				if (now < timeout) {
					lua_pushnumber(L, timeout);
					return luaFIN_yield(L, 1, NULL, luaSleepContinue);
				}
				return 0;
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
				double timeout = FPlatformTime::Seconds() + luaL_checknumber(L, 1);
				lua_settop(L, 0);
				lua_pushnumber(L, timeout);
				luaFIN_pushLuaFutureCFunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaSleepContinue)), 1);
				luaFIN_pushTimeout(L, -1, timeout);
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
				lua_callk(L, 0, LUA_MULTRET, NULL, &luaLoopContinue);
				return luaLoopContinue(L, 0, NULL);
			}
			int luaLoopContinue(lua_State* L, int, lua_KContext) {
				if (static_cast<bool>(lua_toboolean(L, 1))) {
					return luaFIN_yield(L, lua_gettop(L)-1, NULL, reinterpret_cast<lua_KFunction>(&loop));
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
			future::sleep(L);
			return luaFIN_await(L, -1);
		}
		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		sleep	fun(seconds: number)
		 * @DisplayName		Sleep
		 *
		 * Blocks the current thread/future until the given amount of time passed
		 */)", sleep) {
			lua_pushcfunction(L, luaSleep);
			luaFIN_persistValue(L, -1, PersistName);
		}

		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		timeoutTask	Future
		 * @DisplayName		Timeout Task
		 *
		 * A future that is used as task to handle Timeouts.
		 */)", timeoutTask) {
			lua_pushnil(L);
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

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(future::joinContinue)));
			PersistValue("JoinContinue");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(future::anyContinue)));
			PersistValue("AnyContinue");

			lua_pushcfunction(L, lua_futureStruct);
			PersistValue("LuaFutureStruct");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(lua_futureStructContinue)));
			PersistValue("LuaFutureStructContinue");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(future::luaSleepContinue)));
			PersistValue("FutureSleepContinue");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(future::luaLoopContinue)));
			PersistValue("LoopContinue");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_futureRun_continue1)));
			PersistValue("luaFIN_futureRun_continue1");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_futureRun_continue2)));
			PersistValue("luaFIN_futureRun_continue2");

			lua_pushcfunction(L, luaFIN_callbackPoll);
			PersistValue("luaFIN_callbackPoll");
			lua_pushcfunction(L, luaFIN_runCallbacks);
			PersistValue("luaFIN_runCallbacks");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_runCallbacks_continue)));
			PersistValue("luaFIN_runCallbacks_continue");

			lua_pushcfunction(L, luaFIN_runTasks);
			PersistValue("luaFIN_runTasks");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_runTasks_continue)));
			PersistValue("luaFIN_runTasks_continue");

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_timeoutTask)));
			PersistValue("luaFIN_pollTimeouts");

			lua_geti(L, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS);
			lua_newtable(L);
			lua_setfield(L, -2, LUAFIN_HIDDENGLOBAL_FUTUREREGISTRY);
			lua_newtable(L);
			lua_setfield(L, -2, LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY);

			luaFIN_getFutureDelegate(L); // Ensure delegate got pushed

			lua_getglobal(L, "future");
			luaFIN_pushLuaFutureCFunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_timeoutTask)), 0);
			lua_setfield(L, -2, "timeoutTask");
			lua_pop(L, 1);
		}
	}

	void luaFIN_pushFutureStruct(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future) {
		FLuaFuture* future = static_cast<FLuaFuture*>(lua_newuserdata(L, sizeof(FLuaFuture)));
		new (future) FLuaFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(Future));
		luaL_setmetatable(L, FutureModule::FutureStruct::_Name);
		if (!(**future)->IsDone()) {
			luaFIN_getFutureDelegate(L).Broadcast(*future);
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
		lua_setiuservalue(L, -2, 3);
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

	int luaFIN_handlePollResults(lua_State* L, int index, TOptional<int> dependant, TOptional<int> setAsTask, TOptional<int> unsetAsTask, double* timeout) {
		index = lua_absindex(L, index);
		if (lua_toboolean(L, index)) {
			// Finished
			return 0;
		} else {
			// Pending
			if (luaL_testudata(L, index+1, FutureModule::Future::_Name)) {
				// Waiting for future

				if (dependant) {
					int num = lua_gettop(L);
					for (int i = index+1; i <= num; ++i) {
						if (luaL_testudata(L, i, FutureModule::Future::_Name)) {
							luaFIN_futureDependsOn(L, *dependant, i);
						}
					}
				}
				if (unsetAsTask) {
					luaFIN_removeTask(L, *unsetAsTask);
				}

				return 1;
			} else if (lua_isnil(L, index+1)) {
				// Waiting for Wakeup

				if (unsetAsTask) {
					luaFIN_removeTask(L, *unsetAsTask);
				}

				return 2;
			} else if (lua_isnumber(L, index+1)) {
				// Active Polling with Timeout

				if (timeout) {
					*timeout = lua_tonumber(L, index+1);
				}
				if (setAsTask) {
					luaFIN_addTask(L, *setAsTask);
				}

				return 3;
			} else {
				// Active Polling

				if (setAsTask) {
					luaFIN_addTask(L, *setAsTask);
				}

				return 4;
			}
		}
	}

	int luaFIN_await(lua_State* L, int index) {
		lua_settop(L, index);
		lua_insert(L, 1);
		lua_settop(L, 1);
		return FutureModule::Future::await(L);
	}

	void luaFIN_await(lua_State* L, int index, lua_KContext ctx, lua_KFunction func) {
		lua_pushcfunction(L, &FutureModule::Future::await);
		lua_insert(L, -2);
		lua_callk(L, 1, LUA_MULTRET, ctx, func);
	}

	void luaFIN_futureDependsOn(lua_State* L, int dependant, int dependency) {
		dependant = lua_absindex(L, dependant);
		dependency = lua_absindex(L, dependency);
		void* t1 = luaL_checkudata(L, dependant, FutureModule::Future::_Name);
		void* t2 = luaL_checkudata(L, dependency, FutureModule::Future::_Name);
		if (t1 == t2) {
			luaFIN_warning(L, "Future tried to depend on it self", true);
			return;
		}
		lua_getiuservalue(L, dependency, 3);
		lua_pushvalue(L, dependant);
		lua_pushinteger(L, 0);
		lua_settable(L, -3);
		lua_pop(L, 1);
	}

	int luaFIN_futureRun(lua_State* L) {
		lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_runCallbacks)));
		lua_callk(L, 0, 0, NULL, luaFIN_futureRun_continue1);
		return luaFIN_futureRun_continue1(L, 0, NULL);
	}
	int luaFIN_futureRun_continue1(lua_State* L, int, lua_KContext) {
		lua_pushcfunction(L, luaFIN_runTasks);
		lua_callk(L, 0, 1, NULL, luaFIN_futureRun_continue2);
		return luaFIN_futureRun_continue2(L, 0, NULL);
	}
	int luaFIN_futureRun_continue2(lua_State* L, int, lua_KContext) {
		TOptional<double> timeout;

		// Get allowed timeout from tasks
		if (lua_isnumber(L, -1)) {
			timeout = lua_tonumber(L, -1);
		}

		// Disable timeout if callbacks exist
		lua_settop(L, 0);
		lua_getglobal(L, "future");
		if (lua_getfield(L, -1, "callbacks") != LUA_TTABLE) {
			return luaL_typeerror(L, -1, "table");
		}
		if (luaL_len(L, -1) > 0) {
			timeout.Reset();
		}
		lua_pop(L, 2);

		// TODO: Determine if there is anything left to do
		lua_pushboolean(L, true);
		if (timeout) {
			lua_pushnumber(L, *timeout);
			return 2;
		}
		return 1;
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

	void luaFIN_addBackgroundTask(lua_State* L, int index) {
		luaFIN_addTask(L, index);
	}

	void luaFIN_removeTask(lua_State* L, int index) {
		index = lua_absindex(L, index);
		lua_getglobal(L, "future");
		lua_getfield(L, -1, "tasks");
		lua_pushvalue(L, index);
		lua_pushnil(L);
		lua_settable(L, -3);
		lua_pop(L, 2);
	}

	int luaFIN_runTasks(lua_State* L) {
		lua_settop(L, 0);
		lua_getglobal(L, "future");
		if (lua_getfield(L, -1, "tasks") != LUA_TTABLE) {
			return luaL_typeerror(L, -1, "table");
		}
		lua_pushnil(L);
		lua_pushnumber(L, TNumericLimits<LUA_NUMBER>::Max());
		lua_replace(L, 1);
		return luaFIN_runTasks_continue(L, 0, NULL);
	}
	void futureRunTasksResult(lua_State* L) {   // timeout, tasks, prev_key, ...
		if (lua_gettop(L) > 3) {
			double timeout;
			// We do not unset because we are within a task iteration loop, removing the task here would be bad and break the loop
			switch (luaFIN_handlePollResults(L, 4, 3, 3, {}, &timeout)) {
				case 0: {
					lua_getuservalue(L, 3);
					lua_State* thread = lua_tothread(L, -1);
					lua_pop(L, 1);
					EFutureState state = luaFIN_getFutureState(thread);
					if (state == Future_Failed) {
						lua_xmove(thread, L, 1);
						lua_error(L);
					}
				}
				case 1:
				case 2:
					lua_settop(L, 3);
					lua_pushvalue(L, -1);
					break;
				case 3:
					if (lua_isnumber(L, 1)) {
						double current = lua_tonumber(L, 1);
						if (timeout < current) {
							lua_settop(L, 5);
							lua_replace(L, 1);
						}
					}
					lua_settop(L, 3);
					break;
				case 4:
					lua_pushnil(L);
					lua_replace(L, 1);
					lua_settop(L, 3);
					break;
				default:
					lua_settop(L, 3);
					break;
			}
		}
	}
	int luaFIN_runTasks_continue(lua_State* L, int, lua_KContext) {
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
				lua_callk(L, 1, LUA_MULTRET, NULL, luaFIN_runTasks_continue);
				futureRunTasksResult(L); // timeout, tasks, next_key
			}
			if (lua_gettop(L) > 2) {
				// remove prev_key
				lua_pushnil(L);                 // timeout, tasks, prev_key, nil
				lua_settable(L, 2);         // timeout, tasks
			}
		}
		lua_settop(L, 1);
		return 1;
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

	int luaFIN_callbackPoll_continue(lua_State* L, int, lua_KContext) {
		futureRunTasksResult(L);
		return 0;
	}
	int luaFIN_callbackPoll(lua_State* L) {
		lua_settop(L, 2);
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_pushcfunction(L, &FutureModule::Future::poll);
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_callk(L, 1, LUA_MULTRET, NULL, luaFIN_callbackPoll_continue);
		return luaFIN_callbackPoll_continue(L, 0, NULL);
	}

	void luaFIN_pushPollCallback(lua_State* L, int future) {
		lua_pushvalue(L, future);
		lua_pushcclosure(L, luaFIN_callbackPoll, 1);
		luaFIN_pushCallback(L);
	}

	int luaFIN_runCallbacks(lua_State* L) {
		lua_settop(L, 0);
		lua_getglobal(L, "future");
		if (lua_getfield(L, -1, "callbacks") != LUA_TTABLE) {
			return luaL_typeerror(L, -1, "table");
		}
		lua_newtable(L);
		lua_setfield(L, 1, "callbacks");
		lua_remove(L, 1);
		lua_pushinteger(L, 1);
		return luaFIN_runCallbacks_continue(L, 0, NULL);
	}
	int luaFIN_runCallbacks_continue(lua_State* L, int, lua_KContext) {
		int i = lua_tointeger(L, 2);
		while (i <= luaL_len(L, 1)) {
			lua_geti(L, 1, i);
			lua_pushinteger(L, ++i);
			lua_replace(L, 2);
			luaL_checktype(L, -1, LUA_TFUNCTION);
			lua_callk(L, 0, 0, 0, &luaFIN_runCallbacks_continue);
		}
		return 0;
	}

	void luaFIN_pushTimeout(lua_State* L, int index, double timeout) {
		index = lua_absindex(L, index);
		luaL_checkudata(L, index, FutureModule::Future::_Name);
		lua_geti(L, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS);
		lua_getfield(L, -1, LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY);
		lua_pushvalue(L, index);
		lua_pushnumber(L, timeout);
		lua_settable(L, -3);
		lua_pop(L, 2);

		lua_getglobal(L, "future");
		lua_getfield(L, -1, "timeoutTask");
		luaFIN_pushPollCallback(L, -1);
		lua_pop(L, 2);
	}

	int luaFIN_timeoutTask(lua_State* L, int, lua_KContext) {
		lua_settop(L, 0);
		lua_geti(L, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS);
		lua_getfield(L, -1, LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY);

		float now = FPlatformTime::Seconds();

		TOptional<double> minTimeout;

		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			double timeout = lua_tonumber(L, -1);
			lua_pop(L, 1);

			if (lua_gettop(L) > 3) {
				lua_rotate(L, -2, -1);
				lua_pushnil(L);
				lua_settable(L, 2);
			}

			if (now >= timeout) {
				lua_pushvalue(L, -1);
				luaFIN_pushPollCallback(L, -1);
			} else {
				if (minTimeout) {
					minTimeout = FMath::Min(*minTimeout, timeout);
				} else {
					minTimeout = timeout;
				}
			}
		}

		if (lua_gettop(L) > 2) {
			lua_pushnil(L);
			lua_settable(L, 2);
		}

		if (minTimeout) {
			lua_pushnumber(L, *minTimeout);
			return luaFIN_yield(L, 1, NULL, luaFIN_timeoutTask);
		} else {
			lua_pushnil(L);
			return luaFIN_yield(L, 1, NULL, luaFIN_timeoutTask);
		}
	}

	FLuaFutureDelegate& luaFIN_createFutureDelegate(lua_State* L) {
		FLuaFutureDelegate* delegate = (FLuaFutureDelegate*)lua_newuserdata(L, sizeof(FLuaFutureDelegate));
		new (delegate) FLuaFutureDelegate();
		lua_newtable(L);
		lua_pushcfunction(L, [](lua_State* L) {
			FLuaFutureDelegate* delegate = (FLuaFutureDelegate*)lua_touserdata(L, 1);
			delegate->~TMulticastDelegate();
			return 0;
		});
		luaFIN_persistValue(L, -1, TEXT("FutureDelegate__gc"));
		lua_setfield(L, -2, "__gc");
		lua_setmetatable(L, -2);
		luaFIN_persistValue(L, -1, TEXT("FutureDelegate"));
		lua_seti(L, LUA_REGISTRYINDEX, LUAFIN_RIDX_FUTUREDELEGATE);
		return *delegate;
	}

	FLuaFutureDelegate& luaFIN_getFutureDelegate(lua_State* L) {
		lua_geti(L, LUA_REGISTRYINDEX, LUAFIN_RIDX_FUTUREDELEGATE);
		FLuaFutureDelegate* delegate = static_cast<FLuaFutureDelegate*>(lua_touserdata(L, -1));
		if (!delegate) luaL_error(L, "Future Delegate not valid!");
		lua_pop(L, 1);
		return *delegate;
	}
}
