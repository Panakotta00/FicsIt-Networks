#include "FINLua/Reflection/LuaRef.h"

#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLua/Reflection/LuaStruct.h"
#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "tracy/Tracy.hpp"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		FullReflectionModule
	 * @DisplayName		Reflection-System Base Module
	 * @Dependency		ReflectionSystemObjectModule
	 * @Dependency		ReflectionSystemClassModule
	 * @Dependency		ReflectionSystemStructModule
	 *
	 * This Module is used as dependency to depend on the entire Reflection System related stuff.
	 */)", FullReflectionModule) {}
	LuaModule(R"(/**
	 * @LuaModule		ReflectionSystemBaseModule
	 * @DisplayName		Reflection-System Base Module
	 *
	 * This Module provides common functionallity required by further reflection related modules.
	 */)", ReflectionSystemBase) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	ReflectionFunction
		 * @DisplayName		Reflection-Function
		 *
		 * Functions of the reflection system are not directly Lua Functions, instead they are this type.
		 * This has various reason, but one neat thing it allows us to do, is to provide documentation capabilities.
		 * Instead of just calling it, you can also ask for information about the function it self.
		 * And it makes debugging a bit easier.
		 */)", ReflectionFunction) {

			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	asFunctionObject	Function
			 * @DisplayName		As Function Object
			 *
			 * Returns the Reflection-System Object that represents this Reflected Function.
			 * This way you can deeply inspect the function and its associations.
			 */)", asFunctionObject) {
				lua_pushnil(L);
			}

			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	quickRef	string
			 * @DisplayName		Quick Reference
			 *
			 * A string containing the signature and description of the function for quick way to get info one the function within code.
			 */)", quickRef) {
				lua_pushnil(L);
			}

			int lua_callDeferred(lua_State* L) {
				UFIRFunction* function = luaFIN_checkReflectionFunction(L, lua_upvalueindex(1));

				return luaFIN_callReflectionFunction(L, function, false);
			}

			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	callDeferred	function
			 * @DisplayName		Call Deferred
			 *
			 * Calls the function deferred in the next tick. Returns a Future to allow check for execution and get the return parameters.
			 */)", callDeferred) {
				lua_pushnil(L);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				UFIRFunction* function = luaFIN_checkReflectionFunction(L, 1);

				FString key = luaFIN_checkFString(L, 2);
				if (key == asFunctionObject_Name) {
					luaFIN_pushObject(L, FFIRTrace(function));
				} else if (key == quickRef_Name) {
					FString signature = luaFIN_getFunctionSignature(L, function);
					FString description = function->GetDescription().ToString();

					FString str = FString::Printf(TEXT("# %ls\n%ls"), *signature, *description);

					luaFIN_pushFString(L, str);
				} else if (key == callDeferred_Name) {
					luaFIN_pushReflectionFunction(L, function);
					lua_pushcclosure(L, &lua_callDeferred, 1);
					return 1;
				} else {
					return 0;
				}

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__call
			 * @DisplayName		Call
			 */)", __call) {
				ZoneScoped;

				UFIRFunction* Function = luaFIN_checkReflectionFunction(L, 1);
				lua_remove(L, 1);

				return luaFIN_callReflectionFunction(L, Function, false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
				UFIRFunction* function = luaFIN_checkReflectionFunction(L, 1);
				luaFIN_pushFString(L, FString::Printf(TEXT("function: %s"), *luaFIN_getFunctionSignature(L, function)));
				return 1;
			}

			int luaReflectionFunctionUnpersist(lua_State* L) {
				FString TypeName = luaFIN_checkFString(L, lua_upvalueindex(1));
				FString FunctionName = luaFIN_checkFString(L, lua_upvalueindex(2));

				UFIRStruct* Type;
				if (TypeName.RemoveFromEnd(TEXT("-Class"))) {
					Type = FFicsItReflectionModule::Get().FindClass(TypeName);
				} else {
					Type = FFicsItReflectionModule::Get().FindStruct(TypeName);
				}
				UFIRFunction* Function = Type->FindFIRFunction(TypeName);

				luaFIN_pushReflectionFunction(L, Function);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				UFIRFunction* Function = luaFIN_checkReflectionFunction(L, 1);
				UFIRStruct* Type = Function->GetTypedOuter<UFIRStruct>();

				FString TypeName = Type->GetInternalName();
				if (Type->IsA<UFIRClass>()) TypeName.Append(TEXT("-Class"));
				FString FunctionName = Function->GetInternalName();

				luaFIN_pushFString(L, TypeName);
				luaFIN_pushFString(L, FunctionName);

				lua_pushcclosure(L, &luaReflectionFunctionUnpersist, 2);

				return 1;
			}
		}

		LuaModulePostSetup() {
			PersistenceNamespace("ReflectionBase");

			lua_pushcfunction(L, ReflectionFunction::luaReflectionFunctionUnpersist);
			PersistValue("ReflectionFunctionUnpersist");
		}
	}

	TArray<FIRAny> luaFIN_callReflectionFunctionProcessInput(lua_State* L, const UFIRFunction* Function, int nArgs) {
		// ReSharper disable once CppTooWideScope
		constexpr int startArg = 2;
		TArray<FIRAny> Input;
		TArray<UFIRProperty*> Parameters = Function->GetParameters();
		for (UFIRProperty* Parameter : Parameters) {
			if (Parameter->GetPropertyFlags() & FIR_Prop_Param && !(Parameter->GetPropertyFlags() & (FIR_Prop_OutParam | FIR_Prop_RetVal))) {
				int index = lua_absindex(L, -nArgs);
				TOptional<FIRAny> Value = luaFIN_toNetworkValueByProp(L, index, Parameter, true, true);
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
			TOptional<FIRAny> Value = luaFIN_toNetworkValue(L, lua_absindex(L, -nArgs));
			if (Value.IsSet()) Input.Add(*Value);
			else Input.Add(FIRAny());
		}
		return Input;
	}

	int luaFIN_callReflectionFunctionProcessOutput(lua_State* L, const TArray<FIRAny>& Output, const FFIRTrace& PrefixTrace, int nResults) {
		int pushed = 0;
		for (const FIRAny& Value : Output) {
			if (pushed >= nResults && nResults != LUA_MULTRET) break;
			luaFIN_pushNetworkValue(L, Value, PrefixTrace);
			++pushed;
		}
		return pushed;
	}

	int luaFIN_callReflectionFunctionDirectly(lua_State* L, const UFIRFunction* Function, const FFIRExecutionContext& Ctx, int nArgs, int nResults) {
		if (!Ctx.IsValid()) {
			return luaFIN_argError(L, 1, FString::Printf(TEXT("Reference to %s is invalid."), *luaFIN_typeName(L, 1)));
		}

		TArray<FIRAny> Parameters = luaFIN_callReflectionFunctionProcessInput(L, Function, nArgs);

		TArray<FIRAny> Output;
		try {
			Output = Function->Execute(Ctx, Parameters);
		} catch (const FFIRFunctionBadArgumentException& Ex) {
			return luaFIN_argError(L, Ex.ArgumentIndex+2, Ex.GetMessage()); // TODO: Change Argument Index Offset for C++ ArgumentException
		} catch (const FFIRReflectionException& Ex) {
			return luaL_error(L, TCHAR_TO_UTF8(*Ex.GetMessage()));
		}

		return luaFIN_callReflectionFunctionProcessOutput(L, Output, Ctx.GetTrace(), nResults);
	}

UE_DISABLE_OPTIMIZATION_SHIP
	int luaFIN_callReflectionFunction(lua_State* L, UFIRFunction* Function, const FFIRExecutionContext& Ctx, int nArgs, int nResults, bool bForceSync) {
		FFINLuaLogScope LogScope(L);
		const EFIRFunctionFlags FuncFlags = Function->GetFunctionFlags();
		if (FuncFlags & FIR_Func_RT_Async && !bForceSync) {
			return luaFIN_callReflectionFunctionDirectly(L, Function, Ctx, nArgs, nResults);
		} else if (FuncFlags & FIR_Func_RT_Parallel && !bForceSync) {
			[[maybe_unused]] FLuaSync SyncCall(L);
			return luaFIN_callReflectionFunctionDirectly(L, Function, Ctx, nArgs, nResults);
		} else {
			TArray<FIRAny> Input = luaFIN_callReflectionFunctionProcessInput(L, Function, nArgs);
			luaFIN_pushFuture(L, FFINFutureReflection(Function, Ctx, Input));
			if (!bForceSync) {
				return luaFIN_await(L, -1);
			}
			return 1;
		}
	}
UE_ENABLE_OPTIMIZATION_SHIP

	int luaFIN_callReflectionFunction(lua_State* L, UFIRFunction* Function, bool bForceSync) {
		UFIRStruct* Type = Function->GetTypedOuter<UFIRStruct>();

		FFIRExecutionContext Context;
		if (Function->GetFunctionFlags() & FIR_Func_StaticFunc) {
			Context = FFIRExecutionContext(Function->GetOuter());
		} else if (UFIRClass* Class = Cast<UFIRClass>(Type)) {
			if (Function->GetFunctionFlags() & FIR_Func_ClassFunc) {
				FLuaClass* LuaClass = luaFIN_checkLuaClass(L, 1);
				if (!LuaClass->FIRClass->IsChildOf(Class)) luaL_argerror(L, 1, "Expected Class");
				Context = FFIRExecutionContext(LuaClass->UClass);
			} else {
				FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, Class);
				Context = FFIRExecutionContext(LuaObject->Object);
			}
		} else {
			FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, 1, Type);
			Context = FFIRExecutionContext(LuaStruct->Struct->GetData()); // TODO: Add wrapper to Execution Context ot be able to hold a TSharedRef to the FINStruct, in Sync call, GC may collect struct!
		}

		return luaFIN_callReflectionFunction(L, Function, Context, lua_gettop(L)-1, LUA_MULTRET, bForceSync);
	}

	void luaFIN_pushReflectionFunction(lua_State* L, UFIRFunction* Function) {
		if (!Function) {
			lua_pushnil(L);
			return;
		}

		*static_cast<UFIRFunction**>(lua_newuserdata(L, sizeof(UFIRFunction*))) = Function;
		luaL_setmetatable(L, ReflectionSystemBase::ReflectionFunction::_Name);
	}

	UFIRFunction* luaFIN_checkReflectionFunction(lua_State* L, int Index) {
		return *static_cast<UFIRFunction**>(luaL_checkudata(L, Index, ReflectionSystemBase::ReflectionFunction::_Name));
	}

	int luaFIN_getProperty(lua_State* L, UFIRProperty* Property, const FFIRExecutionContext& PropertyCtx, lua_KContext kCtx, lua_KFunction kFunc) {
		EFIRPropertyFlags PropFlags = Property->GetPropertyFlags();
		// TODO: Add C++ try catch block to GetProperty Execution
		if (PropFlags & FIR_Prop_RT_Async) {
			ZoneScopedN("Lua Get Property");
			luaFIN_pushNetworkValue(L, Property->GetValue(PropertyCtx));
		} else if (PropFlags & FIR_Prop_RT_Parallel) {
			ZoneScopedN("Lua Get Property SyncCall");
			[[maybe_unused]] FLuaSync SyncCall(L);
			{
				ZoneScopedN("Lua Get Property");
				luaFIN_pushNetworkValue(L, Property->GetValue(PropertyCtx));
			}
		} else {
			luaFIN_pushFuture(L, FFINFutureReflection(Property, PropertyCtx));
			if (kFunc) {
				luaFIN_await(L, -1, kCtx, kFunc);
			} else {
				luaFIN_await(L, -1);
			}
		}
		return 1;
	}

	int luaFIN_tryIndexGetProperty(lua_State* L, int Index, UFIRStruct* Type, const FString& MemberName, EFIRPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx) {
		ZoneScoped;
		if (!IsValid(Type)) return 0;
		if (UFIRProperty* Property = Type->FindFIRProperty(MemberName, PropertyFilterFlags)) {
			if (!PropertyCtx.IsValid()) {
				return luaFIN_argError(L, Index, FString::Printf(TEXT("Reference to %s is invalid."), *luaFIN_typeName(L, Index)));
			}

			return luaFIN_getProperty(L, Property, PropertyCtx);
		}
		return 0;
	}

	int luaFIN_tryIndexFunction(lua_State* L, UFIRStruct* Struct, const FString& MemberName, EFIRFunctionFlags FunctionFilterFlags) {
		if (!IsValid(Struct)) return 0;
		if (UFIRFunction* Function = Struct->FindFIRFunction(MemberName, FunctionFilterFlags)) {
			// TODO: Add caching
			luaFIN_pushReflectionFunction(L, Function);
			return 1;
		}
		return 0;
	}

	int luaFIN_pushFunctionOrGetProperty(lua_State* L, int Index, UFIRStruct* Struct, const FString& MemberName,  EFIRFunctionFlags FunctionFilterFlags, EFIRPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx, bool bCauseError) {
		ZoneScoped;
		if (!IsValid(Struct)) return 0;
		int arg = luaFIN_tryIndexGetProperty(L, Index, Struct, MemberName, PropertyFilterFlags, PropertyCtx);
		if (arg == 0) arg = luaFIN_tryIndexFunction(L, Struct, MemberName, FunctionFilterFlags);
		if (arg > 0) return arg;

		if (bCauseError) luaFIN_warning(L, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found. Nil return is deprecated and this will become an error.")), true);
		lua_pushnil(L);
		return 1; // TODO: Remove return val and bCauseError param
	}

	bool luaFIN_tryExecuteSetProperty(lua_State* L, int Index, UFIRStruct* Type, const FString& MemberName, EFIRPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx, int ValueIndex, bool bCauseError) {
		if (!IsValid(Type)) return false;
		if (UFIRProperty* Property = Type->FindFIRProperty(MemberName, PropertyFilterFlags)) {
			if (!PropertyCtx.IsValid()) {
				luaFIN_argError(L, Index, FString::Printf(TEXT("Reference to %s is invalid."), *luaFIN_typeName(L, Index)));
				return true;
			}

			TOptional<FIRAny> Value = luaFIN_toNetworkValueByProp(L, ValueIndex, Property, true, true);
			if (!Value.IsSet()) {
				luaFIN_propertyError(L, ValueIndex, Property);
				return true;
			}

			// TODO: Add C++ try catch block to SetProperty Execution
			EFIRPropertyFlags PropFlags = Property->GetPropertyFlags();
			if (PropFlags & FIR_Prop_RT_Async) {
				Property->SetValue(PropertyCtx, Value.GetValue());
			} else if (PropFlags & FIR_Prop_RT_Parallel) {
				[[maybe_unused]] FLuaSync SyncCall(L);
				Property->SetValue(PropertyCtx, Value.GetValue());
			} else {
				luaFIN_pushFuture(L, FFINFutureReflection(Property, PropertyCtx, Value.GetValue()));
				luaFIN_await(L, -1);
			}
			return true;
		}
		if (bCauseError) luaL_argerror(L, Index, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found")));
		return false;
	}
}
