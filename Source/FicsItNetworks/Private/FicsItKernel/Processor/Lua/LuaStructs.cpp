#include "FicsItKernel/Processor/Lua/LuaStructs.h"
#include "FicsItKernel/Processor/Lua/LuaRef.h"
#include "FicsItKernel/Processor/Lua/LuaUtil.h"
#include "FicsItKernel/Processor/Lua/LuaProcessor.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStruct.h"
#include "FGRailroadSubsystem.h"

#define PersistParams \
	const std::string& _persist_namespace, \
	const int _persist_permTableIdx, \
	const int _persist_upermTableIdx

namespace FicsItKernel {
	namespace Lua {
		TMap<UFINStruct*, FString> StructToMetaName;
		TMap<FString, UFINStruct*> MetaNameToStruct;
		FCriticalSection StructMetaNameLock;
		
		TSharedPtr<FINStruct> luaGetStruct(lua_State* L, int i, LuaStruct** LStructPtr) {
			UFINStruct* Type = luaGetStructType(L, i);
			if (!Type) return nullptr;
			TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(Cast<UScriptStruct>(Type->GetOuter()));
			LuaStruct* LStruct = luaGetStruct(L, i, Struct);
			if (LStructPtr) *LStructPtr = LStruct;
			return Struct;
		}

		LuaStruct* luaGetStruct(lua_State* L, int i, TSharedRef<FINStruct>& Struct) {
			i = lua_absindex(L, i);
			UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct->GetStruct());
			if (!Type) {
				return nullptr;
			}
			if (lua_istable(L, i)) {
				int j = 0;
				for (UFINProperty* Prop : Type->GetProperties()) {
					if (!(Prop->GetPropertyFlags() & FIN_Prop_Attrib)) continue;
					++j;
					if (lua_getfield(L, i, TCHAR_TO_UTF8(*Prop->GetInternalName())) == LUA_TNIL) {
						lua_pop(L, 1);
						lua_geti(L, i, j);
					}
					FINAny Value;
					luaToNetworkValue(L, -1, Value);
					lua_pop(L, 1);
					Prop->SetValue(Struct->GetData(), Value);
				}
			} else if (lua_isuserdata(L, i)) {
				StructMetaNameLock.Lock();
				FString MetaName = StructToMetaName[Type];
				StructMetaNameLock.Unlock();
				LuaStruct* LStruct = static_cast<LuaStruct*>(luaL_checkudata(L, i, TCHAR_TO_UTF8(*MetaName)));
				Struct = LStruct->Struct;
				return LStruct;
			}
			return nullptr;
		}
		
		LuaStruct::LuaStruct(UFINStruct* Type, const FFINDynamicStructHolder& Struct, UFINKernelSystem* Kernel) : Type(Type), Struct(MakeShared<FFINDynamicStructHolder>(Struct)), Kernel(Kernel) {
			Kernel->AddReferencer(this, &CollectReferences);
		}

		LuaStruct::LuaStruct(const LuaStruct& Other) : Type(Other.Type), Struct(Other.Struct), Kernel(Other.Kernel) {
			Kernel->AddReferencer(this, &CollectReferences);
		}
		
		LuaStruct::~LuaStruct() {
			Kernel->RemoveReferencer(this);
		}

		void LuaStruct::CollectReferences(void* Obj, FReferenceCollector& Collector) {
			LuaStruct* Self = static_cast<LuaStruct*>(Obj);
			Collector.AddReferencedObject(Self->Type);
			Self->Struct->AddStructReferencedObjects(Collector);
		}

