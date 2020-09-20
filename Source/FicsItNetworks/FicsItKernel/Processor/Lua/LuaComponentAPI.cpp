#include "LuaComponentAPI.h"

#include <string>
#include <vector>

#include "Network/FINNetworkComponent.h"

#include "FicsItKernel/FicsItKernel.h"
#include "LuaProcessor.h"
#include "LuaInstance.h"

#include "FGBlueprintFunctionLibrary.h"
#include "FGRecipeManager.h"

namespace FicsItKernel {
	namespace Lua {
		int luaComponentProxy(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				bool isT = lua_istable(L, i);

				std::vector<std::string> ids;
				if (isT) {
					auto count = lua_rawlen(L, i);
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
				for (auto& id : ids) {
					FFINNetworkTrace comp = LuaProcessor::luaGetProcessor(L)->getKernel()->getNetwork()->getComponentByID(id.c_str());
					UObject* Obj = *comp;
					UObject* Org = Obj;
					if (Obj && Obj->Implements<UFINNetworkComponent>()) {
						UObject* Redirect = IFINNetworkComponent::Execute_GetInstanceRedirect(Obj);
						if (Redirect && Obj != Redirect) comp = comp / Redirect;
					}
					newInstance(L, comp, Org);
					if (isT) lua_seti(L, -2, ++j);
				}
			}
			return LuaProcessor::luaAPIReturn(L, args);
		}

		int luaFindComponent(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				lua_newtable(L);
				std::string nick = luaL_checkstring(L, i);
				TSet<FFINNetworkTrace> comps = LuaProcessor::luaGetProcessor(L)->getKernel()->getNetwork()->getComponentByNick(nick.c_str());
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
			return LuaProcessor::luaAPIReturn(L, args);
		}

		int luaFindItem(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			int nargs = lua_gettop(L);
			if (nargs < 1) return LuaProcessor::luaAPIReturn(L, 0);
			const char* str = luaL_tolstring(L, -1, 0);

			TArray<TSubclassOf<UFGItemDescriptor>> items;
			UFGBlueprintFunctionLibrary::Cheat_GetAllDescriptors(items);
			if (str) for (TSubclassOf<UFGItemDescriptor> item : items) {
				if (IsValid(item) && UFGItemDescriptor::GetItemName(item).ToString() == FString(str)) {
					newInstance(L, item);
					return LuaProcessor::luaAPIReturn(L, 1);
				}
			}

			lua_pushnil(L);
			return LuaProcessor::luaAPIReturn(L, 1);
		}

		static const luaL_Reg luaComponentLib[] = {
			{"proxy", luaComponentProxy},
			{"findComponent", luaFindComponent},
			{"findItem", luaFindItem},
			{NULL,NULL}
		};

		void setupComponentAPI(lua_State* L) {
			PersistSetup("Component", -2);
			lua_newtable(L);
			luaL_setfuncs(L, luaComponentLib, 0);
			PersistTable("Lib", -1);
			lua_setglobal(L, "component");
		}
	}
}