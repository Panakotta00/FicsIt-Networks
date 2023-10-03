#pragma once

#include "LuaUtil.h"

namespace FINLua {
	/**
	 * Registers globals, metatables, registries and persistence data relevant to the Lua Object System
	 *
	 * @param[in]	L	the lua stack
	 */
	void setupObjectSystem(lua_State* L);
}