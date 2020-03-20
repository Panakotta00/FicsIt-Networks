#include "LuaInstance.h"

#include "UObject/UnrealType.h"

#include "Network/FINNetworkComponent.h"

#define OffsetParam(type, off) (type*)((std::uint64_t)param + off)

namespace FicsItKernel {
	namespace Lua {
		std::map<UClass*, std::vector<std::pair<std::string, LuaLibFunc>>> instanceClasses;
		std::map<UClass*, std::vector<std::pair<std::string, LuaLibClassFunc>>> instanceSubclasses;

		Network::NetworkTrace getObjInstance(lua_State* L, int index, UClass* clazz) {
			if (!lua_istable(L, index)) return Network::NetworkTrace();
			lua_getfield(L, index, "__object");
			auto instance = (LuaInstance*)luaL_checkudata(L, -1, "InstanceObj");
			lua_pop(L, 1);
			if (!instance) return Network::NetworkTrace();
			UObject* obj = **instance;
			if (!(obj) || !obj->IsA(clazz)) return Network::NetworkTrace();
			return *instance;
		}

		UClass* getClassInstance(lua_State* L, int index, UClass* clazz) {
			if (!lua_istable(L, index)) return nullptr;
			lua_getfield(L, index, "__object");
			auto o = (UClass*)luaL_checkudata(L, -1, "InstanceClass");
			lua_pop(L, 1);
			if (!o || !o->IsChildOf(clazz)) return nullptr;
			return o;
		}

		int luaInstanceUFuncCall(lua_State* L) {
			int args = lua_gettop(L);

			// get data from closure
			auto& ud = *(LuaInstanceUFunc*)lua_touserdata(L, lua_upvalueindex(1));
			UObject* comp = *ud.first;
			UFunction* func = ud.second;

			// check object validity
			if (!comp) {
				luaL_error(L, "component is invalid");
				return 0;
			}

			// allocate parameter space
			void* params = malloc(func->ParmsSize);
			memset(params, 0, func->ParmsSize);

			// init and set parameter values
			std::string error = "";
			int i = 1;
			for (auto property = TFieldIterator<UProperty>(func); property; ++property) {
				auto flags = property->GetPropertyFlags();
				if (flags & CPF_Parm && !(flags & (CPF_OutParm | CPF_ReturnParm))) {
					try {
						luaToProperty(L, *property, params, i++);
					} catch (std::exception e) {
						error = "argument #" + std::to_string(i) + " is not of type " + e.what();
						break;
					}
				}
			}

			// execute native function only if no error
			if (error.length() <= 0) comp->ProcessEvent(func, params);

			// free parameter space
			for (auto property = TFieldIterator<UProperty>(func); property; ++property) {
				auto flags = property->GetFlags();
				if (flags & CPF_Parm && !(flags & (CPF_OutParm | CPF_ReturnParm))) {
					if (--i > 0) break;
					if (property->GetClass()->ClassCastFlags & CASTCLASS_UStrProperty) {
						property->ContainerPtrToValuePtr<FString>(params)->~FString();
					}
				}
			}
			if (error.length() > 0) {
				free(params);
				return luaL_error(L, std::string("Error at ").append(std::to_string(i).c_str()).append("# parameter: ").append(error).c_str());
			}

			// free parameter space and eventualy push return falues to lua
			i = 0;
			for (auto property = TFieldIterator<UProperty>(func); property; ++property) {
				auto flags = property->GetFlags();
				if (flags & (CPF_OutParm | CPF_ReturnParm)) {
					propertyToLua(L, *property, params);
					auto c = property->GetClass()->ClassCastFlags;
					if (c & CASTCLASS_UStrProperty) {
						property->ContainerPtrToValuePtr<FString>(params)->~FString();
					}
					++i;
				}
			}
			free(params);

			return i;
		}

		int luaInstanceFuncCall(lua_State* L) {
			auto& data = *(LuaInstanceFunc*) lua_touserdata(L, lua_upvalueindex(1));
			auto obj = data.first;
			auto func = data.second;

			if (!*obj) return luaL_error(L, "component is invalid");

			return func(L, lua_gettop(L), obj);
		}

		int luaClassInstanceFuncCall(lua_State* L) {
			int args = lua_gettop(L);

			auto& data = *(LuaClassInstanceFunc*) lua_touserdata(L, lua_upvalueindex(1));
			auto obj = data.first;
			auto func = data.second;
			func(L, lua_gettop(L), obj);
			return luaL_error(L, "invalid native function ptr");
		}

		void addCompFuncs(lua_State* L, Network::NetworkTrace obj) {
			auto comp = Cast<IFINNetworkComponent>(*obj);
			if (!comp) return;
			for (auto func = TFieldIterator<UFunction>(obj->GetClass()); func; ++func) {
				auto funcName = func->GetName();
				if (!(funcName.RemoveFromStart("netFunc_") && funcName.Len() > 0)) continue;
				auto comp_ud = (LuaInstanceUFunc*) lua_newuserdata(L, sizeof(LuaInstanceUFunc));
				new (comp_ud) LuaInstanceUFunc{obj, *func};
				luaL_setmetatable(L, "InstanceUFunc");
				lua_pushcclosure(L, luaInstanceUFuncCall, 1);
				lua_setfield(L, -2, TCHAR_TO_UTF8(*funcName));
			}
		}

		void addPreFuncs(lua_State* L, Network::NetworkTrace obj) {
			int j = 0;
			for (auto clazz : instanceClasses) {
				if (!obj->IsA(clazz.first)) continue;
				for (auto& func : clazz.second) {
					auto comp_ud = (LuaInstanceFunc*) lua_newuserdata(L, sizeof(LuaInstanceFunc));
					new (comp_ud) LuaInstanceFunc(obj, func.second);
					luaL_setmetatable(L, "InstanceFunc");
					lua_pushcclosure(L, luaInstanceFuncCall, 1);
					lua_setfield(L, -2, func.first.c_str());
				}
			}
		}

