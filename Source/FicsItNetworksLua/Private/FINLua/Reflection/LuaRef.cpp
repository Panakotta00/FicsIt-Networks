#include "FINLua/Reflection/LuaRef.h"

#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLua/Reflection/LuaStruct.h"
#include "FINLuaProcessor.h"
#include "tracy/Tracy.hpp"

namespace FINLua {
	TArray<FINAny> luaFIN_callReflectionFunctionProcessInput(lua_State* L, UFINFunction* Function, int nArgs) {
		int startArg = 2;
		TArray<FINAny> Input;
		TArray<UFINProperty*> Parameters = Function->GetParameters();
		for (UFINProperty* Parameter : Parameters) {
			if (Parameter->GetPropertyFlags() & FIN_Prop_Param && !(Parameter->GetPropertyFlags() & (FIN_Prop_OutParam | FIN_Prop_RetVal))) {
				int index = lua_absindex(L, -nArgs);
				TOptional<FINAny> Value = luaFIN_toNetworkValueByProp(L, index, Parameter, true, true);
				if (Value.IsSet()) Input.Add(*Value);
				else {
					lua_gettop(L);
					luaFIN_propertyError(L, Input.Num() + startArg, Parameter);
				}
				nArgs -= 1;
				if (nArgs <= 0) break;
			}
		}
		for (; nArgs > 0; nArgs -= 1) {
			TOptional<FINAny> Value = luaFIN_toNetworkValue(L, lua_absindex(L, -nArgs));
			if (Value.IsSet()) Input.Add(*Value);
			else Input.Add(FINAny());
		}
		return Input;
	}

	int luaFIN_callReflectionFunctionProcessOutput(lua_State* L, const TArray<FINAny>& Output, const FFINNetworkTrace& PrefixTrace, int nResults) {
		int pushed = 0;
		for (const FINAny& Value : Output) {
			if (pushed >= nResults && nResults != LUA_MULTRET) break;
			luaFIN_pushNetworkValue(L, Value, PrefixTrace);
			++pushed;
		}
		return pushed;
	}
	
	int luaFIN_callReflectionFunctionDirectly(lua_State* L, UFINFunction* Function, const FFINExecutionContext& Ctx, int nArgs, int nResults) {
		if (!Ctx.IsValid()) {
			return luaFIN_argError(L, 1, FString::Printf(TEXT("Reference to %s is invalid."), *luaFIN_typeName(L, 1)));
		} 
		
		TArray<FINAny> Parameters = luaFIN_callReflectionFunctionProcessInput(L, Function, nArgs);
		
		TArray<FINAny> Output;
		try {
			Output = Function->Execute(Ctx, Parameters);
		} catch (const FFINFunctionBadArgumentException& Ex) {
			return luaFIN_argError(L, Ex.ArgumentIndex+2, Ex.GetMessage()); // TODO: Change Argument Index Offset for C++ ArgumentException
		} catch (const FFINReflectionException& Ex) {
			return luaL_error(L, TCHAR_TO_UTF8(*Ex.GetMessage()));
		}

		return luaFIN_callReflectionFunctionProcessOutput(L, Output, Ctx.GetTrace(), nResults);
	}
	
	int luaFIN_callReflectionFunction(lua_State* L, UFINFunction* Function, const FFINExecutionContext& Ctx, int nArgs, int nResults) {
		FFINLuaLogScope LogScope(L);
		const EFINFunctionFlags FuncFlags = Function->GetFunctionFlags();
		if (FuncFlags & FIN_Func_RT_Async) {
			return luaFIN_callReflectionFunctionDirectly(L, Function, Ctx, nArgs, nResults);
		} else if (FuncFlags & FIN_Func_RT_Parallel) {
			[[maybe_unused]] FLuaSyncCall SyncCall(L);
			return luaFIN_callReflectionFunctionDirectly(L, Function, Ctx, nArgs, nResults);
		} else {
			TArray<FINAny> Input = luaFIN_callReflectionFunctionProcessInput(L, Function, nArgs);
			luaFuture(L, FFINFutureReflection(Function, Ctx, Input));
			return 1;
		}
	}
	
