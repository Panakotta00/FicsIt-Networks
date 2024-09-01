#pragma once

#include "FINLua/LuaUtil.h"
#include "Reflection/FIRExecutionContext.h"
#include "Reflection/FIRFunction.h"
#include "Reflection/FIRProperty.h"

#define LUA_REF_CACHE "FINRefCache"

namespace FINLua {
	/**
	 * See \link luaFIN_callReflectionFunction \endlink for more details.
	 * Difference: Allowed Runtime States have been taken care of.
	 */
	int luaFIN_callReflectionFunctionDirectly(lua_State* L, const UFIRFunction* Function, const FFIRExecutionContext& Ctx, int nArgs, int nResults);

	/**
	 * Call the given FINFunction with the given Execution Context and follows the Lua Calling Convention.
	 * It uses the topmost lua values as input values and pushes the return values onto the lua stack.
	 * Sync only functions will get wrapped in a Future.
	 * Parallel functions will automatically demote the tick state.
	 * 
	 * @brief Calls the given FINFunction with the given Execution Context
	 * @param L the lua state
	 * @param Function the FINFunction that should get executed
	 * @param Ctx the execution context the function will get executed with
	 * @param nArgs the count of input parameters on top of the stack that get passed to the function
	 * @param nResults the max count of output parameters that can get pushed to the stack, LUA_MULTRET for any count
	 * @return the count of return values
	 */
	int luaFIN_callReflectionFunction(lua_State* L, UFIRFunction* Function, const FFIRExecutionContext& Ctx, int nArgs, int nResults);

	/**
	 * @brief pushes a Reflection Function value onto the lua stack
	 * @param L the lua stack
	 * @param Function the function that should get pushed
	 */
	void luaFIN_pushReflectionFunction(lua_State* L, UFIRFunction* Function);

	/**
	 * @brief returns a Reflection Function from the lua value at the given index in the lua stack. Causes a lua error if unable to get the function data.
	 * @param L the lua state
	 * @param Index the index of the lua value you want to get
	 * @return a pointer to the ReflectionFunctionData within the lua stack (Attention to GC!)
	 */
	UFIRFunction* luaFIN_checkReflectionFunction(lua_State* L, int Index);

	/**
	 * @brief Pushes the value returned by the Property with the given settings onto the lua stack, if no GetProperty was found, pushes nothing. If the context is invalid, causes Lua error
	 * @param L the lua state
	 * @param Index the index of the lua value used for the property context
	 * @param Type the Reflection Type in which to search for a property with the given name
	 * @param MemberName the name of the property that gets searched for
	 * @param PropertyFilterFlags property flags that will be used as filter when searching for the member
	 * @param PropertyCtx the call context that will be used if the property gets found
	 * @return the amount of return values, 0 if no property was found, guaranteed to be 1 if property found and valid
	 */
	int luaFIN_tryIndexGetProperty(lua_State* L, int Index, UFIRStruct* Type, const FString& MemberName, EFIRPropertyFlags PropertyFilterFlags,  const FFIRExecutionContext& PropertyCtx);

	/**
	 * @brief Pushes the return values of a function call with the given name onto the lua stack, if no function was found, pushes nothing
	 * @param L the lua state
	 * @param Type the Reflection Type in which to search for a function with the given name
	 * @param MemberName the name of the function that gets searched for
	 * @param FunctionFilterFlags function flags that will be used as filter when searching for the member
	 * @return 1 if a function was found and executed, otherwise 0
	 */
	int luaFIN_tryIndexFunction(lua_State* L, UFIRStruct* Type, const FString& MemberName, EFIRFunctionFlags FunctionFilterFlags);
	
	/**
	 * @brief Pushes the value of the get property, or a Reflection Function with the given member name onto the stack, If property executed failed, causes a lua error.
	 * @param L the lua state
	 * @param Index the argument index used for the optional lua arg error
	 * @param Type the Reflection Type that will be used to search the property or function
	 * @param MemberName the function or property name that will be searched
	 * @param FunctionFilterFlags function flags that will be used as filter when searching for a function
	 * @param PropertyFilterFlags property flags that will be used as filter when searching for a property
	 * @param PropertyCtx the execution context that will be used for the get property, if such property got found
	 * @param bCauseError if true, causes a lua error
	 * @return 0 if unable to find a property or function with the given name, otherwise 1.
	 */
	int luaFIN_pushFunctionOrGetProperty(lua_State* L, int Index, UFIRStruct* Type, const FString& MemberName, EFIRFunctionFlags FunctionFilterFlags, EFIRPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx, bool bCauseError = true);

	/**
	 * @brief Tries to execute the SetProperty with the value at the given index in the lua stack, if no SetProperty was found, pushes nothing. Causes a lua error if property execution failed.
	 * @param L the lua state
	 * @param Index the argument index used for the optional lua arg error
	 * @param Type the Reflection Type in which to search for a property with the given name
	 * @param MemberName the name of the property that gets searched for
	 * @param PropertyFilterFlags property flags that will be used as filter when searching for the member
	 * @param PropertyCtx the call context that will be used if the property gets found
	 * @param ValueIndex the index of the lua value that should get used for the set operation
	 * @param bCauseError if true, causes a lua error
	 * @return 1 if a property was found and executed, 0 if no property was found
	 */
	bool luaFIN_tryExecuteSetProperty(lua_State* L, int Index, UFIRStruct* Type, const FString& MemberName, EFIRPropertyFlags PropertyFilterFlags, const FFIRExecutionContext& PropertyCtx, int ValueIndex, bool bCauseError);
}
