#pragma once

#include "FicsItReflection.h"
#include "FINLuaReferenceCollector.h"
#include "FINLua/LuaUtil.h"

struct FFINLuaReferenceCollector;
class UFINKernelSystem;

namespace FINLua {
	/**
	 * Contains all information about the struct
	 */
	struct FICSITNETWORKSLUA_API FLuaStruct : FFINLuaReferenceCollected {
		UFIRStruct* Type = nullptr;
		TSharedRef<FFIRInstancedStruct> Struct;
		FLuaStruct(UFIRStruct* Type, const FFIRInstancedStruct& Struct, FFINLuaReferenceCollector* ReferenceCollector);
		virtual void CollectReferences(FReferenceCollector& Collector) override;
	};

	/**
	 * @brief Pushes the given struct onto the lua stack. If no corresponding FINStruct Type was found, pushes nil.
	 * @param L the lua state
	 * @param Struct the struct you want to push
	 * @return false if no FIN Reflection Type got found for the struct
	 */
	FICSITNETWORKSLUA_API bool luaFIN_pushStruct(lua_State* L, const FIRStruct& Struct, int numUserValues = 1);

	/**
	 * @brief Tries to convert a table at the given index to a struct of the given template type.
	 * @param L the lua state
	 * @param Index the index of the lua table you try to convert
	 * @param TemplateType the Struct Type you try to construct from the table
	 * @param bAllowImplicitConstruction if true, construction arguments that are structs can be constructed implicitly 
	 * @return the Constructed FINStruct, otherwise nullptr
	 */
	FICSITNETWORKSLUA_API TSharedPtr<FIRStruct> luaFIN_convertToStruct(lua_State* L, int Index, UFIRStruct* TemplateType, bool bAllowImplicitConstruction);

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as Lua Struct.
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @return the lua struct got from the lua value (be aware of GC!), otherwise nullptr
	 */
	FICSITNETWORKSLUA_API FLuaStruct* luaFIN_toLuaStruct(lua_State* L, int Index, UFIRStruct* ParentType);

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as Lua Struct. Causes a lua error if unable to get Lua Struct
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @return the lua struct got from the lua value (be aware of GC!)
	 */
	FICSITNETWORKSLUA_API FLuaStruct* luaFIN_checkLuaStruct(lua_State* L, int Index, UFIRStruct* ParentType);
	
	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as FINStruct.
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @param bAllowConstruction if set to true and the lua value is a table, it will try to convert the table to the given Struct Type, if no struct type is given, does nothing
	 * @return the struct got from the lua value, otherwise nullptr
	 */
	FICSITNETWORKSLUA_API TSharedPtr<FIRStruct> luaFIN_toStruct(lua_State* L, int Index, UFIRStruct* ParentType, bool bAllowConstruction);
	FICSITNETWORKSLUA_API TSharedPtr<FIRStruct> luaFIN_toUStruct(lua_State* L, int Index, UStruct* ParentType, bool bAllowConstruction);
	template<typename T>
	TSharedPtr<T> luaFIN_toStruct(lua_State* L, int Index, bool bAllowConstruction) {
		TSharedPtr<FIRStruct> val = luaFIN_toUStruct(L, Index, TBaseStructure<T>::Get(), bAllowConstruction);
		if (val) return TSharedPtr<T>(val, &val->Get<T>());
		return nullptr;
	}

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as FINStruct. Causes a lua type error if not able to get as struct of given type.
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get as struct
	 * @param ParentType the struct type that will be used to convert the lua table to a struct if enabled, if its a struct already, allows to check the type to match or be child of the given type
	 * @param bAllowConstruction if set to true and the lua value is a table, it will try to convert the table to the given Struct Type, if no struct type is given, does nothing
	 * @return the struct got from the lua value
	 */
	FICSITNETWORKSLUA_API TSharedRef<FIRStruct> luaFIN_checkStruct(lua_State* L, int Index, UFIRStruct* ParentType, bool bAllowConstruction);
	FICSITNETWORKSLUA_API TSharedRef<FIRStruct> luaFIN_checkUStruct(lua_State* L, int Index, UStruct* ParentType, bool bAllowConstruction);
	template<typename T>
	TSharedRef<T> luaFIN_checkStruct(lua_State* L, int Index, bool bAllowConstruction) {
		auto Struct = luaFIN_checkUStruct(L, Index, TBaseStructure<T>::Get(), bAllowConstruction);
		return TSharedRef<T>(Struct, &Struct->template Get<T>());
	}

	/**
	 * @brief Pushes the given struct type onto the lua stack
	 * @param L the lua state
	 * @param Struct the struct want to push
	 */
	FICSITNETWORKSLUA_API void luaFIN_pushStructType(lua_State* L, UFIRStruct* Struct);

	/**
	 * @brief Tries to get the lua value at the given index in the lua stack as struct type.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get 
	 * @return the struct stored in the lua value, otherwise nullptr
	 */
	FICSITNETWORKSLUA_API UFIRStruct* luaFIN_toStructType(lua_State* L, int Index);

	/**
	 * @brief Retrieves the lua value at the given index in the lua stack as struct type.
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get 
	 * @return the struct stored in the lua value, otherwise nullptr
	 */
	FICSITNETWORKSLUA_API UFIRStruct* luaFIN_checkStructType(lua_State* L, int Index);

	/**
	 * @return The Lua Metatable/Type-Name of Struct
	 */
	FICSITNETWORKSLUA_API FString luaFIN_getLuaStructTypeName();
}
