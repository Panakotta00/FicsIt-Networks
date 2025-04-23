#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"

class AFINMicrocontroller;

namespace FINLua {
	void luaFIN_setMicrocontroller(lua_State* L, AFINMicrocontroller* microcontroller);
	AFINMicrocontroller* luaFIN_getMicrocontroller(lua_State* L);
}