		void luaStruct(lua_State* L, const FINStruct& Struct) {
			UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct.GetStruct());
			if (!Type) {
				lua_pushnil(L);
				return;
			}
			setupStructMetatable(L, Type);
			LuaStruct* LStruct = static_cast<LuaStruct*>(lua_newuserdata(L, sizeof(LuaStruct)));
			new (LStruct) LuaStruct(Type, Struct, UFINLuaProcessor::luaGetProcessor(L)->GetKernel());
			luaL_setmetatable(L, TCHAR_TO_UTF8(*StructToMetaName[Type]));
		}

		UFINStruct* luaGetStructType(lua_State* L, int i) {
			FString TypeName;
			if (luaL_getmetafield(L, i, "__name") == LUA_TSTRING) {
				TypeName = lua_tostring(L, -1);
				lua_pop(L, 1);
			} else if (lua_type(L, i) == LUA_TLIGHTUSERDATA) {
				TypeName = "light userdata";
			} else {
				TypeName = luaL_typename(L, i);
			}
			UFINStruct** Type = MetaNameToStruct.Find(TypeName);
			if (!Type) return nullptr;
			return *Type;
		}

		int luaStructFuncCall(lua_State* L) {
			// get function
			LuaRefFuncData* Func = static_cast<LuaRefFuncData*>(luaL_checkudata(L, lua_upvalueindex(1), LUA_REF_FUNC_DATA));
			
			// get and check instance
			StructMetaNameLock.Lock();
			FString* MetaNamePtr = StructToMetaName.Find(Cast<UFINStruct>(Func->Struct));
			if (!MetaNamePtr) {
				StructMetaNameLock.Unlock();
				return luaL_argerror(L, 1, "Function name is invalid (internal error)");
			}
			const FString MetaName = *MetaNamePtr;
			StructMetaNameLock.Unlock();
			LuaStruct* Instance = static_cast<LuaStruct*>(luaL_checkudata(L, 1, TCHAR_TO_UTF8(*MetaName)));
			if (!Instance->Struct->GetData()) return luaL_argerror(L, 1, "Struct is invalid");

			// call the function
			return luaCallFINFunc(L, Func->Func, FFINExecutionContext(Instance->Struct->GetData()), "Struct");
		}
		
		int luaStructIndex(lua_State* L) {
			// get struct
			const TSharedPtr<FINStruct> Struct = luaGetStruct(L, 1);
			if (!Struct.IsValid()) return luaL_error(L, "Struct is invalid");
			
			// get member name
			const FString MemberName = lua_tostring(L, 2);
			
			UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct->GetStruct());
			if (!IsValid(Type)) {
				return luaL_error(L, "Struct is invalid");
			}

			StructMetaNameLock.Lock();
			const FString MetaName = StructToMetaName[Type];
			StructMetaNameLock.Unlock();
			return luaFindGetMember(L, Type, FFINExecutionContext(Struct->GetData()), MemberName, MetaName + "_" + MemberName, &luaStructFuncCall, false);
		}

		int luaStructNewIndex(lua_State* L) {
			// get struct
			const TSharedPtr<FINStruct> Struct = luaGetStruct(L, 1);
			if (!Struct.IsValid()) return luaL_error(L, "Struct is invalid");
				
			// get member name
			const FString MemberName = lua_tostring(L, 2);
			
			UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct->GetStruct());
			if (!IsValid(Type)) {
				return luaL_error(L, "Struct is invalid");
			}
			
			return luaFindSetMember(L, Type, FFINExecutionContext(Struct->GetData()), MemberName, false);
		}

		int luaStructEQ(lua_State* L) {
			const TSharedPtr<FINStruct> Struct1 = luaGetStruct(L, 1);
			const TSharedPtr<FINStruct> Struct2 = luaGetStruct(L, 1);
			if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
				lua_pushboolean(L, false);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, Struct1->GetStruct()->CompareScriptStruct(Struct1->GetData(), Struct2->GetData(), 0));
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}

		int luaStructLt(lua_State* L) {
			const TSharedPtr<FINStruct> Struct1 = luaGetStruct(L, 1);
			const TSharedPtr<FINStruct> Struct2 = luaGetStruct(L, 1);
			if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
				lua_pushboolean(L, false);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, Struct1->GetStruct()->GetStructTypeHash(Struct1->GetData()) < Struct2->GetStruct()->GetStructTypeHash(Struct2->GetData()));
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}

		int luaStructLe(lua_State* L) {
			const TSharedPtr<FINStruct> Struct1 = luaGetStruct(L, 1);
			const TSharedPtr<FINStruct> Struct2 = luaGetStruct(L, 1);
			if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
				lua_pushboolean(L, false);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushboolean(L, Struct1->GetStruct()->GetStructTypeHash(Struct1->GetData()) <= Struct2->GetStruct()->GetStructTypeHash(Struct2->GetData()));
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}

		int luaStructToString(lua_State* L) {
			const TSharedPtr<FINStruct> Struct = luaGetStruct(L, 1);
			if (!Struct->GetData()) {
				lua_pushboolean(L, false);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}
			
			lua_pushstring(L, TCHAR_TO_UTF8(*(FFINReflection::Get()->FindStruct(Struct->GetStruct())->GetInternalName() + "-Struct")));
			return 1;
		}

		int luaStructUnpersist(lua_State* L) {
			UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
			
			// get persist storage
			FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;
			
			// get struct
			const FFINDynamicStructHolder& Struct = *Storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)));
			
			// create instance
			luaStruct(L, Struct);
			
			return 1;
		}

		int luaStructPersist(lua_State* L) {
			// get struct
			TSharedPtr<FINStruct> Struct = luaGetStruct(L, 1);

			UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
			
			// get persist storage
			FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;

			// push struct to persist
			lua_pushinteger(L, Storage.Add(Struct));
			
			// create & return closure
			lua_pushcclosure(L, &luaStructUnpersist, 1);
			return 1;
		}
		
		int luaStructGC(lua_State* L) {
			LuaStruct* Struct = static_cast<LuaStruct*>(lua_touserdata(L, 1));
			Struct->~LuaStruct();
			return 0;
		}

		static const luaL_Reg luaStructLib[] = {
			{"__index", luaStructIndex},
			{"__newindex", luaStructNewIndex},
			{"__eq", luaStructEQ},
			{"__lt", luaStructLt},
			{"__le", luaStructLe},
			{"__tostring", luaStructToString},
			{"__persist", luaStructPersist},
			{"__gc", luaStructGC},
			{NULL, NULL}
		};

		void setupStructSystem(lua_State* L) {
			PersistSetup("StructSystem", -2);
			
			lua_pushcfunction(L, luaStructFuncCall);			// ..., InstanceFuncCall
			PersistValue("StructFuncCall");				// ...
			lua_pushcfunction(L, luaStructUnpersist);			// ..., LuaInstanceUnpersist
			PersistValue("StructUnpersist");				// ...
		}

		void setupStructMetatable(lua_State* L, UFINStruct* Struct) {
			FScopeLock ScopeLock(&StructMetaNameLock);
			
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistUperm");
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistPerm");
			PersistSetup("StructSystem", -2);

			FString TypeName = Struct->GetInternalName();
			if (luaL_getmetatable(L, TCHAR_TO_UTF8(*TypeName)) != LUA_TNIL) {
				lua_pop(L, 3);
				return;
			}
			lua_pop(L, 1);
			luaL_newmetatable(L, TCHAR_TO_UTF8(*TypeName));							// ..., InstanceMeta
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "__metatable");
			luaL_setfuncs(L, luaStructLib, 0);
			lua_newtable(L);															// ..., InstanceMeta, InstanceCache
			lua_setfield(L, -2, LUA_REF_CACHE);									// ..., InstanceMeta
			PersistTable(TCHAR_TO_UTF8(*TypeName), -1);
			lua_pop(L, 3);															// ...
			MetaNameToStruct.FindOrAdd(TypeName) = Struct;
			StructToMetaName.FindOrAdd(Struct) = TypeName;
		}
	}
}
