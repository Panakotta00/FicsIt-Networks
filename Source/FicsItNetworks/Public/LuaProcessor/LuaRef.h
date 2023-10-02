#pragma once

#include "LuaUtil.h"
#include "Reflection/FINExecutionContext.h"

class UFINFunction;
class UFINStruct;

#define LUA_REF_CACHE "FINRefCache"
#define LUA_REF_FUNC_DATA "FINRefFuncData"

namespace FINLua {
	/**
	 * Holds information about type and function of the referenced function
	 */
	struct LuaRefFuncData {
		UFINStruct* Struct;
		UFINFunction* Func;
	};
	
	/**
	 * Calls the given FINFunction with the given context, error messages and lua contenxt
	 */
	int luaCallFINFunc(lua_State* L, UFINFunction* Func, const FFINExecutionContext& Ctx, const std::string& typeName);

	/**
	 * Trys to find function or property by membername in the given struct. Uses also given cache.
	 */
	int luaFindGetMember(lua_State* L, UFINStruct* Struct, const FFINExecutionContext& Ctx, const FString& MemberName, const FString& MetaName, int(*callFunc)(lua_State*), bool classInstance);

	/**
	 * Trys to find property by memebername and sets the value in the given struct. Uses also given cache.
	 */
	int luaFindSetMember(lua_State* L, UFINStruct* Struct, const FFINExecutionContext& Ctx, const FString& MemberName, bool classInstance, bool bCauseError = true);

	/**
	 * Registers all metatables and persistency infromation
	 * for the reflection util types to given lua stack.
	 *
	 * @param[in]	L	the lua stack the metatables should get registered to.
	 */
	void setupRefUtils(lua_State* L);
}
