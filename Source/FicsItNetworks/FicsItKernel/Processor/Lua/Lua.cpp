#include "Lua.h"

#include "Engine/EngineTypes.h"
#include "LuaInstance.h"

namespace FicsItKernel {
	namespace Lua {
		LuaDataType propertyToLua(lua_State* L, UProperty* p, void* data, Network::NetworkTrace trace) {
			auto c = p->GetClass()->ClassCastFlags;
			if (c & EClassCastFlags::CASTCLASS_UBoolProperty) {
				lua_pushboolean(L, *p->ContainerPtrToValuePtr<bool>(data));
				return LuaDataType::LUA_BOOL;
			} else if (c & EClassCastFlags::CASTCLASS_UIntProperty) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int32_t>(data));
				return LuaDataType::LUA_INT;
			} else if (c & EClassCastFlags::CASTCLASS_UFloatProperty) {
				lua_pushnumber(L, *p->ContainerPtrToValuePtr<float>(data));
				return LuaDataType::LUA_NUM;
			} else if (c & EClassCastFlags::CASTCLASS_UStrProperty) {
				lua_pushstring(L, TCHAR_TO_UTF8(**p->ContainerPtrToValuePtr<FString>(data)));
				return LuaDataType::LUA_STR;
			} else if (c & EClassCastFlags::CASTCLASS_UObjectProperty) {
				return newInstance(L, trace / *p->ContainerPtrToValuePtr<UObject*>(data)) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
			} else {
				lua_pushnil(L);
				return LuaDataType::LUA_NIL;
			}
		}

		LuaDataType luaToProperty(lua_State* L, UProperty* p, void* data, int i) {
			auto c = p->GetClass()->ClassCastFlags;
			if (c & EClassCastFlags::CASTCLASS_UBoolProperty) {
				*p->ContainerPtrToValuePtr<bool>(data) = static_cast<bool>(lua_toboolean(L, i));
				return LuaDataType::LUA_BOOL;
			} else if (c & EClassCastFlags::CASTCLASS_UIntProperty) {
				*p->ContainerPtrToValuePtr<std::int32_t>(data) = static_cast<std::int32_t>(lua_tointeger(L, i));
				return LuaDataType::LUA_INT;
			} else if (c & EClassCastFlags::CASTCLASS_UFloatProperty) {
				*p->ContainerPtrToValuePtr<float>(data) = static_cast<float>(lua_tonumber(L, i));
				return LuaDataType::LUA_NUM;
			} else if (c & EClassCastFlags::CASTCLASS_UStrProperty) {
				auto s = lua_tostring(L, i);
				if (!s) throw std::exception("string");
				auto o = p->ContainerPtrToValuePtr<FString>(data);
				*o = FString(s);
				return LuaDataType::LUA_STR;
			} else if (c & EClassCastFlags::CASTCLASS_UObjectProperty) {
				if (Cast<UObjectProperty>(p)->PropertyClass->IsChildOf<UClass>()) {
					auto o = getClassInstance(L, i, Cast<UObjectProperty>(p)->PropertyClass);
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