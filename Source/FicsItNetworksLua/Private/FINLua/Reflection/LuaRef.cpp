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
			 * A stirng containing the signature and description of the function for quick way to get info one the function within code.
			 */)", quickRef) {
				lua_pushnil(L);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				UFINFunction* function = luaFIN_checkReflectionFunction(L, 1);

				FString key = luaFIN_checkFString(L, 2);
				if (key == asFunctionObject_Name) {
					luaFIN_pushObject(L, FFIRTrace(function));
				} else if (key == quickRef_Name) {
					FString signature = luaFIN_getFunctionSignature(L, function);
					FString description = function->GetDescription().ToString();

					FString str = FString::Printf(TEXT("# %ls\n%ls"), *signature, *description);

					luaFIN_pushFString(L, str);
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

				UFINFunction* Function = luaFIN_checkReflectionFunction(L, 1);
				UFIRStruct* Type = Function->GetTypedOuter<UFIRStruct>();
				lua_remove(L, 1);

				FFIRExecutionContext Context;
				if (Function->GetFunctionFlags() & FIN_Func_StaticFunc) {
					Context = FFIRExecutionContext(Function->GetOuter());
				} else if (UFINClass* Class = Cast<UFINClass>(Type)) {
					if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) {
						FLuaClass* LuaClass = luaFIN_checkLuaClass(L, 1);
						if (!LuaClass->FINClass->IsChildOf(Class)) luaL_argerror(L, 1, "Expected Class");
						Context = FFIRExecutionContext(LuaClass->UClass);
					} else {
						FLuaObject* LuaObject = luaFIN_checkLuaObject(L, 1, Class);
						Context = FFIRExecutionContext(LuaObject->Object);
					}
				} else {
					FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, 1, Type);
					Context = FFIRExecutionContext(LuaStruct->Struct->GetData()); // TODO: Add wrapper to Execution Context ot be able to hold a TSharedRef to the FINStruct, in Sync call, GC may collect struct!
				}

				return luaFIN_callReflectionFunction(L, Function, Context, lua_gettop(L)-1, LUA_MULTRET);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
				UFINFunction* function = luaFIN_checkReflectionFunction(L, 1);
				luaFIN_pushFString(L, FString::Printf(TEXT("function: %s"), *luaFIN_getFunctionSignature(L, function)));
				return 1;
			}

			int luaReflectionFunctionUnpersist(lua_State* L) {
				FString TypeName = luaFIN_checkFString(L, lua_upvalueindex(1));
				FString FunctionName = luaFIN_checkFString(L, lua_upvalueindex(2));

				UFIRStruct* Type;
				if (TypeName.RemoveFromEnd(TEXT("-Class"))) {
					Type = FFINReflection::Get()->FindClass(TypeName);
				} else {
					Type = FFINReflection::Get()->FindStruct(TypeName);
				}
				UFINFunction* Function = Type->FindFINFunction(TypeName);

				luaFIN_pushReflectionFunction(L, Function);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				UFINFunction* Function = luaFIN_checkReflectionFunction(L, 1);
				UFIRStruct* Type = Function->GetTypedOuter<UFIRStruct>();

				FString TypeName = Type->GetInternalName();
				if (Type->IsA<UFINClass>()) TypeName.Append(TEXT("-Class"));
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

	TArray<FINAny> luaFIN_callReflectionFunctionProcessInput(lua_State* L, const UFINFunction* Function, int nArgs) {
		// ReSharper disable once CppTooWideScope
		constexpr int startArg = 2;
		TArray<FINAny> Input;
		TArray<UFIRProperty*> Parameters = Function->GetParameters();
		for (UFIRProperty* Parameter : Parameters) {
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

	int luaFIN_callReflectionFunctionProcessOutput(lua_State* L, const TArray<FINAny>& Output, const FFIRTrace& PrefixTrace, int nResults) {
		int pushed = 0;
		for (const FINAny& Value : Output) {
			if (pushed >= nResults && nResults != LUA_MULTRET) break;
			luaFIN_pushNetworkValue(L, Value, PrefixTrace);
			++pushed;
		}
		return pushed;
	}

	int luaFIN_callReflectionFunctionDirectly(lua_State* L, const UFINFunction* Function, const FFIRExecutionContext& Ctx, int nArgs, int nResults) {
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

	int luaFIN_callReflectionFunction(lua_State* L, UFINFunction* Function, const FFIRExecutionContext& Ctx, int nArgs, int nResults) {
		FFINLuaLogScope LogScope(L);
		const EFINFunctionFlags FuncFlags = Function->GetFunctionFlags();
		if (FuncFlags & FIN_Func_RT_Async) {
			return luaFIN_callReflectionFunctionDirectly(L, Function, Ctx, nArgs, nResults);
		} else if (FuncFlags & FIN_Func_RT_Parallel) {
			[[maybe_unused]] FLuaSyncCall SyncCall(L);
			return luaFIN_callReflectionFunctionDirectly(L, Function, Ctx, nArgs, nResults);
		} else {
			TArray<FINAny> Input = luaFIN_callReflectionFunctionProcessInput(L, Function, nArgs);
			luaFIN_pushFuture(L, FFINFutureReflection(Function, Ctx, Input));
			return 1;
		}
	}

	void luaFIN_pushReflectionFunction(lua_State* L, UFINFunction* Function) {
		if (!Function) {
			lua_pushnil(L);
			return;
		}

		*static_cast<UFINFunction**>(lua_newuserdata(L, sizeof(UFINFunction*))) = Function;
		luaL_setmetatable(L, ReflectionSystemBase::ReflectionFunction::_Name);
	}

	UFINFunction* luaFIN_checkReflectionFunction(lua_State* L, int Index) {
		return *static_cast<UFINFunction**>(luaL_checkudata(L, Index, ReflectionSystemBase::ReflectionFunction::_Name));
	}

	int luaFIN_tryIndexGetProperty(lua_State* L, int Index, UFIRStruct* Type, const FString& MemberName, EFINRepPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx) {
		ZoneScoped;
		if (UFIRProperty* Property = Type->FindFINProperty(MemberName, PropertyFilterFlags)) {
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
				luaFIN_pushFuture(L, FFINFutureReflection(Property, PropertyCtx));
			}
			return 1;
		}
		return 0;
	}

	int luaFIN_tryIndexFunction(lua_State* L, UFIRStruct* Struct, const FString& MemberName, EFINFunctionFlags FunctionFilterFlags) {
		if (UFINFunction* Function = Struct->FindFINFunction(MemberName, FunctionFilterFlags)) {
			// TODO: Add caching
			luaFIN_pushReflectionFunction(L, Function);
			return 1;
		}
		return 0;
	}

	int luaFIN_pushFunctionOrGetProperty(lua_State* L, int Index, UFIRStruct* Struct, const FString& MemberName,  EFINFunctionFlags FunctionFilterFlags, EFINRepPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx, bool bCauseError) {
		ZoneScoped;
		int arg = luaFIN_tryIndexGetProperty(L, Index, Struct, MemberName, PropertyFilterFlags, PropertyCtx);
		if (arg == 0) arg = luaFIN_tryIndexFunction(L, Struct, MemberName, FunctionFilterFlags);
		if (arg > 0) return arg;

		if (bCauseError) luaFIN_warning(L, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found. Nil return is deprecated and this will become an error.")), true);
		lua_pushnil(L);
		return 1; // TODO: Remove return val and bCauseError param
	}

	bool luaFIN_tryExecuteSetProperty(lua_State* L, int Index, UFIRStruct* Type, const FString& MemberName, EFINRepPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx, int ValueIndex, bool bCauseError) {
		if (UFIRProperty* Property = Type->FindFINProperty(MemberName, PropertyFilterFlags)) {
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
				luaFIN_pushFuture(L, FFINFutureReflection(Property, PropertyCtx, Value.GetValue()));
			}
			return true;
		}
		if (bCauseError) luaL_argerror(L, Index, TCHAR_TO_UTF8(*("No property or function with name '" + MemberName + "' found")));
		return false;
	}
}
