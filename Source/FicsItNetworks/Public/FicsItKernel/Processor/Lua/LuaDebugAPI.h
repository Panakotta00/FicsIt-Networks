#pragma once

#include "lua/lua.h"

namespace FicsItKernel {
	namespace Lua {
		/**
		* Adds the Debug API and overrides base functions like pcall and xpcall of the given Lua state.
		*
		* @param[in]	L	The lua state you want to add the Debug API to and the other functions.
		*/
		void setupDebugAPI(lua_State* L);
	}
}
