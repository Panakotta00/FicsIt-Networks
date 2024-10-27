#pragma once

#include "FINFuture.h"
#include "LuaUtil.h"

namespace FINLua {
	typedef TSharedRef<TFIRInstancedStruct<FFINFuture>> FLuaFuture;

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
	 * @brief Pushes a LuaFuture that wraps the given C-Function to the stack.
	 * @param L the lua state
	 * @param Func the C-Function you want to wrap as future
	 * @param args the amount of parameters you want to pop from the top of the stack and push into the new thread
	 */
	void luaFIN_pushLuaFutureCFunction(lua_State* L, lua_CFunction Func, int args);
}
