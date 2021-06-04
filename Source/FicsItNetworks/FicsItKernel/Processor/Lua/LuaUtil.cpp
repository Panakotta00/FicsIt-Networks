#include "LuaUtil.h"

#include "LuaFuture.h"
#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "LuaStructs.h"
#include "FicsItNetworks/Network/FINNetworkComponent.h"
#include "FicsItNetworks/Reflection/FINArrayProperty.h"
#include "FicsItNetworks/Reflection/FINClassProperty.h"
#include "FicsItNetworks/Reflection/FINObjectProperty.h"
#include "FicsItNetworks/Reflection/FINStruct.h"
#include "FicsItNetworks/Reflection/FINTraceProperty.h"

namespace FicsItKernel {
	namespace Lua {
		void propertyToLua(lua_State* L, UProperty* p, void* data, const FFINNetworkTrace& trace) {
			auto c = p->GetClass()->GetCastFlags();
			if (c & EClassCastFlags::CASTCLASS_FBoolProperty) {
				lua_pushboolean(L, *p->ContainerPtrToValuePtr<bool>(data));
			} else if (c & EClassCastFlags::CASTCLASS_FIntProperty) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int32_t>(data));
			} else if (c & EClassCastFlags::CASTCLASS_FInt64Property) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int64_t>(data));
			} else if (c & EClassCastFlags::CASTCLASS_FFloatProperty) {
				lua_pushnumber(L, *p->ContainerPtrToValuePtr<float>(data));
			} else if (c & EClassCastFlags::CASTCLASS_FStrProperty) {
				lua_pushstring(L, TCHAR_TO_UTF8(**p->ContainerPtrToValuePtr<FString>(data)));
			} else if (c & EClassCastFlags::CASTCLASS_FClassProperty) {
				newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data));
			} else if (c & EClassCastFlags::CASTCLASS_FObjectProperty) {
				if (Cast<UObjectProperty>(p)->PropertyClass->IsChildOf<UClass>()) {
					newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data));
				} else {
					UObject* Obj = *p->ContainerPtrToValuePtr<UObject*>(data);
					FINTrace newTrace = trace / Obj;
					if (Obj && Obj->Implements<UFINNetworkComponent>()) newTrace = newTrace / IFINNetworkComponent::Execute_GetInstanceRedirect(Obj);
					newInstance(L, newTrace);
				}
			} else if (c & EClassCastFlags::CASTCLASS_FStructProperty) {
				UStructProperty* prop = Cast<UStructProperty>(p);
				if (prop->Struct == FFINDynamicStructHolder::StaticStruct()) {
					luaStruct(L, *p->ContainerPtrToValuePtr<FFINDynamicStructHolder>(data));
				} else {
					luaStruct(L, FFINDynamicStructHolder::Copy(prop->Struct, p->ContainerPtrToValuePtr<void>(data)));
				}
			} else if (c & EClassCastFlags::CASTCLASS_FArrayProperty) {
				UArrayProperty* prop = Cast<UArrayProperty>(p);
				const FScriptArray& arr = prop->GetPropertyValue_InContainer(data);
				lua_newtable(L);
				for (int i = 0; i < arr.Num(); ++i) {
					FScriptArrayHelper Helper(prop, data);
					propertyToLua(L, prop->Inner, ((uint8*)Helper.GetRawPtr()) + (prop->Inner->ElementSize * i), trace);
					lua_seti(L, -2, i+1);
				}
			} else {
				lua_pushnil(L);
			}
		}

		void luaToProperty(lua_State* L, UProperty* p, void* data, int i) {
			auto c = p->GetClass()->GetCastFlags();
			if (c & EClassCastFlags::CASTCLASS_FBoolProperty) {
				*p->ContainerPtrToValuePtr<bool>(data) = static_cast<bool>(lua_toboolean(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_FIntProperty) {
				*p->ContainerPtrToValuePtr<std::int32_t>(data) = static_cast<std::int32_t>(lua_tointeger(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_FInt64Property) {
				*p->ContainerPtrToValuePtr<std::int64_t>(data) = static_cast<std::int64_t>(lua_tointeger(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_FFloatProperty) {
				*p->ContainerPtrToValuePtr<float>(data) = static_cast<float>(lua_tonumber(L, i));
			} else if (c & EClassCastFlags::CASTCLASS_FStrProperty) {
				size_t len;
				const char* s = lua_tolstring(L, i, &len);
				if (!s) throw std::exception("Invalid String in string property parse");
				FString* o = p->ContainerPtrToValuePtr<FString>(data);
				FUTF8ToTCHAR Conv(s, len);
				*o = FString(Conv.Length(), Conv.Get());
			} else if (c & EClassCastFlags::CASTCLASS_FClassProperty) {
				UClass* o = getClassInstance(L, i, Cast<UClassProperty>(p)->PropertyClass);
				*p->ContainerPtrToValuePtr<UClass*>(data) = o;
			} else if (c & EClassCastFlags::CASTCLASS_FObjectProperty) {
				if (Cast<UObjectProperty>(p)->PropertyClass->IsChildOf<UClass>()) {
					UClass* o = getClassInstance(L, i, Cast<UObjectProperty>(p)->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = o;
				} else {
					auto o = getObjInstance(L, i, Cast<UObjectProperty>(p)->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = *o;
				}
			} else if (c & EClassCastFlags::CASTCLASS_FStructProperty) {
				UStructProperty* prop = Cast<UStructProperty>(p);
				TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(prop->Struct);
				luaGetStruct(L, i, Struct);
				prop->Struct->CopyScriptStruct(p->ContainerPtrToValuePtr<void>(data), Struct->GetData());
			} else if (c & EClassCastFlags::CASTCLASS_FArrayProperty) {
				UArrayProperty* prop = Cast<UArrayProperty>(p);
				const FScriptArray& arr = prop->GetPropertyValue_InContainer(data);
				lua_pushnil(L);
				FScriptArrayHelper Helper(prop, data);
				while (lua_next(L, i) != 0) {
					if (!lua_isinteger(L, -1)) break;
					Helper.AddValue();
					luaToProperty(L, prop->Inner, ((uint8*)Helper.GetRawPtr()) + (prop->Inner->ElementSize * (Helper.Num()-1)), -1);
					
					lua_pop(L, 1);
				}
			} else {
				lua_pushnil(L);
			}
		}

		FINAny luaToProperty(lua_State* L, UFINProperty* Prop, int Index) {
			switch (Prop->GetType()) {
			case FIN_NIL:
				return FINAny();
			case FIN_BOOL:
				return static_cast<FINBool>(lua_toboolean(L, Index));
			case FIN_INT:
				return static_cast<FINInt>(luaL_checkinteger(L, Index));
			case FIN_FLOAT:
				return static_cast<FINFloat>(luaL_checknumber(L, Index));
			case FIN_STR:
				return static_cast<FINStr>(luaL_checkstring(L, Index));
			case FIN_OBJ: {
				UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Prop);
				if (ObjProp && ObjProp->GetSubclass()) {
					return static_cast<FINObj>(getObjInstance(L, Index, ObjProp->GetSubclass()).Get());
				}
				return static_cast<FINObj>(getObjInstance(L, Index).Get());
			} case FIN_CLASS: {
				UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Prop);
				if (ClassProp && ClassProp->GetSubclass()) {
					return static_cast<FINClass>(getClassInstance(L, Index, ClassProp->GetSubclass()));
				}
				return static_cast<FINClass>(getObjInstance(L, Index).Get());
			} case FIN_TRACE: {
				UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Prop);
				if (TraceProp && TraceProp->GetSubclass()) {
					return static_cast<FINTrace>(getObjInstance(L, Index, TraceProp->GetSubclass()));
				}
				return static_cast<FINTrace>(getObjInstance(L, Index));
			} case FIN_STRUCT: {
				UFINStructProperty* StructProp = Cast<UFINStructProperty>(Prop);
				if (StructProp && StructProp->GetInner()) {
					TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(StructProp->GetInner());
					luaGetStruct(L, Index, Struct);
					return *Struct;
				}
				return *luaGetStruct(L, Index);
			} case FIN_ARRAY: {
				luaL_checktype(L, Index, LUA_TTABLE);
				UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Prop);
				FINArray Array;
				lua_pushnil(L);
				while (lua_next(L, Index) != 0) {
					if (!lua_isinteger(L, -2)) break;
					FINAny Value;
					if (ArrayProp && ArrayProp->GetInnerType()) {
						Value = luaToProperty(L, ArrayProp->GetInnerType(), -1);
					} else {
						luaToNetworkValue(L, -1, Value);
					}
					Array.Add(Value);
					lua_pop(L, 1);
				}
				return static_cast<FINArray>(Array);
			} case FIN_ANY: {
				FINAny Value;
				luaToNetworkValue(L, Index, Value);
				return Value;
			} default: ;
			}
			return FINAny();
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
				const char* s = lua_tolstring(L, i, &len);
				FUTF8ToTCHAR Conv(s, len);
				Val = FFINAnyNetworkValue(FString(Conv.Length(), Conv.Get()));
				break;
			} default:
				UFINStruct* StructType = luaGetStructType(L, i);
				if (StructType) {
					Val = FFINAnyNetworkValue(*luaGetStruct(L, i));
					break;
				}
				FFINNetworkTrace Trace = getObjInstance(L, i);
				if (Trace.IsValid()) {
					Val = FFINAnyNetworkValue(Trace);
					break;
				}
				UClass* Class = getClassInstance(L, i, UObject::StaticClass());
				if (Class) {
					Val = Class;
					break;
				}
				Val = FFINAnyNetworkValue();
				break;
			}
		}

		void networkValueToLua(lua_State* L, const FFINAnyNetworkValue& Val, const FFINNetworkTrace& Trace) {
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
				FTCHARToUTF8 Conv(*Val.GetString(), Val.GetString().Len());
				lua_pushlstring(L, Conv.Get(), Conv.Length());
				break;
			} case FIN_OBJ:
				newInstance(L, Trace / Val.GetObj().Get());
				break;
			case FIN_CLASS:
				newInstance(L, Val.GetClass());
				break;
			case FIN_TRACE:
				newInstance(L, Val.GetTrace());
				break;
			case FIN_STRUCT: {
				const FINStruct& Struct = Val.GetStruct();
				if (Struct.GetStruct()->IsChildOf(FFINFuture::StaticStruct())) {
					luaFuture(L, Struct);
				} else {
					luaStruct(L, Val.GetStruct());
				}
				break;
			} case FIN_ARRAY: {
				lua_newtable(L);
				int i = 0;
				for (const FFINAnyNetworkValue& Entry : Val.GetArray()) {
					networkValueToLua(L, Entry, Trace);
					lua_seti(L, -2, ++i);
				}
				break;
			} case FIN_ANY:
				networkValueToLua(L, Val.GetAny(), Trace);
				lua_pushnil(L);
				break;
			default:
				lua_pushnil(L);
			}
		}
	}
}
