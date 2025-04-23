#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"

class UFINKernelSystem;

namespace FINLua {
	FICSITNETWORKSLUA_API void luaFIN_setKernel(lua_State* L, UFINKernelSystem* Kernel);
	FICSITNETWORKSLUA_API UFINKernelSystem* luaFIN_getKernel(lua_State* L);
}
