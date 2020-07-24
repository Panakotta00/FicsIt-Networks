#include "Lua.h"

#include "Engine/EngineTypes.h"
#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "LuaStructs.h"
#include "FicsItKernel/Network/NetworkFuture.h"

namespace FicsItKernel {
	namespace Lua {
		LuaDataType propertyToLua(lua_State* L, UProperty* p, void* data, FFINNetworkTrace trace) {
			auto c = p->GetClass()->ClassCastFlags;
			if (c & EClassCastFlags::CASTCLASS_UBoolProperty) {
				lua_pushboolean(L, *p->ContainerPtrToValuePtr<bool>(data));
				return LuaDataType::LUA_BOOL;
			} else if (c & EClassCastFlags::CASTCLASS_UIntProperty) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int32_t>(data));
				return LuaDataType::LUA_INT;
			} else if (c & EClassCastFlags::CASTCLASS_UInt64Property) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int64_t>(data));
				return LuaDataType::LUA_INT;
			} else if (c & EClassCastFlags::CASTCLASS_UFloatProperty) {
				lua_pushnumber(L, *p->ContainerPtrToValuePtr<float>(data));
				return LuaDataType::LUA_NUM;
			} else if (c & EClassCastFlags::CASTCLASS_UStrProperty) {
				lua_pushstring(L, TCHAR_TO_UTF8(**p->ContainerPtrToValuePtr<FString>(data)));
				return LuaDataType::LUA_STR;
			} else if (c & EClassCastFlags::CASTCLASS_UClassProperty) {
				return newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data)) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
			} else if (c & EClassCastFlags::CASTCLASS_UObjectProperty) {
				if (Cast<UObjectProperty>(p)->PropertyClass->IsChildOf<UClass>()) {
					return newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data)) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
				} else {
					return newInstance(L, trace / *p->ContainerPtrToValuePtr<UObject*>(data)) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
				}
			} else if (c & EClassCastFlags::CASTCLASS_UStructProperty) {
				UStructProperty* prop = Cast<UStructProperty>(p);
				if (prop->Struct == FFINNetworkFuture::StaticStruct()) {
					FFINNetworkFuture Future = *p->ContainerPtrToValuePtr<FFINNetworkFuture>(data);
					luaFuture(L, Future.Future);
					return LuaDataType::LUA_FUTURE;
				}
			}
			lua_pushnil(L);
			return LuaDataType::LUA_NIL;
		}

		LuaDataType luaToProperty(lua_State* L, UProperty* p, void* data, int i) {
			auto c = p->GetClass()->ClassCastFlags;
			if (c & EClassCastFlags::CASTCLASS_UBoolProperty) {
				*p->ContainerPtrToValuePtr<bool>(data) = static_cast<bool>(lua_toboolean(L, i));
				return LuaDataType::LUA_BOOL;
			} else if (c & EClassCastFlags::CASTCLASS_UIntProperty) {
				*p->ContainerPtrToValuePtr<std::int32_t>(data) = static_cast<std::int32_t>(lua_tointeger(L, i));
				return LuaDataType::LUA_INT;
			} else if (c & EClassCastFlags::CASTCLASS_UInt64Property) {
				*p->ContainerPtrToValuePtr<std::int64_t>(data) = static_cast<std::int64_t>(lua_tointeger(L, i));
				return LuaDataType::LUA_INT;
			} else if (c & EClassCastFlags::CASTCLASS_UFloatProperty) {
				*p->ContainerPtrToValuePtr<float>(data) = static_cast<float>(lua_tonumber(L, i));
				return LuaDataType::LUA_NUM;
			} else if (c & EClassCastFlags::CASTCLASS_UStrProperty) {
				const char* s = lua_tostring(L, i);
				if (!s) throw std::exception("Invalid String in string property parse");
				FString* o = p->ContainerPtrToValuePtr<FString>(data);
				*o = FString(s);
				return LuaDataType::LUA_STR;
			} else if (c & EClassCastFlags::CASTCLASS_UClassProperty) {
				UClass* o = getClassInstance(L, i, Cast<UClassProperty>(p)->PropertyClass);
				*p->ContainerPtrToValuePtr<UClass*>(data) = o;
				return (o) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
			} else if (c & EClassCastFlags::CASTCLASS_UObjectProperty) {
				if (Cast<UObjectProperty>(p)->PropertyClass->IsChildOf<UClass>()) {
					UClass* o = getClassInstance(L, i, Cast<UObjectProperty>(p)->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = o;
					return (o) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
				} else {
					auto o = getObjInstance(L, i, Cast<UObjectProperty>(p)->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = *o;
					return (*o) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
				}
			} else {
				lua_pushnil(L);
				return LuaDataType::LUA_NIL;
			}
		}
	}
}
