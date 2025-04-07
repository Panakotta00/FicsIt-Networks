#include "FINLua/LuaUtil.h"

#include "FicsItNetworksLuaModule.h"
#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/LuaPersistence.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLua/Reflection/LuaStruct.h"
#include "Reflection/FIRArrayProperty.h"
#include "Reflection/FIRClassProperty.h"
#include "Reflection/FIRObjectProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/FIRTraceProperty.h"

namespace FINLua {
	void luaFIN_pushNetworkValue(lua_State* L, const FFIRAnyValue& Val, const FFIRTrace& Trace) {
		switch (Val.GetType()) {
		case FIR_NIL:
			lua_pushnil(L);
			break;
		case FIR_BOOL:
			lua_pushboolean(L, Val.GetBool());
			break;
		case FIR_INT:
			lua_pushinteger(L, Val.GetInt());
			break;
		case FIR_FLOAT:
			lua_pushnumber(L, Val.GetFloat());
			break;
		case FIR_STR: {
			luaFIN_pushFString(L, Val.GetString());
			break;
		} case FIR_OBJ:
			luaFIN_pushObject(L, Trace / Val.GetObj().Get());
			break;
		case FIR_CLASS:
			luaFIN_pushClass(L, Val.GetClass());
			break;
		case FIR_TRACE:
			luaFIN_pushObject(L, Val.GetTrace());
			break;
		case FIR_STRUCT: {
			const FIRStruct& Struct = Val.GetStruct();
			luaFIN_pushStruct(L, Val.GetStruct());
			break;
		} case FIR_ARRAY: {
			lua_newtable(L);
			int i = 0;
			for (const FFIRAnyValue& Entry : Val.GetArray()) {
				luaFIN_pushNetworkValue(L, Entry, Trace);
				lua_seti(L, -2, ++i);
			}
			break;
		} case FIR_ANY:
			luaFIN_pushNetworkValue(L, Val.GetAny(), Trace);
			lua_pushnil(L);
			break;
		default:
			lua_pushnil(L);
		}
	}

	TOptional<EFIRValueType> luaFIN_getNetworkValueType(lua_State* L, int Index) {
		switch (lua_type(L, Index)) {
		case LUA_TNIL:
			return FIR_NIL;
		case LUA_TBOOLEAN:
			return FIR_BOOL;
		case LUA_TNUMBER:
			if (lua_isinteger(L, Index)) {
				return FIR_INT;
			} else {
				return FIR_FLOAT;
			}
		case LUA_TSTRING:
			return FIR_STR;
		case LUA_TUSERDATA:
			if (luaFIN_toLuaStruct(L, Index, nullptr)) return FIR_STRUCT;
			if (luaFIN_toLuaObject(L, Index, nullptr)) return FIR_TRACE;
			if (luaFIN_toLuaClass(L, Index)) return FIR_CLASS;
		default: return TOptional<EFIRValueType>();
		}
	}

