#pragma once


#include "FicsItKernel/FicsItFS/FileSystem.h"
#include "FGInventoryLibrary.h"

#include "Lua.h"

namespace FicsItKernel {
	namespace Lua {
		void luaStruct(lua_State* L, FInventoryItem item);
		void luaStruct(lua_State* L, FItemAmount amount);
		void luaStruct(lua_State* L, FInventoryStack stack);

		void setupStructs(lua_State* L);
	}
}