#include "LuaComponentAPI.h"

#include <string>
#include <vector>

#include "FicsItKernel/FicsItKernel.h"
#include "LuaProcessor.h"
#include "LuaInstance.h"

#include "FGBlueprintFunctionLibrary.h"
#include "FGRecipeManager.h"

namespace FicsItKernel {
	namespace Lua {
		int luaComponentProxy(lua_State* L) {
			int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				bool isT = lua_istable(L, i);

				std::vector<std::string> ids;
				if (isT) {
					auto count = lua_rawlen(L, i);
					for (int j = 1; j <= count; ++j) {
						lua_geti(L, i, j);
						ids.push_back(luaL_checkstring(L, -1));
						lua_pop(L, 1);
					}
					lua_newtable(L);
				} else ids.push_back(luaL_checkstring(L, i));

				int j = 0;
				for (auto& id : ids) {
					auto comp = LuaProcessor::getCurrentProcessor()->getKernel()->getNetwork()->getComponentByID(id);
					newInstance(L, Network::NetworkTrace());
					if (isT) lua_seti(L, -2, ++j);
				}
			}
			return args;
		}

		int luaFindComponent(lua_State* L) {
			int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				lua_newtable(L);
				std::string nick = luaL_checkstring(L, i);
				auto comps = LuaProcessor::getCurrentProcessor()->getKernel()->getNetwork()->getComponentByNick(nick);
				int j = 0;
				for (auto& comp : comps) {
					++j;
					lua_pushstring(L, TCHAR_TO_UTF8(*Cast<IFINNetworkComponent>(*comp)->GetID().ToString()));
					lua_seti(L, -2, j);
				}
			}
			return args;
		}

		int luaFindItem(lua_State* L) {
			int nargs = lua_gettop(L);
			if (nargs < 1) return 0;
			const char* str = luaL_tolstring(L, -1, 0);

			TArray<TSubclassOf<UFGItemDescriptor>> items;
			UFGBlueprintFunctionLibrary::Cheat_GetAllDescriptors(items);
			for (auto item : items) {
				if (TCHAR_TO_UTF8(*UFGItemDescriptor::GetItemName(item).ToString()) == str) {
					newInstance(L, item);
					return 1;
				}
			}

			lua_pushnil(L);
			return 1;
		}

		static const luaL_Reg luaComponentLib[] = {
			{"proxy", luaComponentProxy},
			{"findComponent", luaFindComponent},
			{"findItem", luaFindItem},
			{NULL,NULL}
		};

		void setupComponentAPI(lua_State* L) {
			lua_newtable(L);
			luaL_setfuncs(L, luaComponentLib, 0);
			lua_setglobal(L, "component");
		}
	}
}