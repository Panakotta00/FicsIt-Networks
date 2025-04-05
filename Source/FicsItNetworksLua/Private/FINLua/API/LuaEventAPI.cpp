#include "LuaEventAPI.h"

#include "FicsItNetworksComputer.h"
#include "FicsItNetworksLuaModule.h"
#include "FicsItReflection.h"
#include "FINEventFilter.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FINLuaThreadedRuntime.h"
#include "FINNetworkUtils.h"
#include "LuaFuture.h"
#include "LuaStruct.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "Signals/FINSignalSubsystem.h"
#include "FicsItKernel/FicsItKernel.h"

UE_DISABLE_OPTIMIZATION_SHIP

FFINLuaEventQueue::FFINLuaEventQueue(FFINLuaReferenceCollector* ReferenceCollector, int64 Key): FFINLuaReferenceCollected(ReferenceCollector), Key(Key) {}

void FFINLuaEventQueue::CollectReferences(FReferenceCollector& Collector) {
	Collector.AddPropertyReferencesWithStructARO(StaticStruct(), this);
}

namespace FINLua {
	void luaFIN_pushEventQueueInternal(lua_State* L, const FEventQueue& queueRef);

	FFINEventFilterExpression checkFilter(lua_State* L, int index) {
		index = lua_absindex(L, index);
		FFINEventFilterExpression filterExp;
		if (TSharedPtr<FFINEventFilterExpression> filterPtr = luaFIN_toStruct<FFINEventFilterExpression>(L, index, false)) {
			filterExp = *filterPtr;
		} else {
			luaL_checktype(L, index, LUA_TTABLE);
			FFINEventFilter filter;
			if (lua_getfield(L, index, "event") != LUA_TNIL) {
				if (lua_istable(L, -1)) {
					int len = luaL_len(L, -1);
					for (int i = 0; i < len; ++i) {
						lua_geti(L, -1, i);
						FString event = luaFIN_toFString(L, -1);
						filter.Events.Add(event);
						lua_pop(L, 1);
					}
				} else {
					FString event = luaFIN_toFString(L, -1);
					filter.Events.Add(event);
				}
				lua_pop(L, 1);
			}
			if (lua_getfield(L, index, "sender") != LUA_TNIL) {
				if (lua_istable(L, -1)) {
					int len = luaL_len(L, -1);
					for (int i = 0; i < len; ++i) {
						lua_geti(L, -1, i);
						UObject* sender = luaFIN_checkObject<UObject>(L, -1);
						filter.Senders.Add(sender);
						lua_pop(L, 1);
					}
				} else {
					UObject* sender = luaFIN_checkObject<UObject>(L, -1);
					filter.Senders.Add(sender);
				}
				lua_pop(L, 1);
			}
			if (lua_getfield(L, index, "values") != LUA_TNIL) {
				luaL_checktype(L, -1, LUA_TTABLE);
				lua_pushnil(L);
				while (lua_next(L, -2)) {
					FString paramName = luaFIN_toFString(L, -2);
					TOptional<FFIRAnyValue> value = luaFIN_toNetworkValue(L, -1);
					if (value) {
						filter.ValueFilters.Add(paramName, *value);
					}
					lua_pop(L, 1);
				}
				lua_pop(L, 2);
			}
			filterExp = filter;
		}
		return filterExp;
	}

