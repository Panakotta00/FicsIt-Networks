#pragma once

#include "LuaUtil.h"
#include "Reflection/FINReflection.h"

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

	/**
	 * @brief Pushes the given trace/object on-top of the lua stack.
	 * @param L the lua state
	 * @param Object the object you want to push
	 */
	void luaFIN_pushObject(lua_State* L, const FFINNetworkTrace& Object);

	/**
	 * @brief Tries to retrieve a Lua Object from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Lua Object
	 * @param ParentType if not nullptr, used to check the type of the Lua Object, causes nullptr return if mismatch
	 * @return the pointer to the lua object in the lua stack (attention to GC!), nullptr if type check failed
	 */
	FLuaObject* luaFIN_toLuaObject(lua_State* L, int Index, UFINClass* ParentType);

	/**
	 * @brief Tries to retrieve a Lua Object from the lua value at the given index in the lua stack. Causes a lua error if failed to get from lua value or type mismatch
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Lua Object
	 * @param ParentType if not nullptr, used to check the type of the Lua Object, causes lua error if mismatch
	 * @return the pointer to the lua object in the lua stack (attention to GC!)
	 */
	FLuaObject* luaFIN_checkLuaObject(lua_State* L, int Index, UFINClass* ParentType);

	/**
	 * @brief Tries to retrieve a Object from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Object
	 * @param ParentType if not nullptr, used to check the type of the Object, causes None return if mismatch
	 * @return the trace/object of the lua object in the lua stack, None if type check failed
	 */
	TOptional<FFINNetworkTrace> luaFIN_toObject(lua_State* L, int Index, UFINClass* ParentType);
	
	/**
	 * @brief Tries to retrieve a Object from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Object
	 * @param ParentType if not nullptr, used to check the type of the Object, causes None return if mismatch
	 * @return the trace/object of the lua object in the lua stack, None if type check failed
	 */
	FFINNetworkTrace luaFIN_checkObject(lua_State* L, int Index, UFINClass* ParentType);
	
	/**
	 * Registers globals, metatables, registries and persistence data relevant to the Lua Object System
	 *
	 * @param[in]	L	the lua stack
	 */
	void setupObjectSystem(lua_State* L);
}