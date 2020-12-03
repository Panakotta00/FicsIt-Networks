#include "LuaInstance.h"

#include "LuaProcessor.h"
#include "LuaProcessorStateStorage.h"

#include "Network/FINNetworkComponent.h"
#include "Network/FINNetworkCustomType.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINClass.h"
#include "Reflection/FINFunction.h"
#include "Reflection/FINReflection.h"
#include "util/Logging.h"

#define INSTANCE_TYPE "InstanceType"
#define INSTANCE_CACHE "InstanceCache"
#define INSTANCE_FUNC_DATA "InstanceFuncData"
#define CLASS_INSTANCE_FUNC_DATA "ClassInstanceFuncData"
#define CLASS_INSTANCE_META_SUFFIX "-Class"

#define OffsetParam(type, off) (type*)((std::uint64_t)param + off)

#pragma optimize("", off)
namespace FicsItKernel {
	namespace Lua {
		std::map<UObject*, std::mutex> objectLocks;
		TMap<UFINClass*, FString> ClassToMetaName;
		TMap<UFINClass*, FString> ClassToClassMetaName;
		TMap<FString, UFINClass*> MetaNameToClass;
		
		void luaInstanceType(lua_State* L, LuaInstanceType&& instanceType);
		int luaInstanceTypeUnpersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));
			
			luaInstanceType(L, LuaInstanceType{Cast<UClass>(storage->GetRef(luaL_checkinteger(L, lua_upvalueindex(1))))});

			return 1;
		}

		int luaInstanceTypePersist(lua_State* L) {
			LuaInstanceType* type = static_cast<LuaInstanceType*>(luaL_checkudata(L, 1, INSTANCE_TYPE));
			
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			lua_pushinteger(L, storage->Add(type->type));
			
			// create & return closure
			lua_pushcclosure(L, &luaInstanceTypeUnpersist, 1);
			return 1;
		}

		int luaInstanceTypeGC(lua_State* L) {
			LuaInstanceType* type = static_cast<LuaInstanceType*>(luaL_checkudata(L, 1, INSTANCE_TYPE));
			type->~LuaInstanceType();
			return 0;
		}

		static const luaL_Reg luaInstanceTypeLib[] = {
			{"__persist", luaInstanceTypePersist},
			{"__gc", luaInstanceTypeGC},
			{NULL, NULL}
		};

		void luaInstanceType(lua_State* L, LuaInstanceType&& instanceType) {
			LuaInstanceType* type = static_cast<LuaInstanceType*>(lua_newuserdata(L, sizeof(LuaInstanceType)));
			new (type) LuaInstanceType(std::move(instanceType));
			luaL_setmetatable(L, INSTANCE_TYPE);
		}

		LuaInstance* GetInstance(lua_State* L, int Index, UFINClass** OutClass = nullptr) {
			Index = lua_absindex(L, Index);
			FString TypeName;
			if (luaL_getmetafield(L, Index, "__name") == LUA_TSTRING) {
				TypeName = lua_tostring(L, -1);
				lua_pop(L, 1);
			} else if (lua_type(L, Index) == LUA_TLIGHTUSERDATA) {
				TypeName = "light userdata";
			} else {
				TypeName = luaL_typename(L, Index);
			}
			UFINClass** Class = MetaNameToClass.Find(TypeName);
			if (!Class) luaL_argerror(L, Index, "Instance is invalid type");
			if (OutClass) *OutClass = *Class;
			return (LuaInstance*) lua_touserdata(L, Index);
		}

		LuaInstance* CheckAndGetInstance(lua_State* L, int Index, UFINClass** OutClass = nullptr) {
			LuaInstance* Instance = GetInstance(L, Index, OutClass);
			if (!Instance) luaL_argerror(L, Index, "Instance is invalid");
			return Instance;
		}

		int luaInstanceFuncCall(lua_State* L) {		// Instance, args..., up: UFINFunction
			// get function
			UFINFunction* Func = (UFINFunction*) luaL_checkudata(L, lua_upvalueindex(1), INSTANCE_FUNC_DATA);
			UFINClass* Class = Cast<UFINClass>(Func->GetOuter());
			if (!Class) return luaL_error(L, "Function is invalid (internal error)");
			
			// get and check instance
			FString* MetaName = ClassToMetaName.Find(Class);
			if (!MetaName) return luaL_error(L, "Function name is invalid (internal error)");
			LuaInstance* Instance = (LuaInstance*) luaL_checkudata(L, 1, TCHAR_TO_UTF8(**MetaName));
			UObject* Obj = *Instance->Trace;
			if (!Obj) return luaL_argerror(L, 1, "Instance is invalid");

			// sync call if necessery
			TSharedPtr<FLuaSyncCall> SyncCall;
			if (!(Func->GetFunctionFlags() & FIN_Func_RT_Async)) SyncCall = MakeShared<FLuaSyncCall>(L);

			TArray<FINAny> Input;
			int args = lua_gettop(L);
			for (int i = 2; i <= args; ++i) {
				FINAny Param;
				luaToNetworkValue(L, i, Param);
				Input.Add(Param);
			}
			args = 0;
			TArray<FINAny> Output = Func->Execute(FFINExecutionContext(Instance->Trace), Input);
			for (const FINAny& Value : Output) {
				networkValueToLua(L, Value);
				++args;
			}
			
			return LuaProcessor::luaAPIReturn(L, args);
		}

		int luaInstanceIndex(lua_State* L) {																			// Instance, FuncName
			// get instance
			UFINClass* Class;
			LuaInstance* Instance = CheckAndGetInstance(L, 1, &Class);
				
			// get member name
			FString MemberName = lua_tostring(L, 2);
			
			UObject* Obj = *Instance->Trace;
			UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(Obj);

			if (!IsValid(Obj)) {
				return luaL_error(L, "Instance is invalid");
			}
			
			// check for network component stuff
			if (NetworkHandler) {
				if (MemberName == "id") {
					lua_pushstring(L, TCHAR_TO_UTF8(*IFINNetworkComponent::Execute_GetID(NetworkHandler).ToString()));
					return LuaProcessor::luaAPIReturn(L, 1);
				}
				if (MemberName == "nick") {
					lua_pushstring(L, TCHAR_TO_UTF8(*IFINNetworkComponent::Execute_GetNick(NetworkHandler)));
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}

			// try to find property
			for (UFINProperty* Property : Class->GetProperties()) {
				if ((Property->GetPropertyFlags() & FIN_Prop_Attrib) && Property->GetInternalName() == MemberName) {
					TSharedPtr<FLuaSyncCall> SyncCall;
					if (!(Property->GetPropertyFlags() & FIN_Prop_RT_Async)) SyncCall = MakeShared<FLuaSyncCall>(L);
					networkValueToLua(L, Property->GetValue(Obj));
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}

			// get cache function
			luaL_getmetafield(L, 1, INSTANCE_CACHE);																// Instance, FuncName, InstanceCache
			if (lua_getfield(L, -1, TCHAR_TO_UTF8(*(ClassToMetaName[Class] + "_" + MemberName))) != LUA_TNIL) {	// Instance, FuncName, InstanceCache, CachedFunc
				return LuaProcessor::luaAPIReturn(L, 1);
			}																											// Instance, FuncName, InstanceCache, nil

			// try to find function
			for (UFINFunction* Function : Class->GetFunctions()) {
				if ((Function->GetFunctionFlags() & FIN_Func_MemberFunc) && Function->GetInternalName() == MemberName) {
					lua_pushlightuserdata(L, Function);
					luaL_setmetatable(L, INSTANCE_FUNC_DATA);
					lua_pushcclosure(L, &luaInstanceFuncCall, 1);
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}
			
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		int luaInstanceNewIndex(lua_State* L) {
			// get instance
			UFINClass* Class;
			LuaInstance* Instance = CheckAndGetInstance(L, 1, &Class);
				
			// get member name
			FString MemberName = lua_tostring(L, 2);
			
			UObject* Obj = *Instance->Trace;
			UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(Obj);

			if (!IsValid(Obj)) {
				return luaL_error(L, "Instance is invalid");
			}
			
			// check for network component stuff
			if (NetworkHandler) {
				if (MemberName == "id") {
					return luaL_error(L, TCHAR_TO_UTF8(*("Property '" + MemberName + "' is read-only")));
				}
				if (MemberName == "nick") {
					FString Nick = luaL_checkstring(L, 3);
					IFINNetworkComponent::Execute_SetNick(NetworkHandler, Nick);
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}

			// try to find property
			for (UFINProperty* Property : Class->GetProperties()) {
				if (Property->GetPropertyFlags() & FIN_Prop_Attrib && Property->GetInternalName() == MemberName) {
					TSharedPtr<FLuaSyncCall> SyncCall;
					if (!(Property->GetPropertyFlags() & FIN_Prop_RT_Async)) SyncCall = MakeShared<FLuaSyncCall>(L);
					FINAny Value;
					luaToNetworkValue(L, 3, Value);
					Property->SetValue(Obj, Value);
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}
			
			return luaL_error(L, TCHAR_TO_UTF8(*("No property with name '" + MemberName + "' found")));
		}

		int luaInstanceEQ(lua_State* L) {
			LuaInstance* inst1 = CheckAndGetInstance(L, 1);
			LuaInstance* inst2 = GetInstance(L, 2);
			if (!inst1 || !inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, inst1->Trace.IsEqualObj(inst2->Trace));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaInstanceLt(lua_State* L) {
			LuaInstance* inst1 = CheckAndGetInstance(L, 1);
			LuaInstance* inst2 = GetInstance(L, 2);
			if (!inst1 || !inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->Trace.GetUnderlyingPtr()) < GetTypeHash(inst2->Trace.GetUnderlyingPtr()));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaInstanceLe(lua_State* L) {
			LuaInstance* inst1 = CheckAndGetInstance(L, 1);
			LuaInstance* inst2 = GetInstance(L, 2);
			if (!inst1 || !inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->Trace.GetUnderlyingPtr()) <= GetTypeHash(inst2->Trace.GetUnderlyingPtr()));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaInstanceToString(lua_State* L) {
			UFINClass* Class;
			LuaInstance* Instance = CheckAndGetInstance(L, 1, &Class);
			UObject* Obj = *Instance->Trace;
			if (!IsValid(Obj)) return luaL_argerror(L, 1, "Instance is invalid");
			UClass* Type = Obj->GetClass();
			FString Msg = Class->GetInternalName();
			if (Type->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
				FString Nick = IFINNetworkComponent::Execute_GetNick(Obj);
				if (Nick.Len() > 0) Msg += " \"" + Nick + "\"";
				Msg += " " + IFINNetworkComponent::Execute_GetID(Obj).ToString();
			}
			lua_pushstring(L,  TCHAR_TO_UTF8(*Msg));
			return 1;
		}

		int luaInstanceUnpersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* Storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			// get trace and typename
			FFINNetworkTrace Trace = Storage->GetTrace(luaL_checkinteger(L, lua_upvalueindex(1)));
			
			// create instance
			newInstance(L, Trace);
			
			return 1;
		}

		int luaInstancePersist(lua_State* L) {
			// get instance
			LuaInstance* Instance = CheckAndGetInstance(L, 1);
			
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* Storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			// add trace to storage
			lua_pushinteger(L, Storage->Add(Instance->Trace));
			
			// create & return closure
			lua_pushcclosure(L, &luaInstanceUnpersist, 1);
			return 1;
		}

		int luaInstanceGC(lua_State* L) {
			LuaInstance* instance = CheckAndGetInstance(L, 1);
			instance->~LuaInstance();
			return 0;
		}

		static const luaL_Reg luaInstanceLib[] = {
			{"__index", luaInstanceIndex},
			{"__newindex", luaInstanceNewIndex},
			{"__eq", luaInstanceEQ},
			{"__lt", luaInstanceLt},
			{"__le", luaInstanceLe},
			{"__tostring", luaInstanceToString},
			{"__persist", luaInstancePersist},
			{"__gc", luaInstanceGC},
			{NULL, NULL}
		};

		bool newInstance(lua_State* L, FFINNetworkTrace Trace) {
			// check obj and if type is registered
			UObject* Obj = Trace.GetUnderlyingPtr().Get();
			UFINClass* Class = FFINReflection::Get()->FindClass(Obj->GetClass());
			if (!IsValid(Obj) || !Class) {
				lua_pushnil(L);
				return false;
			}

			// create instance
			LuaInstance* Instance = static_cast<LuaInstance*>(lua_newuserdata(L, sizeof(LuaInstance)));
			new (Instance) LuaInstance{Trace};

			FString MetaName = Class->GetInternalName();
			MetaNameToClass.FindOrAdd(MetaName) = Class;
			ClassToMetaName.FindOrAdd(Class) = MetaName;

			luaL_setmetatable(L, TCHAR_TO_UTF8(*MetaName));
			return true;
		}

		FFINNetworkTrace getObjInstance(lua_State* L, int index, UClass* clazz) {
			if (lua_isnil(L, index)) return FFINNetworkTrace(nullptr);
			LuaInstance* instance = CheckAndGetInstance(L, index);
			if (!instance->Trace.IsValid() || !instance->Trace->GetClass()->IsChildOf(clazz)) return FFINNetworkTrace(nullptr);
			return instance->Trace;
		}

		LuaClassInstance* GetClassInstance(lua_State* L, int Index, UFINClass** OutClass = nullptr) {
			Index = lua_absindex(L, Index);
			FString TypeName;
			if (luaL_getmetafield(L, Index, "__name") == LUA_TSTRING) {
				TypeName = lua_tostring(L, -1);
				lua_pop(L, 1);
			} else if (lua_type(L, Index) == LUA_TLIGHTUSERDATA) {
				TypeName = "light userdata";
			} else {
				TypeName = luaL_typename(L, Index);
			}
			UFINClass** Class = MetaNameToClass.Find(TypeName);
			if (!Class || !TypeName.EndsWith(CLASS_INSTANCE_META_SUFFIX)) luaL_argerror(L, Index, "ClassInstance is invalid type");
			if (OutClass) *OutClass = *Class;
			return (LuaClassInstance*) lua_touserdata(L, Index);
		}

		LuaClassInstance* CheckAndGetClassInstance(lua_State* L, int Index, UFINClass** OutClass = nullptr) {
			LuaClassInstance* Instance = GetClassInstance(L, Index, OutClass);
			if (!Instance) luaL_argerror(L, Index, "ClassInstance is invalid");
			return Instance;
		}
		
		int luaClassInstanceFuncCall(lua_State* L) {	// ClassInstance, Args..., up: FuncName, up: ClassInstance
			// get function
			UFINFunction* Func = (UFINFunction*) luaL_checkudata(L, lua_upvalueindex(1), CLASS_INSTANCE_FUNC_DATA);
			UFINClass* Class = Cast<UFINClass>(Func->GetOuter());
			if (!Class) return luaL_argerror(L, 1, "Function is invalid (internal error)");
			
			// get and check instance
			FString* MetaName = ClassToClassMetaName.Find(Class);
			if (!MetaName) return luaL_argerror(L, 1, "Function name is invalid (internal error)");
			LuaInstance* Instance = (LuaInstance*) luaL_checkudata(L, 1, TCHAR_TO_UTF8(**MetaName));
			UObject* Obj = *Instance->Trace;
			if (!Obj) return luaL_argerror(L, 1, "ClassInstance is invalid");

			// sync call if necessery
			TSharedPtr<FLuaSyncCall> SyncCall;
			if (!(Func->GetFunctionFlags() & FIN_Func_RT_Async)) SyncCall = MakeShared<FLuaSyncCall>(L);

			// TODO: Call the actual function!
			int args = 0;
			
			return LuaProcessor::luaAPIReturn(L, args);
		}
		
		int luaClassInstanceIndex(lua_State* L) {																		// ClassInstance, MemberName
			// get instance
			UFINClass* Type;
			LuaClassInstance* Instance = CheckAndGetClassInstance(L, 1, &Type);
				
			// get member name
			FString MemberName = lua_tostring(L, 2);
			
			UClass* Class = Instance->Class;
			
			if (!IsValid(Class)) {
				return luaL_error(L, "ClassInstance is invalid");
			}

			// try to find property
			for (UFINProperty* Property : Type->GetProperties()) {
				if (Property->GetPropertyFlags() & FIN_Prop_ClassProp && Property->GetInternalName() == MemberName) {
					TSharedPtr<FLuaSyncCall> SyncCall;
					if (!(Property->GetPropertyFlags() & FIN_Prop_RT_Async)) SyncCall = MakeShared<FLuaSyncCall>(L);
					networkValueToLua(L, Property->GetValue(Class));
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}

			// get cache function
			luaL_getmetafield(L, 1, INSTANCE_CACHE);																// Instance, FuncName, InstanceCache
			if (lua_getfield(L, -1, TCHAR_TO_UTF8(*(ClassToClassMetaName[Type] + "_" + MemberName))) != LUA_TNIL) {	// Instance, FuncName, InstanceCache, CachedFunc
				return LuaProcessor::luaAPIReturn(L, 1);
			}																											// Instance, FuncName, InstanceCache, nil

			// try to find function
			for (UFINFunction* Function : Type->GetFunctions()) {
				if (Function->GetFunctionFlags() & FIN_Func_ClassFunc && Function->GetInternalName() == MemberName) {
					lua_pushlightuserdata(L, Function);
					luaL_setmetatable(L, CLASS_INSTANCE_FUNC_DATA);
					lua_pushcclosure(L, &luaClassInstanceFuncCall, 1);
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}
			
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		int luaClassInstanceNewIndex(lua_State* L) {
			// get instance
			UFINClass* Type;
			LuaClassInstance* Instance = CheckAndGetClassInstance(L, 1, &Type);
				
			// get member name
			FString MemberName = lua_tostring(L, 2);
			
			UObject* Class = Instance->Class;

			if (!IsValid(Class)) {
				return luaL_error(L, "ClassInstance is invalid");
			}
			
			// try to find property
			for (UFINProperty* Property : Type->GetProperties()) {
				if (Property->GetPropertyFlags() & FIN_Prop_ClassProp && Property->GetInternalName() == MemberName) {
					TSharedPtr<FLuaSyncCall> SyncCall;
					if (!(Property->GetPropertyFlags() & FIN_Prop_RT_Async)) SyncCall = MakeShared<FLuaSyncCall>(L);
					FINAny Value;
					luaToNetworkValue(L, 3, Value);
					Property->SetValue(Class, Value);
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}
			
			return luaL_error(L, TCHAR_TO_UTF8(*("No property with name '" + MemberName + "' found")));
		}

		int luaClassInstanceEQ(lua_State* L) {
			LuaClassInstance* inst1 = CheckAndGetClassInstance(L, 1);
			LuaClassInstance* inst2 = GetClassInstance(L, 2);
			if (!inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, inst1->Class == inst2->Class);
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaClassInstanceLt(lua_State* L) {
			LuaClassInstance* inst1 = CheckAndGetClassInstance(L, 1);
			LuaClassInstance* inst2 = GetClassInstance(L, 2);
			if (!inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->Class) < GetTypeHash(inst2->Class));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaClassInstanceLe(lua_State* L) {
			LuaClassInstance* inst1 = CheckAndGetClassInstance(L, 1);
			LuaClassInstance* inst2 = GetClassInstance(L, 2);
			if (!inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->Class) <= GetTypeHash(inst2->Class));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaClassInstanceToString(lua_State* L) {
			UFINClass* Type;
			LuaClassInstance* Instance = CheckAndGetClassInstance(L, 1, &Type);
			
			lua_pushstring(L, TCHAR_TO_UTF8(*(Type->GetInternalName() + "-Class")));
			return 1;
		}

		int luaClassInstanceUnpersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* Storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			// get class
			UClass* Class = Cast<UClass>(Storage->GetRef(luaL_checkinteger(L, lua_upvalueindex(1))));
			
			// create instance
			newInstance(L, Class);
			
			return 1;
		}

		int luaClassInstancePersist(lua_State* L) {
			// get data
			UFINClass* Type;
			LuaClassInstance* Instance = CheckAndGetClassInstance(L, 1, &Type);

			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* Storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			// push type name to persist
			lua_pushinteger(L, Storage->Add(Type));
			
			// create & return closure
			lua_pushcclosure(L, &luaClassInstanceUnpersist, 1);
			return 1;
		}
		
		int luaClassInstanceGC(lua_State* L) {
			LuaClassInstance* Instance = CheckAndGetClassInstance(L, 1);
			Instance->~LuaClassInstance();
			return 0;
		}

		static const luaL_Reg luaClassInstanceLib[] = {
			{"__index", luaClassInstanceIndex},
			{"__newindex", luaClassInstanceNewIndex},
			{"__eq", luaClassInstanceEQ},
			{"__lt", luaClassInstanceLt},
			{"__le", luaClassInstanceLe},
			{"__tostring", luaClassInstanceToString},
			{"__persist", luaClassInstancePersist},
			{"__gc", luaClassInstanceGC},
			{NULL, NULL}
		};

		bool newInstance(lua_State* L, UClass* clazz) {
			// check obj and if type is registered
			UFINClass* Type = FFINReflection::Get()->FindClass(clazz);
			if (!IsValid(Type)) {
				lua_pushnil(L);
				return false;
			}

			// create instance
			LuaClassInstance* instance = static_cast<LuaClassInstance*>(lua_newuserdata(L, sizeof(LuaClassInstance)));
			new (instance) LuaClassInstance{clazz};

			FString MetaName = Type->GetInternalName() + CLASS_INSTANCE_META_SUFFIX;
			MetaNameToClass.FindOrAdd(MetaName) = Type;
			ClassToClassMetaName.FindOrAdd(Type) = MetaName;

			luaL_setmetatable(L, TCHAR_TO_UTF8(*MetaName));
			return true;
		}

		UClass* getClassInstance(lua_State* L, int index, UClass* clazz) {
			LuaClassInstance* instance = CheckAndGetClassInstance(L, index);
			if (!instance->Class->IsChildOf(clazz)) return nullptr;
			return instance->Class;
		}

		void setupInstanceSystem(lua_State* L) {
			PersistSetup("InstanceSystem", -2);
			
			luaL_newmetatable(L, INSTANCE_TYPE);			// ..., InstanceTypeMeta
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "__metatable");
			luaL_setfuncs(L, luaInstanceTypeLib, 0);
			PersistTable(INSTANCE_TYPE, -1);
			lua_pop(L, 1);									// ...

			for (const TPair<UClass*, UFINClass*>& Type : FFINReflection::Get()->GetClasses()) {
				FString TypeName = Type.Value->GetInternalName();
				luaL_newmetatable(L, TCHAR_TO_UTF8(*TypeName));								// ..., InstanceMeta
				lua_pushboolean(L, true);
				lua_setfield(L, -2, "__metatable");
				luaL_setfuncs(L, luaInstanceLib, 0);
				lua_newtable(L);															// ..., InstanceMeta, InstanceCache
				lua_setfield(L, -2, INSTANCE_CACHE);									// ..., InstanceMeta
				PersistTable(TCHAR_TO_UTF8(*TypeName), -1);
				lua_pop(L, 1);															// ...

				TypeName += CLASS_INSTANCE_META_SUFFIX;
				luaL_newmetatable(L, TCHAR_TO_UTF8(*TypeName));								// ..., InstanceMeta
				lua_pushboolean(L, true);
				lua_setfield(L, -2, "__metatable");
				luaL_setfuncs(L, luaClassInstanceLib, 0);
				lua_newtable(L);															// ..., InstanceMeta, InstanceCache
				lua_setfield(L, -2, INSTANCE_CACHE);									// ..., InstanceMeta
				PersistTable(TCHAR_TO_UTF8(*TypeName), -1);
				lua_pop(L, 1);															// ...
			}
			
			lua_pushcfunction(L, luaInstanceFuncCall);			// ..., InstanceFuncCall
			PersistValue("InstanceFuncCall");					// ...
			lua_pushcfunction(L, luaClassInstanceFuncCall);		// ..., LuaClassInstanceFuncCall
			PersistValue("ClassInstanceFuncCall");				// ...
			lua_pushcfunction(L, luaInstanceUnpersist);			// ..., LuaInstanceUnpersist
			PersistValue("InstanceUnpersist");					// ...
			lua_pushcfunction(L, luaClassInstanceUnpersist);		// ..., LuaClassInstanceUnpersist
			PersistValue("ClassInstanceUnpersist");			// ...
			lua_pushcfunction(L, luaInstanceTypeUnpersist);		// ..., LuaInstanceTypeUnpersist
			PersistValue("InstanceTypeUnpersist");				// ...
		}
	}
}
#pragma optimize("", on)
