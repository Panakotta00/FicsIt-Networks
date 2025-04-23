#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"

namespace FINLua {
	FICSITNETWORKSLUA_API void luaFIN_setWorld(lua_State* L, UWorld* world);
	FICSITNETWORKSLUA_API UWorld* luaFIN_getWorld(lua_State* L);
}