#pragma once

#include "CoreMinimal.h"
#include "LuaException.h"
#include "FicsItKernel/Network/NetworkTrace.h"

extern "C" {
	#include "ThirdParty/lua.h"
	#include "ThirdParty/lauxlib.h"
	#include "ThirdParty/lualib.h"
}

namespace FicsItKernel {
	namespace Lua {
		enum LuaDataType {
			LUA_NIL,
			LUA_STR,
			LUA_INT,
			LUA_NUM,
			LUA_BOOL,
			LUA_TBL,
			LUA_OBJ,
		};

		LuaDataType propertyToLua(lua_State* L, UProperty* p, void* data, Network::NetworkTrace trace);
		LuaDataType luaToProperty(lua_State* L, UProperty* p, void* data, int i);
	}
}