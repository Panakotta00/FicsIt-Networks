#include "LuaProcessor/LuaUtil.h"

#include "LuaProcessor/LuaFileSystemAPI.h"
#include "LuaProcessor/LuaFuture.h"
#include "LuaProcessor/LuaClass.h"
#include "LuaProcessor/LuaInstance.h"
#include "LuaProcessor/LuaStruct.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStruct.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"

namespace FINLua {
	void luaFIN_pushNetworkValue(lua_State* L, const FFINAnyNetworkValue& Val, const FFINNetworkTrace& Trace) {
		switch (Val.GetType()) {
		case FIN_NIL:
			lua_pushnil(L);
			break;
		case FIN_BOOL:
			lua_pushboolean(L, Val.GetBool());
			break;
		case FIN_INT:
			lua_pushinteger(L, Val.GetInt());
			break;
		case FIN_FLOAT:
			lua_pushnumber(L, Val.GetFloat());
			break;
		case FIN_STR: {
			luaFIN_pushFString(L, Val.GetString());
			break;
		} case FIN_OBJ:
			newInstance(L, Trace / Val.GetObj().Get());
			break;
		case FIN_CLASS:
			luaFIN_pushClass(L, Val.GetClass());
			break;
		case FIN_TRACE:
			newInstance(L, Val.GetTrace());
			break;
		case FIN_STRUCT: {
			const FINStruct& Struct = Val.GetStruct();
			if (Struct.GetStruct()->IsChildOf(FFINFuture::StaticStruct())) {
				luaFuture(L, Struct);
			} else {
				luaFIN_pushStruct(L, Val.GetStruct());
			}
			break;
		} case FIN_ARRAY: {
			lua_newtable(L);
			int i = 0;
			for (const FFINAnyNetworkValue& Entry : Val.GetArray()) {
				luaFIN_pushNetworkValue(L, Entry, Trace);
				lua_seti(L, -2, ++i);
			}
			break;
		} case FIN_ANY:
			luaFIN_pushNetworkValue(L, Val.GetAny(), Trace);
			lua_pushnil(L);
			break;
		default:
			lua_pushnil(L);
		}
	}

	TOptional<EFINNetworkValueType> luaFIN_getNetworkValueType(lua_State* L, int Index) {
		switch (lua_type(L, Index)) {
		case LUA_TNIL:
			return FIN_NIL;
		case LUA_TBOOLEAN:
			return FIN_BOOL;
		case LUA_TNUMBER:
			if (lua_isinteger(L, Index)) {
				return FIN_INT;
			} else {
				return FIN_FLOAT;
			}
		case LUA_TSTRING:
			return FIN_STR;
		case LUA_TUSERDATA:
			if (luaFIN_toLuaStruct(L, Index)) return FIN_STRUCT;
			if (luaFIN_getObjectType(L, Index)) return FIN_TRACE;
			if (luaFIN_toLuaClass(L, Index)) return FIN_CLASS;
		default: return TOptional<EFINNetworkValueType>();
		}
	}

