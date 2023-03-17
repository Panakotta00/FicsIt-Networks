#pragma once

#include "lua/lua.h"

namespace FicsItKernel {
	namespace Lua {
		/**
		* Adds the Event API to the given lua state.
		*
		* @param[in]	L	The lua state you want to add the Event API to.
		*/
		void setupEventAPI(lua_State* L);
	}
}
