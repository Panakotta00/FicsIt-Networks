#include "FGBlueprintFunctionLibrary.h"
#include "FicsItLogLibrary.h"
#include "FicsItNetworksLuaModule.h"
#include "FILLogContainer.h"
#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "Logging/StructuredLog.h"
#include "Registry/ModContentRegistry.h"
//#include "tracy/Tracy.hpp"
//#include "tracy/TracyLua.hpp"

namespace FINLua {
	int luaReYield(lua_State* L) {
		lua_yield(L,0);
		return 0;
	}

	int luaYieldResume(lua_State* L, int status, lua_KContext ctx) {
		// don't pass pushed bool for user executed yield identification
		return lua_gettop(L)-1;
	}

	int luaYield(lua_State* L) {
		const int args = lua_gettop(L);
		return luaFIN_yield(L, args, NULL, &luaYieldResume);
	}

	int luaResume(lua_State* L);
	int luaResumeResume(lua_State* L, int status, lua_KContext ctx) {
		return luaResume(L);
	}
	int luaResume(lua_State* L) {
		const int args = lua_gettop(L);
		luaL_checktype(L, 1, LUA_TTHREAD);
		lua_State* thread = lua_tothread(L, 1);

		if (!lua_checkstack(thread, args - 1)) {
			lua_pushboolean(L, false);
			lua_pushliteral(L, "too many arguments to resume");
			return 2;
		}

		// attach hook for out-of-time exception if thread got loaded from save and hook is not applied
		//lua_sethook(thread, UFINLuaProcessor::luaHook, LUA_MASKCOUNT, UFINLuaProcessor::luaGetProcessor(L)->GetTickHelper().steps());
		// TODO: Check if this hook is necessary

		// copy passed arguments to coroutine so it can return these arguments from the yield function
		// but don't move the passed coroutine and then resume the coroutine
		lua_xmove(L, thread, args - 1);
		int argCount = 0;
		const int status = lua_resume(thread, L, args - 1, &argCount);

		if (status == LUA_OK || status == LUA_YIELD) {
			if (argCount == 0 && status == LUA_YIELD) {
				// A hook yielded the thread
				return lua_yieldk(L, 0, NULL, &luaResumeResume);
			}
			
			if (status == LUA_YIELD) argCount = argCount-1;

			if (!lua_checkstack(L, argCount)) {
				lua_pop(thread, argCount);
				lua_pushliteral(L, "too many results to resume");
				return lua_error(L);
			}
			lua_pushboolean(L, true);
			lua_xmove(thread, L, argCount);
			return argCount + 1;
		} else {
			lua_pushboolean(L, false);
			lua_xmove(thread, L, 1);
			return 2;
		}
	}

		
	int luaRunning(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		int ismain = lua_pushthread(L);
		lua_State* thread = lua_tothread(L, -1);
		if (thread == runtime.GetLuaThread()) ismain = 1;
		lua_pushboolean(L, ismain);
		return 2;
	}

	int luaXpcallMsg(lua_State* L) {
		lua_newtable(L);
		lua_insert(L, -2);
		lua_setfield(L, -2, "message");
		luaL_traceback(L, L, NULL, 0);
		lua_setfield(L, -2, "trace");
		return 1;
	}

	int luaXpcallFinish(lua_State *L, int status, lua_KContext extra) {
		if (status != LUA_OK && status != LUA_YIELD) {
			lua_pushboolean(L, 0);
			lua_pushvalue(L, -2);
			return 2;
		}
		lua_pushboolean(L, 1);
		lua_rotate(L, 1 + extra, 1);
		return lua_gettop(L) - extra;
	}

	// xpcall-reimplementation adding traceback to error object
	int luaXpcall(lua_State *L) {
		int n = lua_gettop(L);
		luaL_checktype(L, 1, LUA_TFUNCTION);
		lua_pushcfunction(L, &luaXpcallMsg);
		lua_rotate(L, 1, 1);
		int status = lua_pcallk(L, n - 1, LUA_MULTRET, 1, 0, luaXpcallFinish);
		return luaXpcallFinish(L, status, 1);
	}

	// Theoratical better xpcall-reimplementation for FIN but
	// doesn't work because not easy to interact with non LUA_OK or LUA_YIELD coroutines
	/*
	int luaXpcallMsgContinue(lua_State* L2, int args, lua_KContext ctx) {
		lua_State* L = (lua_State*)ctx;
		lua_pushboolean(L, 0);
		return args+1;
	}

	int luaXpcallContinue(lua_State* L, int args, lua_KContext ctx) {
		lua_State* L2 = (lua_State*)ctx;
		int res;
		int status = lua_resume(L2, L, 0, &res);
		if (status == LUA_YIELD) {
			return lua_yieldk(L, 0, ctx, &luaXpcallContinue);
		}
		if (status > LUA_YIELD) {
			lua_rotate(L2, 1, -1);
			lua_rotate(L2, -2, -1);
			lua_callk(L2, 1, 1, (lua_KContext)L, &luaXpcallMsgContinue);
		}
		lua_pushboolean(L, 1);
		return res+1;
	}
	#pragma optimize("", off)
	int luaXpcall(lua_State* L) {
		int n = lua_gettop(L);
		luaL_checktype(L, 1, LUA_TFUNCTION); // check actual function
		luaL_checktype(L, 2, LUA_TFUNCTION); // check msg function
		lua_State* L2 = lua_newthread(L);
		lua_rotate(L, 1, -1);
		lua_xmove(L, L2, n - 1);
		int L2n = lua_gettop(L2);
		int res;
		int status = lua_resume(L2, L, n - 2, &res);
		if (status == LUA_YIELD) {
			return lua_yieldk(L, 0, (lua_KContext)L2, &luaXpcallContinue);
		}
		if (status > LUA_YIELD) {
			luaL_checktype(L2, L2n, LUA_TFUNCTION);
			lua_rotate(L2, L2n, -1);
			lua_rotate(L2, -2, -1);
			luaL_checktype(L2, -2, LUA_TFUNCTION);
			lua_pcallk(L2, 1, 1, 0, (lua_KContext)L, &luaXpcallMsgContinue);
		}
		lua_pushboolean(L, 1);
		return res+1;
	}
	#pragma optimize("", on)
	*/

