#pragma once

#include "Lua.h"

namespace FicsItKernel {
	namespace Lua {
		class LuaProcessor;

		/**
		* Adds the Component API to the top stack entry if it is a table in the given Lua state.
		*
		* @param[in]	L	The lua state you want to add the Component API to. Make sure the top stack entry is a table.
		*/
		void setupComponentAPI(lua_State* L);
	}
}
