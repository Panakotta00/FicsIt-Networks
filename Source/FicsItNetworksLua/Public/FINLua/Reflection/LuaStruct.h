#pragma once

#include "FINLua/LuaUtil.h"
#include "Reflection/FINReflection.h"

class UFINKernelSystem;

namespace FINLua {
	/**
	 * Contains all information about the struct
	 */
	struct FLuaStruct {
		UFIRStruct* Type = nullptr;
		TSharedRef<FFINDynamicStructHolder> Struct;
		UFINKernelSystem* Kernel;
		FLuaStruct(UFIRStruct* Type, const FFINDynamicStructHolder& Struct, UFINKernelSystem* Kernel);
		FLuaStruct(const FLuaStruct& Other);
		~FLuaStruct();
		static void CollectReferences(void* Obj, FReferenceCollector& Collector);
	};

	/**
	 * @brief Pushes the given struct onto the lua stack. If no corresponding FINStruct Type was found, pushes nil.
	 * @param L the lua state
	 * @param Struct the struct you want to push
	 * @return false if no FIN Reflection Type got found for the struct
	 */
	bool luaFIN_pushStruct(lua_State* L, const FINStruct& Struct);

	/**
	 * @brief Tries to convert a table at the given index to a struct of the given template type.
	 * @param L the lua state
	 * @param Index the index of the lua table you try to convert
	 * @param TemplateType the Struct Type you try to construct from the table
	 * @param bAllowImplicitConstruction if true, construction arguments that are structs can be constructed implicitly 
	 * @return the Constructed FINStruct, otherwise nullptr
	 */
	TSharedPtr<FINStruct> luaFIN_convertToStruct(lua_State* L, int Index, UFIRStruct* TemplateType, bool bAllowImplicitConstruction);

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as Lua Struct.
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @return the lua struct got from the lua value (be aware of GC!), otherwise nullptr
	 */
	FLuaStruct* luaFIN_toLuaStruct(lua_State* L, int Index, UFIRStruct* ParentType);

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as Lua Struct. Causes a lua error if unable to get Lua Struct
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @return the lua struct got from the lua value (be aware of GC!)
	 */
	FLuaStruct* luaFIN_checkLuaStruct(lua_State* L, int Index, UFIRStruct* ParentType);
	
	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as FINStruct.
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @param bAllowConstruction if set to true and the lua value is a table, it will try to convert the table to the given Struct Type, if no struct type is given, does nothing
	 * @return the struct got from the lua value, otherwise nullptr
	 */
	TSharedPtr<FINStruct> luaFIN_toStruct(lua_State* L, int Index, UFIRStruct* ParentType, bool bAllowConstruction);

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as FINStruct. Causes a lua type error if not able to get as struct of given type.
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @param bAllowConstruction if set to true and the lua value is a table, it will try to convert the table to the given Struct Type, if no struct type is given, does nothing
	 * @return the struct got from the lua value
	 */
	TSharedRef<FINStruct> luaFIN_checkStruct(lua_State* L, int Index, UFIRStruct* ParentType, bool bAllowConstruction);
	template<typename T>
	T& luaFIN_checkStruct(lua_State* L, int Index, bool bAllowConstruction) {
		UFIRStruct* Type = FFINReflection::Get()->FindStruct(TBaseStructure<FVector>::Get());
		return luaFIN_checkStruct(L, Index, Type, true)->Get<T>();
	}

	/**
	 * @brief Pushes the given struct type onto the lua stack
	 * @param L the lua state
	 * @param Struct the struct want to push
	 */
	void luaFIN_pushStructType(lua_State* L, UFIRStruct* Struct);

	/**
	 * @brief Tries to get the lua value at the given index in the lua stack as struct type.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get 
	 * @return the struct stored in the lua value, otherwise nullptr
	 */
	UFIRStruct* luaFIN_toStructType(lua_State* L, int Index);

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as struct type.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get 
	 * @return the struct stored in the lua value, otherwise nullptr
	 */
	UFIRStruct* luaFIN_checkStructType(lua_State* L, int Index);

	/**
	 * @return The Lua Metatable/Type-Name of Struct
	 */
	FString luaFIN_getLuaStructTypeName();
}
