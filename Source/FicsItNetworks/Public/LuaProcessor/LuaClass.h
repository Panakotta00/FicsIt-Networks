#pragma once

#include "LuaUtil.h"
#include "Reflection/FINReflection.h"

#define FIN_LUA_CLASS_LIB_METATABLE_NAME "ClassLib"
#define FIN_LUA_CLASS_METATABLE_NAME "Class-Metatable"

namespace FINLua {
	/**
	 * Could be only one type, but then when the other is needed, always would have to ask Reflection System
	 * This is for means of caching.
	 * Problem causes by the fact that Lua Classes have to support also non-FIN-Classes (Item Types)
	 */
	struct FLuaClass {
		UClass* UClass;
		UFINClass* FINClass;
	};

	/**
	 * @brief Pushes a Class and FINClass onto the lua stack (used for optimization purposes)
	 * @param L the lua stack
	 * @param Class the Class to push onto the stack
	 * @param FINClass the FINClass to push onto the stack
	 * @return true if successfully able to push
	 */
	bool luaFIN_pushClass(lua_State* L, UClass* Class, UFINClass* FINClass);

	/**
	 * @brief Pushes a Class onto the lua stack
	 * @param L the lua stack
	 * @param Class the class to push onto the stack
	 */
	FORCEINLINE void luaFIN_pushClass(lua_State* L, UClass* Class) {
		UFINClass* FINClass = FFINReflection::Get()->FindClass(Class, true, false);
		luaFIN_pushClass(L, Class, FINClass);
	}

	/**
	 * @brief Pushes a FINClass onto the lua stack
	 * @param L the lua stack
	 * @param FINClass the FINClass to push onto the stack
	 */
	FORCEINLINE void luaFIN_pushClass(lua_State* L, UFINClass* FINClass) {
		UClass* Class = FFINReflection::Get()->FindUClass(FINClass);
		luaFIN_pushClass(L, Class, FINClass);
	}

	/**
	 * @brief Tries to retrieve a LuaClass from the lua value with the given index in the lua stack
	 * @param L the lua stack
	 * @param Index the index of the lua value you try to get as LuaClass
	 * @return The pointer to the LuaClass in the stack (attention to GC!), nullptr if not able to get as LuaClass 
	 */
	FLuaClass* luaFIN_toLuaClass(lua_State* L, int Index);

	/**
	 * @brief Retrieves a LuaClass from the lua value with the given index in the lua stack. Causes an Lua Error if unable to get as LuaClass or is no child of given parent class
	 * @param L teh lua stack
	 * @param Index the index of the lua value you get as LuaClass
	 * @param ParentClass the expected parent class of the value
	 * @return The pointer to the LuaClass in the stack (attention to GC!)
	 */
	FLuaClass* luaFIN_checkLuaClass(lua_State* L, int Index, UFINClass* ParentClass);

	/**
	 * @brief Tries to retrieve a Class from the lua value with the given index in the lua stack
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as class
	 * @param OutFINClass if not nullptr, sets the FINClass Pointer to the FINClass retrieved
	 * @return The Class from the stack, nullptr if unable to get as class
	 */
	FORCEINLINE UClass* luaFIN_toUClass(lua_State* L, int Index, UFINClass** OutFINClass = nullptr) {
		FLuaClass* luaClass = luaFIN_toLuaClass(L, Index);
		if (luaClass) {
			if (OutFINClass) *OutFINClass = luaClass->FINClass;
			return luaClass->UClass;
		} else {
			if (OutFINClass) *OutFINClass = nullptr;
			return nullptr;
		}
	}
	FORCEINLINE UClass* luaFIN_toSubUClass(lua_State* L, int Index, UClass* ParentClass) {
		FLuaClass* luaClass = luaFIN_toLuaClass(L, Index);
		if (!luaClass || !luaClass->UClass->IsChildOf(ParentClass)) return nullptr;
		return luaClass->UClass;
	}

	/**
	 * @brief Tries to retrieve a FINClass from the lua value with the given index in the lua stack
	 * @param L the lua state
	 * @param Index the index of the lua value you try to get as FINClass
	 * @param[out] OutClass if not nullptr, sets the UClass Pointer to the UClass retrieved
	 * @return The FINClass from the stack, nullptr if unable to get as FINClass
	 */
	FORCEINLINE UFINClass* luaFIN_toFINClass(lua_State* L, int Index, UClass** OutClass = nullptr) {
		FLuaClass* luaClass = luaFIN_toLuaClass(L, Index);
		if (luaClass) {
			if (OutClass) *OutClass = luaClass->UClass;
			return luaClass->FINClass;
		} else {
			if (OutClass) *OutClass = nullptr;
			return nullptr;
		}
	}
	

	/**
	 * @brief Registers globals, metatables, registries and persistence data relevant to the Lua Object System
	 * @param L the lua state
	 */
	void setupClassSystem(lua_State* L);
}
