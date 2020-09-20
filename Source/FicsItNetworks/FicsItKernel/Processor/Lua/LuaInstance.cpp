#include "LuaInstance.h"

#include "LuaProcessor.h"
#include "LuaProcessorStateStorage.h"

#include "Network/FINNetworkComponent.h"
#include "Network/FINNetworkCustomType.h"
#include "Network/FINVariadicParameterList.h"
#include "util/Logging.h"

#define INSTANCE_TYPE "InstanceType"
#define INSTANCE_CACHE "InstanceCache"
#define INSTANCE_UFUNC_DATA "InstanceUFuncData"
#define CLASS_INSTANCE_FUNC_DATA "ClassInstanceFuncData"

#define OffsetParam(type, off) (type*)((std::uint64_t)param + off)

#pragma optimize("", off)
namespace FicsItKernel {
	namespace Lua {
		std::map<UObject*, std::mutex> objectLocks;
		
		LuaInstanceRegistry* LuaInstanceRegistry::get() {
			static LuaInstanceRegistry* instance = nullptr;
			if (!instance) instance = new LuaInstanceRegistry();
			return instance;
		}

		void LuaInstanceRegistry::registerType(UClass* type, FString name, bool isClassInstance) {
			instanceTypes.FindOrAdd(type) = TPair<FString, bool>{name, isClassInstance};
			instanceTypeNames.FindOrAdd(name) = type;
			type->AddToRoot();
		}

		void LuaInstanceRegistry::registerFunction(UClass* type, FString name, LuaLibFunc func) {
			auto i = instanceTypes.Find(type);
			check(i&& !i->Value);
			instanceFunctions.FindOrAdd(type).FindOrAdd(name) = func;
		}

		void LuaInstanceRegistry::registerProperty(UClass* type, FString name, LuaLibProperty prop) {
			auto i = instanceTypes.Find(type);
			check(i && !i->Value);
			instanceProperties.FindOrAdd(type).FindOrAdd(name) = prop;
		}

		void LuaInstanceRegistry::registerClassFunction(UClass* type, FString name, LuaLibClassFunc func) {
			auto i = instanceTypes.Find(type);
			check(i && i->Value);
			classInstanceFunctions.FindOrAdd(type).FindOrAdd(name) = func;
		}

		FString LuaInstanceRegistry::findTypeName(UClass* type) {
			while (type) {
				auto i = instanceTypes.Find(type);
				if (i) {
					return i->Key;
				}
				if (type == UObject::StaticClass()) type = nullptr;
				else type = type->GetSuperClass();
			}
			return "";
		}

		UClass* LuaInstanceRegistry::findType(const FString& typeName, bool* isClass) {
			auto i = instanceTypeNames.Find(typeName);
			if (!i) return nullptr;
			if (isClass) *isClass = instanceTypes[*i].Value;
			return *i;
		}

		bool LuaInstanceRegistry::findLibFunc(UClass* instanceType, FString name, LuaLibFunc& outFunc) {
			while (instanceType) {
				auto i = instanceFunctions.Find(instanceType);
				if (i) {
					auto j = i->Find(name);
					if (j) {
						outFunc = *j;
						return true;
					}
				}
				if (instanceType == UObject::StaticClass()) instanceType = nullptr;
				else instanceType = instanceType->GetSuperClass();
			}
			return false;
		}

		bool LuaInstanceRegistry::findLibProperty(UClass* instanceType, FString name, LuaLibProperty& outProp) {
			while (instanceType) {
				auto i = instanceProperties.Find(instanceType);
				if (i) {
					auto j = i->Find(name);
					if (j) {
						outProp = *j;
						return true;
					}
				}
				if (instanceType == UObject::StaticClass()) instanceType = nullptr;
				else instanceType = instanceType->GetSuperClass();
			}
			return false;
		}

		bool LuaInstanceRegistry::findClassLibFunc(UClass* instanceType, FString name, LuaLibClassFunc& outFunc) {
			while (instanceType) {
				auto i = classInstanceFunctions.Find(instanceType);
				if (i) {
					auto j = i->Find(name);
					if (j) {
						outFunc = *j;
						return true;
					}
				}
				if (instanceType == UObject::StaticClass()) instanceType = nullptr;
				else instanceType = instanceType->GetSuperClass();
			}
			return false;
		}

