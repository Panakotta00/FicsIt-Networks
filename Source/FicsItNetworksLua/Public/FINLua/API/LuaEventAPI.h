#pragma once

#include "FINLua/LuaUtil.h"

namespace FINLua {
	/**
	* Adds the Event API to the given lua state.
	*
	* @param[in]	L	The lua state you want to add the Event API to.
	*/
	void setupEventAPI(lua_State* L);
}
