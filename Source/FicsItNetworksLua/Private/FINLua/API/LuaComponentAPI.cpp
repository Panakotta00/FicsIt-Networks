#include "FINLua/API/LuaComponentAPI.h"

#include <string>
#include <vector>

#include "FINChallengeSubsystem.h"
#include "FINComputerCase.h"
#include "FINLuaThreadedRuntime.h"
#include "FINNetworkUtils.h"
#include "NetworkController.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/Reflection/LuaObject.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		ComponentModule
	 * @DisplayName		Component Module
	 * @Dependency		FullReflectionModule
	 *
	 * The Component Module contains the component Library.
	 */)", ComponentModule) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		component
		 * @DisplayName		Component Library
		 *
		 * The component library contains functions that allow interaction with the component network.
		 */)", ComponentLibrary) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(Object | Object[])...	proxy((id: string | ids: string[])...)
			 * @DisplayName		Proxy
			 *
			 * Generates and returns instances of the network components with the given UUIDs. +
			 * You can pass any amount of parameters and each parameter will then have a corresponding return value. +
			 * Each parameter can be either a string, or an array of strings.
			 * These strings should then contain just the UUID of a network component. +
			 * If a network component cannot be found for a given string, nil will be used for the return.
			 * Otherwise, an instance of the network component will be returned. +
			 * If a parameter is a string array, the return value will be an array of network component instances.
			 *
			 * @parameter	...		string | string[]			ID[s]		The UUID[-Arrays] of the network component[s].
			 * @return		...		Object | Object[] | nil		Object[s]	The Network-Component[-Array]s associated with the UUIDs, nil if the UUID was not found.
			 */)", proxy) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSync SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					const bool isT = lua_istable(L, i);

					std::vector<std::string> ids;
					if (isT) {
						const auto count = lua_rawlen(L, i);
						for (int j = 1; j <= count; ++j) {
							lua_geti(L, i, j);
							if (!lua_isstring(L, -1)) return luaL_argerror(L, i, "array contains non-string");
							ids.push_back(lua_tostring(L, -1));
							lua_pop(L, 1);
						}
						lua_newtable(L);
					} else {
						if (!lua_isstring(L, i)) return luaL_argerror(L, i, "is not string");
						ids.push_back(lua_tostring(L, i));
					}
					int j = 0;
					for (const auto& id : ids) {
						IFINLuaComponentNetworkAccessInterface* network = luaFIN_getComponentNetwork(L);
						FGuid UUID;
						FGuid::Parse(FString(id.c_str()), UUID);
						FFIRTrace comp;
						try {
							comp = network->GetComponentByID(UUID);
						} catch (FFINLuaPanic panic) {
							luaFIN_pushFString(L, panic.Message);
							return lua_error(L);
						}
						FFIRTrace obj = UFINNetworkUtils::RedirectIfPossible(comp);
						luaFIN_pushObject(L, obj);
						FINChallenge(ComponentProxy, true);
						FINChallenge(ProxyComputer, obj && obj->IsA<AFINComputerCase>());
						if (isT) lua_seti(L, -2, ++j);
					}
				}
				return args;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		string[]...		findComponent((string query | Class type)...)
			 * @DisplayName		Find Component
			 *
			 * Searches the component network for components with the given query or have the given type. +
			 * You can pass multiple parameters and each parameter will be handled separately and returns a corresponding return value.
			 *
			 * @parameter	...		string | Object-Class	Query	A nick/group query as string or a class for the components in the network you try to find.
			 * @return		...		string[]				UUIDs	List of network component UUIDs which pass the given nick query or are of the given type.
			 */)", findComponent) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSync SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					lua_newtable(L);
					TSet<FFIRTrace> comps;
					IFINLuaComponentNetworkAccessInterface* network = luaFIN_getComponentNetwork(L);
					if (lua_isstring(L, i)) {
						std::string nick = lua_tostring(L, i);
						comps = network->GetComponentByNick(nick.c_str());
					} else {
						UClass* Class = luaFIN_toUClass(L, i, nullptr);
						comps = network->GetComponentByClass(Class, true);
					}
					int j = 0;
					for (const FFIRTrace& comp : comps) {
						UObject* obj = *comp;
						if (obj) {
							++j;
							FString id = IFINNetworkComponent::Execute_GetID(obj).ToString();
							lua_pushstring(L, TCHAR_TO_UTF8(*id));
							lua_seti(L, -2, j);
						}
					}
				}
				return args;
			}
		}
	}

	void luaFIN_setComponentNetwork(lua_State* L, IFINLuaComponentNetworkAccessInterface* ComponentNetwork) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		runtime.GlobalPointers.Add(TEXT("component-network"), ComponentNetwork);
	}

	IFINLuaComponentNetworkAccessInterface* luaFIN_getComponentNetwork(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		IFINLuaComponentNetworkAccessInterface** value = reinterpret_cast<IFINLuaComponentNetworkAccessInterface**>(runtime.GlobalPointers.Find(TEXT("component-network")));
		fgcheck(value != nullptr);
		return *value;
	}
}
