#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"

namespace FINLua {
	void luaFIN_setWorld(lua_State* L, UWorld* world);
	UWorld* luaFIN_getWorld(lua_State* L);
}