		LuaInstance* LuaInstanceRegistry::getInstance(lua_State* L, int index, std::string* name) {
			index = lua_absindex(L, index);
			FString typeName;
			if (luaL_getmetafield(L, index, "__name") == LUA_TSTRING) {
				typeName = lua_tostring(L, -1);
				lua_pop(L, 1);
			} else if (lua_type(L, index) == LUA_TLIGHTUSERDATA) {
				typeName = "light userdata";
			} else {
				typeName = luaL_typename(L, index);
			}
			if (name) *name = TCHAR_TO_UTF8(*typeName);
			bool isClass;
			UClass* type = findType(typeName, &isClass);
			if (!type || isClass) return nullptr;
			return static_cast<LuaInstance*>(lua_touserdata(L, index));
		}

		LuaInstance* LuaInstanceRegistry::checkAndGetInstance(lua_State* L, int index, std::string* name) {
			std::string typeName;
			LuaInstance* instance = getInstance(L, index, &typeName);
			if (!instance) luaL_argerror(L, index, ("'Instance' expected, got '" + typeName + "'").c_str());
			if (name) *name = typeName;
			return instance;
		}

		LuaClassInstance* LuaInstanceRegistry::getClassInstance(lua_State* L, int index, std::string* name) {
			index = lua_absindex(L, index);
			FString typeName;
			if (luaL_getmetafield(L, index, "__name") == LUA_TSTRING) {
				typeName = lua_tostring(L, -1);
				lua_pop(L, 1);
			} else if (lua_type(L, index) == LUA_TLIGHTUSERDATA) {
				typeName = "light userdata";
			} else {
				typeName = luaL_typename(L, index);
			}
			if (name) *name = TCHAR_TO_UTF8(*typeName);
			bool isClass;
			UClass* type = findType(typeName, &isClass);
			if (!type || !isClass) return nullptr;
			return static_cast<LuaClassInstance*>(lua_touserdata(L, index));
		}

		LuaClassInstance* LuaInstanceRegistry::checkAndGetClassInstance(lua_State* L, int index, std::string* name) {
			std::string typeName;
			LuaClassInstance* instance = getClassInstance(L, index, &typeName);
			if (!instance) luaL_argerror(L, index, ("'ClassInstance' expected, got '" + typeName + "'").c_str());
			if (name) *name = typeName;
			return instance;
		}

		std::set<UClass*> LuaInstanceRegistry::getInstanceTypes() {
			std::set<UClass*> types;
			for (const auto& typeName : instanceTypeNames) {
				types.insert(typeName.Value);
			}
			return types;
		}

		std::set<TTuple<FString, int>> LuaInstanceRegistry::getMembers(UClass* type) {
			std::set<TTuple<FString, int>> members;
			while (type) {
				TMap<FString, LuaLibFunc>* funcMap = instanceFunctions.Find(type);
				if (funcMap) for (auto& i : *funcMap) {
					members.insert(TTuple<FString, int>(i.Key, 0));
				}
				TMap<FString, LuaLibProperty>* propMap = instanceProperties.Find(type);
				if (propMap) for (auto& i : *propMap) {
					members.insert(TTuple<FString, int>(i.Key, 1));
				}
				if (type == UObject::StaticClass()) type = nullptr;
				else type = type->GetSuperClass();
			}
			return members;
		}

		std::set<FString> LuaInstanceRegistry::getClassFunctionNames(UClass* type) {
			std::set<FString> funcs;
			while (type) {
				TMap<FString, LuaLibClassFunc>* funcMap = classInstanceFunctions.Find(type);
				if (funcMap) for (auto& i : *funcMap) {
					funcs.insert(i.Key);
				}
				if (type == UObject::StaticClass()) type = nullptr;
				else type = type->GetSuperClass();
			}
			return funcs;
		}

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

