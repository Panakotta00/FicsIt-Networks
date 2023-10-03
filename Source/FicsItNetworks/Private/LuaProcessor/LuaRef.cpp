#include "LuaProcessor/LuaRef.h"

#include "LuaProcessor/LuaClass.h"
#include "LuaProcessor/LuaFuture.h"
#include "LuaProcessor/LuaProcessor.h"
#include "LuaProcessor/LuaUtil.h"
#include "Reflection/FINReflection.h"

namespace FINLua {
	TArray<FINAny> luaFIN_callReflectionFunctionProcessInput(lua_State* L, UFINFunction* Function, int nArgs) {
		TArray<FINAny> Input;
		TArray<UFINProperty*> Parameters = Function->GetParameters();
		for (UFINProperty* Parameter : Parameters) {
			if (Parameter->GetPropertyFlags() & FIN_Prop_Param && !(Parameter->GetPropertyFlags() & (FIN_Prop_OutParam | FIN_Prop_RetVal))) {
				int index = lua_absindex(L, -nArgs);
				TOptional<FINAny> Value = luaFIN_toNetworkValueByProp(L, index, Parameter, true, true);
				if (Value.IsSet()) Input.Add(*Value);
				else luaFIN_propertyError(L, index, Parameter);
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
		TArray<FINAny> Parameters = luaFIN_callReflectionFunctionProcessInput(L, Function, nArgs);
		
		TArray<FINAny> Output;
		try {
			Output = Function->Execute(Ctx, Parameters);
		} catch (const FFINFunctionBadArgumentException& Ex) {
			return luaL_argerror(L, Ex.ArgumentIndex+2, TCHAR_TO_UTF8(*Ex.GetMessage())); // TODO: Change Argument Index Offset for C++ ArgumentException
		} catch (const FFINReflectionException& Ex) {
			return luaL_error(L, TCHAR_TO_UTF8(*Ex.GetMessage()));
		}

		return luaFIN_callReflectionFunctionProcessOutput(L, Output, Ctx.GetTrace(), nResults);
	}
	
	int luaFIN_callReflectionFunction(lua_State* L, UFINFunction* Function, const FFINExecutionContext& Ctx, int nArgs, int nResults) {
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
		UFINFunction* Function = luaFIN_checkReflectionFunction(L, 1);
		UFINStruct* Type = Function->GetTypedOuter<UFINStruct>();
		lua_remove(L, 1);

		FFINExecutionContext Context;
		if (UFINClass* Class = Cast<UFINClass>(Type)) {
			if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) {
				FLuaClass* LuaClass = luaFIN_checkLuaClass(L, 1, Cast<UFINClass>(Class));
				Context = FFINExecutionContext(LuaClass->UClass);
			} else {
				// TODO: Refactor Objects & Set Object Execution Context
				luaL_error(L, "Not implemented for Objects");
			}
		} else {
			// TODO: Refactor Structs & Set Struct Execution Context
			luaL_error(L, "Not implemented for Structs");
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

	int luaCallFINFunc(lua_State* L, UFINFunction* Func, const FFINExecutionContext& Ctx, const std::string& typeName) {
		// get input parameters from lua stack
		TArray<FINAny> Input;
		int args = lua_gettop(L);
		int paramsLoaded = 2;
		for (UFINProperty* Param : Func->GetParameters()) {
			if (Param->GetPropertyFlags() & FIN_Prop_Param && !(Param->GetPropertyFlags() & (FIN_Prop_OutParam | FIN_Prop_RetVal))) {
				FINAny NewParam = luaToProperty(L, Param, paramsLoaded++);
				Input.Add(MoveTemp(NewParam));
			}
		}
		for (; paramsLoaded <= args; ++paramsLoaded) {
			FINAny Param;
			luaToNetworkValue(L, paramsLoaded, Param);
			Input.Add(MoveTemp(Param));
		}
		args = 0;

		// sync tick if necessary
		const EFINFunctionFlags FuncFlags = Func->GetFunctionFlags();
		// ReSharper disable once CppEntityAssignedButNoRead
		// ReSharper disable once CppJoinDeclarationAndAssignment
		TSharedPtr<FLuaSyncCall> SyncCall;
		bool bRunDirectly = false;
		if (FuncFlags & FIN_Func_RT_Async) {
			bRunDirectly = true;
		} else if (FuncFlags & FIN_Func_RT_Parallel) {
			SyncCall = MakeShared<FLuaSyncCall>(L);
			bRunDirectly = true;
		} else {
			luaFuture(L, FFINFutureReflection(Func, Ctx, Input));
			args = 1;
		}

		if (bRunDirectly) {
			TArray<FINAny> Output;
			try {
				Output = Func->Execute(Ctx, Input);
			} catch (const FFINFunctionBadArgumentException& Ex) {
				return luaL_argerror(L, Ex.ArgumentIndex+2, TCHAR_TO_UTF8(*Ex.GetMessage()));
			} catch (const FFINReflectionException& Ex) {
				return luaL_error(L, TCHAR_TO_UTF8(*Ex.GetMessage()));
			}

			// push output onto lua stack
			for (const FINAny& Value : Output) {
				luaFIN_pushNetworkValue(L, Value, Ctx.GetTrace());
				++args;
			}
		}
		
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}

	int luaFindGetMember(lua_State* L, UFINStruct* Struct, const FFINExecutionContext& Ctx, const FString& MemberName, const FString& MetaName, int(*callFunc)(lua_State*), bool classInstance) {
		// get cache function
		luaL_getmetafield(L, 1, LUA_REF_CACHE);																// Instance, FuncName, InstanceCache
		
		if (lua_getfield(L, -1, TCHAR_TO_UTF8(*MetaName)) != LUA_TNIL)  {										// Instance, FuncName, InstanceCache, CachedFunc
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}																											// Instance, FuncName, InstanceCache, nil

		// try to find property
		UFINProperty* Property = Struct->FindFINProperty(MemberName, classInstance ? FIN_Prop_ClassProp : FIN_Prop_Attrib);
		if (Property) {
			// ReSharper disable once CppEntityAssignedButNoRead
			// ReSharper disable once CppJoinDeclarationAndAssignment
			TSharedPtr<FLuaSyncCall> SyncCall;
			if (!(Property->GetPropertyFlags() & FIN_Prop_RT_Async)) SyncCall = MakeShared<FLuaSyncCall>(L);
			
			luaFIN_pushNetworkValue(L, Property->GetValue(Ctx), Ctx.GetTrace());
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}

		// try to find function
		UFINFunction* Function = Struct->FindFINFunction(MemberName, classInstance ? FIN_Func_ClassFunc : FIN_Func_MemberFunc);
		if (Function) {
			luaFIN_pushReflectionFunction(L, Function);
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}
		
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	int luaFindSetMember(lua_State* L, UFINStruct* Struct, const FFINExecutionContext& Ctx, const FString& MemberName, bool classInstance, bool bCauseError) {
		// try to find property
		UFINProperty* Property = Struct->FindFINProperty(MemberName, classInstance ? FIN_Prop_ClassProp : FIN_Prop_Attrib);
		if (Property) {
			// ReSharper disable once CppEntityAssignedButNoRead
			// ReSharper disable once CppJoinDeclarationAndAssignment
			FINAny Value = luaToProperty(L, Property, 3);
			
			TSharedPtr<FLuaSyncCall> SyncCall;
			EFINRepPropertyFlags PropFlags = Property->GetPropertyFlags();
			bool bRunDirectly = false;
			if (PropFlags & FIN_Prop_RT_Async) {
				bRunDirectly = true;
			} else if (PropFlags & FIN_Prop_RT_Parallel) {
				SyncCall = MakeShared<FLuaSyncCall>(L);
				bRunDirectly = true;
			} else {
				luaFuture(L, FFINFutureReflection(Property, Ctx, Value));
			}
			
			if (bRunDirectly) Property->SetValue(Ctx, Value);
			
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}
		
		if (bCauseError) return luaL_argerror(L, 2, TCHAR_TO_UTF8(*("No property with name '" + MemberName + "' found")));
		return 0;
	}

	bool luaFIN_tryExecuteGetProperty(lua_State* L, UFINStruct* Type, const FString& MemberName, EFINRepPropertyFlags PropertyFilterFlags, const FFINExecutionContext& PropertyCtx) {
		UFINProperty* Property = Type->FindFINProperty(MemberName, PropertyFilterFlags);
		if (Property) {
			EFINRepPropertyFlags PropFlags = Property->GetPropertyFlags();
			// TODO: Add C++ try catch block to GetProperty Execution
			if (PropFlags & FIN_Prop_RT_Async) {
				luaFIN_pushNetworkValue(L, Property->GetValue(PropertyCtx));
			} else if (PropFlags & FIN_Prop_RT_Parallel) {
				[[maybe_unused]] FLuaSyncCall SyncCall(L);
				luaFIN_pushNetworkValue(L, Property->GetValue(PropertyCtx));
			} else {
				luaFuture(L, FFINFutureReflection(Property, PropertyCtx));
			}
			return true;
		}
		return false;
	}

	bool luaFIN_tryExecuteFunction(lua_State* L, UFINStruct* Struct, const FString& MemberName, EFINFunctionFlags FunctionFilterFlags) {
		UFINFunction* Function = Struct->FindFINFunction(MemberName, FunctionFilterFlags);
		if (Function) {
			// TODO: Add caching
			luaFIN_pushReflectionFunction(L, Function);
			return true;
		}
		return false;
	}

	bool luaFIN_pushFunctionOrGetProperty(lua_State* L, int Index, UFINStruct* Struct, const FString& MemberName,  EFINFunctionFlags FunctionFilterFlags, EFINRepPropertyFlags PropertyFilterFlags, const FFINExecutionContext& PropertyCtx, bool bCauseError) {
		if (luaFIN_tryExecuteGetProperty(L, Struct, MemberName, PropertyFilterFlags, PropertyCtx)) return true;
		if (luaFIN_tryExecuteFunction(L, Struct, MemberName, FunctionFilterFlags)) return true;

		if (bCauseError) luaL_argerror(L, Index, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found")));
		return false;
	}

	bool luaFIN_tryExecuteSetProperty(lua_State* L, int Index, UFINStruct* Type, const FString& MemberName, EFINRepPropertyFlags PropertyFilterFlags, const FFINExecutionContext& PropertyCtx, int ValueIndex, bool bCauseError) {
		UFINProperty* Property = Type->FindFINProperty(MemberName, PropertyFilterFlags);
		if (Property) {
			FINAny Value = luaToProperty(L, Property, ValueIndex);
			
			// TODO: Add C++ try catch block to SetProperty Execution
			EFINRepPropertyFlags PropFlags = Property->GetPropertyFlags();
			if (PropFlags & FIN_Prop_RT_Async) {
				Property->SetValue(PropertyCtx, Value);
			} else if (PropFlags & FIN_Prop_RT_Parallel) {
				[[maybe_unused]] FLuaSyncCall SyncCall(L);
				Property->SetValue(PropertyCtx, Value);
			} else {
				luaFuture(L, FFINFutureReflection(Property, PropertyCtx));
			}
			return true;
		}
		if (bCauseError) luaL_argerror(L, Index, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found")));
		return false;
	}

	void setupRefUtils(lua_State* L) {
		PersistSetup("ReflectionUtils", -2);

		lua_pushcfunction(L, luaReflectionFunctionUnpersist);					// ..., LuaReflectionFunctionUnpersist
		PersistValue("ReflectionFunctionUnpersist");						// ...

		// Register & Persist Class-Metatable
		luaL_newmetatable(L, LUA_REFLECTION_FUNCTION_METATABLE_NAME);		// ..., ReflectionFunctionMetatable
		luaL_setfuncs(L, luaReflectionFunctionMetatable, 0);
		lua_pushstring(L, LUA_REFLECTION_FUNCTION_METATABLE_NAME);			// ..., ReflectionFunctionMetatable, string
		lua_setfield(L, -2, "__metatable");								// ..., ReflectionFunctionMetatable
		PersistTable(LUA_REFLECTION_FUNCTION_METATABLE_NAME, -1);
		lua_pop(L, 1);														// ...
	}
}