	TOptional<FIRAny> luaFIN_toNetworkValueByProp(lua_State* L, int Index, UFIRProperty* Property, bool bImplicitConversion, bool bImplicitConstruction) {
		int LuaType = lua_type(L, Index);
		
		switch (Property->GetType()) {
		case FIR_NIL:
			return FIRAny();
		case FIR_BOOL:
			if (!bImplicitConversion && LuaType != LUA_TBOOLEAN) return TOptional<FIRAny>();
			return FIRAny(luaFIN_toFinBool(L, Index));
		case FIR_INT:
			if (!bImplicitConversion && (LuaType != LUA_TNUMBER || !lua_isinteger(L, Index))) return TOptional<FIRAny>();
			return FIRAny(luaFIN_toFinInt(L, Index));
		case FIR_FLOAT:
			if (!bImplicitConversion && LuaType != LUA_TNUMBER) return TOptional<FIRAny>();
			return FIRAny(luaFIN_toFinFloat(L, Index));
		case FIR_STR: {
			if (!bImplicitConversion && LuaType != LUA_TSTRING) return TOptional<FIRAny>();
			return FIRAny(luaFIN_toFinString(L, Index));
		} case FIR_OBJ: {
			TOptional<FIRTrace> Object;
			UFIRObjectProperty* ObjectProp = Cast<UFIRObjectProperty>(Property);
			if (ObjectProp && ObjectProp->GetSubclass()) {
				Object = luaFIN_toObject(L, Index, FFicsItReflectionModule::Get().FindClass(ObjectProp->GetSubclass()));
				if (bImplicitConversion && !Object.IsSet() && UFIRClass::StaticClass()->IsChildOf(ObjectProp->GetSubclass())) {
					UClass* Class = luaFIN_toUClass(L, Index, nullptr);
					if (Class) Object = FIRTrace(Class);
				}
			} else {
				Object = luaFIN_toObject(L, Index, nullptr);
				if (bImplicitConversion && !Object.IsSet()) {
					UClass* Class = luaFIN_toUClass(L, Index, nullptr);
					if (Class) Object = FIRTrace(Class);
				}
			}
			if (Object.IsSet()) return FIRAny(static_cast<FIRObj>(Object.GetValue().Get()));
			return TOptional<FIRAny>();
		} case FIR_CLASS: {
			UClass* Class;
			UFIRClassProperty* ClassProp = Cast<UFIRClassProperty>(Property);
			if (ClassProp && ClassProp->GetSubclass()) {
				Class = luaFIN_toSubUClass(L, Index, ClassProp->GetSubclass());
			} else {
				Class = luaFIN_toUClass(L, Index, nullptr);
			}
			if (Class) return FIRAny(static_cast<FIRClass>(Class));
			return TOptional<FIRAny>();
		} case FIR_TRACE: {
			TOptional<FIRTrace> Trace;
			UFIRTraceProperty* TraceProp = Cast<UFIRTraceProperty>(Property);
			if (TraceProp && TraceProp->GetSubclass()) {
				Trace = luaFIN_toObject(L, Index, FFicsItReflectionModule::Get().FindClass(TraceProp->GetSubclass()));
			} else {
				Trace = luaFIN_toObject(L, Index, nullptr);
			}
			if (Trace.IsSet()) return FIRAny(Trace.GetValue());
			return TOptional<FIRAny>();
		} case FIR_STRUCT: {
			TSharedPtr<FIRStruct> Struct;
			UFIRStructProperty* StructProp = Cast<UFIRStructProperty>(Property);
			if (StructProp && StructProp->GetInner()) {
				UFIRStruct* Type = FFicsItReflectionModule::Get().FindStruct(StructProp->GetInner());
				Struct = luaFIN_toStruct(L, Index, Type, bImplicitConstruction);
			} else {
				Struct = luaFIN_toStruct(L, Index, nullptr, false);
			}
			if (Struct.IsValid()) return FIRAny(*Struct);
			return TOptional<FIRAny>();
		} case FIR_ARRAY: {
			if (LuaType != LUA_TTABLE) return TOptional<FIRAny>();
			UFIRArrayProperty* ArrayProp = Cast<UFIRArrayProperty>(Property);
			FIRArray Array;
			lua_pushnil(L);
			while (lua_next(L, Index) != 0) {
				if (!lua_isinteger(L, -2)) break;
				TOptional<FIRAny> Value;
				if (ArrayProp && ArrayProp->GetInnerType()) {
					Value = luaFIN_toNetworkValueByProp(L, -1, ArrayProp->GetInnerType(), bImplicitConversion, bImplicitConstruction);
				} else {
					Value = luaFIN_toNetworkValue(L, -1);
				}
				if (Value.IsSet()) Array.Add(Value.GetValue());
				lua_pop(L, 1);
			}
			return FIRAny(Array);
		} case FIR_ANY: return luaFIN_toNetworkValue(L, Index);
		default: ;
		}
		return FIRAny();
	}
	
	TOptional<FIRAny> luaFIN_toNetworkValue(lua_State* L, int Index, UFIRProperty* Property, bool bImplicitConversion, bool bImplicitConstruction) {
		if (Property) return luaFIN_toNetworkValueByProp(L, Index, Property, bImplicitConversion, bImplicitConstruction);
		else return luaFIN_toNetworkValue(L, Index);
	}

