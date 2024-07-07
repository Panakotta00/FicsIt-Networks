#include "FINLua/FINLuaModule.h"

#include "FicsItNetworksLuaModule.h"
#include "FINLua/LuaExtraSpace.h"
#include "FINLua/LuaPersistence.h"
#include "Logging/StructuredLog.h"

void FFINLuaModuleBareValue::PushLuaValue(lua_State* L, const FString& PersistName) {
	Function(L, PersistName);
}

void FFINLuaFunction::PushLuaValue(lua_State* L, const FString& PersistName) {
	lua_pushnil(L);
	lua_pushcclosure(L, Function, 1);

	FINLua::luaFIN_persistValue(L, -1, PersistName);
}

void FFINLuaTable::PushLuaValue(lua_State* L, const FString& PersistName) {
	lua_createtable(L, 0, Fields.Num());	// table

	for (FFINLuaTableField& field : Fields) {
		if (!field.Value.IsValid()) continue;

		FINLua::luaFIN_pushFString(L, field.Key);	// table, string
		field.Value->PushLuaValue(L, PersistName + TEXT("-") + field.Key);	// table, string, value

		if (field.Value->TypeID() == FINTypeId<FFINLuaFunction>::ID() && PersistName != TEXT("ModuleSystem-Metatable-ModuleTableFunction")) {
			lua_pushlightuserdata(L, &field);
			lua_setupvalue(L, -2, 1);
			luaL_setmetatable(L, "ModuleTableFunction");
		}

		lua_settable(L, -3);	// table
	}
}

void FFINLuaModule::SetupModule(lua_State* L) {
	PreSetup.ExecuteIfBound(*this, L);

	PersistenceNamespace(InternalName);

	for (const FFINLuaMetatable& metatable : Metatables) {
		FINLua::luaFIN_pushFString(L, metatable.InternalName);	// string
		lua_pushvalue(L, -1);	// string, string
		if (lua_gettable(L, LUA_REGISTRYINDEX) != LUA_TNIL) {	// string, nil|any
			lua_pop(L, 2);	//
			UE_LOGFMT(LogFicsItNetworksLuaPersistence, Warning, "Failed to create Metatable '{MetatableName}' of Module '{ModuleName}'! Probably already exists!", metatable.InternalName, InternalName);
			continue;
		}
		lua_pop(L, 1);	// string

		metatable.Table->PushLuaValue(L, InternalName + TEXT("-Metatable-") + metatable.InternalName); // string, Metatable

		lua_pushvalue(L, -2);	// string, Metatable, string
		lua_setfield(L, -2, "__name");	// string, Metatable
		lua_pushvalue(L, -2); // string, Metatable, string
		lua_pushvalue(L, -2); // string, Metatable, string, Metatable
		lua_settable(L, LUA_REGISTRYINDEX); // string, Metatable

		//FINLua::luaFIN_pushFString(L, Metatable.InternalName);
		//lua_setfield(L, -2, "__metatable);

		PersistValue(metatable.InternalName + TEXT("-Metatable"));	// string
		lua_pop(L, 1); //
	}

	lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);	// Globals
	for (const FFINLuaGlobal& global : Globals) {
		FINLua::luaFIN_pushFString(L, global.InternalName);	// Globals, string

		lua_pushvalue(L, -1);	// Globals, string, string
		if (lua_gettable(L, -2) > 0) {	// Globals, string, nil|any
			lua_pop(L, 2); //
			UE_LOGFMT(LogFicsItNetworksLuaPersistence, Warning, "Failed to add Global Library '{LibraryName}' of Module '{ModuleName}'! Probably already exists!", global.InternalName, InternalName);
			continue;
		}
		lua_pop(L, 1);	// Globals, string

		global.Value->PushLuaValue(L, InternalName + TEXT("-Global-") + global.InternalName);	// Globals, string, Value

		lua_settable(L, 1);	// Globals
	}
	lua_pop(L, 1);	//

	PostSetup.ExecuteIfBound(*this, L);

	FINLua::luaFIN_getExtraSpace(L).LoadedModules.Add(AsShared());
}

void FFINLuaModule::AddLuaFunctionsToTable(lua_State* L, int index, const TArray<FFINLuaFunction>& Functions) {

}

FFINLuaModuleRegistry& FFINLuaModuleRegistry::GetInstance() {
	static FFINLuaModuleRegistry registry;
	return registry;
}

namespace FINLua {
#define LOCTEXT_NAMESPACE "ModuleSystemModule"
	BeginLuaModule(ModuleSystem, LOCTEXT("DisplayName", "Module-System Module"), LOCTEXT("Description", ""))
#define LOCTEXT_NAMESPACE "ModuleTableFunction"
	BeginMetatable(ModuleTableFunction, LOCTEXT("DisplayName", "Module Table-Function"), LOCTEXT("Description", ""))

	FieldBare(name, LOCTEXT("name_DisplayName", "Name"), LOCTEXT("name_Description", "")) { lua_pushnil(L); }

	FieldFunction(__index, LOCTEXT("index_DisplayName", "Index"), LOCTEXT("index_Description", "")) {
		lua_getupvalue(L, 1, 1);
		if (!lua_isuserdata(L, -1)) {
			return 0;
		}
		FFINLuaTableField* field = static_cast<FFINLuaTableField*>(lua_touserdata(L, -1));

		FString key = luaFIN_checkFString(L, 2);
		if (key == TEXT("name")) {
			luaFIN_pushFString(L, field->Key);
		} else {
			return 0;
		}

		return 1;
	}

	EndMetatable()

	EndLuaModule()
}