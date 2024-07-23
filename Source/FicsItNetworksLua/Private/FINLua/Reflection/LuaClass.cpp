#include "FINLua/Reflection/LuaClass.h"

#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "FINLua/Reflection/LuaRef.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		ReflectionSystemClassModule
	 * @DisplayName		Reflection-System Class Module
	 * @Dependency		ReflectionSystemBaseModule
	 *
	 * This module provides all the functionallity for the usage of Types/Classes from the reflection system in Lua.
	 */)", ReflectionSystemClass) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	Class
		 * @DisplayName		Class
		 */)", Class) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__eq
			 * @DisplayName		Equal
			 */)", __eq) {
				FLuaClass* LuaClass1 = luaFIN_toLuaClass(L, 1);
				FLuaClass* LuaClass2 = luaFIN_toLuaClass(L, 2);

				if (!LuaClass1 || !LuaClass2) {
					lua_pushboolean(L, false);
				} else {
					lua_pushboolean(L, LuaClass1->UClass == LuaClass2->UClass);
				}

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__lt
			 * @DisplayName		Less Than
			 */)", __lt) {
				FLuaClass* LuaClass1 = luaFIN_toLuaClass(L, 1);
				FLuaClass* LuaClass2 = luaFIN_toLuaClass(L, 2);

				if (!LuaClass1 || !LuaClass2) {
					lua_pushboolean(L, false);
				} else {
					lua_pushboolean(L, LuaClass1->UClass < LuaClass2->UClass);
				}

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__le
			 * @DisplayName		Less or Equal Than
			 */)", __le) {
				FLuaClass* LuaClass1 = luaFIN_toLuaClass(L, 1);
				FLuaClass* LuaClass2 = luaFIN_toLuaClass(L, 2);

				if (!LuaClass1 || !LuaClass2) {
					lua_pushboolean(L, false);
				} else {
					lua_pushboolean(L, LuaClass1->UClass <= LuaClass2->UClass);
				}

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				const int thisIndex = 1;
				const int nameIndex = 2;

				FLuaClass* LuaClass = luaFIN_checkLuaClass(L, thisIndex);
				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFINExecutionContext Context(LuaClass->UClass);
				return luaFIN_pushFunctionOrGetProperty(L, thisIndex, LuaClass->FINClass, MemberName, FIN_Func_ClassFunc, FIN_Prop_ClassProp, Context, true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__newindex
			 * @DisplayName		New Index
			 */)", __newindex) {
				const int thisIndex = 1;
				const int nameIndex = 2;
				const int valueIndex = 3;

				FLuaClass* LuaClass = luaFIN_checkLuaClass(L, thisIndex);
				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFINExecutionContext Context(LuaClass->UClass);
				luaFIN_tryExecuteSetProperty(L, thisIndex, LuaClass->FINClass, MemberName, FIN_Prop_ClassProp, Context, valueIndex, true);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				FLuaClass* LuaClass = luaFIN_checkLuaClass(L, 1);
				luaFIN_pushFString(L, LuaClass->FINClass->GetInternalName());
				luaFIN_pushFString(L, LuaClass->UClass->GetClassPathName().ToString());
				lua_pushcclosure(L, luaClassUnpersist, 2);
				return 1;
			}
		}

		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	ClassLib
		 * @DisplayName		ClassLib
		 */)", ClassLib) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				FString ClassName = luaFIN_checkFString(L, 2);
				UFINClass* Class = FFINReflection::Get()->FindClass(ClassName);
				if (Class) {
					luaFIN_pushClass(L, Class);
				} else {
					lua_pushnil(L);
				}
				return 1;
			}
		}

		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		classes
		 * @DisplayName		Classes
		 *
		 * A peseudo table that can be used to look up classes/types.
		 * Ideal usage of it is `classes.Inventory`.
		 * Since the type lookup occurs in the metatable-function, you can still use the []-Operator in the case
		 * you want to look up something based on a dynamic string e.g. `structs[myStringVar]` works just fine.
		)", structs) {
			lua_pushnil(L);
		}

		LuaModulePostSetup() {
			PersistenceNamespace("ReflectionClass");

			lua_pushcfunction(L, Class::luaClassUnpersist);
			PersistValue("ClassUnpersist");

			lua_newuserdata(L, 0);
			luaL_setmetatable(L, ClassLib::_Name);
			lua_setglobal(L, "classes");
			PersistGlobal("classes");
		}
	}

	bool luaFIN_pushClass(lua_State* L, UClass* Class, UFINClass* FINClass) {
		if (!Class || !FINClass) {
			lua_pushnil(L);
			return false;
		}

		FLuaClass* LuaClass = (FLuaClass*)lua_newuserdata(L, sizeof(FLuaClass));
		LuaClass->UClass = Class;
		LuaClass->FINClass = FINClass;
		luaL_setmetatable(L, ReflectionSystemClass::Class::_Name);

		return true;
	}

	FLuaClass* luaFIN_toLuaClass(lua_State* L, int Index) {
		return (FLuaClass*)luaL_testudata(L, Index, ReflectionSystemClass::Class::_Name);
	}

	FLuaClass* luaFIN_checkLuaClass(lua_State* L, int Index) {
		FLuaClass* LuaClass = luaFIN_toLuaClass(L, Index);
		if (!LuaClass) luaFIN_typeError(L, Index, FFINReflection::ClassReferenceText(nullptr));
		return LuaClass;
	}

	FString luaFIN_getLuaClassTypeName() {
		return UTF8_TO_TCHAR(ReflectionSystemClass::Class::_Name);
	}
}
