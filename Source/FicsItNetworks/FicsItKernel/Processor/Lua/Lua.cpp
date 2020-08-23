#include "Lua.h"

#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "LuaStructs.h"
#include "Network/FINNetworkComponent.h"

namespace FicsItKernel {
	namespace Lua {
		void propertyToLua(lua_State* L, UProperty* p, void* data, FFINNetworkTrace trace) {
			auto c = p->GetClass()->ClassCastFlags;
			if (c & EClassCastFlags::CASTCLASS_UBoolProperty) {
				lua_pushboolean(L, *p->ContainerPtrToValuePtr<bool>(data));
			} else if (c & EClassCastFlags::CASTCLASS_UIntProperty) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int32_t>(data));
			} else if (c & EClassCastFlags::CASTCLASS_UInt64Property) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int64_t>(data));
			} else if (c & EClassCastFlags::CASTCLASS_UFloatProperty) {
				lua_pushnumber(L, *p->ContainerPtrToValuePtr<float>(data));
			} else if (c & EClassCastFlags::CASTCLASS_UStrProperty) {
				lua_pushstring(L, TCHAR_TO_UTF8(**p->ContainerPtrToValuePtr<FString>(data)));
			} else if (c & EClassCastFlags::CASTCLASS_UClassProperty) {
				newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data));
			} else if (c & EClassCastFlags::CASTCLASS_UObjectProperty) {
				if (Cast<UObjectProperty>(p)->PropertyClass->IsChildOf<UClass>()) {
					newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data));
				} else {
					UObject* Obj = *p->ContainerPtrToValuePtr<UObject*>(data);
					UObject* Org = Obj;
					if (Obj && Cast<UObjectProperty>(p)->PropertyClass == UObject::StaticClass() && Obj->Implements<UFINNetworkComponent>()) trace = trace / Obj / IFINNetworkComponent::Execute_GetInstanceRedirect(Obj);
					else trace = trace / Obj;
					newInstance(L, trace, Org);
				}
			} else if (c & EClassCastFlags::CASTCLASS_UStructProperty) {
				UStructProperty* prop = Cast<UStructProperty>(p);
				if (prop->Struct == FFINDynamicStructHolder::StaticStruct()) {
					luaStruct(L, *p->ContainerPtrToValuePtr<FFINDynamicStructHolder>(data));
				} else {
					luaStruct(L, FFINDynamicStructHolder::Copy(prop->Struct, p->ContainerPtrToValuePtr<void>(data)));
				}
			} else {
				lua_pushnil(L);
			}
		}

		void luaToProperty(lua_State* L, UProperty* p, void* data, int i) {
			auto c = p->GetClass()->ClassCastFlags;
			if (c & EClassCastFlags::CASTCLASS_UBoolProperty) {
				*p->ContainerPtrToValuePtr<bool>(data) = static_cast<bool>(lua_toboolean(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_UIntProperty) {
				*p->ContainerPtrToValuePtr<std::int32_t>(data) = static_cast<std::int32_t>(lua_tointeger(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_UInt64Property) {
				*p->ContainerPtrToValuePtr<std::int64_t>(data) = static_cast<std::int64_t>(lua_tointeger(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_UFloatProperty) {
				*p->ContainerPtrToValuePtr<float>(data) = static_cast<float>(lua_tonumber(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_UStrProperty) {
				size_t len;
				const char* s = lua_tolstring(L, i, &len);
				if (!s) throw std::exception("Invalid String in string property parse");
				FString* o = p->ContainerPtrToValuePtr<FString>(data);
				*o = FString(UTF8_TO_TCHAR(s), len);
			} else if (c & EClassCastFlags::CASTCLASS_UClassProperty) {
				UClass* o = getClassInstance(L, i, Cast<UClassProperty>(p)->PropertyClass);
				*p->ContainerPtrToValuePtr<UClass*>(data) = o;
			} else if (c & EClassCastFlags::CASTCLASS_UObjectProperty) {
				if (Cast<UObjectProperty>(p)->PropertyClass->IsChildOf<UClass>()) {
					UClass* o = getClassInstance(L, i, Cast<UObjectProperty>(p)->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = o;
				} else {
					auto o = getObjInstance(L, i, Cast<UObjectProperty>(p)->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = *o;
				}
			} else if (c & EClassCastFlags::CASTCLASS_UStructProperty) {
				UStructProperty* prop = Cast<UStructProperty>(p);
				FFINDynamicStructHolder Struct(prop->Struct);
				luaGetStruct(L, i, Struct);
				prop->Struct->CopyScriptStruct(p->ContainerPtrToValuePtr<void>(data), Struct.GetData());
			} else {
				lua_pushnil(L);
			}
		}

		void luaToNetworkValue(lua_State* L, int i, FFINAnyNetworkValue& Val) {
			switch (lua_type(L, i)) {
			case LUA_TNIL:
				Val = FFINAnyNetworkValue();
				break;
			case LUA_TBOOLEAN:
				Val = FFINAnyNetworkValue(static_cast<FINBool>(lua_toboolean(L, i)));
				break;
			case LUA_TNUMBER:
				if (lua_isinteger(L, i)) {
					Val = FFINAnyNetworkValue(static_cast<FINInt>(lua_tointeger(L, i)));
				} else {
					Val = FFINAnyNetworkValue(static_cast<FINFloat>(lua_tonumber(L, i)));
				}
				break;
			case LUA_TSTRING: {
				size_t len;
				Val = FFINAnyNetworkValue(FINStr(lua_tolstring(L, i, &len), len));
				break;
			}
			default:
				FString TypeName = luaL_typename(L, i);
				UScriptStruct* StructType = FFINLuaStructRegistry::Get().GetType(TypeName);
				if (StructType) {
					Val = FFINAnyNetworkValue(luaGetStruct(L, i));
					break;
				}
				Val = FFINAnyNetworkValue();
				break;
			}
		}
	}
}
