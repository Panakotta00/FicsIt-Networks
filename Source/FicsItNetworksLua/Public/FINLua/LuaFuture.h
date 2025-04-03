#pragma once

#include "FINFuture.h"
#include "LuaUtil.h"

#define LUAFIN_HIDDENGLOBAL_FUTUREREGISTRY "future-registry"
#define LUAFIN_HIDDENGLOBAL_TIMEOUTREGISTRY "timeout-registry"

namespace FINLua {
	typedef TSharedRef<TFIRInstancedStruct<FFINFuture>> FLuaFuture;

	enum EFutureState {
		Future_Pending,
		Future_Ready,
		Future_Failed,
	};

	/**
	 * @brief Pushes a FFINFuture on to the Lua Stack.
	 * @param L the lua state
	 * @param Future the Future Struct you want to push onto the lua stack
	 */
	void luaFIN_pushFuture(lua_State* L, const TFIRInstancedStruct<FFINFuture>& Future);

	/**
	 * @brief Tries to retrieve a dynamic Future Struct from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as dynamic Future Struct
	 * @return a shared pointer to the dynamic Future Struct of the lua future in the lua stack. Will error if lua value is not future.
	 */
	[[nodiscard]] const FLuaFuture& luaFIN_checkFutureStruct(lua_State* L, int Index);

	/**
	 * @brief Pushes a LuaFuture that wraps the thread/coroutine at the given index in the lua stack to the stack.
	 * @param L the lua state
	 * @param Thread the index of the thread/coroutine you want to wrap as future
	 */
	void luaFIN_pushLuaFuture(lua_State* L, int Thread);

	/**
	 * @brief Pushes a LuaFuture that wraps lua function call ontop of the stack. Follow the lua_call protocol.
	 * @param L the lua state
	 * @param args the amount of parameters you want to pop from the top of the stack and push into the new thread
	 */
	void luaFIN_pushLuaFutureLuaFunction(lua_State* L, int args);

	/**
	 * @brief Pushes a LuaFuture that wraps the given C-Function to the stack.
	 * @param L the lua state
	 * @param Func the C-Function you want to wrap as future
	 * @param args the amount of parameters you want to pop from the top of the stack and push into the new thread
	 * @param upvals the amount of up values you want to pop from the top of the stack and use as upvalues for the function
	 */
	void luaFIN_pushLuaFutureCFunction(lua_State* L, lua_CFunction Func, int args, int upvals = 0);

	/**
	 * @brief Awaits the Future at the given index and pushes its results onto the stack.
	 * @param L the lua state
	 * @param index the index of the future
	 */
	int luaFIN_await(lua_State* L, int index);

	/**
	 * Adds the Future ad the given index to the tasks list.
	 * @param L the lua state
	 * @param index the index of the future
	 */
	void luaFIN_addTask(lua_State* L, int index);

	/**
	 * A yield able Lua Function
	 * that executes one iteration of the future scheduler.
	 *
	 * Returns a boolean that is true if there are still pending futures or deferred callbacks.
	 */
	int luaFIN_futureRun(lua_State* L);

	EFutureState luaFIN_getFutureState(lua_State* thread);

	/**
	 * Adds the function on top of the stack to the schedules callbacks.
	 * Pops the function from the lua stack.
	 */
	int luaFIN_pushCallback(lua_State* L);

	/**
	 * A Lua Function.
	 * Calls the poll function of the future stored as first up-value.
	 * Useful as callback function to poll/continue a dependant future.
	 */
	int luaFIN_callbackPoll(lua_State* L);

	void luaFIN_pushPollCallback(lua_State* L, int future);

	/**
	 * Tries to find and push the future the lua thread is associated with.
	 * Pushes Nil if no future was found.
	 * Returns true if it found a future.
	 */
	bool luaFIN_pushThisFuture(lua_State* L);

void luaFIN_futureDependsOn(lua_State* L, int dependent, int dependency);

	/**
	 * Adds this threads future as dependent to the future at the given index.
	 * Lets the future at the given index know, the future of the thread L should be polled on finish.
	 * May error if this thread is not associated with a thread.
	 *
	 * @param L          the lua state
	 * @param dependency the index of the future this thread should be woken up by
	 */
	void luaFIN_futureDependsOn(lua_State* L, int dependency);

	/**
	 * Central Poll Functions that checks if registered timeouts have reached their timeout.
	 * If they did, queues their functions as callback.
	 *
	 * Returns the next timeout timestamp. (Not duration!)
	 */
	double luaFIN_pollTimeouts(lua_State* L);

	/**
	 * Adds a Function on top of the lua stack to the timeout registry with the given timeout duration from now.
	 * Pops the Function from the stack.
	 */
	void luaFIN_pushTimeout(lua_State* L, float timeout);
}
