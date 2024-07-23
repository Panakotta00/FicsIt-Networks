#include "FINLua/Reflection/LuaObject.h"

#include "FicsItNetworksLuaModule.h"
#include "FINLua/Reflection/LuaRef.h"
#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "Logging/StructuredLog.h"
#include "Network/FINNetworkUtils.h"
#include "tracy/Tracy.hpp"

namespace FINLua {
	FLuaObject::FLuaObject(const FFINNetworkTrace& Object, UFINKernelSystem* Kernel) : Object(Object), Kernel(Kernel) {
		Type = FFINReflection::Get()->FindClass(Object.GetUnderlyingPtr()->GetClass());
		Kernel->AddReferencer(this, &CollectReferences);
	}

	FLuaObject::FLuaObject(const FLuaObject& Other) : Type(Other.Type), Object(Other.Object), Kernel(Other.Kernel) {
		Kernel->AddReferencer(this, &CollectReferences);
	}

	FLuaObject::~FLuaObject() {
		Kernel->RemoveReferencer(this);
	}

	void FLuaObject::CollectReferences(void* Obj, FReferenceCollector& Collector) {
		static_cast<FLuaObject*>(Obj)->Object.AddStructReferencedObjects(Collector);
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
					return UFINLuaProcessor::luaAPIReturn(L, 1);
				}

				lua_pushboolean(L, GetTypeHash(LuaObject1->Object) == GetTypeHash(LuaObject2->Object));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__lt
			 * @DisplayName		Less Than
			 */)", __lt) {
				FLuaObject* LuaObject1 = luaFIN_toLuaObject(L, 1, nullptr);
				FLuaObject* LuaObject2 = luaFIN_toLuaObject(L, 2, nullptr);
				if (!LuaObject1 || !LuaObject2) {
					lua_pushboolean(L, false);
					return UFINLuaProcessor::luaAPIReturn(L, 1);
				}

				lua_pushboolean(L, GetTypeHash(LuaObject1->Object) < GetTypeHash(LuaObject2->Object));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__le
			 * @DisplayName		Less or Equal Than
			 */)", __le) {
				FLuaObject* LuaObject1 = luaFIN_toLuaObject(L, 1, nullptr);
				FLuaObject* LuaObject2 = luaFIN_toLuaObject(L, 2, nullptr);
				if (!LuaObject1 || !LuaObject2) {
					lua_pushboolean(L, false);
					return UFINLuaProcessor::luaAPIReturn(L, 1);
				}

				lua_pushboolean(L, GetTypeHash(LuaObject1->Object) <= GetTypeHash(LuaObject2->Object));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
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

				UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(*LuaObject->Object);
				if (NetworkHandler) {
					if (MemberName == "id") {
						lua_pushstring(L, TCHAR_TO_UTF8(*IFINNetworkComponent::Execute_GetID(NetworkHandler).ToString()));
						return 1;
					}
					if (MemberName == "nick") {
						lua_pushstring(L, TCHAR_TO_UTF8(*IFINNetworkComponent::Execute_GetNick(NetworkHandler)));
						return 1;
					}
				}

				FFINExecutionContext Context(LuaObject->Object);
				return luaFIN_pushFunctionOrGetProperty(L, thisIndex, LuaObject->Type, MemberName, FIN_Func_MemberFunc, FIN_Prop_Attrib, Context, true);
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

				UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(*LuaObject->Object);
				if (NetworkHandler) {
					if (MemberName == "nick") {
						FString nick = luaFIN_toFString(L, valueIndex);
						IFINNetworkComponent::Execute_SetNick(NetworkHandler, nick);
						return 0;
					}
				}

				FFINExecutionContext Context(LuaObject->Object);
				luaFIN_tryExecuteSetProperty(L, thisIndex, LuaObject->Type, MemberName, FIN_Prop_Attrib, Context, valueIndex, true);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, nullptr);
				luaFIN_pushFString(L, FFINReflection::ObjectReferenceText(LuaObject->Type));
				return 1;
			}

			int luaObjectUnpersist(lua_State* L) {
				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;
				FFINNetworkTrace Object = Storage.GetTrace(luaL_checkinteger(L, lua_upvalueindex(1)));

				luaFIN_pushObject(L, Object);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, nullptr);

				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;
				lua_pushinteger(L, Storage.Add(LuaObject->Object));

				lua_pushcclosure(L, &luaObjectUnpersist, 1);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		Garbage Collect
			 */)", __gc) {
				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, nullptr);
				LuaObject->~FLuaObject();
				return 0;
			}
		}

		LuaModulePostSetup() {
			PersistenceNamespace("ReflectionObject");

			lua_pushcfunction(L, Object::luaObjectUnpersist);
			PersistValue("ObjectUnpersist");
		}
	}

	void luaFIN_pushObject(lua_State* L, const FFINNetworkTrace& Object) {
		if (!Object.GetUnderlyingPtr()) {
			lua_pushnil(L);
			UE_LOGFMT(LogFicsItNetworksLuaReflection, Verbose, "[{Runtime}] Tried to push invalid/null Object to Lua-Stack ({Index})", L, lua_gettop(L));
			return;
		}
		FLuaObject* LuaObject = static_cast<FLuaObject*>(lua_newuserdata(L, sizeof(FLuaObject)));
		new(LuaObject) FLuaObject(Object, UFINLuaProcessor::luaGetProcessor(L)->GetKernel());
		luaL_setmetatable(L, ReflectionSystemObject::Object::_Name);
		UE_LOGFMT(LogFicsItNetworksLuaReflection, VeryVerbose, "[{Runtime}] Pushed Object '{Object}' to Lua-Stack ({Index})", L, Object->GetFullName(), lua_gettop(L));
	}

	FLuaObject* luaFIN_toLuaObject(lua_State* L, int Index, UFINClass* ParentClass) {
		FLuaObject* LuaObject = static_cast<FLuaObject*>(luaL_testudata(L, Index, ReflectionSystemObject::Object::_Name));
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

	FLuaObject* luaFIN_checkLuaObject(lua_State* L, int Index, UFINClass* ParentClass) {
		FLuaObject* LuaObject = luaFIN_toLuaObject(L, Index, ParentClass);
		if (!LuaObject) luaFIN_typeError(L, Index, FFINReflection::ObjectReferenceText(ParentClass));
		return LuaObject;
	}

	TOptional<FFINNetworkTrace> luaFIN_toObject(lua_State* L, int Index, UFINClass* ParentType) {
		FLuaObject* LuaObject = luaFIN_toLuaObject(L, Index, ParentType);
		if (!LuaObject) {
			if (lua_isnil(L, Index)) return FFINNetworkTrace();
			return TOptional<FFINNetworkTrace>();
		}
		return LuaObject->Object;
	}

	FFINNetworkTrace luaFIN_checkObject(lua_State* L, int Index, UFINClass* ParentType) {
		TOptional<FFINNetworkTrace> Object = luaFIN_toObject(L, Index, ParentType);
		if (!Object.IsSet()) luaFIN_typeError(L, Index, FFINReflection::ObjectReferenceText(ParentType));
		return *Object;
	}

	FString luaFIN_getLuaObjectTypeName() {
		return UTF8_TO_TCHAR(ReflectionSystemObject::Object::_Name);
	}
}