		int luaInstanceFuncCall(lua_State* L) {		// Instance, args..., up: FuncName, up: InstanceType
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get and check instance
			std::string typeName;
			LuaInstance* instance = reg->checkAndGetInstance(L, 1, &typeName);
			UClass* type = reg->findType(typeName.c_str());
			UObject* obj = *instance->Trace;
			if (!IsValid(obj)) return luaL_argerror(L, 1, "Instance is invalid");

			// check type
			LuaInstanceType* instType = static_cast<LuaInstanceType*>(luaL_checkudata(L, lua_upvalueindex(2), INSTANCE_TYPE));
			if (!type->IsChildOf(instType->type)) return luaL_argerror(L, 1, "Instance type is not allowed to call this function");

			// get func name
			FString funcName = luaL_checkstring(L, lua_upvalueindex(1));
			
			LuaLibFunc func;
			if (reg->findLibFunc(type, funcName, func)) {
				int args = func(L, lua_gettop(L), instance);
				return LuaProcessor::luaAPIReturn(L, args);
			}
			return luaL_error(L, "Unable to call function");
		}

		int luaInstanceUFuncCall(lua_State* L) {	// Instance, args..., up: UFunc, up: InstanceType
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get and check instance
			std::string typeName;
			LuaInstance* instance = reg->checkAndGetInstance(L, 1, &typeName);
			UClass* type = reg->findType(typeName.c_str());
			UObject* comp = *instance->Trace;
			if (!IsValid(comp)) return luaL_argerror(L, 1, "Instance is invalid");

			// check type
			LuaInstanceType* instType = static_cast<LuaInstanceType*>(luaL_checkudata(L, lua_upvalueindex(2), INSTANCE_TYPE));
			
			UFunction* func = static_cast<UFunction*>(lua_touserdata(L, lua_upvalueindex(1)));
			if (func) {
				UClass* funcClass = Cast<UClass>(func->GetOuter());
				FFINNetworkTrace trace = instance->Trace;
				if (!comp->GetClass()->IsChildOf(funcClass)) return luaL_argerror(L, 1, "Instance type is not allowed to call this function");;
				
				// allocate parameter space
				uint8* params = (uint8*)FMemory::Malloc(func->PropertiesSize);
				FMemory::Memzero(params + func->ParmsSize, func->PropertiesSize - func->ParmsSize);
				func->InitializeStruct(params);
				for (UProperty* LocalProp = func->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
					LocalProp->InitializeValue_InContainer(params);
				}
	
				// init and set parameter values
				int i = 2;
				for (auto property = TFieldIterator<UProperty>(func); property; ++property) {
					auto flags = property->GetPropertyFlags();
					if (flags & CPF_Parm && !(flags & (CPF_OutParm | CPF_ReturnParm))) {
						UStructProperty* StructProp = Cast<UStructProperty>(*property);
						if (StructProp && StructProp->Struct == FFINDynamicStructHolder::StaticStruct()) {
							// Variadic Parameters now
							TFINDynamicStruct<FFINVariadicParameterList> VariadicParams;
							int paramCount = lua_gettop(L);
							while (i <= paramCount) {
								FFINAnyNetworkValue Val;
								luaToNetworkValue(L, i++, Val);
								VariadicParams->Add(Val);
							}
							FFINDynamicStructHolder& Params = *StructProp->ContainerPtrToValuePtr<FFINDynamicStructHolder>(params);
							Params = VariadicParams;
						} else {
							try {
								luaToProperty(L, *property, params, i++);
							} catch (std::exception e) {
								for (UProperty* P = func->DestructorLink; P; P = P->DestructorLinkNext) {
									if (!P->IsInContainer(func->ParmsSize)) {
										P->DestroyValue_InContainer(params);
									}
								}
								FMemory::Free(params);
								return luaL_error(L, ("Argument #" + std::to_string(i) + " is not of type " + e.what()).c_str());
							}
						}
					}
				}
	
				// execute native function only if no error
				{
					std::lock_guard<std::mutex> m(objectLocks[comp]);
					comp->ProcessEvent(func, params);
				}
				
				int retargs = 0;
				// free parameters and eventualy push return values to lua
				for (auto property = TFieldIterator<UProperty>(func); property; ++property) {
					auto flags = property->GetPropertyFlags();
					if (flags & CPF_Parm && flags & (CPF_OutParm | CPF_ReturnParm)) {
						propertyToLua(L, *property, params, trace);
						++retargs;
					}
				}
				
				for (UProperty* P = func->DestructorLink; P; P = P->DestructorLinkNext) {
					if (!P->IsInContainer(func->ParmsSize)) {
						P->DestroyValue_InContainer(params);
					}
				}
				FMemory::Free(params);
				
				return LuaProcessor::luaAPIReturn(L, retargs);
			}
			return luaL_error(L, "Unable to call function");
		}

