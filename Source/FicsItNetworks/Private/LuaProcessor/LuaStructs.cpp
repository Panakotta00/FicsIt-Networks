#include "LuaProcessor/LuaStructs.h"
#include "LuaProcessor/LuaRef.h"
#include "LuaProcessor/LuaUtil.h"
#include "LuaProcessor/LuaProcessor.h"
#include "LuaProcessor/LuaInstance.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStruct.h"

#define STRUCT_TYPE_METATABLE_NAME "SimpleStructRefMetatable"

#define PersistParams \
	const std::string& _persist_namespace, \
	const int _persist_permTableIdx, \
	const int _persist_upermTableIdx

namespace FINLua {
	TMap<UFINStruct*, FString> StructToMetaName;
	TMap<FString, UFINStruct*> MetaNameToStruct;
	FCriticalSection StructMetaNameLock;
	
	TSharedPtr<FINStruct> luaGetStruct(lua_State* L, int i, FLuaStruct** LStructPtr) {
		UFINStruct* Type = luaGetStructType(L, i);
		if (!Type) return nullptr;
		TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(Cast<UScriptStruct>(Type->GetOuter()));
		FLuaStruct* LStruct = luaGetStruct(L, i, Struct);
		if (LStructPtr) *LStructPtr = LStruct;
		return Struct;
	}

