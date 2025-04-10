#include "FINMicrocontrollerLuaModule.h"

#include "FINLuaRuntime.h"
#include "FINLuaModule.h"
#include "LuaObject.h"
#include "FINMicrocontroller.h"

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
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Object	getSelf()
			 * @DisplayName		Get Self
			 *
			 * Returns an Object reference to the Network Component this Microcontroller is attached to.
			 */)", getSelf) {
				AFINMicrocontroller* microcontroller = luaFIN_getMicrocontroller(L);
				luaFIN_pushObject(L, FIRTrace(microcontroller->NetworkComponent));
				return 1;
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