		bool luaInstanceIndexFindUFunction(lua_State* L, UClass* type, const FString& funcName, LuaInstanceRegistry* reg) {					// Instance, FuncName, InstanceCoche, nil
			UFunction* func = type->FindFunctionByName(FName(*("netFunc_" + funcName)));
			if (IsValid(func)) {
				// create function
				lua_pushlightuserdata(L, func);																		// Instance, FuncName, InstanceCoche, nil, FuncName
				luaInstanceType(L, LuaInstanceType{reg->findType(reg->findTypeName(type))});															// Instance, FuncName, InstanceCache, nil, FuncName, InstanceType
				lua_pushcclosure(L, luaInstanceUFuncCall, 2);													// Instance, FuncName, InstanceCache, nil, InstanceFunc

				// cache function
				lua_pushvalue(L, -1);																			// Instance, FuncName, InstanceCache, nil, InstanceFunc, InstanceFunc
				lua_setfield(L, 3, TCHAR_TO_UTF8(*funcName));														// Instance, FuncName, InstanceCache, nil, InstanceFunc

				return true;
			}
			return false;
		}

		int luaInstanceIndex(lua_State* L) {																			// Instance, FuncName
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get instance
			std::string typeName;
			LuaInstance* instance = reg->checkAndGetInstance(L, 1, &typeName);
			UClass* type = reg->findType(typeName.c_str());
				
			// get function name
			if (!lua_isstring(L, 2)) return 0;
			std::string memberName = lua_tostring(L, 2);

			UObject* obj = *instance->Trace;
			UObject* org = instance->Orignal.Get();

			if (!IsValid(obj)) {
				return luaL_error(L, "Instance is invalid");
			}
			
			// try to get property
			if (memberName == "id") {
				if (!org || !org->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
					return luaL_error(L, "Instance is not a network component");
				}
				lua_pushstring(L, TCHAR_TO_UTF8(*IFINNetworkComponent::Execute_GetID(org).ToString()));
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			if (memberName == "nick") {
				if (!org || !org->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
					return luaL_error(L, "Instance is not a network component");
				}
				lua_pushstring(L, TCHAR_TO_UTF8(*IFINNetworkComponent::Execute_GetNick(org)));
				return LuaProcessor::luaAPIReturn(L, 1);
			}

			// try to find lib property
			LuaLibProperty libProp;
			FFINNetworkTrace realTrace = instance->Trace;
			bool foundLibProp = false;
			if (reg->findLibProperty(reg->findType(typeName.c_str()), memberName.c_str(), libProp)) {
				foundLibProp = true;
			}
			if (foundLibProp) {
				lua_pop(L, 2);
				return LuaProcessor::luaAPIReturn(L, libProp.get(L, realTrace));
			}

			// get cache function
			luaL_getmetafield(L, 1, INSTANCE_CACHE);																// Instance, FuncName, InstanceCache
			if (lua_getfield(L, -1, (typeName + "_" + memberName).c_str()) != LUA_TNIL) {						// Instance, FuncName, InstanceCache, CachedFunc
				return LuaProcessor::luaAPIReturn(L, 1);
			}																											// Instance, FuncName, InstanceCache, nil
			
			// get lib function
			LuaLibFunc libFunc;
			bool foundLibFunc = false;
			if (reg->findLibFunc(reg->findType(typeName.c_str()), memberName.c_str(), libFunc)) {
				foundLibFunc = true;
			}
			if (foundLibFunc) {
				// create function
				lua_pushvalue(L, 2);																				// Instance, FuncName, InstanceCoche, nil, FuncName
				luaInstanceType(L, LuaInstanceType{type});																// Instance, FuncName, InstanceCache, nil, FuncName, InstanceType
				lua_pushcclosure(L, luaInstanceFuncCall, 2);															// Instance, FuncName, InstanceCache, nil, InstanceFunc

				// cache function
				lua_pushvalue(L, -1);																				// Instance, FuncName, InstanceCache, nil, InstanceFunc, InstanceFunc
				lua_setfield(L, 3, (typeName + "_" + memberName).c_str());										// Instance, FuncName, InstanceCache, nil, InstanceFunc

				return LuaProcessor::luaAPIReturn(L, 1);
			}

			// get reflected function
			UClass* instType = obj->GetClass();
			if (luaInstanceIndexFindUFunction(L, instType, memberName.c_str(), reg)) return LuaProcessor::luaAPIReturn(L, 1);
			
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		int luaInstanceNewIndex(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get instance
			std::string typeName;
			LuaInstance* instance = reg->checkAndGetInstance(L, 1, &typeName);
			UClass* type = reg->findType(typeName.c_str());
				
			// get function name
			if (!lua_isstring(L, 2)) return 0;
			std::string memberName = lua_tostring(L, 2);

			UObject* obj = *instance->Trace;
			UObject* Org = instance->Orignal.Get();

			if (!IsValid(obj)) {
				return luaL_error(L, "Instance is invalid");
			}
			
			if (memberName == "nick") {
				if (!Org && !Org->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
					return luaL_error(L, "Instance is not a network component");
				}
				FString nick = luaL_checkstring(L, 3);
				IFINNetworkComponent::Execute_SetNick(Org, nick);
				return LuaProcessor::luaAPIReturn(L, 1);
			}

			// try to find lib property
			LuaLibProperty libProp;
			FFINNetworkTrace realTrace = instance->Trace;
			bool foundLibProp = false;
			if (reg->findLibProperty(reg->findType(typeName.c_str()), memberName.c_str(), libProp)) {
				foundLibProp = true;
			}
			if (foundLibProp) {
				if (libProp.readOnly) return luaL_error(L, "property is read only");
				return LuaProcessor::luaAPIReturn(L, libProp.set(L, realTrace));
			}
			
			return luaL_error(L, ("Instance doesn't have property with name " + memberName + "'").c_str());
		}

		int luaInstanceEQ(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			LuaInstance* inst1 = reg->checkAndGetInstance(L, 1);
			LuaInstance* inst2 = reg->getInstance(L, 2);
			if (!inst1 || !inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, inst1->Trace.IsEqualObj(inst2->Trace));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaInstanceLt(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			LuaInstance* inst1 = reg->checkAndGetInstance(L, 1);
			LuaInstance* inst2 = reg->getInstance(L, 2);
			if (!inst1 || !inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->Trace.GetUnderlyingPtr()) < GetTypeHash(inst2->Trace.GetUnderlyingPtr()));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaInstanceLe(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			LuaInstance* inst1 = reg->checkAndGetInstance(L, 1);
			LuaInstance* inst2 = reg->getInstance(L, 2);
			if (!inst1 || !inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->Trace.GetUnderlyingPtr()) <= GetTypeHash(inst2->Trace.GetUnderlyingPtr()));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaInstanceToString(lua_State* L) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			std::string typeName;
			LuaInstance* inst = reg->checkAndGetInstance(L, 1, &typeName);
			UObject* obj = *inst->Trace;
			if (!IsValid(obj)) return luaL_argerror(L, 1, "Instance is invalid");
			if (obj->Implements<UFINNetworkCustomType>()) {
				typeName = TCHAR_TO_UTF8(*IFINNetworkCustomType::Execute_GetCustomTypeName(obj));
			}
			UClass* type = obj->GetClass();
			std::stringstream msg;
			msg << typeName;
			if (type->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
				FString nick = IFINNetworkComponent::Execute_GetNick(obj);
				if (nick.Len() > 0) msg << " \"" << TCHAR_TO_UTF8(*nick) << "\"";
				msg << " " << TCHAR_TO_UTF8(*IFINNetworkComponent::Execute_GetID(obj).ToString());
			}
			lua_pushstring(L,  msg.str().c_str());
			return 1;
		}

		int luaInstanceUnpersist(lua_State* L) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			// get trace and typename
			FFINNetworkTrace trace = storage->GetTrace(luaL_checkinteger(L, lua_upvalueindex(1)));
			std::string typeName = luaL_checkstring(L, lua_upvalueindex(2));
			UObject* obj = nullptr;
			if (lua_isinteger(L, lua_upvalueindex(3))) obj = storage->GetRef(luaL_checkinteger(L, lua_upvalueindex(3)));

			// create instance
			LuaInstance* instance = static_cast<LuaInstance*>(lua_newuserdata(L, sizeof(LuaInstance)));
			new (instance) LuaInstance{trace, obj};
			luaL_setmetatable(L, typeName.c_str());

			return 1;
		}

		int luaInstancePersist(lua_State* L) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get instance
			std::string typeName;
			LuaInstance* instance = LuaInstanceRegistry::get()->checkAndGetInstance(L, 1, &typeName);

			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			// add trace to storage & push id
			lua_pushinteger(L, storage->Add(instance->Trace));
			lua_pushstring(L, typeName.c_str());
			lua_pushinteger(L, storage->Add(instance->Orignal.Get()));
			
			// create & return closure
			lua_pushcclosure(L, &luaInstanceUnpersist, 3);
			return 1;
	}

