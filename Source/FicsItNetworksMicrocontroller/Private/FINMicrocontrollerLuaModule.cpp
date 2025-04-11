#include "FINMicrocontrollerLuaModule.h"

#include "FINComputerNetworkCard.h"
#include "FINLuaRuntime.h"
#include "FINLuaModule.h"
#include "LuaObject.h"
#include "FINMicrocontroller.h"
#include "FINNetworkCircuit.h"
#include "LuaPersistence.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		MicrocontrollerModule
	 * @DisplayName		Microcontroller Module
	 * @Dependency		ReflectionSystemObjectModule
	 *
	 * This Module provides an interface to interact with the Microcontroller.
	 */)", MicrocontrollerModule) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		microcontroller
		 * @DisplayName		Microcontroller Library
		 */)", microcontroller) {
			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	self	FINMicrocontroller
			 * @DisplayName		Self
			 */)", self) {
				AFINMicrocontroller* microcontroller = luaFIN_getMicrocontroller(L);
				luaFIN_pushObject(L, FIRTrace(microcontroller));
				luaFIN_persistValue(L, -1, "Microcontroller_self");
			}

			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	component	Object
			 * @DisplayName		Component
			 */)", component) {
				AFINMicrocontroller* microcontroller = luaFIN_getMicrocontroller(L);
				luaFIN_pushObject(L, FIRTrace(microcontroller->NetworkComponent));
				luaFIN_persistValue(L, -1, "Microcontroller_component");
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		setStorage(string)
			 * @DisplayName		Set Storage
			 */)", setStorage) {
				FString storage = luaFIN_convToFString(L, 1);
				AFINMicrocontroller* microcontroller = luaFIN_getMicrocontroller(L);
				microcontroller->SetStorage(storage);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		getStorage(string)
			 * @DisplayName		Get Storage
			 */)", getStorage) {
				AFINMicrocontroller* microcontroller = luaFIN_getMicrocontroller(L);
				luaFIN_pushFString(L, microcontroller->GetStorage());
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		open(integer...)
			 * @DisplayName		Open
			 */)", open) {
				AFINMicrocontroller* mc = luaFIN_getMicrocontroller(L);
				if (!IsValid(mc->NetworkCircuit)) throw FFIRException(TEXT("The Network Component this Microcontroller is attached to does not support network messaging!"));

				int top = lua_gettop(L);
				for (int i = 1; i <= top; ++i) {
					int port = luaL_checkinteger(L, i);
					if (port < 0 || port > 10000) continue;
					mc->OpenPorts.Add(port);
				}

				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		close(integer...)
			 * @DisplayName		Close
			 */)", close) {
				AFINMicrocontroller* mc = luaFIN_getMicrocontroller(L);
				if (!IsValid(mc->NetworkCircuit)) throw FFIRException(TEXT("The Network Component this Microcontroller is attached to does not support network messaging!"));

				int top = lua_gettop(L);
				for (int i = 1; i <= top; ++i) {
					int port = luaL_checkinteger(L, i);
					mc->OpenPorts.Remove(port);
				}

				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		closeAll()
			 * @DisplayName		Close All
			 */)", closeAll) {
				AFINMicrocontroller* mc = luaFIN_getMicrocontroller(L);
				if (!IsValid(mc->NetworkCircuit)) throw FFIRException(TEXT("The Network Component this Microcontroller is attached to does not support network messaging!"));
				mc->OpenPorts.Empty();
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		send()
			 * @DisplayName		Send
			 */)", send) {
				AFINMicrocontroller* mc = luaFIN_getMicrocontroller(L);
				if (!IsValid(mc->NetworkCircuit)) throw FFIRException(TEXT("The Network Component this Microcontroller is attached to does not support network messaging!"));

				FString receiver = luaFIN_toFString(L, 1);
				int port = luaL_checkinteger(L, 2);
				if (port < 0 || port > 10000) return 0;

				TArray<FFIRAnyValue> args;
				int top = lua_gettop(L);
				for (int i = 3; i <= top; ++i) {
					args.Add(luaFIN_toNetworkValue(L, i).Get(FFIRAnyValue()));
				}

				if (!AFINComputerNetworkCard::CheckNetMessageData(args)) return 0;
				FGuid receiverID;
				FGuid::Parse(receiver, receiverID);
				if (!receiverID.IsValid()) return 0;
				UObject* Obj = mc->NetworkCircuit->FindComponent(receiverID, nullptr).GetObject();
				IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Obj);
				FGuid MsgID = FGuid::NewGuid();
				FGuid SenderID = mc->ID;
				if (NetMsgI) {
					// send to specific component directly
					NetMsgI->HandleMessage(MsgID, SenderID, receiverID, port, args);
				} else {
					// distribute to all routers
					for (UObject* Router : mc->NetworkCircuit->GetComponents()) {
						IFINNetworkMessageInterface* MsgI = Cast<IFINNetworkMessageInterface>(Router);
						if (!MsgI || !MsgI->IsNetworkMessageRouter()) continue;
						MsgI->HandleMessage(MsgID, SenderID, receiverID, port, args);
					}
				}
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		broadcast(integer, ...)
			 * @DisplayName		Broadcast
			 */)", broadcast) {
				AFINMicrocontroller* mc = luaFIN_getMicrocontroller(L);
				if (!IsValid(mc->NetworkCircuit)) throw FFIRException(TEXT("The Network Component this Microcontroller is attached to does not support network messaging!"));

				int port = luaL_checkinteger(L, 1);
				if (port < 0 || port > 10000) return 0;

				TArray<FFIRAnyValue> args;
				int top = lua_gettop(L);
				for (int i = 2; i <= top; ++i) {
					args.Add(luaFIN_toNetworkValue(L, i).Get(FFIRAnyValue()));
				}
				if (!AFINComputerNetworkCard::CheckNetMessageData(args)) return 0;

				FGuid MsgID = FGuid::NewGuid();
				FGuid SenderID = mc->ID;
				for (UObject* Component : mc->NetworkCircuit->GetComponents()) {
					IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Component);
					if (NetMsgI) {
						NetMsgI->HandleMessage(MsgID, SenderID, FGuid(), port, args);
					}
				}
				return 0;
			}
		}
	}

	void luaFIN_setMicrocontroller(lua_State* L, AFINMicrocontroller* microcontroller) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		runtime.GlobalPointers.Add(TEXT("microcontroller"), microcontroller);
	}

	AFINMicrocontroller* luaFIN_getMicrocontroller(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		AFINMicrocontroller** value = reinterpret_cast<AFINMicrocontroller**>(runtime.GlobalPointers.Find(TEXT("microcontroller")));
		fgcheck(value != nullptr);
		return *value;
	}
}