	int luaTableMerge(lua_State* L) {
		int args = lua_gettop(L);
		luaL_checktype(L, 1, LUA_TTABLE);
		for (int i = 1; i <= args; ++i) {
			luaL_checktype(L, i, LUA_TTABLE);
			lua_pushnil(L);
			while (lua_next(L, i) != 0) {
				lua_pushvalue(L, -2);
				lua_insert(L, -2);
				luaFIN_setOrMergeField(L, 1);
			}
			lua_pop(L, 1);
		}
		return 1;
	}

	int luaGetItems(lua_State* L) {
		FLuaSync SyncCall(L);
		int top = lua_gettop(L);
		FString Filter;
		int i = 1;
		if (lua_isstring(L, i)) {
			Filter = luaFIN_toFString(L, i++);
		}

		int64 pageSize = TNumericLimits<int64>::Max();
		int64 pageOffset = 0;
		if (top >= i) {
			pageSize = FMath::Clamp(lua_tointeger(L, i++), 1, 100);
			pageOffset = FMath::Max(0, lua_tointeger(L, i++));
		}

		lua_newtable(L);

		TArray<TSubclassOf<UFGItemDescriptor>> items;
		UFGBlueprintFunctionLibrary::Cheat_GetAllDescriptors(items);

		int64 CurrentIndex = 0;
		for (TSubclassOf<UFGItemDescriptor> ItemDescriptor : items) {
			FString Name = UFGItemDescriptor::GetItemName(ItemDescriptor).ToString();
			if (Name.Contains(Filter)) {
				if (pageOffset * pageSize <= CurrentIndex) {
					luaFIN_pushFString(L, Name);
					luaFIN_pushClass(L, ItemDescriptor);
					lua_settable(L, -3);
				}
				CurrentIndex += 1;
				if ((pageOffset+1) * pageSize <= CurrentIndex) break;
			}
		}
		return 1;
	}

	int luaFindItem(lua_State* L) {
		FLuaSync SyncCall(L);
		const int NumArgs = lua_gettop(L);
		if (NumArgs < 1) return 0;
		const char* str = luaL_tolstring(L, -1, nullptr);

		TArray<TSubclassOf<UFGItemDescriptor>> items;
		UFGBlueprintFunctionLibrary::Cheat_GetAllDescriptors(items);
		if (str) for (TSubclassOf<UFGItemDescriptor> item : items) {
			if (IsValid(item) && UFGItemDescriptor::GetItemName(item).ToString() == FString(str)) {
				luaFIN_pushClass(L, item);
				return 1;
			}
		}

		lua_pushnil(L);

		return 1;
	}

	LuaModule(R"(/**
	 * @LuaModule		BaseModule
	 * @DisplayName		Base Module
	 *
	 * This module provides based functionallity to the Lua runtime.
	 * Inluding base Lua Libraries (table, math, coroutine, etc).
	 */)", BaseModule) {
		LuaModulePostSetup() {
			PersistenceNamespace("Base");

			luaopen_base(L);
			lua_pushnil(L);
			lua_setfield(L, -2, "collectgarbage");
			lua_pushnil(L);
			lua_setfield(L, -2, "dofile");
			lua_pushnil(L);
			lua_setfield(L, -2, "print");
			lua_pushnil(L);
			lua_setfield(L, -2, "loadfile");
			lua_pushcfunction(L, luaXpcall);
			lua_setfield(L, -2, "xpcall");
			PersistTable("global", -1);
			lua_pop(L, 1);

			luaL_requiref(L, "table", luaopen_table, true);
			lua_pushcfunction(L, luaTableMerge);
			lua_setfield(L, -2, "merge");
			PersistTable("table", -1);
			lua_pop(L, 1);

			luaL_requiref(L, "math", luaopen_math, true);
			PersistTable("math", -1);
			lua_pop(L, 1);

			luaL_requiref(L, "string", luaopen_string, true);
			PersistTable("string", -1);
			lua_pop(L, 1);

			luaL_requiref(L, "coroutine", luaopen_coroutine, true);
			lua_pushcfunction(L, luaResume);
			lua_setfield(L, -2, "resume");
			lua_pushcfunction(L, luaYield);
			lua_setfield(L, -2, "yield");
			lua_pushcfunction(L, luaRunning);
			lua_setfield(L, -2, "running");
			PersistTable("coroutine", -1);
			lua_pop(L, 1);

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaYieldResume)));
			PersistValue("coroutineYieldContinue");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaResumeResume)));
			PersistValue("coroutineResumeContinue");

			lua_register(L, "findItem", luaFindItem);
			PersistGlobal("findItem");
			lua_register(L, "getItems", luaGetItems);
			PersistGlobal("getItems");

	/*#ifdef TRACY_ENABLE
			tracy::LuaRegister(L);
			PersistGlobal("tracy");
	#endif*/
		}
	}
}
