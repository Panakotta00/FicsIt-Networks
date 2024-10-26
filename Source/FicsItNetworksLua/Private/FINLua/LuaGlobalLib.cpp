#include "FINLua/LuaGlobalLib.h"

#include "FGBlueprintFunctionLibrary.h"
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

	int luaPrint(lua_State* L) {
		// ReSharper disable once CppDeclaratorNeverUsed
		FLuaSyncCall SyncCall(L);
		const int args = lua_gettop(L);
		std::string log;
		for (int i = 1; i <= args; ++i) {
			size_t s_len = 0;
			const char* s = luaL_tolstring(L, i, &s_len);
			if (!s) luaL_argerror(L, i, "is not valid type");
			log += std::string(s, s_len) + " ";
		}
		if (log.length() > 0) log = log.erase(log.length()-1);
		
		UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
		Processor->GetKernel()->GetLog()->PushLogEntry(FIL_Verbosity_Info, UTF8_TO_TCHAR(log.c_str()));
		
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	int luaYieldResume(lua_State* L, int status, lua_KContext ctx) {
		// don't pass pushed bool for user executed yield identification
		return UFINLuaProcessor::luaAPIReturn(L, lua_gettop(L)-1);
	}

	int luaYield(lua_State* L) {
		const int args = lua_gettop(L);

		// insert a boolean to indicate a user executed yield
		lua_pushboolean(L, true);
		lua_insert(L, 1);
		
		return lua_yieldk(L, args+1, NULL, &luaYieldResume);
	}

	int luaResume(lua_State* L); // pre-declare

	int luaResumeResume(lua_State* L, int status, lua_KContext ctx) {
		return UFINLuaProcessor::luaAPIReturn(L, luaResume(L));
	}

	int luaResume(lua_State* L) {
		const int args = lua_gettop(L);
		if (!lua_isthread(L, 1)) return luaL_argerror(L, 1, "is no thread");
		lua_State* thread = lua_tothread(L, 1);

		if (!lua_checkstack(thread, args - 1)) {
			lua_pushboolean(L, false);
			lua_pushliteral(L, "too many arguments to resume");
			return 1;
		}

		// attach hook for out-of-time exception if thread got loaded from save and hook is not applied
		lua_sethook(thread, UFINLuaProcessor::luaHook, LUA_MASKCOUNT, UFINLuaProcessor::luaGetProcessor(L)->GetTickHelper().steps());

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
		UFINLuaProcessor* p = UFINLuaProcessor::luaGetProcessor(L);
		int ismain = lua_pushthread(L);
		lua_State* thread = lua_tothread(L, -1);
		if (thread == p->GetLuaThread()) ismain = 1;
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
	
	int luaFindItem(lua_State* L) {
		FLuaSyncCall SyncCall(L);
		const int NumArgs = lua_gettop(L);
		if (NumArgs < 1) return UFINLuaProcessor::luaAPIReturn(L, 0);
		const char* str = luaL_tolstring(L, -1, nullptr);

		TArray<TSubclassOf<UFGItemDescriptor>> items;
		UFGBlueprintFunctionLibrary::Cheat_GetAllDescriptors(items);
		if (str) for (TSubclassOf<UFGItemDescriptor> item : items) {
			if (IsValid(item) && UFGItemDescriptor::GetItemName(item).ToString() == FString(str)) {
				luaFIN_pushClass(L, item);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}
		}

		lua_pushnil(L);
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	int luaGetItems(lua_State* L) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		UFINKernelSystem* kernel = processor->GetKernel();
		UModContentRegistry* ContentRegistry = UModContentRegistry::Get(processor);
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
		UClass* ItemDescriptorClass = UFGItemDescriptor::StaticClass();
		int64 CurrentIndex = 0;
		bool bDone = false;
		ForEachObjectOfClass(UClass::StaticClass(), [&](UObject* LoadedClassObject) {
			if (bDone) return;
			UClass* Class = Cast<UClass>(LoadedClassObject);
			if (Class->IsChildOf(ItemDescriptorClass)) {
				const TSubclassOf<UFGItemDescriptor> ItemDescriptor = Class;
				FString Name = UFGItemDescriptor::GetItemName(ItemDescriptor).ToString();
				if (Name.Contains(Filter)) {
					if (pageOffset * pageSize <= CurrentIndex) {
						luaFIN_pushFString(L, Name);
						luaFIN_pushClass(L, ItemDescriptor);
						lua_settable(L, -3);
					}
					CurrentIndex += 1;
					if ((pageOffset+1) * pageSize <= CurrentIndex) bDone = true; 
				}
			}
		});
		return 1;
	}

	[[deprecated]]
	int luaFindClass_DEPRECATED(lua_State* L) {
		luaFIN_warning(L, "Deprecated function call 'findClass', use 'classes' global library instead", false);
		
		const int args = lua_gettop(L);

		for (int i = 1; i <= args; ++i) {
			const bool isT = lua_istable(L, i);

			TArray<FString> ClassNames;
			if (isT) {
				const auto count = lua_rawlen(L, i);
				for (int j = 1; j <= count; ++j) {
					lua_geti(L, i, j);
					if (!lua_isstring(L, -1)) return luaL_argerror(L, i, "array contains non-string");
					ClassNames.Add(lua_tostring(L, -1));
					lua_pop(L, 1);
				}
				lua_newtable(L);
			} else {
				if (!lua_isstring(L, i)) return luaL_argerror(L, i, "is not string");
				ClassNames.Add(lua_tostring(L, i));
			}
			int j = 0;
			for (const FString& ClassName : ClassNames) {
				UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(ClassName);
				if (Class) luaFIN_pushClass(L, Class);
				else lua_pushnil(L);
				if (isT) lua_seti(L, -2, ++j);
			}
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}
	
	void setupGlobals(lua_State* L) {
		PersistenceNamespace("Globals");

#pragma warning( push )
#pragma warning( disable : 4996 )
		lua_register(L, "findClass", luaFindClass_DEPRECATED);
		PersistGlobal("findClass");
#pragma warning( pop )

		luaopen_base(L);
		lua_pushnil(L);
		lua_setfield(L, -2, "collectgarbage");
		lua_pushnil(L);
		lua_setfield(L, -2, "dofile");
		lua_pushnil(L);
		lua_setfield(L, -2, "loadfile");
		lua_pushcfunction(L, luaXpcall);
		lua_setfield(L, -2, "xpcall");
		lua_pushcfunction(L, luaPrint);
		lua_setfield(L, -2, "print");
		PersistTable("global", -1);
		lua_pop(L, 1);
	
		luaL_requiref(L, "table", luaopen_table, true);
		PersistTable("table", -1);
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

		lua_pushcfunction(L, static_cast<lua_CFunction>(reinterpret_cast<void*>(luaYieldResume)));
		PersistValue("coroutineYieldContinue");
		lua_pushcfunction(L, static_cast<lua_CFunction>(reinterpret_cast<void*>(luaResumeResume)));
		PersistValue("coroutineResumeContinue");
	
		luaL_requiref(L, "math", luaopen_math, true);
		PersistTable("math", -1);
		lua_pop(L, 1);

		luaL_requiref(L, "string", luaopen_string, true);
		PersistTable("string", -1);
		lua_pop(L, 1);

		lua_register(L, "findItem", luaFindItem);
		PersistGlobal("findItem");
		lua_register(L, "getItems", luaGetItems);
		PersistGlobal("getItems");

/*#ifdef TRACY_ENABLE
		tracy::LuaRegister(L);
		PersistGlobal("tracy");
#endif*/
		
		setupUtilLib(L);

		TMap<FString, TSharedRef<FFINLuaModule>> modules;
		for (const TSharedRef<FFINLuaModule>& module : FFINLuaModuleRegistry::GetInstance().Modules) {
			modules.Add(module->InternalName, module);
		}
		TSet<FString> loadedModules;
		while (modules.Num() > 0) {
			FString FailedToLoadPrev;
			TArray<FString> modulesToLoad = { modules.begin().Key() };
			while (modulesToLoad.Num() > 0) {
				FString moduleName = modulesToLoad.Top();

				if (!modules.Contains(moduleName)) {
					if (FailedToLoadPrev.IsEmpty()) {
						UE_LOGFMT(LogFicsItNetworksLua, Error, "Failed to load Lua Module '{module}': Module not found!", moduleName);
						FailedToLoadPrev = moduleName;
					}
					modulesToLoad.Pop();
					continue;
				}

				TSharedRef<FFINLuaModule> module = modules[moduleName];

				if (!FailedToLoadPrev.IsEmpty()) {
					if (module->Dependencies.Contains(FailedToLoadPrev)) {
						UE_LOGFMT(LogFicsItNetworksLua, Error, "Failed to load Lua Module '{module}': Failed to load a dependency due to previous error.", moduleName);
						modules.Remove(moduleName);
						FailedToLoadPrev = moduleName;
					}
					modulesToLoad.Pop();
					continue;
				}

				bool bLoadDependenciesFirst = false;
				for (const FString& dependency : module->Dependencies) {
					if (!loadedModules.Contains(dependency)) {
						bLoadDependenciesFirst = true;
						if (modulesToLoad.Contains(dependency)) {
							FString chain = dependency;
							for (int i = modulesToLoad.Num(); i > 0; --i) {
								chain = modulesToLoad[i-1] + " -> " + chain;
								if (modulesToLoad[i-1] == dependency) break;
							}
							UE_LOGFMT(LogFicsItNetworksLua, Error, "Failed to load Lua Module '{dependency}': Circular dependency '{chain}' found!", dependency, chain);
							FailedToLoadPrev = moduleName;
							break;
						}
						modulesToLoad.Add(dependency);
					}
				}
				if (bLoadDependenciesFirst) continue;

				module->SetupModule(L);

				modulesToLoad.Pop();
				modules.Remove(moduleName);
				loadedModules.Add(moduleName);

				UE_LOGFMT(LogFicsItNetworksLua, Display, "Lua Module '{module}' loaded!", moduleName);
			}
		}
	}
}
