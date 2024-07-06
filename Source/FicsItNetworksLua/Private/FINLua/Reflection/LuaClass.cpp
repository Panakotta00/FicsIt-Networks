#include "FINLua/Reflection/LuaClass.h"

#include "FINLua/Reflection/LuaRef.h"

namespace FINLua {
	int luaClassEq(lua_State* L) {
		FLuaClass* LuaClass1 = luaFIN_toLuaClass(L, 1);
		FLuaClass* LuaClass2 = luaFIN_toLuaClass(L, 2);

		if (!LuaClass1 || !LuaClass2) {
			lua_pushboolean(L, false);
		} else {
			lua_pushboolean(L, LuaClass1->UClass == LuaClass2->UClass);
		}

		return 1;
	}

	int luaClassLt(lua_State* L) {
		FLuaClass* LuaClass1 = luaFIN_toLuaClass(L, 1);
		FLuaClass* LuaClass2 = luaFIN_toLuaClass(L, 2);

		if (!LuaClass1 || !LuaClass2) {
			lua_pushboolean(L, false);
		} else {
			lua_pushboolean(L, LuaClass1->UClass < LuaClass2->UClass);
		}

		return 1;
	}

	int luaClassLe(lua_State* L) {
		FLuaClass* LuaClass1 = luaFIN_toLuaClass(L, 1);
		FLuaClass* LuaClass2 = luaFIN_toLuaClass(L, 2);

		if (!LuaClass1 || !LuaClass2) {
			lua_pushboolean(L, false);
		} else {
			lua_pushboolean(L, LuaClass1->UClass <= LuaClass2->UClass);
		}

		return 1;
	}
	
	int luaClassIndex(lua_State* L) {
		const int thisIndex = 1;
		const int nameIndex = 2;
		
		FLuaClass* LuaClass = luaFIN_checkLuaClass(L, thisIndex);
		FString MemberName = luaFIN_toFString(L, nameIndex);

		FFINExecutionContext Context(LuaClass->UClass);
		return luaFIN_pushFunctionOrGetProperty(L, thisIndex, LuaClass->FINClass, MemberName, FIN_Func_ClassFunc, FIN_Prop_ClassProp, Context, true);
	}

	int luaClassNewIndex(lua_State* L) {
		const int thisIndex = 1;
		const int nameIndex = 2;
		const int valueIndex = 3;
		
		FLuaClass* LuaClass = luaFIN_checkLuaClass(L, thisIndex);
		FString MemberName = luaFIN_toFString(L, nameIndex);

		FFINExecutionContext Context(LuaClass->UClass);
		luaFIN_tryExecuteSetProperty(L, thisIndex, LuaClass->FINClass, MemberName, FIN_Prop_ClassProp, Context, valueIndex, true);
		return 0;
	}

	int luaClassToString(lua_State* L) {
		FLuaClass* LuaClass = luaFIN_checkLuaClass(L, 1);
		luaFIN_pushFString(L, FFINReflection::ClassReferenceText(LuaClass->FINClass));
		return 1;
	}

	int luaClassUnpersist(lua_State* L) {
		FString FINClassInternalName = luaFIN_checkFString(L, lua_upvalueindex(1));
		FString UClassPath = luaFIN_checkFString(L, lua_upvalueindex(2));

		UFINClass* FINClass = FFINReflection::Get()->FindClass(FINClassInternalName);
		UClass* Class = Cast<UClass>(FSoftObjectPath(UClassPath).TryLoad());

		luaFIN_pushClass(L, Class, FINClass);
		return 1;
	}

	int luaClassPersist(lua_State* L) {
		FLuaClass* LuaClass = luaFIN_checkLuaClass(L, 1);
		luaFIN_pushFString(L, LuaClass->FINClass->GetInternalName());
		luaFIN_pushFString(L, LuaClass->UClass->GetClassPathName().ToString());
		lua_pushcclosure(L, luaClassUnpersist, 2);
		return 1;
	}
	
	static const luaL_Reg luaClassMetatable[] = {
		{"__eq", luaClassEq},
		{"__lt", luaClassLt},
		{"__le", luaClassLe},
		{"__index", luaClassIndex},
		{"__newindex", luaClassNewIndex},
		{"__tostring", luaClassToString},
		{"__persist", luaClassPersist},
		{NULL, NULL}
	};
	
	bool luaFIN_pushClass(lua_State* L, UClass* Class, UFINClass* FINClass) {
		if (!Class || !FINClass) {
			lua_pushnil(L);
			return false;
		}

		FLuaClass* LuaClass = (FLuaClass*)lua_newuserdata(L, sizeof(FLuaClass));
		LuaClass->UClass = Class;
		LuaClass->FINClass = FINClass;
		luaL_setmetatable(L, FIN_LUA_CLASS_METATABLE_NAME);
		
		return true;
	}
	
	FLuaClass* luaFIN_toLuaClass(lua_State* L, int Index) {
		return (FLuaClass*)luaL_testudata(L, Index, FIN_LUA_CLASS_METATABLE_NAME);
	}

	FLuaClass* luaFIN_checkLuaClass(lua_State* L, int Index) {
		FLuaClass* LuaClass = luaFIN_toLuaClass(L, Index);
		if (!LuaClass) luaFIN_typeError(L, Index, FFINReflection::ClassReferenceText(nullptr));
		return LuaClass;
	}

	int luaClassLibIndex(lua_State* L) {
		FString ClassName = luaFIN_checkFString(L, 2);
		UFINClass* Class = FFINReflection::Get()->FindClass(ClassName);
		if (Class) {
			luaFIN_pushClass(L, Class);
		} else {
			lua_pushnil(L);
		}
		return 1;
	}
	
	static const luaL_Reg luaClassLibMetatable[] = {
		{"__index", luaClassLibIndex},
		{NULL, NULL}
	};

	void setupClassSystem(lua_State* L) {
		PersistSetup("ClassSystem", -2);

		// Register & Persist Class-Metatable
		luaL_newmetatable(L, FIN_LUA_CLASS_METATABLE_NAME);		// ..., ClassMetatable
		luaL_setfuncs(L, luaClassMetatable, 0);
		lua_pushstring(L, FIN_LUA_CLASS_METATABLE_NAME);				// ..., ClassMetatable, string
		lua_setfield(L, -2, "__metatable");						// ..., ClassMetatable
		PersistTable(FIN_LUA_CLASS_METATABLE_NAME, -1);
		lua_pop(L, 1);												// ...
		lua_pushcfunction(L, luaClassUnpersist);						// ..., ClassUnpersist
		PersistValue("ClassUnpersist");							// ...

		// Add & Persist ClassLib as global 'classes'
		lua_newuserdata(L, 0);										// ..., ClassLib
		luaL_newmetatable(L, FIN_LUA_CLASS_LIB_METATABLE_NAME);		// ..., ClassLib, ClassLibMetatable
		luaL_setfuncs(L, luaClassLibMetatable, 0);
		lua_pushstring(L, FIN_LUA_CLASS_LIB_METATABLE_NAME);			// ..., ClassLib, ClassLibMetatable, bool
		lua_setfield(L, -2, "__metatable");						// ..., ClassLib, ClassLibMetatable
		PersistTable(FIN_LUA_CLASS_LIB_METATABLE_NAME, -1);
		lua_setmetatable(L, -2);									// ..., ClassLib
		lua_setglobal(L, "classes");								// ...
		PersistGlobal("classes");
	}
}
