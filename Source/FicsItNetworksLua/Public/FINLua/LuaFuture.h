#pragma once

#include "LuaUtil.h"
#include "Network/FINFuture.h"

namespace FINLua {
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
	[[nodiscard]] const TSharedRef<TFIRInstancedStruct<FFINFuture>>& luaFIN_checkLuaFuture(lua_State* L, int Index);
	[[nodiscard]] FORCEINLINE const TFIRInstancedStruct<FFINFuture>& luaFIN_checkFuture(lua_State* L, int Index) {
		TSharedRef<TFIRInstancedStruct<FFINFuture>> future = luaFIN_checkLuaFuture(L, Index);
		return *future;
	}
}