	TOptional<FINAny> luaFIN_toNetworkValueByProp(lua_State* L, int Index, UFINProperty* Property, bool bImplicitConversion, bool bImplicitConstruction) {
		int LuaType = lua_type(L, Index);
		
		switch (Property->GetType()) {
		case FIN_NIL:
			return FINAny();
		case FIN_BOOL:
			if (!bImplicitConversion && LuaType != LUA_TBOOLEAN) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinBool(L, Index));
		case FIN_INT:
			if (!bImplicitConversion && (LuaType != LUA_TNUMBER || !lua_isinteger(L, Index))) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinInt(L, Index));
		case FIN_FLOAT:
			if (!bImplicitConversion && LuaType != LUA_TNUMBER) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinFloat(L, Index));
		case FIN_STR: {
			if (!bImplicitConversion && LuaType != LUA_TSTRING) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinString(L, Index));
		} case FIN_OBJ: {
			// TODO: Refactored getObjInstance function
			if (LuaType == LUA_TNIL || LuaType == LUA_TNONE) return FINAny(FINObj());
			// TODO: Expect LuaObj type, otherwise return None (right now it returns nullptr Obj)
			UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Property);
			if (ObjProp && ObjProp->GetSubclass()) {
				return FINAny(static_cast<FINObj>(getObjInstance(L, Index, ObjProp->GetSubclass()).Get()));
			}
			return FINAny(static_cast<FINObj>(getObjInstance(L, Index).Get()));
		} case FIN_CLASS: {
			UClass* Class;
			UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Property);
			if (ClassProp && ClassProp->GetSubclass()) {
				Class = luaFIN_toSubUClass(L, Index, ClassProp->GetSubclass());
			} else {
				Class = luaFIN_toUClass(L, Index);
			}
			if (Class) return FINAny(static_cast<FINClass>(Class));
			return TOptional<FINAny>();
		} case FIN_TRACE: {
			// TODO: Refactored getObjInstance function
			if (LuaType == LUA_TNIL || LuaType == LUA_TNONE) return FINAny(FINTrace());
			// TODO: Expect LuaObj type, otherwise return None (right now it returns nullptr Trace)
			
			UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Property);
			if (TraceProp && TraceProp->GetSubclass()) {
				return FINAny(static_cast<FINTrace>(getObjInstance(L, Index, TraceProp->GetSubclass())));
			}
			return FINAny(static_cast<FINTrace>(getObjInstance(L, Index)));
		} case FIN_STRUCT: {
			TSharedPtr<FINStruct> Struct;
			UFINStructProperty* StructProp = Cast<UFINStructProperty>(Property);
			if (StructProp && StructProp->GetInner()) {
				UFINStruct* Type = FFINReflection::Get()->FindStruct(StructProp->GetInner());
				Struct = luaFIN_toStruct(L, Index, Type, bImplicitConstruction);
			} else {
				Struct = luaFIN_toStruct(L, Index, nullptr, false);
			}
			if (Struct.IsValid()) return FINAny(*Struct);
			return TOptional<FINAny>();
		} case FIN_ARRAY: {
			if (LuaType != LUA_TTABLE) return TOptional<FINAny>();
			UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Property);
			FINArray Array;
			lua_pushnil(L);
			while (lua_next(L, Index) != 0) {
				if (!lua_isinteger(L, -2)) break;
				TOptional<FINAny> Value;
				if (ArrayProp && ArrayProp->GetInnerType()) {
					Value = luaFIN_toNetworkValueByProp(L, -1, ArrayProp->GetInnerType(), bImplicitConversion, bImplicitConstruction);
				} else {
					Value = luaFIN_toNetworkValue(L, -1);
				}
				if (Value.IsSet()) Array.Add(Value.GetValue());
				lua_pop(L, 1);
			}
			return FINAny(Array);
		} case FIN_ANY: return luaFIN_toNetworkValue(L, Index);
		default: ;
		}
		return FINAny();
	}
	
	TOptional<FINAny> luaFIN_toNetworkValue(lua_State* L, int Index, UFINProperty* Property, bool bImplicitConversion, bool bImplicitConstruction) {
		if (Property) return luaFIN_toNetworkValueByProp(L, Index, Property, bImplicitConversion, bImplicitConstruction);
		else return luaFIN_toNetworkValue(L, Index);
	}

	TOptional<FINAny> luaFIN_toNetworkValue(lua_State* L, int Index) {
		switch (lua_type(L, Index)) {
		case LUA_TNIL:
			return FINAny();
		case LUA_TBOOLEAN:
			return FINAny(luaFIN_toFinBool(L, Index));
		case LUA_TNUMBER:
			if (lua_isinteger(L, Index)) {
				return FINAny(luaFIN_toFinInt(L, Index));
			} else {
				return FINAny(luaFIN_toFinFloat(L, Index));
			}
		case LUA_TSTRING:
			return FINAny(luaFIN_toFinString(L, Index));
		case LUA_TTABLE: {
			FINArray Array;
			lua_pushnil(L);
			while (lua_next(L, Index) != 0) {
				if (!lua_isinteger(L, -2)) break;
				TOptional<FINAny> Value = luaFIN_toNetworkValue(L, -1);
				lua_pop(L, 1);
				if (!Value.IsSet()) return TOptional<FINAny>();
				Array.Add(*Value);
			}
			return FINAny(Array);
		} default:
			TSharedPtr<FINStruct> Struct = luaFIN_toStruct(L, Index, nullptr, false);
			if (Struct.IsValid()) return FINAny(*Struct);
			// TODO: Check optional Object
			//FFINNetworkTrace Trace = getObjInstance(L, Index, UObject::StaticClass(), false);
			//if (Trace.IsValid()) return FINAny(Trace);
			UClass* Class = luaFIN_toUClass(L, Index);
			if (Class) return FINAny(static_cast<FINClass>(Class));
		}
		return TOptional<FINAny>();
	}

	FString luaFIN_getPropertyTypeName(lua_State* L, UFINProperty* Property) {
		switch (Property->GetType()) {
		case FIN_NIL: return UTF8_TO_TCHAR(lua_typename(L, LUA_TNIL));
		case FIN_BOOL: return UTF8_TO_TCHAR(lua_typename(L, LUA_TBOOLEAN));
		case FIN_INT: return TEXT("integer");
		case FIN_FLOAT: return TEXT("float");
		case FIN_STR: return UTF8_TO_TCHAR(lua_typename(L, LUA_TSTRING));
		case FIN_OBJ: {
			UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Property);
			if (ObjProp && ObjProp->GetSubclass()) {
				UFINClass* Class = FFINReflection::Get()->FindClass(ObjProp->GetSubclass());
				if (Class) return Class->GetInternalName();
			}
			return TEXT("Object");
		} case FIN_CLASS: {
			UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Property);
			if (ClassProp && ClassProp->GetSubclass()) {
				UFINClass* Class = FFINReflection::Get()->FindClass(ClassProp->GetSubclass());
				if (Class) return Class->GetInternalName().Append(TEXT("-Class"));
			}
			return TEXT("Class");
		} case FIN_TRACE: {
			UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Property);
			if (TraceProp && TraceProp->GetSubclass()) {
				UFINClass* Class = FFINReflection::Get()->FindClass(TraceProp->GetSubclass());
				if (Class) return Class->GetInternalName().Append(TEXT("-Trace"));
			}
			return TEXT("Trace");
		} case FIN_STRUCT: {
			UFINStructProperty* StructProp = Cast<UFINStructProperty>(Property);
			if (StructProp && StructProp->GetInner()) {
				UFINStruct* Type = FFINReflection::Get()->FindStruct(StructProp->GetInner());
				if (Type) return Type->GetInternalName().Append(TEXT("-Struct"));
			}
			return TEXT("Struct");
		} case FIN_ARRAY: {
			FString TypeName = TEXT("Array");
			UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Property);
			if (ArrayProp && ArrayProp->GetInnerType()) {
				TypeName.Append(TEXT("<")).Append(luaFIN_getPropertyTypeName(L, ArrayProp->GetInnerType())).Append(TEXT(">"));
			}
			return TypeName;
		} case FIN_ANY: return TEXT("Any");
		default: ;
		}
		return TEXT("Unkown");
	}

	FString luaFIN_getUserDataMetaName(lua_State* L, int Index) {
		if (lua_type(L, Index) != LUA_TUSERDATA) return FString();
		int fieldType = luaL_getmetafield(L, Index, "__name");
		if (fieldType != LUA_TSTRING) {
			if (fieldType != LUA_TNONE) lua_pop(L, 1);
			return FString();
		}
		FString metaName = luaFIN_toFString(L, -1);
		lua_pop(L, 1);
		return metaName;
	}

	void luaFIN_pushFString(lua_State* L, const FString& Str) {
		FTCHARToUTF8 conv(*Str, Str.Len());
		lua_pushlstring(L, conv.Get(), conv.Length());
	}

	FString luaFIN_checkFString(lua_State* L, int Index) {
		size_t len;
		const char* str = luaL_checklstring(L, Index, &len);
		FUTF8ToTCHAR conv(str, len);
		return FString(conv.Length(), conv.Get());
	}

	FString luaFIN_toFString(lua_State* L, int index) {
		size_t len;
		const char* str = luaL_tolstring(L, index, &len);
		FUTF8ToTCHAR conv(str, len);
		return FString(conv.Length(), conv.Get());
	}

	void setupUtilLib(lua_State* L) {
		PersistSetup("UtilLib", -2);
		
		
	}
}
