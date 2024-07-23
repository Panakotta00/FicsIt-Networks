#include "FINLua/LuaFuture.h"

#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"

namespace FINLua {
	typedef TSharedRef<TFINDynamicStruct<FFINFuture>> FLuaFuture;

	LuaModule(R"(/**
	 * @LuaModule		FutureModule
	 * @DisplayName		Future Module
	 *
	 * This Module provides the Future type and all its necessary functionallity.
	 */)", FutureModule) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	Future
		 * @DisplayName		Future
		 */)", Future) {
			LuaModuleTableTable(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				int luaFutureAwait(lua_State* L, int Index);
				int luaFutureAwaitContinue(lua_State* L, int, lua_KContext) {
					return luaFutureAwait(L, 1);
				}
				int luaFutureAwait(lua_State* L, int Index) {
					const TFINDynamicStruct<FFINFuture>& future = luaFIN_checkFuture(L, Index);
					if ((*future)->IsDone()) {
						TArray<FFINAnyNetworkValue> Data = future->GetOutput();
						FFINNetworkTrace Trace;
						if (future.GetStruct() == FFINFutureReflection::StaticStruct()) {
							FFINFutureReflection& refFuture = future.Get<FFINFutureReflection>();
							FScopeLock Lock(&refFuture.Mutex);
							Trace = refFuture.Context.GetTrace();
						}
						for (const FFINAnyNetworkValue& Param : Data) luaFIN_pushNetworkValue(L, Param, Trace);
						return Data.Num();
					} else {
						return lua_yieldk(L, LUA_MULTRET, NULL, luaFutureAwaitContinue);
					}
				}

				LuaModuleTableFunction(R"(/**
				 * @LuaFunction		await
				 * @DisplayName		Await
				 */)", await) {
					return luaFutureAwait(L, 1);
				}

				LuaModuleTableFunction(R"(/**
				 * @LuaFunction		get
				 * @DisplayName		Get
				 */)", get) {
					const TFINDynamicStruct<FFINFuture>& future = luaFIN_checkFuture(L, 1);
					if (!future->IsDone()) luaFIN_argError(L, 1, "Future is not ready");
					const TArray<FFINAnyNetworkValue>& Data = future->GetOutput();
					FFINNetworkTrace Trace;
					if (future.GetStruct() == FFINFutureReflection::StaticStruct()) Trace = future.Get<FFINFutureReflection>().Context.GetTrace();
					for (const FFINAnyNetworkValue& Param : Data) luaFIN_pushNetworkValue(L, Param, Trace);
					return Data.Num();
				}

				LuaModuleTableFunction(R"(/**
				 * @LuaFunction		canGet
				 * @DisplayName		Can Get
				 */)", canGet) {
					const TFINDynamicStruct<FFINFuture>& future = luaFIN_checkFuture(L, 1);
					lua_pushboolean(L, future->IsDone());
					return 1;
				}
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__newindex
			 * @DisplayName		New Index
			 */)", __newindex) {
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		Garbage Collect
			 */)", __gc) {
				const FLuaFuture& future = luaFIN_checkLuaFuture(L, 1);
				future.~FLuaFuture();
				return 0;
			}

			int luaFutureUnpersist(lua_State* L) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);

				// get persist storage
				FFINLuaProcessorStateStorage& storage = processor->StateStorage;

				luaFIN_pushFuture(L, *storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1))));

				FLuaFuture* future = static_cast<FLuaFuture*>(lua_newuserdata(L, sizeof(FLuaFuture)));
				new (future) FLuaFuture(new TFINDynamicStruct<FFINFuture>(*storage.GetStruct(lua_tointeger(L, lua_upvalueindex(1)))));
				if (!(**future)->IsDone()) {
					UFINKernelSystem* kernel = processor->GetKernel();
					kernel->PushFuture(*future);
				}
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				FLuaFuture future = luaFIN_checkLuaFuture(L, 1);

				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);

				FFINLuaProcessorStateStorage& storage = processor->StateStorage;

				lua_pushinteger(L, storage.Add(future));

				lua_pushcclosure(L, luaFutureUnpersist, 1);

				return 1;
			}
		}

		LuaModulePostSetup() {
			PersistenceNamespace("FutureModule");

			lua_pushcfunction(L, Future::luaFutureUnpersist);
			PersistValue("FutureUnpersist");
			//lua_pushcfunction(L, Future::__index::luaFutureAwaitContinue); // TODO: Check how to persist continuation function
			//PersistValue("FutureAwaitContinue");
		}
	}

	void luaFIN_pushFuture(lua_State* L, const TFINDynamicStruct<FFINFuture>& Future) {
		FLuaFuture* future = static_cast<FLuaFuture*>(lua_newuserdata(L, sizeof(FLuaFuture)));
		new (future) FLuaFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(Future));
		if (!(**future)->IsDone()) {
			UFINKernelSystem* kernel = UFINLuaProcessor::luaGetProcessor(L)->GetKernel();
			kernel->PushFuture(*future);
		}
		luaL_setmetatable(L, FutureModule::Future::_Name);
	}

	const TSharedRef<TFINDynamicStruct<FFINFuture>>& luaFIN_checkLuaFuture(lua_State* L, int Index) {
		FLuaFuture& future = *static_cast<FLuaFuture*>(luaL_checkudata(L, Index, FutureModule::Future::_Name));
		return future;
	}
}
