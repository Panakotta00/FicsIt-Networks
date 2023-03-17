#pragma once

#include "lua/lua.h"
#include "Network/FINFuture.h"

namespace FicsItKernel {
	namespace Lua {
		/**
		 * Creates a Lua representation of the given future structure and pushes it onto the given Lua stack.
		 * Also pushes the future to the future queue of the kernel of the lua state.
		 */
		void luaFuture(lua_State* L, const TFINDynamicStruct<FFINFuture>& Future);

		/**
		 * Initializes the lua future library in the given lua stack
		 */
		void setupFutureAPI(lua_State* L);
	}
}