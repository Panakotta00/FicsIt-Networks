#include "FicsItKernel/Processor/Lua/LuaUtil.h"
#include "FicsItKernel/Processor/Lua/LuaFileSystemAPI.h"
#include "FicsItKernel/Processor/Lua/LuaFuture.h"
#include "FicsItKernel/Processor/Lua/LuaInstance.h"
#include "FicsItKernel/Processor/Lua/LuaStructs.h"
#include "Network/FINNetworkComponent.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINStruct.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"

namespace FicsItKernel {
	namespace Lua {
		void propertyToLua(lua_State* L, FProperty* p, void* data, const FFINNetworkTrace& trace) {
			if (p->IsA<FBoolProperty>()) {
				lua_pushboolean(L, *p->ContainerPtrToValuePtr<bool>(data));
			} else if (p->IsA<FIntProperty>()) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int32_t>(data));
			} else if (p->IsA<FInt64Property>()) {
				lua_pushinteger(L, *p->ContainerPtrToValuePtr<std::int64_t>(data));
			} else if (p->IsA<FFloatProperty>()) {
				lua_pushnumber(L, *p->ContainerPtrToValuePtr<float>(data));
			} else if (p->IsA<FDoubleProperty>()) {
				lua_pushnumber(L, *p->ContainerPtrToValuePtr<double>(data));
			} else if (p->IsA<FStrProperty>()) {
				lua_pushstring(L, TCHAR_TO_UTF8(**p->ContainerPtrToValuePtr<FString>(data)));
			} else if (p->IsA<FClassProperty>()) {
				newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data));
			} else if (FObjectProperty* objProp = CastField<FObjectProperty>(p)) {
				if (objProp->PropertyClass->IsChildOf<UClass>()) {
					newInstance(L, *p->ContainerPtrToValuePtr<UClass*>(data));
				} else {
					UObject* Obj = *p->ContainerPtrToValuePtr<UObject*>(data);
					FINTrace newTrace = trace / Obj;
					if (Obj && Obj->Implements<UFINNetworkComponent>()) newTrace = newTrace / IFINNetworkComponent::Execute_GetInstanceRedirect(Obj);
					newInstance(L, newTrace);
				}
			} else if (FStructProperty* sProp = CastField<FStructProperty>(p)) {
				if (sProp->Struct == FFINDynamicStructHolder::StaticStruct()) {
					luaStruct(L, *p->ContainerPtrToValuePtr<FFINDynamicStructHolder>(data));
				} else {
					luaStruct(L, FFINDynamicStructHolder::Copy(sProp->Struct, p->ContainerPtrToValuePtr<void>(data)));
				}
			} else if (FArrayProperty* aProp = CastField<FArrayProperty>(p)) {
				//const FScriptArray& arr = prop->GetPropertyValue_InContainer(data);
				const FScriptArray& arr = *aProp->ContainerPtrToValuePtr<FScriptArray>(data); // TODO: Check in Game
				lua_newtable(L);
				for (int i = 0; i < arr.Num(); ++i) {
					FScriptArrayHelper Helper(aProp, data);
					propertyToLua(L, aProp->Inner, ((uint8*)Helper.GetRawPtr()) + (aProp->Inner->ElementSize * i), trace);
					lua_seti(L, -2, i+1);
				}
			} else {
				lua_pushnil(L);
			}
		}

		void luaToProperty(lua_State* L, FProperty* p, void* data, int i) {
			if (p->IsA<FBoolProperty>()) {
				*p->ContainerPtrToValuePtr<bool>(data) = static_cast<bool>(lua_toboolean(L, i));
			} else if (p->IsA<FIntProperty>()) {
				*p->ContainerPtrToValuePtr<std::int32_t>(data) = static_cast<std::int32_t>(lua_tointeger(L, i));
			} else if (p->IsA<FInt64Property>()) {
				*p->ContainerPtrToValuePtr<std::int64_t>(data) = static_cast<std::int64_t>(lua_tointeger(L, i));
			} else if (p->IsA<FFloatProperty>()) {
				*p->ContainerPtrToValuePtr<float>(data) = static_cast<float>(lua_tonumber(L, i));
			} else if (p->IsA<FDoubleProperty>()) {
				*p->ContainerPtrToValuePtr<double>(data) = static_cast<double>(lua_tonumber(L, i));
			} else if (p->IsA<FStrProperty>()) {
				size_t len;
				const char* s = lua_tolstring(L, i, &len);
				if (!s) throw std::exception("Invalid String in string property parse");
				FString* o = p->ContainerPtrToValuePtr<FString>(data);
				FUTF8ToTCHAR Conv(s, len);
				*o = FString(Conv.Length(), Conv.Get());
			} else if (FClassProperty* cProp = CastField<FClassProperty>(p)) {
				UClass* o = getClassInstance(L, i, cProp->PropertyClass);
				*p->ContainerPtrToValuePtr<UClass*>(data) = o;
			} else if (FObjectProperty* objProp = CastField<FObjectProperty>(p)) {
				if (objProp->PropertyClass->IsChildOf<UClass>()) {
					UClass* o = getClassInstance(L, i, objProp->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = o;
				} else {
					FFINNetworkTrace o = getObjInstance(L, i, objProp->PropertyClass);
					*p->ContainerPtrToValuePtr<UObject*>(data) = *o;
				}
			} else if (FStructProperty* sProp = CastField<FStructProperty>(p)) {
				TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(sProp->Struct);
				luaGetStruct(L, i, Struct);
				sProp->Struct->CopyScriptStruct(p->ContainerPtrToValuePtr<void>(data), Struct->GetData());
			} else if (FArrayProperty* aProp = CastField<FArrayProperty>(p)) {
				//const FScriptArray& arr = prop->GetPropertyValue_InContainer(data);
				if (lua_istable(L, i)) {
					const FScriptArray& arr = *aProp->ContainerPtrToValuePtr<FScriptArray>(data); // TODO: Check in Game
					lua_pushnil(L);
					FScriptArrayHelper Helper(aProp, data);
					while (lua_next(L, i) != 0) {
						if (!lua_isinteger(L, -1)) break;
						Helper.AddValue();
						luaToProperty(L, aProp->Inner, ((uint8*)Helper.GetRawPtr()) + (aProp->Inner->ElementSize * (Helper.Num()-1)), -1);
					
						lua_pop(L, 1);
					}
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
				return static_cast<FINInt>(lua_tointeger(L, Index));
			case FIN_FLOAT:
				return static_cast<FINFloat>(lua_tonumber(L, Index));
			case FIN_STR: {
				size_t len;
				const char* s = luaL_tolstring(L, Index, &len);
				FUTF8ToTCHAR Conv(s, len);
				return FString(Conv.Length(), Conv.Get());
			} case FIN_OBJ: {
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
				FFINNetworkTrace Trace = getObjInstance(L, i, UObject::StaticClass(), false);
				if (Trace.IsValid()) {
					Val = FFINAnyNetworkValue(Trace);
					break;
				}
				UClass* Class = getClassInstance(L, i, UObject::StaticClass(), false);
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
