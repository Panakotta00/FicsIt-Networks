#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"
#include "LuaKernelAPI.h"

class UFINKernelSystem;

namespace FINLua {
	void luaFIN_setKernel(lua_State* L, UFINKernelSystem* Kernel);
	UFINKernelSystem* luaFIN_getKernel(lua_State* L);
}
