#include "LuaStructs.h"

#include "LuaInstance.h"

namespace FicsItKernel {
	namespace Lua {

		// Begin FInventoryItem

		int luaItemEQ(lua_State* L) {
			lua_getfield(L, 1, "type");
			lua_getfield(L, 2, "type");
			lua_pushboolean(L, lua_compare(L, 3, 4, LUA_OPEQ));
			return 1;
		}

		static const luaL_Reg luaItemLib[] = {
			{"__eq", luaItemEQ},
			{NULL, NULL}
		};

		void luaStruct(lua_State* L, FInventoryItem item) {
			lua_newtable(L);
			luaL_setmetatable(L, "Item");
			newInstance(L, item.ItemClass);
			lua_setfield(L, -2, "type");
			// TODO: Add Item state to struct
			//luaItemState(L, item.ItemState);
			//lua_setfield(L, -2, "state");
		}

		// End FInventoryItem

		// Begin FItemAmount

		int luaItemAmountEQ(lua_State* L) {
			lua_getfield(L, 1, "count");
			lua_getfield(L, 2, "count");
			lua_getfield(L, 1, "item");
			lua_getfield(L, 2, "item");
			lua_pushboolean(L, lua_compare(L, 3, 4, LUA_OPEQ) && lua_compare(L, 5, 6, LUA_OPEQ));
			return 1;
		}

		static const luaL_Reg luaItemAmountLib[] = {
			{"__eq", luaItemAmountEQ},
			{NULL, NULL}
		};

		void luaStruct(lua_State* L, FItemAmount amount) {
			lua_newtable(L);
			luaL_setmetatable(L, "ItemAmount");
			lua_pushinteger(L, amount.Amount);
			lua_setfield(L, -2, "count");
			newInstance(L, amount.ItemClass);
			lua_setfield(L, -2, "item");
		}

		// End FItemAmount

		// Begin FInventoryStack

		int luaItemStackEQ(lua_State* L) {
			lua_getfield(L, 1, "count");
			lua_getfield(L, 2, "count");
			lua_getfield(L, 1, "item");
			lua_getfield(L, 2, "item");
			lua_pushboolean(L, lua_compare(L, 3, 4, LUA_OPEQ) && lua_compare(L, 5, 6, LUA_OPEQ));
			return 1;
		}

		static const luaL_Reg luaItemStackLib[] = {
			{"__eq", luaItemStackEQ},
			{NULL, NULL}
		};

		void luaStruct(lua_State* L, FInventoryStack stack) {
			lua_newtable(L);
			luaL_setmetatable(L, "ItemStack");
			lua_pushinteger(L, stack.NumItems);
			lua_setfield(L, -2, "count");
			luaStruct(L, stack.Item);
			lua_setfield(L, -2, "item");
		}

		// End FInventoryStack

		void FicsItKernel::Lua::setupStructs(lua_State* L) {
			PersistSetup("Structs", -2);
			
			luaL_newmetatable(L, "Item");
			luaL_setfuncs(L, luaItemLib, 0);
			PersistTable("Item", -1);
			lua_pop(L, 1);

			luaL_newmetatable(L, "ItemAmount");
			luaL_setfuncs(L, luaItemAmountLib, 0);
			PersistTable("ItemAmount", -1);
			lua_pop(L, 1);

			luaL_newmetatable(L, "ItemStack");
			luaL_setfuncs(L, luaItemStackLib, 0);
			PersistTable("ItemStack", -1);
			lua_pop(L, 1);
		}
	}
}
