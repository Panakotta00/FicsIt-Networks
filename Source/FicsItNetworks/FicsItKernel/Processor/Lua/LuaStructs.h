#pragma once

#include "FGBuildableRailroadTrack.h"
#include "Lua.h"
#include "LuaProcessorStateStorage.h"
#include "Network/FINNetworkValues.h"

namespace FicsItKernel {
	namespace Lua {
		class FFINLuaStructRegistry {
		public:
			/** Function that gets executed when a struct needs to get converted to lua type, needs to failsafe and push exact one value onto the lua stack */
			typedef TFunction<void(lua_State*, const FFINDynamicStructHolder&)> StructConstructorFunc;

			/** Function that gets executed when a struct needs to get converted back from a lua type */
			typedef TFunction<void(lua_State*, int i, FFINDynamicStructHolder&)> StructGetterFunc;

			/** Function that gets executed at the setup of a lua state for registering the metatable */
			typedef TFunction<void(lua_State*, const std::string&, int, int)> StructSetupFunc;
			
			struct FFINLuaStructRegisterData {
				StructSetupFunc Setup;
				StructConstructorFunc Constructor;
				StructGetterFunc Getter;
			};
			
		private:
			TMap<UScriptStruct*, FFINLuaStructRegisterData> RegisteredStructTypes;
			TMap<UScriptStruct*, FString> RegisteredStructTypeNames;
			TMap<FString, UScriptStruct*> RegisteredNamesOfStructTypes;

			FFINLuaStructRegistry() = default;

			static FFINLuaStructRegistry Instance;
		public:
			static FFINLuaStructRegistry& Get() { return Instance; }

			/**
			 * Registers a new struct type.
			 */
			void RegisterStructType(UScriptStruct* Type, FString Name, StructSetupFunc Setup, StructConstructorFunc Constructor, StructGetterFunc Getter);

			/**
			 * Setup all registered struct types for the given lua state.
			 */
			void Setup(lua_State* L);

			/**
			 * Returns the name of the given type.
			 * Empty String if not found.
			 */
			FString GetName(UScriptStruct* Type);

			/**
			 * Returns the struct type of the given name.
			 * Nullptr if not found.
			 */
			UScriptStruct* GetType(FString Name);
			
			/**
			 * Trys to find the Constructor and Getter for the given struct type.
			 *
			 * @param[in]	Type			the struct type you want to search for
			 * @param[out]	Constructor		if given, referenced value gets set to the constructor of the struct type
			 * @param[out]	Getter			if given, referenced value gets set to the getter of the struct type
			 * @return	returns true if struct got found
			 */
			bool FindStructType(UScriptStruct* Type, StructConstructorFunc* Constructor, StructGetterFunc* Getter);
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
		void luaGetStruct(lua_State* L, int i, FINStruct& Struct);

		/**
		 * Trys to convert the lua value at the given index
		 * back to a struct of any type.
		 * If unable to convert the lua value to a struct,
		 * throws a lua argument error.
		 */
		FINStruct luaGetStruct(lua_State* L, int i);

		/**
		 * Try to convert the lua value at the given index to the given struct.
		 * If able to convert, returns the resulting struct.
		 * If unable to convert, throws a lua argument error.
		 */
		template<typename T>
		T luaGetStruct(lua_State* L, int i) {
			FFINDynamicStructHolder Struct(T::StaticStruct());
			luaGetStruct(L, i, Struct);
			return Struct.Get<T>();
		}
	}
}
