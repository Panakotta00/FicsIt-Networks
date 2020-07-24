#pragma once

#include "CoreMinimal.h"
#include "SubclassOf.h"

#include "Lua.h"

#include <string>
#include <map>
#include <set>
#include <functional>

#include "Network/FINNetworkTrace.h"

namespace FicsItKernel {
	namespace Lua {
		/**
		 * Declared the function type for instance library functions.
		 * Used when a instance cfunction gets called which refers to a library function.
		 */
		typedef TFunction<int(lua_State*, int, const FFINNetworkTrace&)> LuaLibFunc;

		/**
		 * Declares the function type for class instance library functions.
		 * Used when a class instance cfunction gets called which refers to a class library function.
		 */
		typedef TFunction<int(lua_State*, int, UClass*)> LuaLibClassFunc;

		/**
		 * Declares the functions used for setting and getting
		 * a property value from a instance.
		 */
		struct LuaLibProperty {
			TFunction<int(lua_State*, const FFINNetworkTrace&)> get;
			bool readOnly = true;
			TFunction<int(lua_State*, const FFINNetworkTrace&)> set;
		};
		
		/**
		 * Structure used in the userdata representing a instance.
		 */
		struct LuaInstance {
			FFINNetworkTrace trace;
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
			UClass* clazz;
		};

		/**
		 * Manages the registry of instance types and library functions.
		 */
		class LuaInstanceRegistry {
		private:
			/**
			 * Holds LibFunctions with their associated lua names and instance types.
			 * Used for creating the lua cfunctions for the instances.
			 */
			TMap<UClass*, TMap<FString, LuaLibFunc>> instanceFunctions;

			/**
			 * Holds LibProperties with their associated lua names and instance types.
			 * Used for creating calling them in the __index and __newindex functions.
			 */
			TMap<UClass*, TMap<FString, LuaLibProperty>> instanceProperties;

			/**
			* Holds ClassLibFunctions with their associated lua names and class instance types.
			* Used for creating the lua cfunctions for the class instances.
			*/
			TMap<UClass*, TMap<FString, LuaLibClassFunc>> classInstanceFunctions;

			/**
			* Holds the names and types of the different (class) instance types.
			* Used for getting the names and types for creating the different metatables for the (class) instances.
			*
			* key: the type of the instance.
			* value:
			* 	first: the metatable name of the instance
			* 	second: true if instance is a class instance
			*/
			TMap<UClass*, TPair<FString, bool>> instanceTypes;

			/**
			 * Map instance types names to the instance types allowing for an faster lookup
			 * of the instance type by the instance type name.
			 * key: instance type name
			 * value: instance type
			 */
			TMap<FString, UClass*> instanceTypeNames;

			LuaInstanceRegistry() = default;
			
		public:
			
			/**
			 * Returns the instance of the singleton.
			 *
			 * @return	instance of the singleton.
			 */
			static LuaInstanceRegistry* get();

			/**
			 * Registers a new instance type of given type with the given name as given type of instance.
			 *
			 * @param[in]	type				the instance type
			 * @param[in]	name				the name of the instance
			 * @param[in]	isClassInstance		true if the instance is of type class instance
			 */
			void registerType(UClass* type, FString name, bool isClassInstance);

			/**
			 * Registers a new function for the given instance type with the given lua function name.
			 *
			 * @param[in]	type	the instance type
			 * @param[in]	name	the lua function name
			 * @param[in]	func	the new function
			 */
			void registerFunction(UClass* type, FString name, LuaLibFunc func);

			/**
			* Registers a new property for the given instance type with the given lua property name.
			*
			* @param[in]	type	the instance type
			* @param[in]	name	the lua property name
			* @param[in]	prop	the new property
			*/
			void registerProperty(UClass* type, FString name, LuaLibProperty prop);

			/**
			* Registers a new function for the given class instance type with the given lua function name.
			*
			* @param[in]	type	the class instance type
			* @param[in]	name	the lua function name
			* @param[in]	func	the new function
			*/
			void registerClassFunction(UClass* type, FString name, LuaLibClassFunc func);

			/**
			 * Searches for the uppermost instance type name of the given class hirachy.
			 * If non is found, returns an empty string.
			 *
			 * @param[in]	type	class type of the instance
			 * @return	the upper most instance type name
			 */
			FString findTypeName(UClass* type);

			/**
			 * Searches for the instance type with given type name.
			 * Returns nullptr if instance type was not found.
			 *
			 * @param[in]	typeName	the name of the instance type you want to get
			 * @param[out]	isClass		gets set to true if type of instance type is class instance
			 * @return	the instance type
			 */
			UClass* findType(const FString& typeName, bool* isClass = nullptr);

