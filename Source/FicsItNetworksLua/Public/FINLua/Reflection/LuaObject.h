#pragma once

#include "FicsItReflection.h"
#include "FINLuaReferenceCollector.h"
#include "FINLua/LuaUtil.h"
#include "Reflection/FIRClass.h"

struct FFINLuaReferenceCollector;

namespace FINLua {
	/**
	 * Structure used in the userdata representing a instance.
	 */
	struct FLuaObject : FFINLuaReferenceCollected {
		UFIRClass* Type;
		FFIRTrace Object;

		FLuaObject(const FFIRTrace& Trace, FFINLuaReferenceCollector* ReferenceCollector);

		virtual void CollectReferences(FReferenceCollector& Collector) override;
	};

	/**
	 * @brief Pushes the given trace/object on-top of the lua stack.
	 * @param L the lua state
	 * @param Object the object you want to push
	 */
	void luaFIN_pushObject(lua_State* L, const FFIRTrace& Object);

	/**
	 * @brief Tries to retrieve a Lua Object from the lua value at the givne index in the lua stack. No further checks except the metatable check are applied.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Lua Object
	 */
	FLuaObject* luaFIN_toRawLuaObject(lua_State* L, int Index);

	/**
	 * @brief Tries to retrieve a Lua Object from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Lua Object
	 * @param ParentType if not nullptr, used to check the type of the Lua Object, causes nullptr return if mismatch
	 * @return the pointer to the lua object in the lua stack (attention to GC!), nullptr if type check failed
	 */
	FLuaObject* luaFIN_toLuaObject(lua_State* L, int Index, UFIRClass* ParentType);

	/**
	 * @brief Tries to retrieve a Lua Object from the lua value at the given index in the lua stack. Causes a lua error if failed to get from lua value or type mismatch
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Lua Object
	 * @param ParentType if not nullptr, used to check the type of the Lua Object, causes lua error if mismatch
	 * @return the pointer to the lua object in the lua stack (attention to GC!)
	 */
	FLuaObject* luaFIN_checkLuaObject(lua_State* L, int Index, UFIRClass* ParentType);

	/**
	 * @brief Tries to retrieve a Object from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Object
	 * @param ParentType if not nullptr, used to check the type of the Object, causes None return if mismatch
	 * @return the trace/object of the lua object in the lua stack, None if type check failed
	 */
	TOptional<FFIRTrace> luaFIN_toObject(lua_State* L, int Index, UFIRClass* ParentType);
	
	/**
	 * @brief Tries to retrieve a Object from the lua value at the given index in the lua stack.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as Object
	 * @param ParentType if not nullptr, used to check the type of the Object, causes None return if mismatch
	 * @return the trace/object of the lua object in the lua stack, None if type check failed
	 */
	FFIRTrace luaFIN_checkObject(lua_State* L, int Index, UFIRClass* ParentType);
	template<class T>
	FFIRTrace luaFIN_checkTrace(lua_State* L, int Index) {
		return luaFIN_checkObject(L, Index, FFicsItReflectionModule::Get().FindClass(T::StaticClass()));
	}
	template<class T>
	T* luaFIN_checkObject(lua_State* L, int Index) {
		return Cast<T>(luaFIN_checkObject(L, Index, FFicsItReflectionModule::Get().FindClass(T::StaticClass())).Get());
	}

	/**
	 * @return The Lua Metatable/Type-Name of Object
	 */
	FString luaFIN_getLuaObjectTypeName();
}