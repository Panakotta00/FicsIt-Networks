#include "LuaStructs.h"

#include "CoreMinimal.h"
#include "FGBuildableTrainPlatform.h"
#include "FGRailroadSubsystem.h"
#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "LuaProcessorStateStorage.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStruct.h"
#include "Utils/FINTargetPoint.h"

#define PersistParams \
	const std::string& _persist_namespace, \
	const int _persist_permTableIdx, \
	const int _persist_upermTableIdx

namespace FicsItKernel {
	namespace Lua {
		TMap<UFINStruct*, FString> StructToMetaName;
		TMap<FString, UFINStruct*> MetaNameToStruct;

		FFINDynamicStructHolder luaGetStruct(lua_State* L, int i) {
			UFINStruct* Type = luaGetStructType(L, i);
			if (!Type) return FFINDynamicStructHolder();
			FFINDynamicStructHolder Struct(Cast<UScriptStruct>(Type->GetOuter()));
			luaGetStruct(L, i, Struct);
			return Struct;
		}

		void luaGetStruct(lua_State* L, int i, FINStruct& Struct) {
			i = lua_absindex(L, i);
			UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct.GetStruct());
			if (!Type) {
				return;
			}
			for (UFINProperty* Prop : Type->GetProperties()) {
				if (!(Prop->GetPropertyFlags() & FIN_Prop_Attrib)) continue;
				lua_getfield(L, i, TCHAR_TO_UTF8(*Prop->GetInternalName()));
				FINAny Value;
				luaToNetworkValue(L, -1, Value);
				lua_pop(L, 1);
				Prop->SetValue(Struct.GetData(), Value);
			}
		}
		
		void luaStruct(lua_State* L, const FINStruct& Struct) {
			UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct.GetStruct());
			if (!Type) {
				lua_pushnil(L);
				return;
			}
			lua_newtable(L);
			for (UFINProperty* Prop : Type->GetProperties()) {
				if (!(Prop->GetPropertyFlags() & FIN_Prop_Attrib)) continue;
				FINAny Value = Prop->GetValue(Struct.GetData());
				networkValueToLua(L, Value);
				lua_setfield(L, -2, TCHAR_TO_UTF8(*Prop->GetInternalName()));
			}
			luaL_setmetatable(L, TCHAR_TO_UTF8(*StructToMetaName[Type]));
		}

		UFINStruct* luaGetStructType(lua_State* L, int i) {
			FString TypeName = luaL_typename(L, i);
			UFINStruct** Type = MetaNameToStruct.Find(TypeName);
			if (!Type) return nullptr;
			return *Type;
		}

		void setupStructSystem(lua_State* L) {
			for (const TPair<UScriptStruct*, UFINStruct*>& Struct : FFINReflection::Get()->GetStructs()) {
				FString MetaName = Struct.Value->GetInternalName();
				StructToMetaName.FindOrAdd(Struct.Value) = MetaName;
				MetaNameToStruct.FindOrAdd(MetaName) = Struct.Value;
				luaL_newmetatable(L, TCHAR_TO_UTF8(*MetaName));
				lua_pop(L, 1);
			}
		}
	}
}
