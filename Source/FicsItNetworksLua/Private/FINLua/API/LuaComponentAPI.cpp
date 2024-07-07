#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/Reflection/LuaObject.h"
#include "Network/FINNetworkUtils.h"

namespace FINLua {
#define LOCTEXT_NAMESPACE "ComponentModule"
	BeginLuaModule(component, LOCTEXT("DisplayName", "Component Module"), LOCTEXT("Description", "The Component Module contains the component Library."))
#define LOCTEXT_NAMESPACE "ComponentLibrary"
	BeginLibrary(component, LOCTEXT("DisplayName", "Component Library"), LOCTEXT("Description", "The component library contains functions that allow interaction with the component network."))

	LuaReturnSignature(proxy, "(Component | Component[])...");
	LuaParameterSignature(proxy, "(id: string | ids: string[])...");
	LuaParameter(proxy, 0, id, "string", LOCTEXT("proxy_id_DisplayName", "ID"), LOCTEXT("proxy_id_Description", "The UUID of the network component."))
	LuaParameter(proxy, 1, ids, "string[]", LOCTEXT("proxy_ids_DisplayName", "IDs"), LOCTEXT("proxy_ids_Description", "A array of UUIDs of the networks components."))
	LuaReturnValue(proxy, 0, Component, "Component", LOCTEXT("proxy_Component_DisplayName", "Component"), LOCTEXT("proxy_Component_Description", "The lua table representation of the network component."))
	LuaReturnValue(proxy, 1, Components, "Component[]", LOCTEXT("proxy_Components_DisplayName", "Components"), LOCTEXT("proxy_Components_Description", "A array of the lua table representation of the network component."))
	FieldFunction(proxy, LOCTEXT("proxy_DisplayName", "Proxy"), LOCTEXT("proxy_Description", R"(
		Generates and returns instances of the network components with the given UUIDs. +
		You can pass any amount of parameters and each parameter will then have a corresponding return value. +
		Each parameter can be either a string, or an array of strings.
		These strings should then contain just the UUID of a network component. +
		If a network component cannot be found for a given string, nil will be used for the return.
		Otherwise, an instance of the network component will be returned. +
		If a parameter is a string array, the return value will be an array of network component instances.
	)")) {
		// ReSharper disable once CppDeclaratorNeverUsed
		FLuaSyncCall SyncCall(L);
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
				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				FGuid UUID;
				FGuid::Parse(FString(id.c_str()), UUID);
				FFINNetworkTrace comp = Processor->GetKernel()->GetNetwork()->GetComponentByID(UUID);
				luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(comp));
				if (isT) lua_seti(L, -2, ++j);
			}
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}

	LuaReturnSignature(findComponent, "(string query | Class type )...")
	LuaParameterSignature(findComponent, "string[]...")
	LuaParameter(findComponent, 0, query, "string", LOCTEXT("findComponent_query_DisplayName", "Query"), LOCTEXT("findComponent_query_Description", "A nick/group query as string used to get a list of component in the network."))
	LuaParameter(findComponent, 1, type, "Class", LOCTEXT("findComponent_type_DisplayName", "Query"), LOCTEXT("findComponent_type_Description", "The type of the components in the network you want to get."))
	LuaReturnValue(findComponent, 0, uuids, "string[]", LOCTEXT("findComponent_uuids_DisplayName", "UUIDS"), LOCTEXT("findComponent_uuids_Description", "List of network component UUIDs which pass the given nick query or are of the given type."))
	FieldFunction(findComponent, LOCTEXT("findComponent_DisplayName", "Find Component"), LOCTEXT("findComponent_Description", R"(
		Searches the component network for components with the given query or have the given type. +
		You can pass multiple parameters and each parameter will be handled separately and returns a corresponding return value.
	)")) {
		// ReSharper disable once CppDeclaratorNeverUsed
		FLuaSyncCall SyncCall(L);
		const int args = lua_gettop(L);

		for (int i = 1; i <= args; ++i) {
			lua_newtable(L);
			TSet<FFINNetworkTrace> comps;
			if (lua_isstring(L, i)) {
				std::string nick = lua_tostring(L, i);
				comps = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->GetComponentByNick(nick.c_str());
			} else {
				UClass* Class = luaFIN_toUClass(L, i, nullptr);
				comps = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->GetComponentByClass(Class, true);
			}
			int j = 0;
			for (const FFINNetworkTrace& comp : comps) {
				UObject* obj = *comp;
				if (obj) {
					++j;
					FString id = IFINNetworkComponent::Execute_GetID(obj).ToString();
					lua_pushstring(L, TCHAR_TO_UTF8(*id));
					lua_seti(L, -2, j);
				}
			}
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}

	EndLibrary()

	EndLuaModule()
}
