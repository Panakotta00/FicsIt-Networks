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
					auto comp = LuaProcessor::getCurrentProcessor()->getKernel()->getNetwork()->getComponentByID(id);
					newInstance(L, comp);
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
					UObject* obj = *comp;
					lua_pushstring(L, TCHAR_TO_UTF8(*Cast<IFINNetworkComponent>(obj)->Execute_GetID(obj).ToString()));
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
			if (str) for (TSubclassOf<UFGItemDescriptor> item : items) {
				if (IsValid(item) && UFGItemDescriptor::GetItemName(item).ToString() == FString(str)) {
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