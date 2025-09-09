#pragma once

#include "FINFuture.h"
#include "LuaUtil.h"

#define LUAFIN_HIDDENGLOBAL_FUTUREREGISTRY "future-registry"
#define LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY "timeout-registry"

namespace FINLua {
	typedef TSharedRef<TFIRInstancedStruct<FFINFuture>> FLuaFuture;
	typedef TMulticastDelegate<void(const FLuaFuture&)> FLuaFutureDelegate;

	enum EFutureState {
		Future_Pending,
		Future_Ready,
		Future_Failed,
	};

	FICSITNETWORKSLUA_API void luaFIN_pushFutureStruct(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future);

	/**
	 * @brief Pushes a FFINFuture on to the Lua Stack.
	 * @param L the lua state
	 * @param Future the Future Struct you want to push onto the lua stack
	 */
	FICSITNETWORKSLUA_API void luaFIN_pushFuture(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future);

	/**
	 * @brief Tries to retrieve a dynamic Future Struct from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as dynamic Future Struct
	 * @return a shared pointer to the dynamic Future Struct of the lua future in the lua stack. Will error if lua value is not future.
	 */
	FICSITNETWORKSLUA_API const FLuaFuture& luaFIN_checkFutureStruct(lua_State* L, int Index);

	/**
	 * @brief Pushes a LuaFuture that wraps the thread/coroutine at the given index in the lua stack to the stack.
	 * @param L the lua state
	 * @param Thread the index of the thread/coroutine you want to wrap as future
	 */
	FICSITNETWORKSLUA_API void luaFIN_pushLuaFuture(lua_State* L, int Thread);

	/**
	 * @brief Pushes a LuaFuture that wraps lua function call ontop of the stack. Follow the lua_call protocol.
	 * @param L the lua state
	 * @param args the amount of parameters you want to pop from the top of the stack and push into the new thread
	 */
	FICSITNETWORKSLUA_API void luaFIN_pushLuaFutureLuaFunction(lua_State* L, int args);

	/**
	 * @brief Pushes a LuaFuture that wraps the given C-Function to the stack.
	 * @param L the lua state
	 * @param Func the C-Function you want to wrap as future
	 * @param args the amount of parameters you want to pop from the top of the stack and push into the new thread
	 * @param upvals the amount of up values you want to pop from the top of the stack and use as upvalues for the function
	 */
	FICSITNETWORKSLUA_API void luaFIN_pushLuaFutureCFunction(lua_State* L, lua_CFunction Func, int args, int upvals = 0);

	FICSITNETWORKSLUA_API EFutureState luaFIN_getFutureState(lua_State* thread);

	/**
	 * Tries to process the results of calling the poll function sensibly.
	 * As the user should be able to specify the behaviour this is not part of the poll function itself.
	 * It also is slightly depending on the execution scenario (within a future, as task, as callback or standalone).
	 * There should be no additional values be on the stack above the return values.
	 *
	 * As the various return values define different processing routines,
	 * the function return an integer allowing to identify what the polled future desires to do:
	 * - 0 the future finished (if failure or success is not determined)
	 * - 1 the future is pending and waiting for at least one future to finish
	 * - 2 the future is pending and waiting to be woken up
	 * - 3 the future is pending and desires to be polled actively after a given timeout
	 * - 4 the future is pending and desires to be polled actively
	 *
	 * Additionally, boolean parameters allow to opt-in for specific default behaviour in various cases.
	 * @param L the lua state containing the poll return values
	 * @param index the index of the first poll return value (the boolean indicating pending or finished)
	 * @param dependant if given, adds a dependency from the future at the given index to the futures returned by poll
	 * @param setAsTask if given, sets the future at the given index as task when the poll desires so (polled actively with or without timeout)
	 * @param unsetAsTask if given, unsets the future at the given index as task when the poll desires so (finished or waiting for anything)
	 * @param timeout if given, updates the given double reference with the timeout value returned by the poll
	 * @return an integer identifying the type of result poll returned
	 */
	FICSITNETWORKSLUA_API int luaFIN_handlePollResults(lua_State* L, int index, TOptional<int> dependant = {}, TOptional<int> setAsTask = {}, TOptional<int> unsetAsTask = {}, double* timeout = {});

