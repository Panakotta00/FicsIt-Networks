#pragma once

#include "LuaUtil.h"
#include "FINLuaRuntimePersistence.h"

namespace FINLua {
	/**
	 * Adds some variables to the C-Scope needed by the Helper-Persist-Macros.
	 * The Perm-Table and Uprem-Table need to be directly next to each other in that order on the stack.
	 *
	 * @param[in]	Name	the name of the namespace used as prefix for the perm table names
	 */
	#define PersistenceNamespace(Name) \
		const FString _persist_namespace = Name;

	/**
	 * Merges the given name with the persist namespace
	 *
	 * @param[in]	Name	name you want to merge
	 */
	#define PersistName(Name) _persist_namespace + TEXT("-") + Name

	/**
	 * Adds the value at the given index to both perm-tables.
	 *
	 * @param[in]	Name	the name of the value in the perm-tables
	 * @param[in]	Idx		the index of the value
	 */
	#define PersistIndex(Name, Idx) \
		FINLua::luaFIN_persistValue(L, Idx, PersistName(Name));

	/**
	 * Adds the value at the top of the stack to both perm-tables.
	 * Pops the value from stack
	 *
	 * @param[in]	Name	the name of the value in the perm-tables
	 */
	#define PersistValue(Name) \
		PersistIndex(Name, -1) \
		lua_pop(L, 1);

	/**
	 * Adds the global value with the given name to both perm-tables.
	 *
	 * @param[in]	Name	the name of the global value (also used as name in the perm-tables)
	 */
	#define PersistGlobal(Name) \
		lua_getglobal(L, Name); \
		PersistValue(Name);

	/**
	 * Adds a value to the persistence tables.
	 * Does NOT pop the value from the stack
	 *
	 * @param L The Lua State
	 * @param index The index of the value that should be added to persistence
	 * @param name The associated name of the value
	 */
	inline void luaFIN_persistValue(lua_State* L, int index, const FString& name) {
		if (lua_isnil(L, index)) {
			return;
		}

		int absIndex = lua_absindex(L, index);
		lua_getfield(L, LUA_REGISTRYINDEX, "PersistPerm");
		lua_pushvalue(L, absIndex);
		luaFIN_pushFString(L, name);
		lua_settable(L, -3);
		lua_pop(L, 1);

		lua_getfield(L, LUA_REGISTRYINDEX, "PersistUperm");
		luaFIN_pushFString(L, name);
		lua_pushvalue(L, absIndex);
		lua_settable(L, -3);
		lua_pop(L, 1);
	}

	#define PersistTable(Name, Idx) FINLua::luaFIN_PersistTable(L, _persist_namespace, Name, Idx)
	inline void luaFIN_PersistTable(lua_State* L, const FString& _persist_namespace, const FString& name, int idx) {
		idx = lua_absindex(L, idx);
		lua_pushnil(L);
		while (lua_next(L, idx) != 0) {
			FString str = luaFIN_toFString(L, -2);
			if (lua_iscfunction(L, -1) || lua_isuserdata(L, -1)) {
				PersistValue(name + "-" + str);
	        } else {
        		lua_pop(L, 1);
	        }
		}
	}

	inline FFINLuaRuntimePersistenceState& luaFIN_getPersistence(lua_State* L) {
		return *luaFIN_getRuntime(L).PersistenceState;
	}
}