	TOptional<FIRAny> luaFIN_toNetworkValue(lua_State* L, int Index) {
		switch (lua_type(L, Index)) {
		case LUA_TNIL:
			return FIRAny();
		case LUA_TBOOLEAN:
			return FIRAny(luaFIN_toFinBool(L, Index));
		case LUA_TNUMBER:
			if (lua_isinteger(L, Index)) {
				return FIRAny(luaFIN_toFinInt(L, Index));
			} else {
				return FIRAny(luaFIN_toFinFloat(L, Index));
			}
		case LUA_TSTRING:
			return FIRAny(luaFIN_toFinString(L, Index));
		case LUA_TTABLE: {
			FIRArray Array;
			lua_pushnil(L);
			while (lua_next(L, Index) != 0) {
				if (!lua_isinteger(L, -2)) break;
				TOptional<FIRAny> Value = luaFIN_toNetworkValue(L, -1);
				lua_pop(L, 1);
				if (!Value.IsSet()) return TOptional<FIRAny>();
				Array.Add(*Value);
			}
			return FIRAny(Array);
		} default:
			TSharedPtr<FIRStruct> Struct = luaFIN_toStruct(L, Index, nullptr, false);
			if (Struct.IsValid()) return FIRAny(static_cast<FIRStruct>(*Struct));
			TOptional<FFIRTrace> Object = luaFIN_toObject(L, Index, nullptr);
			if (Object.IsSet()) return FIRAny(static_cast<FIRObj>(Object.GetValue().Get()));
			UClass* Class = luaFIN_toUClass(L, Index, nullptr);
			if (Class) return FIRAny(static_cast<FIRClass>(Class));
		}
		return TOptional<FIRAny>();
	}

	FString luaFIN_getPropertyTypeName(lua_State* L, UFIRProperty* Property) {
		switch (Property->GetType()) {
		case FIR_NIL: return UTF8_TO_TCHAR(lua_typename(L, LUA_TNIL));
		case FIR_BOOL: return UTF8_TO_TCHAR(lua_typename(L, LUA_TBOOLEAN));
		case FIR_INT: return TEXT("integer");
		case FIR_FLOAT: return TEXT("float");
		case FIR_STR: return UTF8_TO_TCHAR(lua_typename(L, LUA_TSTRING));
		case FIR_OBJ: {
			UFIRObjectProperty* ObjProp = Cast<UFIRObjectProperty>(Property);
			UFIRClass* Class = nullptr;
			if (ObjProp && ObjProp->GetSubclass()) {
				Class = FFicsItReflectionModule::Get().FindClass(ObjProp->GetSubclass());
			}
			return FFicsItReflectionModule::ObjectReferenceText(Class);
		} case FIR_CLASS: {
			UFIRClassProperty* ClassProp = Cast<UFIRClassProperty>(Property);
			UFIRClass* Class = nullptr;
			if (ClassProp && ClassProp->GetSubclass()) {
				Class = FFicsItReflectionModule::Get().FindClass(ClassProp->GetSubclass());
			}
			return FFicsItReflectionModule::ClassReferenceText(Class);
		} case FIR_TRACE: {
			UFIRTraceProperty* TraceProp = Cast<UFIRTraceProperty>(Property);
			UFIRClass* Class = nullptr;
			if (TraceProp && TraceProp->GetSubclass()) {
				Class = FFicsItReflectionModule::Get().FindClass(TraceProp->GetSubclass());
			}
			return FFicsItReflectionModule::TraceReferenceText(Class);
		} case FIR_STRUCT: {
			UFIRStructProperty* StructProp = Cast<UFIRStructProperty>(Property);
			UFIRStruct* Type = nullptr;
			if (StructProp && StructProp->GetInner()) {
				Type = FFicsItReflectionModule::Get().FindStruct(StructProp->GetInner());
			}
			return FFicsItReflectionModule::StructReferenceText(Type);
		} case FIR_ARRAY: {
			FString TypeName = TEXT("Array");
			UFIRArrayProperty* ArrayProp = Cast<UFIRArrayProperty>(Property);
			if (ArrayProp && ArrayProp->GetInnerType()) {
				TypeName.Append(TEXT("<")).Append(luaFIN_getPropertyTypeName(L, ArrayProp->GetInnerType())).Append(TEXT(">"));
			}
			return TypeName;
		} case FIR_ANY: return TEXT("Any");
		default: ;
		}
		return TEXT("Unkown");
	}