	/**
	 * @brief Awaits the Future at the given index and pushes its results onto the stack.
	 * @param L the lua state
	 * @param index the index of the future
	 */
	FICSITNETWORKSLUA_API int luaFIN_await(lua_State* L, int index);
	FICSITNETWORKSLUA_API void luaFIN_await(lua_State* L, int index, lua_KContext ctx, lua_KFunction func);

	/**
	 * Adds the dependant future as dependant to the dependency future.
	 * Lets the dependency future at the given index know, the dependant future should be polled on finish.
	 *
	 * @param L          the lua state
	 * @param dependant the index of the future that has the dependency and should get woken up by it
	 * @param dependency the index of the future this the dependant depends on and should wake up the dependant
	 */
	FICSITNETWORKSLUA_API void luaFIN_futureDependsOn(lua_State* L, int dependant, int dependency);

	/**
	 * A yield able Lua Function
	 * that executes one iteration of the future scheduler.
	 *
	 * Returns a boolean that is true if there are still pending futures or deferred callbacks.
	 */
	FICSITNETWORKSLUA_API int luaFIN_futureRun(lua_State* L);

	/**
	 * Adds the function on top of the stack to the schedules callbacks.
	 * Pops the function from the lua stack.
	 */
	FICSITNETWORKSLUA_API int luaFIN_pushCallback(lua_State* L);

	/**
	 * A Lua Function.
	 * Calls the poll function of the future stored as first up-value.
	 * Useful as callback function to poll/continue a dependant future.
	 */
	FICSITNETWORKSLUA_API int luaFIN_callbackPoll(lua_State* L);

	/**
	 * Creates and pushes a poll callback for the given future.
	 */
	FICSITNETWORKSLUA_API void luaFIN_pushPollCallback(lua_State* L, int future);

	/**
	 * A lua function that executes and removes all callbacks.
	 */
	FICSITNETWORKSLUA_API int luaFIN_runCallbacks(lua_State* L);

	/**
	 * Adds the Future at the given index to the tasks list.
	 * @param L the lua state
	 * @param index the index of the future
	 */
	FICSITNETWORKSLUA_API void luaFIN_addTask(lua_State* L, int index);

	/**
	 * Similar to `luaFIN_addTask` but addtionally marks the future as background task.
	 */
	FICSITNETWORKSLUA_API void luaFIN_addBackgroundTask(lua_State* L, int index);

	/**
	 * Removes the Future at the given index from the tasks list.
	 * @param L the lua state
	 * @param index the index of the future
	 */
	FICSITNETWORKSLUA_API void luaFIN_removeTask(lua_State* L, int index);

	/**
	 * A Lua Function that polls every task once.
	 * Returns the allowed timeout as number, nil if no timeout is allowed.
	 */
	FICSITNETWORKSLUA_API int luaFIN_runTasks(lua_State* L);

	/**
	 * Adds a Function on top of the lua stack to the timeout registry with the given timeout duration from now.
	 * Pops the Function from the stack.
	 */
	FICSITNETWORKSLUA_API void luaFIN_pushTimeout(lua_State* L, int index, double timeout);

	/**
	 * Central Poll Lua Function that checks if registered timeouts have reached their timeout.
	 * If they did, queues their functions as callback.
	 *
	 * Returns the next timeout timestamp. (Not duration!)
	 */
	FICSITNETWORKSLUA_API int luaFIN_timeoutTask(lua_State* L, int, lua_KContext);

	/**
	 * Creates, Pushes and Returns the Future Delegate. Required for the Future Structs to work properly.
	 */
	FICSITNETWORKSLUA_API FLuaFutureDelegate& luaFIN_createFutureDelegate(lua_State* L);

	/**
	 * Returns a reference to the Future Delegate that gets called when a Future Struct gets pushed that is not done yet.
	 */
	FICSITNETWORKSLUA_API FLuaFutureDelegate& luaFIN_getFutureDelegate(lua_State* L);
}