	LuaModule(R"(/**
	 * @LuaModule		EventModule
	 * @DisplayName		Event Module
	 * @Dependency		ReflectionSystemStructModule
	 * @Dependency		FutureModule
	 */)", EventModule) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	EventQueue
		 * @DisplayName		Event Queue
		 */)", EventQueue) {
			FEventQueue& close(lua_State* L) {
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);
				if (queue->Key >= 0) {
					TSharedPtr<FFINLuaEventRegistry> registry = luaFIN_getEventRegistry(L);
					if (registry) {
						registry->EventQueues.Remove(queue->Key);
						if (lua_getiuservalue(L, -1, 2) == LUA_TTABLE) {
							lua_pushnil(L);
							lua_seti(L, -2, queue->Key);
						}
					}
					queue->Key = -1;
				}
				return queue;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__close
			 */)", __close) {
				close(L);
				return 0;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 */)", __gc) {
				FEventQueue queue = close(L);
				queue.~FEventQueue();
				return 0;
			}

			int unpersist(lua_State* L) {
				FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
				FFINLuaRuntimePersistenceState& Storage = luaFIN_getPersistence(L);

				const FFIRInstancedStruct& Struct = *Storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)));

				FFINLuaEventQueue& queue = Struct.Get<FFINLuaEventQueue>();
				queue.ReferenceCollector = luaFIN_getReferenceCollector(L);
				luaFIN_pushEventQueueInternal(L, MakeShared<FFINLuaEventQueue>(queue));
				lua_pushvalue(L, lua_upvalueindex(2));
				lua_setiuservalue(L, -2, 1);
				lua_pushvalue(L, lua_upvalueindex(3));
				lua_setiuservalue(L, -2, 2);

				return 1;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 */)", __persist) {
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);

				FFINLuaRuntimePersistenceState& Storage = luaFIN_getPersistence(L);
				lua_pushinteger(L, Storage.Add(MakeShared<FIRStruct>(*queue)));
				lua_getiuservalue(L, 1, 1);
				lua_getiuservalue(L, 1, 2);

				lua_pushcclosure(L, &unpersist, 3);

				return 1;
			}

			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	__index
			 * @DisplayName		Index
			 */)", __index) {
				lua_pushnil(L);
			}

			int luaPullContinue(lua_State* L, int, lua_KContext) {
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);
				double timeout = luaL_checknumber(L, 2);
				if (!queue->Events.IsEmpty()) {
					FFINLuaEvent event = queue->Events[0];
					queue->Events.RemoveAt(0);
					return luaFIN_pushEventData(L, event.Sender, event.Data);
				}
				double currentTime = FPlatformTime::Seconds();
				if (timeout > currentTime) {
					return luaFIN_yield(L, 0, NULL, luaPullContinue);
				}
				return 0;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(string?, Object, ...)	pull(timeout: number?)
			 * @DisplayName		Pull
			 *
			 * Waits for a signal in the queue. Blocks the execution until a signal got pushed to the signal queue, or the timeout is reached.
			 * Returns directly if there is already a signal in the queue (the tick doesn’t get yielded).
			 *
			 * @parameter	self		EventQueue
			 * @parameter	timeout		number			Timeout		The amount of time needs to pass until pull unblocks when no signal got pushed. If not set, the function will block indefinitely until a signal gets pushed. If set to `0` (int), will not yield the tick and directly return with the signal data or nil if no signal was in the queue
			 * @return		event		string?			Event		The name of the returned signal. Nil when timeout got reached
			 * @return		sender		Object			Sender		The component representation of the signal sender. Not set when timeout got reached
			 * @return		...			any				Parameters	The parameters passed to the signal. Not set when timeout got reached
			 */)", pull) {
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);
				double timeout = luaL_checknumber(L, 2);
				timeout += FPlatformTime::Seconds();
				lua_pop(L, 1);
				lua_pushnumber(L, timeout);
				return luaPullContinue(L, 0, NULL);
			}

			int luaWaitForContinue(lua_State* L, int, lua_KContext) {
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);
 				TSharedRef<FFINEventFilterExpression> filter = luaFIN_checkStruct<FFINEventFilterExpression>(L, 2, false);
				while (!queue->Events.IsEmpty()) {
					FFINLuaEvent event = queue->Events[0];
					queue->Events.RemoveAt(0);
					if (filter->Matches(event.Sender.GetUnderlyingPtr(), event.Data)) {
						return luaFIN_pushEventData(L, event.Sender, event.Data);
					}
				}
				return luaFIN_yield(L, 0, NULL, luaWaitForContinue);
			}
			int luaWaitFor(lua_State* L) {
				return luaWaitForContinue(L, 0, NULL);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(string?, Object, ...)	waitFor(filter: EventFilter)
			 * @DisplayName		Wait For
			 *
			 * Returns a Future that resolves when a signal got added to the queue that matches the given Event Filter.
			 *
			 * @parameter	self		EventQueue
			 * @parameter	filter		EventFilter|{event?:string|string[],sender?:Object|Object[],values?:table<string,any>}		Filter		Event filter
			 * @return		event		string?			Event		The name of the returned signal
			 * @return		sender		Object			Sender		The component representation of the signal sender
			 * @return		...			any				Parameters	The parameters passed to the signal
			 */)", waitFor) {
				FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);
				FFINEventFilterExpression filter = checkFilter(L, 2);
				lua_pushvalue(L, 1);
				luaFIN_pushStruct(L, filter);
				luaFIN_pushLuaFutureCFunction(L, luaWaitFor, 2);
				return 1;
			}
		}

		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		event
		 * @DisplayName		Event Library
		 *
		 * The Event API provides classes, functions and variables for interacting with the component network.
		 */)", event) {
			void luaListen(lua_State* L, FFIRTrace o) {
				IFINLuaEventSystem& eventSystem = luaFIN_getEventSystem(L);;
				UObject* obj = *o;
				if (!IsValid(obj)) luaL_error(L, "object is not valid");
				eventSystem.Listen(o);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		listen(...: Object)
			 * @DisplayName		Listen
			 *
			 * Adds the running lua context to the listen queue of the given components.
			 *
			 * @parameter	...			Object			Objects		A list of objects the computer should start listening to
			 */)", listen) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSync SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					FFIRTrace trace = luaFIN_checkObject(L, i, nullptr);
					luaListen(L, trace);
				}
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Object[]	listening()
			 * @DisplayName		Listening
			 *
			 * Returns all signal senders this computer is listening to.
			 *
			 * @return		listening	Object[]		Objects		An array containing all objects this computer is currently listening to
			 */)", listening) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSync SyncCall(L);

				IFINLuaEventSystem& eventSystem = luaFIN_getEventSystem(L);

				TArray<FFIRTrace> Listening = eventSystem.Listening();
				int i = 0;
				lua_newtable(L);
				for (const FFIRTrace& Obj : Listening) {
					luaFIN_pushObject(L, Obj);
					lua_seti(L, -2, ++i);
				}
				return 1;
			}

			// ReSharper disable once CppParameterNeverUsed
			int luaPullContinue(lua_State* L, int status, lua_KContext ctx) {
				IFINLuaEventSystem& eventSystem = luaFIN_getEventSystem(L);
				TOptional<TTuple<FFIRTrace, FFINSignalData>> data = eventSystem.PullSignal();
				if (!data) {
					const int args = lua_gettop(L);
					TOptional<double> timeout;
					if (args > 0) {
						timeout = luaL_checknumber(L, 1);
					}
					if (!timeout || *timeout <= FPlatformTime::Seconds()) {
						return 0;
					}
					return luaFIN_yield(L, 0, 0, luaPullContinue);
				}

				const auto& [sender, signal] = *data;

				luaFIN_handleEvent(L, sender, signal);
				return luaFIN_pushEventData(L, sender, signal);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(string?, Object, ...)	pull(timeout: number?)
			 * @DisplayName		Pull
			 *
			 * Waits for a signal in the queue. Blocks the execution until a signal got pushed to the signal queue, or the timeout is reached. +
			 * Returns directly if there is already a signal in the queue (the tick doesn’t get yielded).
			 *
			 * @parameter	timeout		number			Timeout		The amount of time needs to pass until pull unblocks when no signal got pushed. If not set, the function will block indefinitely until a signal gets pushed. If set to `0` (int), will not yield the tick and directly return with the signal data or nil if no signal was in the queue
			 * @return		event		string?			Event		The name of the returned signal. Nil when timeout got reached
			 * @return		sender		Object			Sender		The component representation of the signal sender. Not set when timeout got reached
			 * @return		...			any				Parameters	The parameters passed to the signal. Not set when timeout got reached
			 */)", pull) {
				const int args = lua_gettop(L);
				if (args > 0) {
					double timeout = luaL_checknumber(L, 1);
					timeout += FPlatformTime::Seconds();
					lua_pop(L, args);
					lua_pushnumber(L, timeout);
				} else {
					lua_pushnumber(L, TNumericLimits<double>::Max());
				}
				return luaPullContinue(L, 0, 0);
			}

			void luaIgnore(lua_State* L, FFIRTrace o) {
				UObject* obj = *o;
				if (!IsValid(obj)) luaL_error(L, "object is not valid");
				AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
				SigSubSys->Ignore(obj, *o.Reverse());
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ignore(...: Object)
			 * @DisplayName		Ignore
			 *
			 * Removes the running lua context from the listen queue of the given components. Basically the opposite of listen.
			 *
			 * @parameter	...			Object			Objects		A list of objects this computer should stop listening to
			 */)", ignore) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSync SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					FFIRTrace Trace = luaFIN_checkObject(L, i, nullptr);
					luaIgnore(L, Trace);
				}
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ignoreAll()
			 * @DisplayName		Ignore All
			 *
			 * Stops listening to any signal sender. If afterwards there are still coming signals in, it might be the system itself or caching bug.
			 */)", ignoreAll) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSync SyncCall(L);
				IFINLuaEventSystem& eventSystem = luaFIN_getEventSystem(L);
				eventSystem.IgnoreAll();
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		clear()
			 * @DisplayName		Clear
			 *
			 * Clears every signal from the signal queue.
			 */)", clear) {
				IFINLuaEventSystem& eventSystem = luaFIN_getEventSystem(L);
				eventSystem.Clear();
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		EventFilter		filter(params: { event?: string|string[], sender?: Object|Object[], values?: table<string,any> })
			 * @DisplayName		Filter
			 *
			 * Creates an Event filter expression.

			 * @parameter	params		{ event?: string|string[], sender?: Object|Object[], values?: table<string,any> }	Params	Filter parameters
			 * @return		filter		EventFilter		Filter		Event filter
			 */)", filter) {
				FFINEventFilterExpression expression = checkFilter(L, 1);
				luaFIN_pushStruct(L, expression);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		integer		registerListener(filter: EventFilter, cb: fun(event, sender, ...))
			 * @DisplayName		Register Listener
			 *
			 * Registers the given function as a listener.
			 * When `event.pull()` pulls a signal from the queue, that matches the given Event-Filter,
			 * a Task will be created using the function and the signals parameters will be passed into the function.
			 * 
			 * @parameter	filter		EventFilter|{event?:string|string[],sender?:Object|Object[],values?:table<string,any>}		Filter		Event filter
			 * @parameter	cb			fun(event, sender, ...)		Callback	Callback that will be called on every event that matches the filter
			 */)", registerListener) {
				luaL_checktype(L, 2, LUA_TFUNCTION);
				FFINEventFilterExpression filter = checkFilter(L, 1);
				TSharedPtr<FFINLuaEventRegistry> registry = luaFIN_getEventRegistry(L);
				int64 key = FFINLuaEventRegistry::FindNextKey(registry->EventListeners);
				registry->EventListeners.Add(key, filter);
				lua_getiuservalue(L, -1, 1);
				lua_pushvalue(L, 2);
				lua_seti(L, -2, key+1);
				lua_pushinteger(L, key);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		EventQueue		queue(filter: EventFilter)
			 * @DisplayName		queue
			 *
			 * Creates a new event queue.
			 * When this variable closes or gets garbage collected, it will stop receiving signals.
			 *
			 * @parameter	filter		EventFilter|{event?:string|string[],sender?:Object|Object[],values?:table<string,any>}		Filter		Event filter
			 * @return		queue		EventQueue		Queue		Event queue
			 */)", queue) {
				FFINEventFilterExpression filter = checkFilter(L, 1);
				TSharedPtr<FFINLuaEventRegistry> registry = luaFIN_getEventRegistry(L);
				int64 key = FFINLuaEventRegistry::FindNextKey(registry->EventQueues);
				registry->EventQueues.Add(key, filter);
				lua_getiuservalue(L, -1, 2);
				luaFIN_pushEventQueue(L, key);
				lua_pushvalue(L, -1);
				lua_seti(L, -3, key+1);
				return 1;
			}

			int luaWaitForContinue(lua_State* L, int, lua_KContext) {
				int key = luaL_checkinteger(L, 2);
				if (lua_geti(L, 1, key+1) == LUA_TNIL) {
					lua_pop(L, 1);
					lua_pushnil(L);
					return luaFIN_yield(L, 1, NULL, luaWaitForContinue);
				}
				lua_pushnil(L);
				lua_seti(L, 1, key+1);
				luaL_checktype(L, 3, LUA_TTABLE);
				int len = luaL_len(L, 3);
				for (int i = 1; i <= len; ++i) {
					lua_geti(L, 3, i);
				}
				return len;
			}
			int luaWaitFor(lua_State* L) {
				return luaWaitForContinue(L, 0, NULL);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Future		waitFor(filter: EventFilter)
			 * @DisplayName		Wait For
			 *
			 * Returns a Future that resolves when a signal got polled that matches the given Event Filter.
			 * 
			 * @parameter	filter		EventFilter|{event?:string|string[],sender?:Object|Object[],values?:table<string,any>}		Filter		Event filter
			 * @return		event		string?			Event		The name of the returned signal
			 * @return		sender		Object			Sender		The component representation of the signal sender
			 * @return		...			any				Parameters	The parameters passed to the signal
			 */)", waitFor) {
				FFINEventFilterExpression filter = checkFilter(L, 1);
				TSharedPtr<FFINLuaEventRegistry> registry = luaFIN_getEventRegistry(L);
				int registry_index = lua_absindex(L, -1);
				int64 key = FFINLuaEventRegistry::FindNextKey(registry->OneShots);
				registry->OneShots.Add(key, filter);
				lua_getiuservalue(L, -1, 3);
				lua_pushinteger(L, key);
				luaFIN_pushLuaFutureCFunction(L, luaWaitFor, 2);

				lua_getiuservalue(L, registry_index, 4);
				lua_pushvalue(L, -2);
				luaFINDebug_dumpStack(L);
				int future_ref = luaL_ref(L, -2);
				lua_pop(L, 1);
				registry->OneShots_Futures.Add(key, future_ref);

				return 1;
			}
		}

		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		eventTask	Future
		 * @DisplayName		Event Task
		 *
		 * A future that is used as task to handle Events.
		 */)", eventTask) {
			lua_pushnil(L);
		}

		LuaModulePostSetup() {
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(event::luaPullContinue)));
			luaFIN_persistValue(L, -1, "Event-luaPullContinue");
			lua_pop(L, 1);

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(EventQueue::unpersist)));
			luaFIN_persistValue(L, -1, "EventQueue-unpersist");
			lua_pop(L, 1);

			luaL_getmetatable(L, EventQueue::_Name);
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			lua_pop(L, 1);

			lua_geti(L, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS);
			luaFIN_pushStruct(L, FFINLuaEventRegistry(), 4);
			lua_newtable(L);
			lua_setiuservalue(L, -2, 1);
			lua_newtable(L);
			lua_setiuservalue(L, -2, 2);
			lua_newtable(L);
			lua_setiuservalue(L, -2, 3);
			lua_newtable(L);
			lua_setiuservalue(L, -2, 4);
			lua_setfield(L, -2, "event-registry");
			lua_pop(L, 1);

			lua_pushcfunction(L, event::luaWaitFor);
			luaFIN_persistValue(L, -1, "WaitFor");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(event::luaWaitForContinue)));
			luaFIN_persistValue(L, -1, "WaitForContinue");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(event::luaPullContinue)));
			luaFIN_persistValue(L, -1, "PullContinue");
			lua_pop(L, 4);

			lua_pushcfunction(L, EventQueue::luaWaitFor);
			luaFIN_persistValue(L, -1, "EventQueueWaitFor");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(EventQueue::luaWaitForContinue)));
			luaFIN_persistValue(L, -1, "EventQueueWaitForContinue");
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(EventQueue::luaPullContinue)));
			luaFIN_persistValue(L, -1, "EventQueuePullContinue");
			lua_pop(L, 3);

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_eventTask)));
			luaFIN_persistValue(L, -1, "luaFIN_eventTask");

			lua_getglobal(L, "event");
			luaFIN_pushLuaFutureCFunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(luaFIN_eventTask)), 0);
			luaFIN_addBackgroundTask(L, -1);
			lua_setfield(L, -2, "eventTask");
			lua_pop(L, 1);
		}
	}

	void luaFIN_pushEventQueueInternal(lua_State* L, const FEventQueue& queueRef) {
		FEventQueue* queue = static_cast<FEventQueue*>(lua_newuserdatauv(L, sizeof(FEventQueue), 2));
		new (queue) FEventQueue(queueRef);
		luaL_setmetatable(L, EventModule::EventQueue::_Name);
	}

	FEventQueue luaFIN_pushEventQueue(lua_State* L, int64 key) {
		FFINLuaReferenceCollector* referenceCollector = luaFIN_getReferenceCollector(L);
		TSharedRef<FFINLuaEventQueue> queueRef = MakeShared<FFINLuaEventQueue>(referenceCollector, key);
		luaFIN_pushEventQueueInternal(L, queueRef);
		lua_newtable(L);
		lua_setiuservalue(L, -2, 1);
		lua_newtable(L);
		lua_setiuservalue(L, -2, 2);
		return queueRef;
	}

	FEventQueue* luaFIN_toEventQueue(lua_State* L, int index) {
		return static_cast<FEventQueue*>(luaL_testudata(L, index, EventModule::EventQueue::_Name));
	}

	FEventQueue& luaFIN_checkEventQueue(lua_State* L, int index) {
		FEventQueue* queue = luaFIN_toEventQueue(L, index);
		if (!queue) luaFIN_typeError(L, index, EventModule::EventQueue::_Name);
		return *queue;
	}

	TSharedPtr<FFINLuaEventRegistry> luaFIN_getEventRegistry(lua_State* L) {
		if (lua_geti(L, LUA_REGISTRYINDEX, LUAFIN_RIDX_HIDDENGLOBALS) == LUA_TNIL) {
			lua_pop(L, 1);
			return nullptr;
		}
		if (lua_getfield(L, -1, "event-registry") == LUA_TNIL) {
			lua_pop(L, 2);
			return nullptr;
		}
		TSharedPtr<FFINLuaEventRegistry> registry = luaFIN_toStruct<FFINLuaEventRegistry>(L, -1, false);
		lua_remove(L, -2);
		return registry;
	}

	int luaFIN_pushEventData(lua_State* L, const FFIRTrace& sender, const FFINSignalData& data) {
		luaFIN_pushFString(L, data.Signal->GetInternalName());
		luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(sender));
		for (const FFIRAnyValue& Value : data.Data) {
			luaFIN_pushNetworkValue(L, Value, sender);
		}
		return 2 + data.Data.Num();
	}

	void luaFIN_handleEvent(lua_State* L, const FFIRTrace& sender, const FFINSignalData& data) {
		TSharedPtr<FFINLuaEventRegistry> registry = luaFIN_getEventRegistry(L);
		if (!registry) return;
		lua_getiuservalue(L, -1, 1);
		for (const auto& [key, filter] : registry->EventListeners) {
			if (filter.Matches(sender.GetUnderlyingPtr(), data)) {
				lua_geti(L, -1, key+1);
				int num = luaFIN_pushEventData(L, sender, data);
				luaFIN_pushLuaFutureLuaFunction(L, num);
				luaFIN_pushPollCallback(L, -1);
				lua_pop(L, 2);
			}
		}
		lua_pop(L, 1);
		lua_getiuservalue(L, -1, 2);
		for (const auto& [key, filter] : registry->EventQueues) {
			if (filter.Matches(sender.GetUnderlyingPtr(), data)) {
				lua_geti(L, -1, key+1);
				FEventQueue& queue = luaFIN_checkEventQueue(L, -1);
				queue->AddEvent(sender, data);
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);
		lua_getiuservalue(L, -1, 4);
		lua_getiuservalue(L, -2, 3);
		for (const auto& [key, filter] : registry->OneShots) {
			if (filter.Matches(sender.GetUnderlyingPtr(), data)) {
				lua_newtable(L);
				int table = lua_absindex(L, -1);
				int num = luaFIN_pushEventData(L, sender, data);
				for (int i = num; i > 0; --i) {
					lua_seti(L, table, i);
				}
				lua_seti(L, -2, key+1);

				// Wake Future
				int ref = registry->OneShots_Futures[key];
				lua_geti(L, -2, ref);
				lua_pushcclosure(L, &luaFIN_callbackPoll, 1);
				luaFIN_pushCallback(L);
				luaL_unref(L, -2, ref);
				registry->OneShots_Futures.Remove(key);

				registry->OneShots.Remove(key);
			}
		}
		lua_pop(L, 2);
		lua_pop(L, 1);
	}

	int luaFIN_eventTask(lua_State* L, int, lua_KContext) {
		lua_settop(L, 0);

		TSharedPtr<FFINLuaEventRegistry> registry = luaFIN_getEventRegistry(L);
		if (registry->EventListeners.Num() + registry->EventQueues.Num() + registry->OneShots.Num() > 0) {
			IFINLuaEventSystem& eventSystem = luaFIN_getEventSystem(L);
			while (true) {
				TOptional<TTuple<FFIRTrace, FFINSignalData>> data = eventSystem.PullSignal();
				if (data) {
					const auto& [sender, signal] = *data;
					luaFIN_handleEvent(L, sender, signal);
				} else {
					break;
				}
			}
		}

		lua_pushnumber(L, TNumericLimits<double>::Max());
		return luaFIN_yield(L, 1, NULL, luaFIN_eventTask);
	}

	void luaFIN_setEventSystem(lua_State* L, IFINLuaEventSystem& EventSystem) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		runtime.GlobalPointers.Add(TEXT("EventSystem"), &EventSystem);
	}

	IFINLuaEventSystem& luaFIN_getEventSystem(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		IFINLuaEventSystem** value = reinterpret_cast<IFINLuaEventSystem**>(runtime.GlobalPointers.Find(TEXT("EventSystem")));
		fgcheck(value != nullptr);
		return **value;
	}
}
UE_ENABLE_OPTIMIZATION_SHIP