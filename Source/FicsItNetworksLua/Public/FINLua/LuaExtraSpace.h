#pragma once

#include "LuaUtil.h"

struct FFINLuaModule;

namespace FINLua {
	struct FLuaExtraSpace {
		UFINLuaProcessor* Processor;
		TArray<LuaFile>& FileStreams;
		TArray<TSharedRef<FFINLuaModule>> LoadedModules;
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