		bool newInstance(lua_State* L, Network::NetworkTrace obj) {
			if (!*obj) {
				lua_pushnil(L);
				return false;
			}

			lua_newtable(L);
			luaL_setmetatable(L, "Instance");

			auto ud_o = (LuaInstance*)lua_newuserdata(L, sizeof(LuaInstance));
			luaL_setmetatable(L, "Instance");
			new (ud_o) LuaInstance(obj);
			lua_setfield(L, -2, "__object");
			addPreFuncs(L, obj);
			addCompFuncs(L, obj);

			if (auto comp = Cast<IFINNetworkComponent>(*obj)) {
				FGuid id = comp->GetID();
				lua_pushstring(L, TCHAR_TO_UTF8(*id.ToString()));
				lua_setfield(L, -2, "id");
				FString nick = comp->GetNick();
				lua_pushstring(L, TCHAR_TO_UTF8(*nick));
				lua_setfield(L, -2, "nick");

				TSet<UObject*> merged = comp->GetMerged();
				for (auto m : merged) {
					if (!m) continue;
					addPreFuncs(L, obj(m));
					addCompFuncs(L, obj(m));
				}
			}
			return true;
		}

		bool newInstance(lua_State* L, UClass* iClazz) {
			lua_newtable(L);
			luaL_setmetatable(L, "ClassInstance");

			auto ud_o = (LuaClassInstance*)lua_newuserdata(L, sizeof(LuaClassInstance));
			luaL_setmetatable(L, "ClassInstance");
			new (ud_o) LuaClassInstance(iClazz);
			lua_setfield(L, -2, "__object");

			int j = 0;
			for (auto clazz : instanceSubclasses) {
				if (iClazz->IsChildOf(clazz.first))
				for (auto func : clazz.second) {
					auto instanceFunc = (LuaClassInstanceFunc*)lua_newuserdata(L, sizeof(LuaClassInstanceFunc));
					new (instanceFunc) LuaClassInstanceFunc{iClazz, func.second};
					luaL_setmetatable(L, "ClassInstanceFunc");
					lua_pushcclosure(L, luaClassInstanceFuncCall, 1);
					lua_setfield(L, -2, func.first.c_str());
				}
			}
			return true;
		}

		int luaInstanceEQ(lua_State* L) {
			bool failed = false;
			if (lua_gettop(L) < 2 || !lua_getmetatable(L, 1) || !lua_getmetatable(L, 2)) failed = true;
			luaL_getmetatable(L, "Instance");
			if (!failed && (!lua_compare(L, 3, 5, LUA_OPEQ) || !lua_compare(L, 4, 5, LUA_OPEQ))) failed = true;
			if (!failed && (lua_getfield(L, 1, "__object") != LUA_TUSERDATA || lua_getfield(L, 2, "__object") != LUA_TUSERDATA)) failed = true;
			if (failed) {
				lua_pushboolean(L, false);
				return 1;
			}

			auto obj1 = *(LuaInstance*)luaL_checkudata(L, -2, "Instance");
			auto obj2 = *(LuaInstance*)luaL_checkudata(L, -1, "Instance");
			lua_pushboolean(L, obj1.isEqualObj(obj2));
			return 1;
		}

		int luaInstanceToString(lua_State* L) {
			lua_getfield(L, 1, "__object");
			auto obj = *(LuaInstance*)luaL_checkudata(L, -1, "Instance");
			
			if (!*obj) lua_pushstring(L, "Unavailable");
			else {
				auto comp = Cast<IFINNetworkComponent>(*obj);
				auto nick = comp->GetNick();
				lua_pushstring(L, (((nick.Len() > 0) ? std::string("\"") + TCHAR_TO_UTF8(*nick) + "\" " : std::string()) + TCHAR_TO_UTF8(*comp->GetID().ToString())).c_str());
			}
			return 1;
		}

		static const luaL_Reg luaInstanceLib[] = {
			{"__eq", luaInstanceEQ},
			{"__tostring", luaInstanceToString},
			{NULL, NULL}
		};

		int luaInstanceFuncGC(lua_State* L) {
			auto c = (LuaInstanceFunc*) luaL_checkudata(L, 1, "InstanceFunc");
			c->~LuaInstanceFunc();
			return 0;
		}

		static const luaL_Reg luaInstanceFuncLib[] = {
			{"__gc", luaInstanceFuncGC},
			{NULL,NULL}
		};

		int luaClassInstanceFuncGC(lua_State* L) {
			auto c = (LuaClassInstanceFunc*) luaL_checkudata(L, 1, "ClassInstanceFunc");
			c->~LuaClassInstanceFunc();
			return 0;
		}

		static const luaL_Reg luaClassInstanceFuncLib[] = {
			{"__gc", luaClassInstanceFuncGC},
			{NULL, NULL}
		};

		// metatables

		void setupInstanceSystem(lua_State* L) {}

		void loadLibs(lua_State* L) {
			luaL_openlibs(L);

			luaL_newmetatable(L, "Instance");
			lua_pop(L, 1);

			luaL_newmetatable(L, "InstanceFunc");
			luaL_setfuncs(L, luaInstanceFuncLib, 0);
			lua_pop(L, 1);

			luaL_newmetatable(L, "ClassInstanceFunc");
			luaL_setfuncs(L, luaClassInstanceFuncLib, 0);
			lua_pop(L, 1);
		}
	}
}