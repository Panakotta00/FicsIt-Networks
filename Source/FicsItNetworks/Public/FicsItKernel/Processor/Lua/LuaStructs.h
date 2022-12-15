#pragma once

#include "Lua.h"
#include "Network/FINNetworkValues.h"

class UFINKernelSystem;
class UFINStruct;

namespace FicsItKernel {
	namespace Lua {
		/**
		 * Contains all information about the struct
		 */
		struct LuaStruct {
			UFINStruct* Type = nullptr;
			TSharedRef<FFINDynamicStructHolder> Struct;
			UFINKernelSystem* Kernel;
			LuaStruct(UFINStruct* Type, const FFINDynamicStructHolder& Struct, UFINKernelSystem* Kernel);
			LuaStruct(const LuaStruct& Other);
			~LuaStruct();
			static void CollectReferences(void* Obj, FReferenceCollector& Collector);
		};
		
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
		LuaStruct* luaGetStruct(lua_State* L, int i, TSharedRef<FINStruct>& Struct);

		/**
		 * Trys to convert the lua value at the given index
		 * back to a struct of any type.
		 * If unable to convert the lua value to a struct,
		 * throws a lua argument error.
		 */
		TSharedPtr<FINStruct> luaGetStruct(lua_State* L, int i, LuaStruct** LStruct = nullptr);

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
}