	FLuaStruct* luaGetStruct(lua_State* L, int i, TSharedRef<FINStruct>& Struct) {
		i = lua_absindex(L, i);
		UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct->GetStruct());
		if (!Type) {
			return nullptr;
		}
		if (lua_istable(L, i)) {
			if (!(Type->GetStructFlags() & FIN_Struct_Constructable)) return nullptr;
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
			FLuaStruct* LStruct = static_cast<FLuaStruct*>(luaL_checkudata(L, i, TCHAR_TO_UTF8(*MetaName)));
			Struct = LStruct->Struct;
			return LStruct;
		}
		return nullptr;
	}
	
	FLuaStruct::FLuaStruct(UFINStruct* Type, const FFINDynamicStructHolder& Struct, UFINKernelSystem* Kernel) : Type(Type), Struct(MakeShared<FFINDynamicStructHolder>(Struct)), Kernel(Kernel) {
		Kernel->AddReferencer(this, &CollectReferences);
	}

	FLuaStruct::FLuaStruct(const FLuaStruct& Other) : Type(Other.Type), Struct(Other.Struct), Kernel(Other.Kernel) {
		Kernel->AddReferencer(this, &CollectReferences);
	}
	
	FLuaStruct::~FLuaStruct() {
		Kernel->RemoveReferencer(this);
	}

	void FLuaStruct::CollectReferences(void* Obj, FReferenceCollector& Collector) {
		FLuaStruct* Self = static_cast<FLuaStruct*>(Obj);
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
		FLuaStruct* LStruct = static_cast<FLuaStruct*>(lua_newuserdata(L, sizeof(FLuaStruct)));
		new (LStruct) FLuaStruct(Type, Struct, UFINLuaProcessor::luaGetProcessor(L)->GetKernel());
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
		FLuaStruct* Instance = static_cast<FLuaStruct*>(luaL_checkudata(L, 1, TCHAR_TO_UTF8(*MetaName)));
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
		FLuaStruct* Struct = static_cast<FLuaStruct*>(lua_touserdata(L, 1));
		Struct->~FLuaStruct();
		return 0;
	}

	static const luaL_Reg luaStructMetatable[] = {
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

	int luaFindStruct(lua_State* L) {
		const int args = lua_gettop(L);

		for (int i = 1; i <= args; ++i) {
			const bool isT = lua_istable(L, i);

			TArray<FString> StructNames;
			if (isT) {
				const auto count = lua_rawlen(L, i);
				for (int j = 1; j <= count; ++j) {
					lua_geti(L, i, j);
					if (!lua_isstring(L, -1)) return luaL_argerror(L, i, "array contains non-string");
					StructNames.Add(lua_tostring(L, -1));
					lua_pop(L, 1);
				}
				lua_newtable(L);
			} else {
				if (!lua_isstring(L, i)) return luaL_argerror(L, i, "is not string");
				StructNames.Add(lua_tostring(L, i));
			}
			int j = 0;
			TArray<UFINStruct*> Structs;
			FFINReflection::Get()->GetStructs().GenerateValueArray(Structs);
			for (const FString& StructName : StructNames) {
				UFINStruct** Struct = Structs.FindByPredicate([StructName](UFINStruct* Struct) {
					if (Struct->GetInternalName() == StructName) return true;
					return false;
				});
				if (Struct) newInstance(L, FINTrace(*Struct));
				else lua_pushnil(L);
				if (isT) lua_seti(L, -2, ++j);
			}
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
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
		luaL_setfuncs(L, luaStructMetatable, 0);
		lua_newtable(L);															// ..., InstanceMeta, InstanceCache
		lua_setfield(L, -2, LUA_REF_CACHE);									// ..., InstanceMeta
		PersistTable(TCHAR_TO_UTF8(*TypeName), -1);
		lua_pop(L, 3);															// ...
		MetaNameToStruct.FindOrAdd(TypeName) = Struct;
		StructToMetaName.FindOrAdd(Struct) = TypeName;
	}

	int luaStructTypeCall(lua_State* L) {
		UFINStruct* Struct = luaFIN_toStructType(L, 1);
		if (!Struct) return 0;
		UScriptStruct* ScriptStruct = FFINReflection::Get()->FindScriptStruct(Struct);
		if (!ScriptStruct) return 0;
		TSharedRef<FINStruct> StructContainer = MakeShared<FINStruct>(ScriptStruct);
		luaGetStruct(L, 2, StructContainer);
		luaStruct(L, *StructContainer);
		return 1;
	}

	int luaStructTypeUnpersist(lua_State* L) {
		FString StructName = luaFIN_checkfstring(L, lua_upvalueindex(1));
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
		luaFIN_pushStructType(L, Struct);
		return 1;
	}

	int luaStructTypePersist(lua_State* L) {
		UFINStruct* Type = (UFINStruct*)lua_getuservalue(L, 1);
		luaFIN_pushfstring(L, Type->GetInternalName());
		lua_pushcclosure(L, &luaStructTypeUnpersist, 1);
		return 1;
	}

	static const luaL_Reg luaStructTypeMetatable[] = {
		{"__call", luaStructTypeCall},
		{"__persist", luaStructTypePersist},
		{nullptr, nullptr}
	};

	void luaFIN_pushStructType(lua_State* L, UFINStruct* Struct) {
		if (Struct) {
			lua_pushlightuserdata(L, Struct);
			luaL_setmetatable(L, STRUCT_TYPE_METATABLE_NAME);
		} else {
			lua_pushnil(L);
		}
	}

	UFINStruct* luaFIN_toStructType(lua_State* L, int index) {
		if (lua_isnil(L, index)) return nullptr;
		return luaFIN_checkStructType(L, index);
	}

	UFINStruct* luaFIN_checkStructType(lua_State* L, int index) {
		UFINStruct* Struct = (UFINStruct*)luaL_checkudata(L, index, STRUCT_TYPE_METATABLE_NAME);
		return Struct;
	}

	int luaStructLibIndex(lua_State* L) {
		FString StructName = luaFIN_checkfstring(L, 2);
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
		if (Struct && (Struct->GetStructFlags() & FIN_Struct_Constructable)) {
			luaFIN_pushStructType(L, Struct);
		} else {
			lua_pushnil(L);
		}
		return 1;
	}

	static const luaL_Reg luaStructLibMetatable[] = {
		{"__index", luaStructLibIndex},
		{NULL, NULL}
	};

	void setupStructSystem(lua_State* L) {
		PersistSetup("StructSystem", -2);
		
		lua_pushcfunction(L, luaStructFuncCall);			// ..., InstanceFuncCall
		PersistValue("StructFuncCall");				// ...
		lua_pushcfunction(L, luaStructUnpersist);			// ..., LuaInstanceUnpersist
		PersistValue("StructUnpersist");				// ...
		lua_register(L, "findStruct", luaFindStruct);
		PersistGlobal("findStruct")

		lua_pushlightuserdata(L, nullptr);					// ..., StructLib
		luaL_newmetatable(L, "StructLib");				// ..., StructLib, StructLibMetatable
		luaL_setfuncs(L, luaStructLibMetatable, 0);
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "__metatable");
		PersistTable("StructLib", -1);
		lua_setmetatable(L, -2);						// ..., StructLib
		lua_setglobal(L, "structs");					// ...
		PersistGlobal("structs");

		lua_pushcfunction(L, luaStructLibIndex);				// ..., LuaStructLibIndex
		PersistValue("StructStructLibIndex");				// ...

		luaL_newmetatable(L, STRUCT_TYPE_METATABLE_NAME);
		luaL_setfuncs(L, luaStructTypeMetatable, 0);
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "__metatable");
		PersistTable(STRUCT_TYPE_METATABLE_NAME, -1);
		lua_pop(L, 1);
	}
}
