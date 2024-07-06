#include "FINLua/FINLuaModule.h"

#include "FINLua/LuaPersistence.h"

void UFINLuaModule::SetupModule(lua_State* L) {
	for (const FFINLuaMetatable& Metatable : Metatables) {
		PersistenceNamespace("Component");
		lua_newtable(L);
		//luaL_setfuncs(L, luaComponentLib, 0);
		//PersistTable("Lib", -1);
		lua_setglobal(L, "component");
	}
}
