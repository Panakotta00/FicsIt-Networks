#pragma once

#include "LuaUtil.h"

namespace FINLua {
	/**
	* Adds the Debug API and overrides base functions like pcall and xpcall of the given Lua state.
	*
	* @param[in]	L	The lua state you want to add the Debug API to and the other functions.
	*/
	void setupDebugAPI(lua_State* L);
}