		int luaInstanceGC(lua_State* L) {
			LuaInstance* instance = LuaInstanceRegistry::get()->checkAndGetInstance(L, 1);
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

		bool newInstance(lua_State* L, FFINNetworkTrace trace, UObject* Original) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// check obj and if type is registered
			UObject* obj = *trace;
			std::string typeName = "";
			if (!IsValid(obj) || (typeName = TCHAR_TO_UTF8(*reg->findTypeName(obj->GetClass()))).length() < 1) {
				lua_pushnil(L);
				return false;
			}

			// create instance
			LuaInstance* instance = static_cast<LuaInstance*>(lua_newuserdata(L, sizeof(LuaInstance)));
			new (instance) LuaInstance{trace, Original};

			luaL_setmetatable(L, typeName.c_str());
			return true;
		}

		FFINNetworkTrace getObjInstance(lua_State* L, int index, UClass* clazz) {
			if (lua_isnil(L, index)) return FFINNetworkTrace(nullptr);
			LuaInstance* instance = LuaInstanceRegistry::get()->checkAndGetInstance(L, index);
			if (!instance->Trace->GetClass()->IsChildOf(clazz)) return FFINNetworkTrace(nullptr);
			return instance->Trace;
		}
		
		int luaClassInstanceFuncCall(lua_State* L) {	// ClassInstance, Args..., up: FuncName, up: ClassInstance
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get and check class instance
			std::string typeName;
			LuaClassInstance* instance = reg->checkAndGetClassInstance(L, 1, &typeName);
			LuaInstanceType* type = static_cast<LuaInstanceType*>(luaL_checkudata(L, lua_upvalueindex(2), INSTANCE_TYPE));

			// check type
			if (!instance || !type || !instance->clazz || !type->type || !instance->clazz->IsChildOf(type->type)) return luaL_argerror(L, 1, "ClassInstance is invalid");

			// get func name
			FString funcName = luaL_checkstring(L, lua_upvalueindex(1));
			
			LuaLibClassFunc func;
			if (reg->findClassLibFunc(instance->clazz, funcName, func)) {
				lua_remove(L, 1);
				return LuaProcessor::luaAPIReturn(L, func(L, lua_gettop(L), instance->clazz));
			}
			return luaL_error(L, "Unable to call function");
		}

