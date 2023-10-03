#include "LuaProcessor/LuaObject.h"

namespace FINLua {
	void setupObjectSystem(lua_State* L) {
		/*PersistSetup("InstanceSystem", -2);
		
		luaL_newmetatable(L, INSTANCE_TYPE);			// ..., InstanceTypeMeta
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "__metatable");
		luaL_setfuncs(L, luaInstanceTypeLib, 0);
		PersistTable(INSTANCE_TYPE, -1);
		lua_pop(L, 1);									// ...

		lua_register(L, "findClass", luaFindClass);
		PersistGlobal("findClass");

		lua_pushcfunction(L, luaInstanceFuncCall);			// ..., InstanceFuncCall
		PersistValue("InstanceFuncCall");					// ...
		lua_pushcfunction(L, luaClassInstanceFuncCall);		// ..., LuaClassInstanceFuncCall
		PersistValue("ClassInstanceFuncCall");				// ...
		lua_pushcfunction(L, luaInstanceUnpersist);			// ..., LuaInstanceUnpersist
		PersistValue("InstanceUnpersist");					// ...
		lua_pushcfunction(L, luaClassInstanceUnpersist);		// ..., LuaClassInstanceUnpersist
		PersistValue("ClassInstanceUnpersist");			// ...
		lua_pushcfunction(L, luaInstanceTypeUnpersist);		// ..., LuaInstanceTypeUnpersist
		PersistValue("InstanceTypeUnpersist");				// ...
		*/
	}
}
