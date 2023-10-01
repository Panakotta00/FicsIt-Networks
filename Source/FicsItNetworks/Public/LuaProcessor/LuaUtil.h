#pragma once

#include "LuaException.h"
#include "Network/FINAnyNetworkValue.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/eris.h"

class UFINProperty;
class UFINStruct;
class UFINFunction;
struct FFINNetworkTrace;

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

namespace FINLua {
	/**
	 * Converts the given property within the given struct pointer
	 * to a lua value and pushes it onto the stack.
	 * The trace allows for object properties to have additionally a this trace attached.
	 */
	void propertyToLua(lua_State* L, FProperty* p, void* data, const FFINNetworkTrace& trace);

	/**
	 * Trys to convert the lua value at the given index on the given lua stack
	 * to the given property in the given struct pointer.
	 */
	void luaToProperty(lua_State* L, FProperty* p, void* data, int i);

	/**
	 * Trys to convert the lua value at the given index on the given lua stack
	 * to the given property and returns that value.
	 * lua error if value not valid
	 */
	FINAny luaToProperty(lua_State* L, UFINProperty* Prop, int Index);

	/**
	 * Trys to convert the lua value at the given index to any kind of network value.
	 */
	void luaToNetworkValue(lua_State* L, int i, FFINAnyNetworkValue& Val);

	/**
	 * Converts the given network value into a lua value and pushes it onto the stack
	 */
	void networkValueToLua(lua_State* L, const FFINAnyNetworkValue& Val, const FFINNetworkTrace& Trace);

	void luaFIN_pushfstring(lua_State* L, const FString& Str);
	FString luaFIN_checkfstring(lua_State* L, int Index);
	void luaFIN_pushStructType(lua_State* L, UFINStruct* Struct);
	UFINStruct* luaFIN_toStructType(lua_State* L, int index);
	UFINStruct* luaFIN_checkStructType(lua_State* L, int index);

	void setupUtilLib(lua_State* L);
}