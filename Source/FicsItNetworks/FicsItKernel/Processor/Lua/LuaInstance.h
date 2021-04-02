#pragma once

#include "CoreMinimal.h"
#include "Lua.h"
#include "FicsItNetworks/Network/FINNetworkTrace.h"

class UFINFunction;
class UFINClass;
class UFINStruct;
class UFINKernelSystem;

namespace FicsItKernel {
	class KernelSystem;
	
	namespace Lua {
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
		 * Structure used in the userdata representing a class instance.
		 */
		struct LuaClassInstance {
			UClass* Class;
		};

		/**
		 * Creates a new Lua Instance for the given network trace and pushes it onto the given stack.
		 * Or pushes nil if not able to create the instance.
		 * 
		 * @param[in]	L					the lua state where the instance should get created.
		 * @param[in]	obj					the obj you want to create the lua instance for.
		 * @return	returns true if the instance got created successfully.
		 */
		bool newInstance(lua_State* L, FFINNetworkTrace obj);

		/**
		 * Trys to get a Lua Instance from the given lua stack at the given index of the given type.
		 * If unable to find it returns an invalid network trace.
		 *
		 * @param[in]	L		the lua stack you want to get the instance from
		 * @param[in]	index	the index of the instance in the stack
		 * @param[in]	clazz	the type of the instance it should be
		 * @retrun	returns a valid network trace if found, and invalid one if not
		 */
		FFINNetworkTrace getObjInstance(lua_State* L, int index, UClass* clazz = UObject::StaticClass());

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

		/**
		 * Creates a new Lua Class Instance for the class and pushes it onto the given stack.
		 * Or pushes nil if not able to create the class instance.
		 * 
		 * @param[in]	L		the lua state where the class instance should get created.
		 * @param[in]	clazz	the clazz you want to create the lua class instance for.
		 * @return	returns true if the class instance got created successfully.
		 */
		bool newInstance(lua_State* L, UClass* clazz);

		/**
		 * Trys to get a Lua Class Instance from the given lua stack at the given index of the given type.
		 * If unable to find it returns nullptr.
		 *
		 * @param[in]	L		the lua stack you want to get the class instance from
		 * @param[in]	index	the index of the class instance in the stack
		 * @param[in]	clazz	the base type of the instance it should be
		 * @retrun	returns the type or nullptr if not found
		 */
		UClass* getClassInstance(lua_State* L, int index, UClass* clazz);

		/**
		 * Trys to get a Lua Class Instance from the given lua stack at the given index of the given type.
		 * If unable to find it returns nullptr.
		 *
		 * @param[in]	T		the type of the class instance it should be
		 * @param[in]	L		the lua stack you want to get the class instance from
		 * @param[in]	index	the index of the class instance in the stack
		 * @retrun	returns a pointer to the type, nullptr if not found
		 */
		template<typename T>
		FORCEINLINE TSubclassOf<T> getClassInstance(lua_State* L, int index) {
			return getClassInstance(L, index, T::StaticClass());
		}

		/**
		 * Registers all metatables and persistency infromation
		 * for the instace types to given lua stack.
		 *
		 * @param[in]	L	the lua stack the metatables should get registered to.
		 */
		void setupInstanceSystem(lua_State* L);

		/**
		 * sets up the metatable for the given class in the given stack
		 */
		void setupMetatable(lua_State* L, UFINClass* Class);
	}
}