	int luaReflectionFunctionCall(lua_State* L) {
		ZoneScoped;
		
		UFINFunction* Function = luaFIN_checkReflectionFunction(L, 1);
		UFINStruct* Type = Function->GetTypedOuter<UFINStruct>();
		lua_remove(L, 1);

		FFINExecutionContext Context;
		if (UFINClass* Class = Cast<UFINClass>(Type)) {
			if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) {
				FLuaClass* LuaClass = luaFIN_checkLuaClass(L, 1);
				if (!LuaClass->FINClass->IsChildOf(Class)) luaL_argerror(L, 1, "Expected Class");
				Context = FFINExecutionContext(LuaClass->UClass);
			} else {
				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, Class);
				Context = FFINExecutionContext(LuaObject->Object);
			}
		} else {
			FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, 1, Type);
			Context = FFINExecutionContext(LuaStruct->Struct->GetData()); // TODO: Add wrapper to Execution Context ot be able to hold a TSharedRef to the FINStruct, in Sync call, GC may collect struct!
		}

		return luaFIN_callReflectionFunction(L, Function, Context, lua_gettop(L)-1, LUA_MULTRET);
	}
	
	int luaReflectionFunctionUnpersist(lua_State* L) {
		FString TypeName = luaFIN_checkFString(L, lua_upvalueindex(1));
		FString FunctionName = luaFIN_checkFString(L, lua_upvalueindex(2));

		UFINStruct* Type;
		bool bClass = TypeName.RemoveFromEnd(TEXT("-Class"));
		if (bClass) {
			Type = FFINReflection::Get()->FindClass(TypeName);
		} else {
			Type = FFINReflection::Get()->FindStruct(TypeName);
		}
		UFINFunction* Function = Type->FindFINFunction(TypeName);
		
		luaFIN_pushReflectionFunction(L, Function);
		
		return 1;
	}

	int luaReflectionFunctionPersist(lua_State* L) {
		UFINFunction* Function = luaFIN_checkReflectionFunction(L, 1);
		UFINStruct* Type = Function->GetTypedOuter<UFINStruct>();

		FString TypeName = Type->GetInternalName();
		if (Type->IsA<UFINClass>()) TypeName.Append(TEXT("-Class"));
		FString FunctionName = Function->GetInternalName();
		
		luaFIN_pushFString(L, TypeName);
		luaFIN_pushFString(L, FunctionName);
		
		lua_pushcclosure(L, &luaReflectionFunctionUnpersist, 2);
		
		return 1;
	}

	static const luaL_Reg luaReflectionFunctionMetatable[] = {
		{"__call", luaReflectionFunctionCall},
		{"__persist", luaReflectionFunctionPersist},
		{nullptr, nullptr}
	};

	void luaFIN_pushReflectionFunction(lua_State* L, UFINFunction* Function) {
		if (!Function) {
			lua_pushnil(L);
			return;
		}
		
		*static_cast<UFINFunction**>(lua_newuserdata(L, sizeof(UFINFunction*))) = Function;
		luaL_setmetatable(L, LUA_REFLECTION_FUNCTION_METATABLE_NAME);
	}

	UFINFunction* luaFIN_checkReflectionFunction(lua_State* L, int Index) {
		return *static_cast<UFINFunction**>(luaL_checkudata(L, Index, LUA_REFLECTION_FUNCTION_METATABLE_NAME));
	}

	int luaFIN_tryIndexGetProperty(lua_State* L, int Index, UFINStruct* Type, const FString& MemberName, EFINRepPropertyFlags PropertyFilterFlags, const FFINExecutionContext& PropertyCtx) {
		ZoneScoped;
		UFINProperty* Property = Type->FindFINProperty(MemberName, PropertyFilterFlags);
		if (Property) {
			if (!PropertyCtx.IsValid()) {
				return luaFIN_argError(L, Index, FString::Printf(TEXT("Reference to %s is invalid."), *luaFIN_typeName(L, Index)));
			}
			
			EFINRepPropertyFlags PropFlags = Property->GetPropertyFlags();
			// TODO: Add C++ try catch block to GetProperty Execution
			if (PropFlags & FIN_Prop_RT_Async) {
				ZoneScopedN("Lua Get Property");
				luaFIN_pushNetworkValue(L, Property->GetValue(PropertyCtx));
			} else if (PropFlags & FIN_Prop_RT_Parallel) {
				ZoneScopedN("Lua Get Property SyncCall");
				[[maybe_unused]] FLuaSyncCall SyncCall(L);
				{
					ZoneScopedN("Lua Get Property");
					luaFIN_pushNetworkValue(L, Property->GetValue(PropertyCtx));
				}
			} else {
				luaFuture(L, FFINFutureReflection(Property, PropertyCtx));
			}
			return 1;
		}
		return 0;
	}

	int luaFIN_tryIndexFunction(lua_State* L, UFINStruct* Struct, const FString& MemberName, EFINFunctionFlags FunctionFilterFlags) {
		UFINFunction* Function = Struct->FindFINFunction(MemberName, FunctionFilterFlags);
		if (Function) {
			// TODO: Add caching
			luaFIN_pushReflectionFunction(L, Function);
			return 1;
		}
		return 0;
	}

	int luaFIN_pushFunctionOrGetProperty(lua_State* L, int Index, UFINStruct* Struct, const FString& MemberName,  EFINFunctionFlags FunctionFilterFlags, EFINRepPropertyFlags PropertyFilterFlags, const FFINExecutionContext& PropertyCtx, bool bCauseError) {
		ZoneScoped;
		int arg = luaFIN_tryIndexGetProperty(L, Index, Struct, MemberName, PropertyFilterFlags, PropertyCtx);
		if (arg == 0) arg = luaFIN_tryIndexFunction(L, Struct, MemberName, FunctionFilterFlags);
		if (arg > 0) return arg;

		if (bCauseError) luaFIN_warning(L, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found. Nil return is deprecated and this will become an error.")), true);
		lua_pushnil(L);
		return 1; // TODO: Remove return val and bCauseError param
	}

	bool luaFIN_tryExecuteSetProperty(lua_State* L, int Index, UFINStruct* Type, const FString& MemberName, EFINRepPropertyFlags PropertyFilterFlags, const FFINExecutionContext& PropertyCtx, int ValueIndex, bool bCauseError) {
		UFINProperty* Property = Type->FindFINProperty(MemberName, PropertyFilterFlags);
		if (Property) {
			if (!PropertyCtx.IsValid()) {
				luaFIN_argError(L, Index, FString::Printf(TEXT("Reference to %s is invalid."), *luaFIN_typeName(L, Index)));
				return true;
			}
			
			TOptional<FINAny> Value = luaFIN_toNetworkValueByProp(L, ValueIndex, Property, true, true);
			if (!Value.IsSet()) {
				luaFIN_propertyError(L, ValueIndex, Property);
				return true;
			}
			
			// TODO: Add C++ try catch block to SetProperty Execution
			EFINRepPropertyFlags PropFlags = Property->GetPropertyFlags();
			if (PropFlags & FIN_Prop_RT_Async) {
				Property->SetValue(PropertyCtx, Value.GetValue());
			} else if (PropFlags & FIN_Prop_RT_Parallel) {
				[[maybe_unused]] FLuaSyncCall SyncCall(L);
				Property->SetValue(PropertyCtx, Value.GetValue());
			} else {
				luaFuture(L, FFINFutureReflection(Property, PropertyCtx, Value.GetValue()));
			}
			return 1;
		}
		if (bCauseError) luaL_argerror(L, Index, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found")));
		return false;
	}

	void setupRefUtils(lua_State* L) {
		PersistSetup("ReflectionUtils", -2);

		// Register & Persist ReflectionFunction-Metatable
		luaL_newmetatable(L, LUA_REFLECTION_FUNCTION_METATABLE_NAME);		// ..., ReflectionFunctionMetatable
		luaL_setfuncs(L, luaReflectionFunctionMetatable, 0);
		lua_pushstring(L, LUA_REFLECTION_FUNCTION_METATABLE_NAME);			// ..., ReflectionFunctionMetatable, string
		lua_setfield(L, -2, "__metatable");								// ..., ReflectionFunctionMetatable
		PersistTable(LUA_REFLECTION_FUNCTION_METATABLE_NAME, -1);
		lua_pop(L, 1);														// ...
	}
}