	FString luaFIN_getFunctionSignature(lua_State* L, UFIRFunction* Function) {
		TArray<FString> parameters;
		TArray<FString> returnValues;

		if (true) { // TODO: Once Static function are added, add check to disable self
			parameters.Add(FString::Printf(TEXT("self : %s"), *FFicsItReflectionModule::TraceReferenceText(Function->GetTypedOuter<UFIRClass>())));
		}

		for (UFIRProperty* parameter : Function->GetParameters()) {
			EFIRPropertyFlags flags = parameter->GetPropertyFlags();
			if (!(flags & FIR_Prop_Param)) continue;
			TArray<FString>& list = (flags & FIR_Prop_OutParam) ? returnValues : parameters;
			list.Add(FString::Printf(TEXT("%s : %s"), *parameter->GetInternalName(), *luaFIN_getPropertyTypeName(L, parameter)));
		}

		FString joinedParameters = FString::Join(parameters, TEXT(", "));
		FString joinedReturnValues = FString::Join(returnValues, TEXT(", "));

		return FString::Printf(TEXT("(%s) %s(%s)"), *joinedParameters, *Function->GetInternalName(), *joinedReturnValues);
	}

	int luaFIN_propertyError(lua_State* L, int Index, UFIRProperty* Property) {
		return luaFIN_typeError(L, Index, luaFIN_getPropertyTypeName(L, Property));
	}

	int luaFIN_typeError(lua_State* L, int Index, const FString& ExpectedTypeName) {
		FString ActualTypeName = luaFIN_typeName(L, Index);
		return luaFIN_argError(L, Index, FString::Printf(TEXT("%s expected, got %s"), *ExpectedTypeName, *ActualTypeName));
	}

	int luaFIN_argError(lua_State* L, int Index, const FString& ExtraMessage) {
		return luaL_argerror(L, Index, TCHAR_TO_UTF8(*ExtraMessage));
	}

	FString luaFIN_typeName(lua_State* L, int Index) {
		const char *typearg;
		int metaNameType = luaL_getmetafield(L, Index, "__name");
		if (metaNameType == LUA_TSTRING) {
			typearg = lua_tostring(L, -1);
		} else if (lua_type(L, Index) == LUA_TLIGHTUSERDATA) {
			typearg = "light userdata";
		} else {
			typearg = luaL_typename(L, Index);
		}
		if (metaNameType != LUA_TNIL) {
			lua_pop(L, 1);
		}

		FString TypeName = UTF8_TO_TCHAR(typearg);
		if (TypeName == luaFIN_getLuaObjectTypeName()) {
			FLuaObject* LuaObject = luaFIN_toLuaObject(L, Index, nullptr);
			UFIRClass* Type = nullptr;
			if (LuaObject) Type = LuaObject->Type;
			return FFicsItReflectionModule::ObjectReferenceText(Type);
		}
		if (TypeName == luaFIN_getLuaClassTypeName()) {
			FLuaClass* LuaClass = luaFIN_toLuaClass(L, Index);
			UFIRClass* Type = nullptr;
			if (LuaClass) Type = LuaClass->FIRClass;
			return FFicsItReflectionModule::ClassReferenceText(Type);
		}
		if (TypeName == luaFIN_getLuaStructTypeName()) {
			FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, nullptr);
			UFIRStruct* Type = nullptr;
			if (LuaStruct) Type = LuaStruct->Type;
			return FFicsItReflectionModule::StructReferenceText(Type);
		}
		return TypeName;
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

