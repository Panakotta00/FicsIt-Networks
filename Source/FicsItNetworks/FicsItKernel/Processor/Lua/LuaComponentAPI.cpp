#include "LuaComponentAPI.h"

#include <string>
#include <vector>

#include "Network/FINNetworkComponent.h"

#include "FicsItKernel/FicsItKernel.h"
#include "LuaProcessor.h"
#include "LuaInstance.h"

#include "FGBlueprintFunctionLibrary.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINClass.h"

namespace FicsItKernel {
	namespace Lua {
		#pragma optimize("", off)
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
					LuaProcessor* Processor = LuaProcessor::luaGetProcessor(L);
					FFINNetworkTrace comp = Processor->getKernel()->getNetwork()->getComponentByID(id.c_str());
					newInstance(L, UFINNetworkUtils::RedirectIfPossible(comp));
					if (isT) lua_seti(L, -2, ++j);
				}
			}
			return LuaProcessor::luaAPIReturn(L, args);
		}
		#pragma optimize("", on)

		int luaFindComponent(lua_State* L) {
			FLuaSyncCall SyncCall(L);
			int args = lua_gettop(L);

			for (int i = 1; i <= args; ++i) {
				lua_newtable(L);
				TSet<FFINNetworkTrace> comps;
				if (lua_isstring(L, i)) {
					std::string nick = lua_tostring(L, i);
					comps = LuaProcessor::luaGetProcessor(L)->getKernel()->getNetwork()->getComponentByNick(nick.c_str());
				} else {
					FFINNetworkTrace Obj = getObjInstance(L, i, UFINClass::StaticClass());
					UFINClass* FINClass = Cast<UFINClass>(Obj.Get());
					if (FINClass) {
						UClass* Class = Cast<UClass>(FINClass->GetOuter());
						comps = LuaProcessor::luaGetProcessor(L)->getKernel()->getNetwork()->getComponentByClass(Class, true);
					}
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
			return LuaProcessor::luaAPIReturn(L, args);
		}

		static const luaL_Reg luaComponentLib[] = {
			{"proxy", luaComponentProxy},
			{"findComponent", luaFindComponent},
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
