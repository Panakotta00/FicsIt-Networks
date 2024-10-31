#include "LuaEventAPI.h"

#include <csignal>

#include "FicsItNetworksComputer.h"
#include "FicsItReflection.h"
#include "FINEventFilter.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FINNetworkUtils.h"
#include "LuaFuture.h"
#include "LuaStruct.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "Signals/FINSignalSubsystem.h"
#include "FicsItKernel/FicsItKernel.h"

UE_DISABLE_OPTIMIZATION_SHIP

FFINLuaEventQueue::FFINLuaEventQueue(UFINKernelSystem* Kernel, int64 Key): Kernel(Kernel), Key(Key) {
	Kernel->AddReferencer(this, &CollectReferences);
}

FFINLuaEventQueue::FFINLuaEventQueue(const FFINLuaEventQueue& Other): Events(Other.Events), Filter(Other.Filter), Kernel(Other.Kernel), Key(Other.Key) {
	Kernel->AddReferencer(this, &CollectReferences);
}

FFINLuaEventQueue::~FFINLuaEventQueue() {
	Kernel->RemoveReferencer(this);
}

void FFINLuaEventQueue::CollectReferences(void* Obj, FReferenceCollector& Collector) {
	FFINLuaEventQueue* Self = static_cast<FFINLuaEventQueue*>(Obj);
	Collector.AddPropertyReferencesWithStructARO(StaticStruct(), Self);
}

namespace FINLua {
	void luaFIN_pushEventQueueInternal(lua_State* L, const FEventQueue& queueRef);

