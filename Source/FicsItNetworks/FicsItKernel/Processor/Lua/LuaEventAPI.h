#pragma once

#include "Lua.h"

namespace FicsItKernel {
	namespace Lua {
		class LuaProcessor;

		/**
		* Adds the Event API to the given lua state.
		*
		* @param[in]	L	The lua state you want to add the Event API to.
		*/
		void setupEventAPI(lua_State* L);
	}
}
