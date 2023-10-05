#pragma once

#include "LuaUtil.h"

class UFINClass;
class UFINKernelSystem;

#define FIN_LUA_OBJECT_METATABLE_NAME "Object"

namespace FINLua {
	/**
	 * Structure used in the userdata representing a instance.
	 */
	struct FLuaObject {
		UFINClass* Type;
		FFINNetworkTrace Object;
		UFINKernelSystem* Kernel;
		FLuaObject(const FFINNetworkTrace& Trace, UFINKernelSystem* Kernel);
		FLuaObject(const FLuaObject& Other);
		~FLuaObject();
		static void CollectReferences(void* Obj, FReferenceCollector& Collector);
	};

	void luaFIN_pushObject(lua_State* L, const FFINNetworkTrace& Object);
	FLuaObject* luaFIN_toLuaObject(lua_State* L, int Index, UFINClass* ParentType);
	FLuaObject* luaFIN_checkLuaObject(lua_State* L, int Index, UFINClass* ParentType);
	TOptional<FFINNetworkTrace> luaFIN_toObject(lua_State* L, int Index, UFINClass* ParentType);
	FFINNetworkTrace luaFIN_checkObject(lua_State* L, int Index, UFINClass* ParentType);
	
	/**
	 * Registers globals, metatables, registries and persistence data relevant to the Lua Object System
	 *
	 * @param[in]	L	the lua stack
	 */
	void setupObjectSystem(lua_State* L);
}