#pragma once

#include "CoreMinimal.h"
#include "LuaException.h"
#include "FicsItKernel/Network/NetworkTrace.h"

extern "C" {
	#include "ThirdParty/lua.h"
	#include "ThirdParty/lauxlib.h"
	#include "ThirdParty/lualib.h"
	#include "ThirdParty/eris.h"
}

/**
 * Adds some variables to the C-Scope needed by the Helper-Persist-Macros.
 * The Perm-Table and Uprem-Table need to be directly next to each other in that order on the stack.
 *
 * @param[in]	Name	the name of the namespace used as prefix for the perm table names
 * @param[in]	Perm	the location of the first perm-table
 */
#define PersistSetup(Name, Perm) \
	const std::string _persist_namespace = Name; \
	const int _persist_permTableIdx = lua_absindex(L, Perm); \
	const int _persist_upermTableIdx = _persist_permTableIdx + 1

/**
 * Merges the given name with the persist namespace
 *
 * @param[in]	Name	name you want to merge
 */
#define PersistName(Name) (_persist_namespace + "-" + Name).c_str()

/**
 * Adds the value at the top of the stack to both perm-tables.
 * Pops the value from stack
 *
 * @param[in]	Name	the name of the value in the perm-tables
 */
#define PersistValue(Name) \
	lua_pushvalue(L, -1); \
	lua_setfield(L, _persist_upermTableIdx, PersistName(Name)); \
	lua_pushstring(L, PersistName(Name)); \
	lua_settable(L, _persist_permTableIdx)

/**
 * Adds the value at the given index to both perm-tables.
 *
 * @param[in]	Name	the name of the value in the perm-tables
 * @param[in]	Idx		the index of the value
 */
#define PersistIndex(Name, Idx) \
	lua_pushvalue(L, Idx); \
	PersistValue(Name);

/**
 * Adds the global value with the given name to both perm-tables.
 *
 * @param[in]	Name	the name of the global value (also used as name in the perm-tables)
 */
#define PersistGlobal(Name) \
	lua_getglobal(L, Name); \
	PersistValue(Name);

#define PersistTable(Name, Idx) _PersistTable(L, _persist_permTableIdx, _persist_upermTableIdx, _persist_namespace, Name, Idx)
inline void _PersistTable(lua_State* L, int _persist_permTableIdx, int _persist_upermTableIdx, const std::string& _persist_namespace, const std::string& name, int idx) {
	idx = lua_absindex(L, idx);
	lua_pushnil(L);
	while (lua_next(L, idx) != 0) {
		const char* str = lua_tostring(L, -2);
		if (str && (lua_iscfunction(L, -1) || lua_isuserdata(L, -1))) {
			PersistValue(name + "-" + std::string(str));
        } else {
        	lua_pop(L, 1);
        }
	}
	lua_pushvalue(L, idx);
	PersistValue(name);
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