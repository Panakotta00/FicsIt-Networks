#include "LuaComputerAPI.h"

#include "LuaInstance.h"

#define LuaFunc(funcName) \
int funcName(lua_State* L) { \
	KernelSystem* kernel = *(KernelSystem**)lua_touserdata(L, lua_upvalueindex(1));


namespace FicsItKernel {
	namespace Lua {
		LuaFunc(luaComputerGetInstance)
			newInstance(L, Network::NetworkTrace(kernel->getNetwork()->component));
			return 1;
		}

		LuaFunc(luaComputerReset)
			kernel->reset();
			return 0;
		}

		LuaFunc(luaComputerStop)
			kernel->stop();
			return 0;
		}

		LuaFunc(luaComputerBeep)
			// TODO: do the beep
			return 0;
		}

		static const luaL_Reg luaComputerLib[] = {
			{"getInstance", luaComputerGetInstance},
			{"reset", luaComputerReset},
			{"stop", luaComputerStop},
			{"beep", luaComputerBeep},
			{NULL,NULL}
		};
		
		void setupComputerAPI(lua_State* L, KernelSystem* kernel) {
			PersistSetup("Computer", -2);
			luaL_newlibtable(L, luaComputerLib);
			auto& fs_ud = *(KernelSystem**)lua_newuserdata(L, sizeof(void*));
			fs_ud = kernel;
			luaL_setfuncs(L, luaComputerLib, 1);
			PersistTable("Lib", -1);
			lua_setglobal(L, "computer");
		}
	}
}