		int luaClassInstanceGetMembers(lua_State* L) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			
			// get and check class instance
			LuaClassInstance* instance = reg->checkAndGetClassInstance(L, 1);
			
			lua_newtable(L);
			int i = 0;
			
			for (const FString& func : reg->getClassFunctionNames(instance->clazz)) {
				lua_pushstring(L, TCHAR_TO_UTF8(*func));
				lua_seti(L, -2, ++i);
			}
			
			return 1;
		}
		
		int luaClassInstanceIndex(lua_State* L) {																		// ClassInstance, FuncName
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			// get class instance
			std::string typeName;
			LuaClassInstance* instance = reg->checkAndGetClassInstance(L, 1, &typeName);
			UClass* type = reg->findType(typeName.c_str());
				
			// get function name
			if (!lua_isstring(L, 2)) return 0;
			FString funcName = lua_tostring(L, 2);

			// try util functions
			if (funcName == "getMembers") {
				lua_pushcfunction(L, luaClassInstanceGetMembers);
				return 1;
			}

			// get cache function
			luaL_getmetafield(L, 1, INSTANCE_CACHE);																// ClassInstance, FuncName, InstanceCache
			if (lua_getfield(L, -1, TCHAR_TO_UTF8(*funcName)) != LUA_TNIL) {									// ClassInstance, FuncName, InstanceCache, CachedFunc
				return LuaProcessor::luaAPIReturn(L, 1);
			}																											// ClassInstance, FuncName, InstanceCache, nil
			
			// get class lib function
			LuaLibClassFunc libFunc;
			if (reg->findClassLibFunc(type, funcName, libFunc)) {
				// create function
				lua_pushvalue(L, 2);																				// ClassInstance, FuncName, InstanceCoche, nil, FuncName
				luaInstanceType(L, LuaInstanceType{type});																// ClassInstance, FuncName, InstanceCache, nil, FuncName, InstanceType
				lua_pushcclosure(L, luaClassInstanceFuncCall, 2);													// ClassInstance, FuncName, InstanceCache, nil, ClassInstanceFunc

				// cache function
				lua_pushvalue(L, -1);																				// ClassInstance, FuncName, InstanceCache, nil, ClassInstanceFunc, InstanceFunc
				lua_setfield(L, 3, TCHAR_TO_UTF8(*funcName));													// ClassInstance, FuncName, InstanceCache, nil, ClassInstanceFunc

				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		int luaClassInstanceNewIndex(lua_State* L) {
			return LuaProcessor::luaAPIReturn(L, 0);
		}

		int luaClassInstanceEQ(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			LuaClassInstance* inst1 = reg->checkAndGetClassInstance(L, 1);
			LuaClassInstance* inst2 = reg->getClassInstance(L, 2);
			if (!inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, inst1->clazz == inst2->clazz);
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaClassInstanceLt(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			LuaClassInstance* inst1 = reg->checkAndGetClassInstance(L, 1);
			LuaClassInstance* inst2 = reg->getClassInstance(L, 2);
			if (!inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->clazz) < GetTypeHash(inst2->clazz));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaClassInstanceLe(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			LuaClassInstance* inst1 = reg->checkAndGetClassInstance(L, 1);
			LuaClassInstance* inst2 = reg->getClassInstance(L, 2);
			if (!inst2) {
				lua_pushboolean(L, false);
				return LuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, GetTypeHash(inst1->clazz) <= GetTypeHash(inst2->clazz));
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		int luaClassInstanceToString(lua_State* L) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			std::string typeName;
			LuaClassInstance* inst = reg->checkAndGetClassInstance(L, 1, &typeName);
			
			LuaLibClassFunc func;
			if (reg->findClassLibFunc(inst->clazz, "__tostring", func)) {
				lua_pop(L, 1);
				func(L, lua_gettop(L), inst->clazz);
				luaL_checkstring(L, -1);
			} else {
				lua_pushstring(L, typeName.c_str());
			}
			return 1;
		}

		int luaClassInstanceUnpersist(lua_State* L) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			FString typeName = lua_tostring(L, lua_upvalueindex(1));
			
			UClass* type = reg->findType(typeName);
			newInstance(L, type);
			
			return 1;
		}

		int luaClassInstancePersist(lua_State* L) {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			
			// get data
			std::string typeName;
			LuaClassInstance* instance = reg->checkAndGetClassInstance(L, 1, &typeName);

			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			// push type name to persist
			lua_pushstring(L, typeName.c_str());
			
			// create & return closure
			lua_pushcclosure(L, &luaClassInstanceUnpersist, 1);
			return 1;
		}
		
		int luaClassInstanceGC(lua_State* L) {
			LuaClassInstance* instance = LuaInstanceRegistry::get()->checkAndGetClassInstance(L, 1);
			instance->~LuaClassInstance();
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
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			
			// check obj and if type is registered
			FString typeName = "";
			if (!IsValid(clazz) || (typeName = reg->findTypeName(clazz)).Len() < 1) {
				lua_pushnil(L);
				return false;
			}

			// create instance
			LuaClassInstance* instance = static_cast<LuaClassInstance*>(lua_newuserdata(L, sizeof(LuaClassInstance)));
			new (instance) LuaClassInstance{clazz};
			luaL_setmetatable(L, TCHAR_TO_UTF8(*typeName));
			return true;
		}

		UClass* getClassInstance(lua_State* L, int index, UClass* clazz) {
			LuaClassInstance* instance = LuaInstanceRegistry::get()->checkAndGetClassInstance(L, index);
			if (!instance->clazz->IsChildOf(clazz)) return nullptr;
			return instance->clazz;
		}

		void setupInstanceSystem(lua_State* L) {
			PersistSetup("InstanceSystem", -2);
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			luaL_newmetatable(L, INSTANCE_TYPE);			// ..., InstanceTypeMeta
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "__metatable");
			luaL_setfuncs(L, luaInstanceTypeLib, 0);
			PersistTable(INSTANCE_TYPE, -1);
			lua_pop(L, 1);									// ...

			for (UClass* type : reg->getInstanceTypes()) {
				FString typeName = reg->findTypeName(type);
				bool isClass = false;
				reg->findType(typeName, &isClass);
				luaL_newmetatable(L, TCHAR_TO_UTF8(*typeName));								// ..., InstanceMeta
				lua_pushboolean(L, true);
				lua_setfield(L, -2, "__metatable");
				luaL_setfuncs(L, isClass ? luaClassInstanceLib : luaInstanceLib, 0);
				lua_newtable(L);															// ..., InstanceMeta, InstanceCache
				lua_setfield(L, -2, INSTANCE_CACHE);									// ..., InstanceMeta
				PersistTable(TCHAR_TO_UTF8(*typeName), -1);
				lua_pop(L, 1);															// ...
			}
			
			lua_pushcfunction(L, luaInstanceFuncCall);			// ..., InstanceFuncCall
			PersistValue("InstanceFuncCall");					// ...
			lua_pushcfunction(L, luaInstanceUFuncCall);			// ..., InstanceUFuncCall
			PersistValue("InstanceUFuncCall");				// ...
			lua_pushcfunction(L, luaClassInstanceFuncCall);		// ..., LuaClassInstanceFuncCall
			PersistValue("ClassInstanceFuncCall");			// ...
			lua_pushcfunction(L, luaClassInstanceGetMembers);	// ..., LuaClassInstanceGetMembers
			PersistValue("ClassInstnaceGetMembers");			// ...
			lua_pushcfunction(L, luaInstanceUnpersist);			// ..., LuaInstanceUnpersist
			PersistValue("InstanceUnpersist");				// ...
			lua_pushcfunction(L, luaClassInstanceUnpersist);		// ..., LuaClassInstanceUnpersist
			PersistValue("ClassInstanceUnpersist");			// ...
			lua_pushcfunction(L, luaInstanceTypeUnpersist);		// ..., LuaInstanceTypeUnpersist
			PersistValue("InstanceTypeUnpersist");			// ...
		}
	}
}
#pragma optimize("", on)
