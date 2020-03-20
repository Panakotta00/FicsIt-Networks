#pragma once

#include "CoreMinimal.h"
#include "LuaException.h"

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

		LuaDataType propertyToLua(lua_State* L, UProperty* p, void* data);
		LuaDataType luaToProperty(lua_State* L, UProperty* p, void* data, int i);
	}
}