	int luaFIN_yield(lua_State* L, int nresults, lua_KContext ctx, lua_KFunction kfunc) {
		lua_pushnil(L);
		lua_insert(L, -nresults - 1);
		return lua_yieldk(L, nresults+1, ctx, kfunc);
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
		const char* str = lua_tolstring(L, index, &len);
		FUTF8ToTCHAR conv(str, len);
		return FString(conv.Length(), conv.Get());
	}

	FString luaFIN_convToFString(lua_State* L, int index) {
		size_t len;
		const char* str = luaL_tolstring(L, index, &len);
		FUTF8ToTCHAR conv(str, len);
		return FString(conv.Length(), conv.Get());
	}

	void luaFIN_warning(lua_State* L, const char* msg, int tocont) {
		lua_Debug ar;
		if (lua_getstack(L, 1, &ar)) {
			lua_getinfo(L, "Sl", &ar);
			if (ar.currentline > 0) {
				lua_pushfstring(L, "%s:%d: %s", ar.short_src, ar.currentline, msg);
				const char* warn = lua_tostring(L, -1);
				lua_warning(L, warn, tocont);
				lua_pop(L, 1);
				return;
			}
		}
		lua_warning(L, msg, tocont);
	}

	FString luaFIN_where(lua_State* L) {
		lua_Debug ar;
		if (lua_getstack(L, 1, &ar)) {
			lua_getinfo(L, "Sl", &ar);
			if (ar.currentline > 0) {
				return FString::Printf(TEXT("%s:%d"), UTF8_TO_TCHAR(ar.short_src), ar.currentline);
			}
		}
		return FString();
	}

	FString luaFIN_stack(lua_State* L) {
		return FString();
	}

	void luaFIN_setOrMergeField(lua_State* L, int targetIndex) {					// ..., key, value
		targetIndex = lua_absindex(L, targetIndex);
		int fieldIndex = lua_absindex(L, -2);
		int mergeIndex = lua_absindex(L, -1);

		int top = lua_gettop(L);

		luaL_checktype(L, targetIndex, LUA_TTABLE);

		lua_pushvalue(L, fieldIndex);
		int fieldType = lua_gettable(L, targetIndex);								// ..., key, value, field
		if (fieldType != LUA_TTABLE) {
			lua_pop(L, 1);
			lua_settable(L, targetIndex);
		} else {
			lua_pushnil(L);
			while (lua_next(L, mergeIndex) != 0) {									// ..., key, value, field, key2, value2
				lua_pushvalue(L, -2);
				lua_insert(L, -2);												// ..., key, value, field, key2, key2, value2
				luaFIN_setOrMergeField(L, -4);								// ..., key, value, field, key2
			}
			lua_pop(L, 3);
		}

		fgcheck(lua_gettop(L) == top-2);
	}

	void luaFINDebug_dumpStack(lua_State* L) {
		UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Dumping stack of thread %p:"), L);
		int args = lua_gettop(L);
		int negative = 0;
		for (; args > 0; --args) {
			UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Lua Stack: [%i/%i] %s"), args, --negative, *luaFIN_typeName(L, args));
		}
	}

	void luaFINDebug_dumpTable(lua_State* L, int index) {
		lua_pushvalue(L, index);
		lua_pushnil(L);
		while (lua_next(L, -2)) {
			lua_pushvalue(L, -2);
			FString key = luaFIN_convToFString(L, -1);
			lua_pop(L, 1);
			FString type = luaFIN_convToFString(L, -2);
			UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Lua Table: [%s] = %s"), *key, *type);
			lua_pop(L, 3);
		}
		lua_pop(L, 1);
	}
}

FFINLuaLogScope::FFINLuaLogScope(lua_State* L) : FFILLogScope(nullptr, FWhereFunction::CreateLambda([L]() {
	return FINLua::luaFIN_where(L);
}), FStackFunction::CreateLambda([L]() {
	return FINLua::luaFIN_stack(L);
})) {}

FCbWriter& operator<<(FCbWriter& Writer, lua_State* const& L) {
	Writer.AddString(FINLua::luaFIN_where(L));
	return Writer;
}
