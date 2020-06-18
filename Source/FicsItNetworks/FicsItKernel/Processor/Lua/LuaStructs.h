#pragma once


#include "FicsItKernel/FicsItFS/FileSystem.h"
#include "FGInventoryLibrary.h"
#include "FGRailroadTimeTable.h"

#include "Lua.h"

namespace FicsItKernel {
	namespace Lua {
		void luaStruct(lua_State* L, FInventoryItem item);
		void luaStruct(lua_State* L, FItemAmount amount);
		void luaStruct(lua_State* L, FInventoryStack stack);
		void luaTrackGraph(lua_State* L, const Network::NetworkTrace& trace ,int trackID);
		void luaTimeTableStop(lua_State* L, const Network::NetworkTrace& station, float duration);
		
		FTimeTableStop luaGetTimeTableStop(lua_State* L, int index);

		void setupStructs(lua_State* L);
	}
}