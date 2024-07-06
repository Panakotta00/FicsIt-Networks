#pragma once

#include "LuaUtil.h"

namespace FINLua {
	struct FLuaExtraSpace {
		UFINLuaProcessor* Processor = nullptr;
		TArray<LuaFile>& FileStreams;
	};

	[[nodiscard]] inline FLuaExtraSpace& luaFIN_getExtraSpace(lua_State* L) {
		return **static_cast<FLuaExtraSpace**>(lua_getextraspace(L));
	}

	inline void luaFIN_createExtraSpace(lua_State* L, FLuaExtraSpace&& ExtraSpace) {
		*static_cast<FLuaExtraSpace**>(lua_getextraspace(L)) = new FLuaExtraSpace(ExtraSpace);
	}

	inline void luaFIN_destroyExtraSpace(lua_State* L) {
		delete *static_cast<FLuaExtraSpace**>(lua_getextraspace(L));
	}
}