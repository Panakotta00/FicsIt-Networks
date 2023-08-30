#include "FicsItKernel/Processor/Lua/LuaComponentAPI.h"
#include "FicsItKernel/Processor/Lua/LuaUtil.h"
#include "FicsItKernel/Processor/Lua/LuaProcessor.h"
#include "FicsItKernel/Processor/Lua/LuaInstance.h"
#include "Network/FINNetworkComponent.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINClass.h"
#include "FGBlueprintFunctionLibrary.h"

// ReSharper disable once IdentifierTypo
namespace FicsItKernel {
	namespace Lua {
				int luaComponentProxy(lua_State* L) {
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
					newInstance(L, UFINNetworkUtils::RedirectIfPossible(comp));
					if (isT) lua_seti(L, -2, ++j);
				}
			}
			return UFINLuaProcessor::luaAPIReturn(L, args);
		}
		
		int luaFindComponent(lua_State* L) {
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
					FFINNetworkTrace Obj = getObjInstance(L, i, UFINClass::StaticClass());
					UFINClass* FINClass = Cast<UFINClass>(Obj.Get());
					if (FINClass) {
						UClass* Class = Cast<UClass>(FINClass->GetOuter());
						comps = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->GetComponentByClass(Class, true);
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
			return UFINLuaProcessor::luaAPIReturn(L, args);
		}

		static const luaL_Reg luaComponentLib[] = {
			{"proxy", luaComponentProxy},
			{"findComponent", luaFindComponent},
			{nullptr, nullptr}
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
