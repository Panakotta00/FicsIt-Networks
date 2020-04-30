#pragma once

#include "Lua.h"

#include "FicsItKernel/FicsItKernel.h"

namespace FicsItKernel {
	namespace Lua {
		/**
		* Adds the Computer API to the top stack entry if it is a table in the given Lua state.
		*
		* @param[in]	L	The lua state you want to add the Computer API to. Make sure the top stack entry is a table.
		* @param[in]	kernel	The system kernel you want to bind the Computer API to
		*/
		void setupComputerAPI(lua_State* L, KernelSystem* kernel);
	}
}