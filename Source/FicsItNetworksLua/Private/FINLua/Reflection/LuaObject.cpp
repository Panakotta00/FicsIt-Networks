#include "FINLua/Reflection/LuaObject.h"

#include "FicsItNetworksLuaModule.h"
#include "FicsItReflection.h"
#include "FINLua/Reflection/LuaRef.h"
#include "FINLuaProcessor.h"
#include "FINLuaReferenceCollector.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "Logging/StructuredLog.h"
#include "tracy/Tracy.hpp"
#include "FINLuaRuntime.h"
#include "FINNetworkComponent.h"

namespace FINLua {
	FLuaObject::FLuaObject(const FFIRTrace& Object, FFINLuaReferenceCollector* ReferenceCollector) : FFINLuaReferenceCollected(ReferenceCollector), Object(Object) {
		Type = FFicsItReflectionModule::Get().FindClass(Object.GetUnderlyingPtr()->GetClass());
	}

	void FLuaObject::CollectReferences(FReferenceCollector& Collector) {
		Object.AddStructReferencedObjects(Collector);
	}

	LuaModule(R"(/**
	 * @LuaModule		ReflectionSystemObjectModule
	 * @DisplayName		Reflection-System Object Module
	 * @Dependency		ReflectionSystemBaseModule
	 *
	 * This module provides all the functionallity for the usage of objects from the reflection system in Lua.
	 */)", ReflectionSystemObject) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	Object
		 * @DisplayName		Object
		 */)", Object) {

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__eq
			 * @DisplayName		Equal
			 */)", __eq) {
				FLuaObject* LuaObject1 = luaFIN_toLuaObject(L, 1, nullptr);
				FLuaObject* LuaObject2 = luaFIN_toLuaObject(L, 2, nullptr);
				if (!LuaObject1 || !LuaObject2) {
					lua_pushboolean(L, false);
					return 1;
				}

				lua_pushboolean(L, GetTypeHash(LuaObject1->Object) == GetTypeHash(LuaObject2->Object));
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__lt
			 * @DisplayName		Less Than
			 */)", __lt) {
				FLuaObject* LuaObject1 = luaFIN_toLuaObject(L, 1, nullptr);
				FLuaObject* LuaObject2 = luaFIN_toLuaObject(L, 2, nullptr);
				if (!LuaObject1 || !LuaObject2) {
					lua_pushboolean(L, false);
					return 1;
				}

				lua_pushboolean(L, GetTypeHash(LuaObject1->Object) < GetTypeHash(LuaObject2->Object));
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__le
			 * @DisplayName		Less or Equal Than
			 */)", __le) {
				FLuaObject* LuaObject1 = luaFIN_toLuaObject(L, 1, nullptr);
				FLuaObject* LuaObject2 = luaFIN_toLuaObject(L, 2, nullptr);
				if (!LuaObject1 || !LuaObject2) {
					lua_pushboolean(L, false);
					return 1;
				}

				lua_pushboolean(L, GetTypeHash(LuaObject1->Object) <= GetTypeHash(LuaObject2->Object));
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				ZoneScoped;
				const int thisIndex = 1;
				const int nameIndex = 2;

				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, thisIndex, nullptr);
				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFIRExecutionContext Context(LuaObject->Object);
				return luaFIN_pushFunctionOrGetProperty(L, thisIndex, LuaObject->Type, MemberName, FIR_Func_MemberFunc, FIR_Prop_Attrib, Context, true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__newindex
			 * @DisplayName		New Index
			 */)", __newindex) {
				ZoneScoped;

				const int thisIndex = 1;
				const int nameIndex = 2;
				const int valueIndex = 3;

				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, thisIndex, nullptr);
				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFIRExecutionContext Context(LuaObject->Object);
				luaFIN_tryExecuteSetProperty(L, thisIndex, LuaObject->Type, MemberName, FIR_Prop_Attrib, Context, valueIndex, true);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, nullptr);
				luaFIN_pushFString(L, luaFIN_getObjectID(LuaObject->Object.GetUnderlyingPtr(), LuaObject->Type));
				return 1;
			}

			int luaObjectUnpersist(lua_State* L) {
				FFINLuaRuntimePersistenceState& Storage = luaFIN_getPersistence(L);
				FFIRTrace Object = Storage.GetTrace(luaL_checkinteger(L, lua_upvalueindex(1)));

				luaFIN_pushObject(L, Object);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, nullptr);

				FFINLuaRuntimePersistenceState& Storage = luaFIN_getPersistence(L);
				lua_pushinteger(L, Storage.Add(LuaObject->Object));

				lua_pushcclosure(L, &luaObjectUnpersist, 1);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		Garbage Collect
			 */)", __gc) {
				FLuaObject* LuaObject = luaFIN_toRawLuaObject(L, 1);
				if (LuaObject) {
					LuaObject->~FLuaObject();
				}
				return 0;
			}
		}

		LuaModulePostSetup() {
			PersistenceNamespace("ReflectionObject");

			lua_pushcfunction(L, Object::luaObjectUnpersist);
			PersistValue("ObjectUnpersist");
		}
	}

	void luaFIN_pushObject(lua_State* L, const FFIRTrace& Object) {
		if (!Object.GetUnderlyingPtr()) {
			lua_pushnil(L);
			UE_LOGFMT(LogFicsItNetworksLuaReflection, Verbose, "[{Runtime}] Tried to push invalid/null Object to Lua-Stack ({Index})", L, lua_gettop(L));
			return;
		}
		FLuaObject* LuaObject = static_cast<FLuaObject*>(lua_newuserdata(L, sizeof(FLuaObject)));
		new(LuaObject) FLuaObject(Object, luaFIN_getReferenceCollector(L));
		luaL_setmetatable(L, ReflectionSystemObject::Object::_Name);
		UE_LOGFMT(LogFicsItNetworksLuaReflection, VeryVerbose, "[{Runtime}] Pushed Object '{Object}' to Lua-Stack ({Index})", L, Object->GetFullName(), lua_gettop(L));
	}

	FLuaObject* luaFIN_toRawLuaObject(lua_State* L, int Index) {
		return static_cast<FLuaObject*>(luaL_testudata(L, Index, ReflectionSystemObject::Object::_Name));
	}

	FLuaObject* luaFIN_toLuaObject(lua_State* L, int Index, UFIRClass* ParentClass) {
		FLuaObject* LuaObject = luaFIN_toRawLuaObject(L, Index);
		if (LuaObject && LuaObject->Object.IsValidPtr()) {
			if (LuaObject->Type->IsChildOf(ParentClass)) {
				UE_LOGFMT(LogFicsItNetworksLuaReflection, VeryVerbose, "[{Runtime}] Got Object '{Object}' from Lua-Stack ({Index}/{AbsIndex})", L, LuaObject->Object->GetFullName(), Index, lua_absindex(L, Index));
				return LuaObject;
			}
			UE_LOGFMT(LogFicsItNetworksLuaReflection, Verbose, "[{Runtime}] Got Object '{Object}' from Lua-Stack ({Index}/{AbsIndex}) but is not of valid Type '{Class}'", L, LuaObject->Object->GetFullName(), Index, lua_absindex(L, Index), ParentClass ? ParentClass->GetInternalName() : "");
			return nullptr;
		}
		UE_LOGFMT(LogFicsItNetworksLuaReflection, Verbose, "[{Runtime}] Failed to get Object of Type '{Class}' from Lua-Stack ({Index}/{AbsIndex})", L, ParentClass ? ParentClass->GetInternalName() : "", Index, lua_absindex(L, Index));
		return nullptr;
	}

	FLuaObject* luaFIN_checkLuaObject(lua_State* L, int Index, UFIRClass* ParentClass) {
		FLuaObject* LuaObject = luaFIN_toLuaObject(L, Index, ParentClass);
		if (!LuaObject) luaFIN_typeError(L, Index, FFicsItReflectionModule::ObjectReferenceText(ParentClass));
		return LuaObject;
	}

	TOptional<FFIRTrace> luaFIN_toObject(lua_State* L, int Index, UFIRClass* ParentType) {
		FLuaObject* LuaObject = luaFIN_toLuaObject(L, Index, ParentType);
		if (!LuaObject) {
			if (lua_isnil(L, Index)) return FFIRTrace();
			return TOptional<FFIRTrace>();
		}
		return LuaObject->Object;
	}

	FFIRTrace luaFIN_checkObject(lua_State* L, int Index, UFIRClass* ParentType) {
		TOptional<FFIRTrace> Object = luaFIN_toObject(L, Index, ParentType);
		if (!Object.IsSet()) luaFIN_typeError(L, Index, FFicsItReflectionModule::ObjectReferenceText(ParentType));
		return *Object;
	}

	FString luaFIN_getLuaObjectTypeName() {
		return UTF8_TO_TCHAR(ReflectionSystemObject::Object::_Name);
	}
}
