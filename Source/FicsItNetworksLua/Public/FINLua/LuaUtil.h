#pragma once

#include "FINLua.h"
#include "FicsItKernel/Logging.h"
#include "Network/FINNetworkValues.h"
#include "Serialization/CompactBinaryWriter.h"

class UFINProperty;
class UFINStruct;
class UFINFunction;

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
	 * @brief Pushes the given network value as lua value onto the stack
	 * @param L the lua state
	 * @param Value the network value you want to push
	 * @param ObjectPrefixTrace if the value is a FINObj then it gets appended to this trace because lua only stores traces
	 */
	void luaFIN_pushNetworkValue(lua_State* L, const FINAny& Value, const FFINNetworkTrace& ObjectPrefixTrace = FFINNetworkTrace());

	/**
	 * Tries to estimate the Network Value Type from a lua value.
	 * Convertibles like tables as structs or arrays are not considered an result in None.
	 */
	TOptional<EFINNetworkValueType> luaFIN_getNetworkValueType(lua_State* L, int Index);

	/**
	 * @brief Tries to retrieve a network value from the lua value at the given lua stack index.
	 * @param L the lua state
	 * @param Index the index on the stack of the lua value you want to retrieve
	 * @param Property  used to as template for the expected value
	 * @param bImplicitConversion if set to true, primitives will be converted to the target value specified by Property, if false lua value has to match the properties expected value
	 * @param bImplicitConstruction if set to true, tables can be converted to the struct specified by Property, if false tables will result in None
	 * @return The retrieved value or None if not able to retrieve
	 */
	TOptional<FINAny> luaFIN_toNetworkValueByProp(lua_State* L, int Index, UFINProperty* Property, bool bImplicitConversion, bool bImplicitConstruction);

	/**
	 * @brief Tries to retrieve a network value from the lua value at the given lua stack index.
	 * @param L the lua state
	 * @param Index the index on the stack of the lua value you want to retrieve
	 * @param Property if not nullptr, used to as template for the expected value
	 * @param bImplicitConversion if set to true, primitives will be converted to the target value specified by Property, if false lua value has to match the properties expected value
	 * @param bImplicitConstruction if set to true, tables can be converted to the struct specified by Property, if false tables will result in None if property was specified, otherwise they get interpreted as arrays
	 * @return The retrieved value or None if not able to retrieve
	 */
	TOptional<FINAny> luaFIN_toNetworkValue(lua_State* L, int Index, UFINProperty* Property, bool bImplicitConversion, bool bImplicitConstruction);
	
	/**
	 * @brief Tries to retrieve a network value from the lua value at the given lua stack index. Tables will be interpreted as None. Unknown UserData will be interpreted as none.
	 * @param L the lua state
	 * @param Index the index on the stack of the lua value you want to retrieve
	 * @return The retrieved value or None if not able to retrieve
	 */
	TOptional<FINAny> luaFIN_toNetworkValue(lua_State* L, int Index);

	/**
	 * @brief Returns the name of the value represented by the given property
	 * @param L the lua state
	 * @param Property the property of witch the lua value name should get found
	 * @return the lua value name of the given property
	 */
	FString luaFIN_getPropertyTypeName(lua_State* L, UFINProperty* Property);

	/**
	 * @breif Causes a lua type error with the expected type derived from the given property
	 * @param L the lua state
	 * @param Index the index of the value that caused the error
	 * @param Property the property of whom the value type name will be used
	 */
	int luaFIN_propertyError(lua_State* L, int Index, UFINProperty* Property);

	int luaFIN_typeError(lua_State* L, int Index, const FString& ExpectedTypeName);
	int luaFIN_argError(lua_State* L, int Index, const FString& ExtraMessage);

	FString luaFIN_typeName(lua_State* L, int Index);

	/**
	 * @brief Retrieves the type name specified in the metatable of the lua value at the given index in the lua stack
	 * @param L the lua stack
	 * @param Index the value of witch you want to get the metatable name from
	 * @return Empty String if no userdata or no name field, otherwise the content of the name field
	 */
	FString luaFIN_getUserDataMetaName(lua_State* L, int Index);
	
	void luaFIN_pushFString(lua_State* L, const FString& str);
	FString luaFIN_checkFString(lua_State* L, int index);
	FString luaFIN_toFString(lua_State* L, int index);

	FORCEINLINE FINBool luaFIN_toFinBool(lua_State* L, int index) { return static_cast<FINBool>(lua_toboolean(L, index)); }
	FORCEINLINE FINInt luaFIN_toFinInt(lua_State* L, int index) { return static_cast<FINInt>(lua_tointeger(L, index)); }
	FORCEINLINE FINFloat luaFIN_toFinFloat(lua_State* L, int index) { return static_cast<FINFloat>(lua_tonumber(L, index)); }
	FORCEINLINE FINStr luaFIN_toFinString(lua_State* L, int index) { return static_cast<FINStr>(luaFIN_toFString(L, index)); }

	void luaFIN_warning(lua_State* L, const char* msg, int tocont);

	FString luaFIN_where(lua_State* L);
	FString luaFIN_stack(lua_State* L);

	void luaFINDebug_dumpStack(lua_State* L);

	void setupUtilLib(lua_State* L);
}

struct FFINLuaLogScope : public FFINLogScope {
	explicit FFINLuaLogScope(lua_State* L);
};

FCbWriter& operator<<(FCbWriter& Writer, lua_State* const& L);
