#pragma once

#include "FINLua/LuaUtil.h"

namespace FINLua {
	/**
	* Adds the Computer API to the top stack entry if it is a table in the given Lua state.
	*
	* @param[in]	L	The lua state you want to add the Computer API to. Make sure the top stack entry is a table.
	*/
	void setupComputerAPI(lua_State* L);
}