			/**
			 * Searches for a instance lib func with the given lua name of the given instance name.
			 *
			 * @param[in]	instanceType	the type of the instance
			 * @param[in]	name			lua function name
			 * @param[out]	outFunc			the lib function if found
			 * @return	true if able to find function
			 */
			bool findLibFunc(UClass* instanceType, FString name, LuaLibFunc& outFunc);

			/**
			* Searches for a instance lib property with the given lua name of the given instance name.
			*
			* @param[in]	instanceType	the type of the instance
			* @param[in]	name			lua property name
			* @param[out]	outProp			the lib property if found
			* @return	true if able to find property
			*/
			bool findLibProperty(UClass* instanceType, FString name, LuaLibProperty& outProp);

			/**
			* Searches for a class instance lib func with the given lua name of the given class instance name.
			*
			* @param[in]	instanceType	the type of the instance
			* @param[in]	name			lua function name
			* @param[out]	outFunc			the class lib function if found
			* @return	true if able to find function
			*/
			bool findClassLibFunc(UClass* instanceType, FString name, LuaLibClassFunc& outFunc);

			/**
			 * Checks if the value at the given index in the lua stack is a instance and outputs the pointer
			 * to the instance if valid, nullptr if it's not a instance.
			 * It also outputs the name of the type if a valid name pointer is provided. Doesn't care if the type is a instance type
			 * will either way set it to the value type name allowing for an type error afterwards.
			 *
			 * @param[in]	L		pointer to the lua stack
			 * @param[in]	index	the index of the value in the lua stack
			 * @param[out]	name	if not nullptr, sets the string to the instance type name
			 * @return	pointer to the instance, nullptr if not a instance.
			 */
			LuaInstance* getInstance(lua_State* L, int index, std::string* name = nullptr);

			/**
			* Checks if the value at the given index in the lua stack is a instance and outputs the pointer
			* to the instance if valid.
			* If the instance is not valid, will cause a lua arg error.
			* It also outputs the name of the type if a valid name pointer is provided.
			*
			* @param[in]	L		pointer to the lua stack
			* @param[in]	index	the index of the value in the lua stack
			* @param[out]	name	if not nullptr, sets the string to the instance type name
			* @return	pointer to the instance.
			*/
			LuaInstance* checkAndGetInstance(lua_State* L, int index, std::string* name = nullptr);

			/**
			* Checks if the value at the given index in the lua stack is a class instance and outputs the pointer
			* to the class instance if valid, nullptr if it's not a class instance.
			* It also outputs the name of the type if a valid name pointer is provided.
			* Doesn't care if the type is a class instance type
			* will either way set it to the value type name allowing for an type error afterwards.
			*
			* @param[in]	L		pointer to the lua stack
			* @param[in]	index	the index of the value in the lua stack
			* @param[out]	name	if not nullptr, sets the string to the class instance type name
			* @return	pointer to the class instance, nullptr if not a class instance.
			*/
			LuaClassInstance* getClassInstance(lua_State* L, int index, std::string* name = nullptr);

			/**
			* Checks if the value at the given index in the lua stack is a class instance and outputs the pointer
			* to the class instance if valid.
			* If the class instance is not valid, will cause a lua arg error.
			* It also outputs the name of the type if a valid name pointer is provided.
			*
			* @param[in]	L		pointer to the lua stack
			* @param[in]	index	the index of the value in the lua stack
			* @param[out]	name	if not nullptr, sets the string to the instance type name
			* @return	pointer to the instance.
			*/
			LuaClassInstance* checkAndGetClassInstance(lua_State* L, int index, std::string* name = nullptr);

			/**
			 * Returns all registered instance types.
			 * Used to generate all metatables when setting up the lua environment.
			 *
			 * @return	set with all registered instance types
			 */
			std::set<UClass*> getInstanceTypes();

			/**
			 * Returns all registered member names of the given instance type.
			 * Set will be empty if type is not registered.
			 *
			 * @param[in]	type	the type you want to get the member name list from
			 * @return	set with all member names
			 */
			std::set<FString> getMemberNames(UClass* type);

			/**
			* Returns all registered class function names of the given class instance type.
			 * Set will be empty if type is not registered.
			*
			* @param[in]	type	the type you want to get the class function name list from
			* @return	set with all class function names
			*/
			std::set<FString> getClassFunctionNames(UClass* type);
		};
		
		/**
		 * Creates a new Lua Instance for the given network trace and pushes it onto the given stack.
		 * Or pushes nil if not able to create the instance.
		 * 
		 * @param[in]	L		the lua state where the instance should get created.
		 * @param[in]	obj		the obj you want to create the lua instance for.
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
		FFINNetworkTrace getObjInstance(lua_State* L, int index, UClass* clazz);

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
	}
}