	LuaModule(R"(/**
	 * @LuaModule		Event
	 * @DisplayName		Event Module
	 * @Dependency		ReflectionSystemStructModule
	 */)", Event) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	EventQueue
		 * @DisplayName		Event Queue
		 */)", EventQueue) {
			FEventQueue& close(lua_State* L) {
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);
				if (queue->Key >= 0) {
					FFINLuaEventRegistry& registry = luaFIN_getEventRegistry(L);
					registry.EventQueues.Remove(queue->Key);
					lua_getiuservalue(L, -1, 2);
					lua_pushnil(L);
					lua_seti(L, -2, queue->Key);
					lua_pop(L, 2);
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
				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;

				const FFIRInstancedStruct& Struct = *Storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)));

				luaFIN_pushEventQueueInternal(L, MakeShared<FFINLuaEventQueue>(Struct.Get<FFINLuaEventQueue>()));
				lua_pushvalue(L, lua_upvalueindex(2));
				lua_setiuservalue(L, -2, 1);

				return 1;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 */)", __persist) {
				FEventQueue& queue = luaFIN_checkEventQueue(L, 1);

				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;
				lua_pushinteger(L, Storage.Add(MakeShared<FIRStruct>(*queue)));
				lua_getiuservalue(L, 1, 1);

				lua_pushcclosure(L, &unpersist, 2);

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
				const UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
				UObject* obj = *o;
				if (!IsValid(obj)) luaL_error(L, "object is not valid");
				AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
				SigSubSys->Listen(obj, o.Reverse() / net->GetComponent().GetObject());
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		listen(Object...)
			 * @DisplayName		Listen
			 *
			 * Adds the running lua context to the listen queue of the given components.
			 *
			 * @param	objects		Object...	A list of objects the computer should start listening to.
			 */)", listen) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					FFIRTrace trace = luaFIN_checkObject(L, i, nullptr);
					luaListen(L, trace);
				}
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Object[]	listening()
			 * @DisplayName		Listening
			 *
			 * Returns all signal senders this computer is listening to.
			 *
			 * @return	listening	Object[]	An array containing all objects this computer is currently listening to.
			 */)", listening) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);

				UObject* netComp = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->GetComponent().GetObject();

				TArray<UObject*> Listening = AFINSignalSubsystem::GetSignalSubsystem(netComp)->GetListening(netComp);
				int i = 0;
				lua_newtable(L);
				for (UObject* Obj : Listening) {
					luaFIN_pushObject(L, FFIRTrace(netComp) / Obj);
					lua_seti(L, -2, ++i);
				}
				return 1;
			}

			// ReSharper disable once CppParameterNeverUsed
			int luaPullContinue(lua_State* L, int status, lua_KContext ctx) {
				const int args = lua_gettop(L);

				return args - ctx;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(string|nil, Object, ...)	pull([timeout: number])
			 * @DisplayName		Pull
			 *
			 * Waits for a signal in the queue. Blocks the execution until a signal got pushed to the signal queue, or the timeout is reached. +
			 * Returns directly if there is already a signal in the queue (the tick doesn’t get yielded).
			 *
			 * @parameter	timeout		number			Timeout		The amount of time needs to pass until pull unblocks when no signal got pushed. If not set, the function will block indefinitely until a signal gets pushed. If set to `0` (int), will not yield the tick and directly return with the signal data or nil if no signal was in the queue.
			 * @return		event		string|nil		Event		The name of the returned signal. Nil when timeout got reached.
			 * @return		sender		Object			Sender		The component representation of the signal sender. Not set when timeout got reached.
			 * @return		parameters	any...			Parameters	The parameters passed to the signal. Not set when timeout got reached.
			 */)", pull) {
				const int args = lua_gettop(L);
				double t = 0.0;
				if (args > 0) t = lua_tonumber(L, 1);

				UFINLuaProcessor* luaProc = UFINLuaProcessor::luaGetProcessor(L);
				check(luaProc);
				const int a = luaProc->DoSignal(L);
				if (!a && !(args > 0 && lua_isinteger(L, 1) && lua_tointeger(L, 1) == 0)) {
					luaProc->Timeout = t;
					luaProc->PullStart =  (FDateTime::Now() - FFicsItNetworksComputerModule::GameStart).GetTotalMilliseconds();
					luaProc->PullState = (args > 0) ? 1 : 2;

					luaProc->GetTickHelper().shouldWaitForSignal();

					return lua_yieldk(L, 0, args, luaPullContinue);
				}
				luaProc->PullState = 0;
				return UFINLuaProcessor::luaAPIReturn(L, a);
			}

			void luaIgnore(lua_State* L, FFIRTrace o) {
				UObject* obj = *o;
				if (!IsValid(obj)) luaL_error(L, "object is not valid");
				AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
				SigSubSys->Ignore(obj, *o.Reverse());
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ignore(Object...)
			 * @DisplayName		Ignore
			 *
			 * Removes the running lua context from the listen queue of the given components. Basically the opposite of listen.
			 *
			 * @param	objects		Object...	A list of objects this computer should stop listening to.
			 */)", ignore) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					FFIRTrace Trace = luaFIN_checkObject(L, i, nullptr);
					luaIgnore(L, Trace);
				}
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ignoreAll()
			 * @DisplayName		Ignore All
			 *
			 * Stops listening to any signal sender. If afterwards there are still coming signals in, it might be the system itself or caching bug.
			 */)", ignoreAll) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);
				UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
				AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(net->GetComponent().GetObject());
				SigSubSys->IgnoreAll(net->GetComponent().GetObject());
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		clear()
			 * @DisplayName		Clear
			 *
			 * Clears every signal from the signal queue.
			 */)", clear) {
				UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->ClearSignals();
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		EventFilter		filter(...)
			 * @DisplayName		Filter
			 *
			 * Creates an Event filter expression.
			 */)", filter) {
				luaL_checktype(L, 1, LUA_TTABLE);
				FFINEventFilter filter;
				if (lua_getfield(L, 1, "event") != LUA_TNIL) {
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
				if (lua_getfield(L, 1, "sender") != LUA_TNIL) {
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
				if (lua_getfield(L, 1, "values") != LUA_TNIL) {
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
				FFINEventFilterExpression expression(filter);
				luaFIN_pushStruct(L, expression);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		integer		registerListener(EventFilter, function(event, sender, ...))
			 * @DisplayName		Register Listener
			 *
			 * Registers the given function as a listener.
			 * When `event.pull()` pulls a signal from the queue, that matches the given Event-Filter,
			 * a Task will be created using the function and the signals parameters will be passed into the function.
			 */)", registerListener) {
				luaL_checktype(L, 2, LUA_TFUNCTION);
				FFINEventFilterExpression filter = luaFIN_checkStruct<FFINEventFilterExpression>(L, 1, false);
				FFINLuaEventRegistry& registry = luaFIN_getEventRegistry(L);
				int64 key = registry.FindNextKey(registry.EventListeners);
				registry.EventListeners.Add(key, filter);
				lua_getiuservalue(L, -1, 1);
				lua_pushvalue(L, 2);
				lua_seti(L, -2, key+1);
				lua_pushinteger(L, key);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		EventQueue		queue([EventFilter])
			 * @DisplayName		queue
			 *
			 * Creates a new event queue.
			 * When this variable closes or gets garbage collected, it will stop receiving signals.
			 */)", queue) {
				FFINEventFilterExpression filter;
				if (FFINEventFilterExpression* filterPtr = luaFIN_toStruct<FFINEventFilterExpression>(L, 1, false)) {
					filter = *filterPtr;
				}
				FFINLuaEventRegistry& registry = luaFIN_getEventRegistry(L);
				int64 key = registry.FindNextKey(registry.EventQueues);
				registry.EventQueues.Add(key, filter);
				lua_getiuservalue(L, -1, 2);
				luaFIN_pushEventQueue(L, key);
				lua_pushvalue(L, -1);
				lua_seti(L, -3, key+1);
				return 1;
			}
		}

		LuaModulePostSetup() {
			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(event::luaPullContinue)));
			luaFIN_persistValue(L, -1, "Event-luaPullContinue");
			lua_pop(L, 1);

			lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(reinterpret_cast<void*>(EventQueue::unpersist)));
			luaFIN_persistValue(L, -1, "EventQueue-unpersist");
			lua_pop(L, 1);

			lua_getfield(L, LUA_REGISTRYINDEX, "hidden-globals");
			luaFIN_pushStruct(L, FFINLuaEventRegistry(), 2);
			lua_newtable(L);
			lua_setiuservalue(L, -2, 1);
			lua_newtable(L);
			lua_setiuservalue(L, -2, 2);
			lua_setfield(L, -2, "event-registry");
			lua_pop(L, 1);
		}
	}

	void luaFIN_pushEventQueueInternal(lua_State* L, const FEventQueue& queueRef) {
		FEventQueue* queue = static_cast<FEventQueue*>(lua_newuserdatauv(L, sizeof(FEventQueue), 1));
		new (queue) FEventQueue(queueRef);
		luaL_setmetatable(L, Event::EventQueue::_Name);
	}

	FEventQueue luaFIN_pushEventQueue(lua_State* L, int64 key) {
		TSharedRef<FFINLuaEventQueue> queueRef = MakeShared<FFINLuaEventQueue>(UFINLuaProcessor::luaGetProcessor(L)->GetKernel(), key);
		luaFIN_pushEventQueueInternal(L, queueRef);
		lua_newtable(L);
		lua_setiuservalue(L, -2, 1);
		return queueRef;
	}

	FEventQueue* luaFIN_toEventQueue(lua_State* L, int index) {
		return static_cast<FEventQueue*>(luaL_testudata(L, index, Event::EventQueue::_Name));
	}

	FEventQueue& luaFIN_checkEventQueue(lua_State* L, int index) {
		FEventQueue* queue = luaFIN_toEventQueue(L, index);
		if (!queue) luaFIN_typeError(L, index, Event::EventQueue::_Name);
		return *queue;
	}

	FFINLuaEventRegistry& luaFIN_getEventRegistry(lua_State* L) {
		lua_getfield(L, LUA_REGISTRYINDEX, "hidden-globals");
		lua_getfield(L, -1, "event-registry");
		FFINLuaEventRegistry& registry = luaFIN_checkStruct<FFINLuaEventRegistry>(L, -1, false);
		lua_remove(L, -2);
		return registry;
	}

	void luaFIN_handleEvent(lua_State* L, const FFIRTrace& sender, const FFINSignalData& data) {
		FFINLuaEventRegistry& registry = luaFIN_getEventRegistry(L);
		lua_getiuservalue(L, -1, 1);
		for (const auto& [key, filter] : registry.EventListeners) {
			if (filter.Matches(sender.GetUnderlyingPtr(), data)) {
				lua_geti(L, -1, key+1);
				luaFIN_pushFString(L, data.Signal->GetInternalName());
				luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(sender));
				for (const FFIRAnyValue& Value : data.Data) {
					luaFIN_pushNetworkValue(L, Value, sender);
				}
				luaFIN_pushLuaFutureLuaFunction(L, 2 + data.Data.Num());
				luaFIN_addTask(L, -1);
				lua_pop(L, 2);
			}
		}
		lua_pop(L, 1);
		luaFINDebug_dumpStack(L);
		lua_getiuservalue(L, -1, 2);
		for (const auto& [key, filter] : registry.EventQueues) {
			if (filter.Matches(sender.GetUnderlyingPtr(), data)) {
				lua_geti(L, -1, key+1);
				FEventQueue& queue = luaFIN_checkEventQueue(L, -1);
				queue->AddEvent(sender, data);
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);
	}
}
UE_ENABLE_OPTIMIZATION_SHIP