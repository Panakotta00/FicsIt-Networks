#pragma once

#include "LuaUtil.h"
#include "Network/FINNetworkValues.h"

class UFINKernelSystem;
class UFINStruct;

#define FIN_LUA_STRUCT_TYPE_METATABLE_NAME "SimpleStructRefMetatable"

namespace FINLua {
	/**
	 * Contains all information about the struct
	 */
	struct FLuaStruct {
		UFINStruct* Type = nullptr;
		TSharedRef<FFINDynamicStructHolder> Struct;
		UFINKernelSystem* Kernel;
		FLuaStruct(UFINStruct* Type, const FFINDynamicStructHolder& Struct, UFINKernelSystem* Kernel);
		FLuaStruct(const FLuaStruct& Other);
		~FLuaStruct();
		static void CollectReferences(void* Obj, FReferenceCollector& Collector);
	};

	UFINStruct* luaStructFindStructFromMetaName(const FString& MetatableName);
	
	/**
	 * Trys to push the given struct onto the lua stack.
	 * What gets pushed, depends on the struct,
	 * but generally, if unable to find f.e. the struct type,
	 * the function pushes nil onto the stack.
	 */
	void luaStruct(lua_State* L, const FINStruct& Struct);

	/**
	 * Trys to convert the lua value at the given index
	 * back to a struct of the type already set in the holder.
	 * If no type is set or unable to convert the lua value to a struct,
	 * throws a lua argument error.
	 */
	FLuaStruct* luaGetStruct(lua_State* L, int i, TSharedRef<FINStruct>& Struct);

	/**
	 * Trys to convert the lua value at the given index
	 * back to a struct of any type.
	 * If unable to convert the lua value to a struct,
	 * throws a lua argument error.
	 */
	TSharedPtr<FINStruct> luaGetStruct(lua_State* L, int i, FLuaStruct** LStruct = nullptr);

	/**
	 * Tries to convert a table at the given index to a struct of the given template type.
	 * If unable to convert, returns nullptr.
	 */
	TSharedPtr<FINStruct> luaFIN_converttostruct(lua_State* L, int i, UFINStruct* templateType, bool bAllowImplicitConstruction);

	/**
	 * Returns the struct at the given index.
	 * If a table and a template type is given, tries to convert the table to the struct.
	 * If a struct, then optional luaStruct pointer will be set to the luaStruct from the stack.
	 * If unable to convert to a struct, the value was not a struct or the value was not the same type as the template, returns nullptr.
	 */
	TSharedPtr<FINStruct> luaFIN_tostruct(lua_State* L, int i, UFINStruct* templateType = nullptr, FLuaStruct** luaStruct = nullptr, bool bAllowConstruction = false);

	/**
	 * Returns the struct at the given index.
	 * If a table and a template type is given, tries to convert the table to the struct.
	 * If a struct, then optional luaStruct pointer will be set to the luaStruct from the stack.
	 * If unable to convert to a struct, the value was not a struct or the value was not the same type as the template, throws an lua error.
	 */
	TSharedRef<FINStruct> luaFIN_checkstruct(lua_State* L, int i, UFINStruct* templateType = nullptr, FLuaStruct** luaStruct = nullptr);

	/**
	 * Try to convert the lua value at the given index to the given struct.
	 * If able to convert, returns the resulting struct.
	 * If unable to convert, throws a lua argument error.
	 */
	template<typename T>
	T luaGetStruct(lua_State* L, int i) {
		TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(T::StaticStruct());
		luaGetStruct(L, i, Struct);
		return Struct->Get<T>();
	}

	/**
	 * Returns the type of struct of the stack entry at the given index.
	 * nullptr if it is not a struct
	 */
	UFINStruct* luaGetStructType(lua_State* L, int i);

	/**
	 * Returns the struct type of the value at the given index.
	 * nullptr if it is not a struct
	 */
	UFINStruct* luaFIN_getstructtype(lua_State* L, int i);

	void luaFIN_pushStructType(lua_State* L, UFINStruct* Struct);
	UFINStruct* luaFIN_toStructType(lua_State* L, int index);
	UFINStruct* luaFIN_checkStructType(lua_State* L, int index);

	/**
	 * Registers all metatables and persistency infromation
	 * for the struct types to given lua stack.
	 *
	 * @param[in]	L	the lua stack the metatables should get registered to.
	 */
	void setupStructSystem(lua_State* L);

	/**
	 * sets up the metatable for the given struct in the given stack
	 */
	void setupStructMetatable(lua_State* L, UFINStruct* Struct);
}
