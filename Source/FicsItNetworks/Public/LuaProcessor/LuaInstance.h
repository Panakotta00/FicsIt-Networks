#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"
#include "Network/FINNetworkTrace.h"

#define CLASS_INSTANCE_META_SUFFIX "-Class"

class UFINFunction;
class UFINClass;
class UFINStruct;
class UFINKernelSystem;

namespace FicsItKernel {
	class KernelSystem;
}
	
namespace FINLua {
	/**
	 * Structure used in the userdata representing a instance.
	 */
	struct LuaInstance {
		FFINNetworkTrace Trace;
		UFINKernelSystem* Kernel;
		LuaInstance(const FFINNetworkTrace& Trace, UFINKernelSystem* Kernel);
		LuaInstance(const LuaInstance& Other);
		~LuaInstance();
		static void CollectReferences(void* Obj, FReferenceCollector& Collector);
	};

	/**
	 * Structure used to store closure data for lua instance functions.
	 */
	struct LuaInstanceType {
		UClass* type;
	};

	/**
	 * Creates a new Lua Instance for the given network trace and pushes it onto the given stack.
	 * Or pushes nil if not able to create the instance.
	 * 
	 * @param[in]	L					the lua state where the instance should get created.
	 * @param[in]	obj					the obj you want to create the lua instance for.
	 * @return	returns true if the instance got created successfully.
	 */
	bool newInstance(lua_State* L, const FFINNetworkTrace& obj);

	/**
	 * Trys to get a Lua Instance from the given lua stack at the given index of the given type.
	 * If unable to find it returns an invalid network trace.
	 *
	 * @param[in]	L		the lua stack you want to get the instance from
	 * @param[in]	index	the index of the instance in the stack
	 * @param[in]	clazz	the type of the instance it should be
	 * @retrun	returns a valid network trace if found, and invalid one if not
	 */
	FFINNetworkTrace getObjInstance(lua_State* L, int index, UClass* clazz = UObject::StaticClass(), bool bError = true);

	/**
	 * Trys to get a Lua Instance from the given lua stack at the given index of the given type.
	 * If unable to find it returns nullptr.
	 *
	 * @param[in]	T		the type of the instance it should be
	 * @param[in]	L		the lua stack you want to get the instance from
	 * @param[in]	index	the index of the instance in the stack
	 * @param[out]	trace	is pointing to a network trace, assignes it to the trace of the instance.
	 * @retrun	returns a pointer to the instance object, nullptr if not found
	 */
	template<typename T>
	FORCEINLINE T* getObjInstance(lua_State* L, int index, FFINNetworkTrace* trace = nullptr) {
		auto obj = getObjInstance(L, index, T::StaticClass());
		if (trace) *trace = obj;
		return Cast<T>(*obj);
	}

	UFINClass* luaFIN_getObjectType(lua_State* L, int index);

	/**
	 * sets up the metatable for the given class in the given stack
	 */
	void setupMetatable(lua_State* L, UFINClass* Class);
	
	/**
	 * Registers globals, metatables, registries and persistence data relevant to the Lua Object System
	 *
	 * @param[in]	L	the lua stack
	 */
	void setupInstanceSystem(lua_State* L);
}
