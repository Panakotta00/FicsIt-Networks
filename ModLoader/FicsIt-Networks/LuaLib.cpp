#include "stdafx.h"
#include "LuaLib.h"

#include <sstream>
#include <fstream>

#include "NetworkAdapter.h"
#include "LUAContext.h"
#include "FileSystem.h"
#include "LuaImplementation.h"
#include <filesystem>
#include <util/Objects/Delegate.h>

#define OffsetParam(type, off) (type*)((std::uint64_t)param + off)

using namespace SML;
using namespace SML::Objects;

std::map<SDK::UClass*, std::vector<LuaFunc>> classes;
std::map<SDK::UClass*, std::vector<LuaFunc>> classClasses;

std::map<SML::Objects::FWeakObjectPtr, FactoryHook> hooks;

void FactoryHook::update() {
	auto now = std::chrono::high_resolution_clock::now() - std::chrono::minutes(1);
	while (iperm.size() > 0 && iperm.front() < now) {
		iperm.pop();
	}
}


LuaObjectPtr::LuaObjectPtr(UObject * obj) : ptr(obj) {}

LuaObjectPtr::~LuaObjectPtr() {}

UObject * LuaObjectPtr::getObject() const {
	return *ptr;
}

LuaComponentPtr::LuaComponentPtr(UObject * obj, UObject * component) : LuaObjectPtr(obj), comp(component) {}

LuaComponentPtr::~LuaComponentPtr() {}

UObject * LuaComponentPtr::getObject() const {
	auto o = *ptr;
	auto c = *comp;
	auto con = ULuaContext::ctx->getConnector();
	if (!o, !c || !con || !con->circuit || !con->circuit->hasNode(c)) return nullptr;
	return o;
}

LuaClass::LuaClass(SML::Objects::UObject * obj, SML::Objects::UObject * component) {
	if (component) ptr = new LuaComponentPtr(obj, component);
	else ptr = new LuaObjectPtr(obj);
}

LuaClass::~LuaClass() {
	delete ptr;
}

LuaClassFunc::LuaClassFunc(SML::Objects::UObject * obj, std::uint16_t func, SML::Objects::UObject * component) : LuaClass(obj, component) {
	this->func = func;
}

LuaClassUFunc::LuaClassUFunc(SML::Objects::UObject * obj, SML::Objects::UFunction* func, SML::Objects::UObject * component) : LuaClass(obj, component) {
	this->func = func;
}

void luaInit() {
	auto recipe = (SDK::UFGRecipe*) SDK::UFGRecipe::StaticClass()->CreateDefaultObject();
	try {
		std::cout << "bla";
	} catch (LuaException& e) {

	}

	classes[SDK::AActor::StaticClass()] = {
		{"getPowerConnectors", [](auto L, auto args,  auto obj) {
			lua_newtable(L);
			int i = 1;
			for (UField* f : *obj->clazz) {
				auto p = (UProperty*)f;
				if (!(p->clazz->castFlags & EClassCastFlags::CAST_UObjectProperty)) continue;
				auto v = *p->getValue<SDK::UFGPowerConnectionComponent*>(obj);
				if (!v || !v->IsA(SDK::UFGPowerConnectionComponent::StaticClass())) continue;
				newInstance(L, v);
				lua_seti(L, -2, i++);
			}
			return 1;
		}},
		{"getFactoryConnectors", [](auto L, auto nargs, auto obj) {
			lua_newtable(L);
			int i = 1;
			for (auto f : *obj->clazz) {
				auto p = (UProperty*)f;
				if (!(p->clazz->castFlags & EClassCastFlags::CAST_UObjectProperty)) continue;
				auto v = *p->getValue<SDK::UFGFactoryConnectionComponent*>(obj);
				if (!v || !v->IsA(SDK::UFGFactoryConnectionComponent::StaticClass())) continue;
				newInstance(L, v);
				lua_seti(L, -2, i++);
			}
			return 1;
		}},
		{"getInventories", [](auto L, auto nargs, auto obj) {
			lua_newtable(L);
			int i = 1;
			for (auto f : *obj->clazz) {
				auto p = (UProperty*)f;
				if (!(p->clazz->castFlags & EClassCastFlags::CAST_UObjectProperty)) continue;
				auto v = *p->getValue<SDK::UFGInventoryComponent*>(obj);
				if (!v || !v->IsA(SDK::UFGInventoryComponent::StaticClass())) continue;
				newInstance(L, v);
				lua_seti(L, -2, i++);
			}
			return 1;
		}}
	};
	classes[SDK::UFGInventoryComponent::StaticClass()] = std::vector<LuaFunc>{
		{"getStack", [](auto L, auto args, auto o) {
			auto c = (SDK::UFGInventoryComponent*)(o);

			SDK::FInventoryStack stack;
			for (int i = 1; i <= args; ++i) {
				if (c->GetStackFromIndex(lua_tointeger(L, i), &stack)) {
					luaItemStack(L, stack);
				} else lua_pushnil(L);
			}

			return args;
		}},
		{"getItemCount", [](lua_State* L, long nargs, UObject* o) {
			auto i = (SDK::UFGInventoryComponent*)(o);
			lua_pop(L, nargs);
			lua_pushinteger(L, i->GetNumItems(nullptr));
			return 1;
		}},
		{"getSize", [](lua_State* L, long nargs, UObject* o) {
			auto i = (SDK::UFGInventoryComponent*)(o);
			lua_pop(L, nargs);
			lua_pushinteger(L, i->GetSizeLinear());
			return 1;
		}},
		{"sort", [](lua_State* L, long nargs, UObject* o) {
			auto i = (SDK::UFGInventoryComponent*)(o);
			i->SortInventory();
			return 0;
		}},
	};
	classes[SDK::UFGPowerConnectionComponent::StaticClass()] = std::vector<LuaFunc>{
		{"getConnections", [](lua_State* L, long nargs, UObject* o) {
			auto c = (SDK::UFGPowerConnectionComponent*)(o);
			lua_pop(L, nargs);
			lua_pushinteger(L, c->GetNumConnections());
			return 1;
		}},
		{"getMaxConnections", [](lua_State* L, long nargs, UObject* o) {
			auto c = (SDK::UFGPowerConnectionComponent*)(o);
			lua_pop(L, nargs);
			lua_pushinteger(L, c->GetMaxNumConnections());
			return 1;
		}},
		{"getPower", [](lua_State* L, long nargs, UObject* o) {
			auto c = (SDK::UFGPowerConnectionComponent*)(o);
			lua_pop(L, nargs);
			newInstance(L, c->GetPowerInfo());
			return 1;
		}},
		{"getCircuit", [](lua_State* L, long nargs, UObject* o) {
			auto c = (SDK::UFGPowerConnectionComponent*)(o);
			lua_pop(L, nargs);
			newInstance(L, c->GetPowerCircuit());
			return 1;
		}},
	};
	classes[SDK::UFGPowerInfoComponent::StaticClass()] = std::vector<LuaFunc>{
		{"getDynProduction", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerInfoComponent*)(o);
			lua_pop(L, nargs);
			lua_pushnumber(L, p->GetRegulatedDynamicProduction());
			return 1;
		}},
		{"getBaseProduction", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerInfoComponent*)(o);
			lua_pop(L, nargs);
			lua_pushnumber(L, p->GetBaseProduction());
			return 1;
		}},
		{"getMaxDynProduction", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerInfoComponent*)(o);
			lua_pop(L, nargs);
			lua_pushnumber(L, p->GetDynamicProductionCapacity());
			return 1;
		}},
		{"getTargetConsumption", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerInfoComponent*)(o);
			lua_pop(L, nargs);
			lua_pushnumber(L, p->GetTargetConsumption());
			return 1;
		}},
		{"getConsumption", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerInfoComponent*)(o);
			lua_pop(L, nargs);
			lua_pushnumber(L, p->GetBaseProduction());
			return 1;
		}},
		{"getCircuit", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerInfoComponent*)(o);
			lua_pop(L, nargs);
			newInstance(L, p->GetPowerCircuit());
			return 1;
		}},
		{"hasPower", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerInfoComponent*)(o);
			lua_pop(L, nargs);
			lua_pushboolean(L, p->HasPower());
			return 1;
		}}
	};
	classes[SDK::UFGPowerCircuit::StaticClass()] = std::vector<LuaFunc>{
		{"getProduction", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerCircuit*)(o);
			lua_pop(L, nargs);
			SDK::FPowerCircuitStats stats;
			p->GetStats(&stats);
			lua_pushnumber(L, stats.PowerProduced);
			return 1;
		}},
		{"getConsumption", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerCircuit*)(o);
			lua_pop(L, nargs);
			SDK::FPowerCircuitStats stats;
			p->GetStats(&stats);
			lua_pushnumber(L, stats.PowerConsumed);
			return 1;
		}},
		{"getProductionCapacity", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGPowerCircuit*)(o);
			lua_pop(L, nargs);
			SDK::FPowerCircuitStats stats;
			p->GetStats(&stats);
			lua_pushnumber(L, stats.PowerProductionCapacity);
			return 1;
		}},
	};
	classes[SDK::UFGFactoryConnectionComponent::StaticClass()] = std::vector<LuaFunc>{
		{"getType", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGFactoryConnectionComponent*)(o);
			lua_pop(L, nargs);
			
			lua_pushinteger(L, (int)p->mConnector);
			return 1;
		}},
		{"getDirection", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGFactoryConnectionComponent*)(o);
			lua_pop(L, nargs);
			lua_pushinteger(L, (int)p->mDirection);
			return 1;
		}},
		{"isConnected", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGFactoryConnectionComponent*)(o);
			lua_pop(L, nargs);
			lua_pushboolean(L, p->mHasConnectedComponent);
			return 1;
		}},
		{"getInventory", [](lua_State* L, long nargs, UObject* o) {
			auto p = (SDK::UFGFactoryConnectionComponent*)(o);
			lua_pop(L, nargs);
			newInstance(L, p->mConnectionInventory);
			return 1;
		}},
		{"hook", [](auto L, auto args, auto o) {
			auto p = (SDK::UFGFactoryConnectionComponent*)(o);
			lua_pop(L, args);
			try {
				hooks.at((UObject*)p).refs += 1;
			} catch (...) {
				hooks[(UObject*)p] = FactoryHook();
			}
			luaFactoryHook(L, p);
			return 1;
		}},
	};
	classes[SDK::AFGBuildableFactory::StaticClass()] = {
		{"getProgress", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			lua_pushnumber(L, ((SDK::AFGBuildableFactory*)o)->GetProductionProgress());
			return 1;
		}},
		{"getPowerConsumProducing", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			lua_pushnumber(L, ((SDK::AFGBuildableFactory*)o)->GetProducingPowerConsumption());
			return 1;
		}},
		{"getProductivity", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			lua_pushnumber(L, ((SDK::AFGBuildableFactory*)o)->GetProductivity());
			return 1;
		}},
		{"getCycleTime", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			lua_pushnumber(L, ((SDK::AFGBuildableManufacturer*)o)->GetProductionCycleTime());
			return 1;
		}},
		{"getPotential", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			lua_pushnumber(L, ((SDK::AFGBuildableFactory*)o)->GetPendingPotential());
			return 1;
		}},
		{"setPotential", [](auto L, auto args, auto o) {
			if (args < 1) {
				lua_pop(L, args);
				return 0;
			}
			float p = lua_tonumber(L, -args);
			lua_pop(L, args);
			auto f = (SDK::AFGBuildableFactory*)o;
			float min = f->GetMinPotential();
			float max = f->GetMaxPossiblePotential();
			f->SetPendingPotential((min > p) ? min : ((max < p) ? max : p));
			return 0;
		}},
		{"getMaxPotential", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			lua_pushnumber(L, ((SDK::AFGBuildableFactory*)o)->GetMaxPossiblePotential());
			return 1;
		}},
		{"getMinPotential", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			lua_pushnumber(L, ((SDK::AFGBuildableFactory*)o)->GetMinPotential());
			return 1;
		}},
	};
	classes[SDK::AFGBuildableManufacturer::StaticClass()] = {
		{"getRecipe", [](auto L, auto args, auto o) {
			lua_pop(L, args);
			newInstance(L, ((SDK::AFGBuildableManufacturer*)o)->GetCurrentRecipe());
			return 1;
		}},
		{"getRecipes", [](auto L, auto args, auto o) {
			static void(*get)(SDK::AFGBuildableManufacturer*, TArray<UClass*>*) = nullptr;
			if (!get) get = (void(*)(SDK::AFGBuildableManufacturer*, TArray<UClass*>*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGBuildableManufacturer::GetAvailableRecipes");

			bool available = true;
			//if (args > 0) childs = lua_toboolean(L, -args);
			lua_pop(L, args);
			TArray<UClass*> recipes;
			get((SDK::AFGBuildableManufacturer*)o, &recipes);
			lua_newtable(L);
			int in = 1;
			for (auto i : recipes) {
				newInstance(L, (SDK::UObject*)i);
				lua_seti(L, -2, in++);
			}
			return 1;
		}},
		{"setRecipe", [](auto L, auto args, auto o) {
			static void(*get)(SDK::AFGBuildableManufacturer*, TArray<UClass*>*) = nullptr;
			if (!get) get = (void(*)(SDK::AFGBuildableManufacturer*, TArray<UClass*>*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGBuildableManufacturer::GetAvailableRecipes");
			static SDK::FText*(*getn)(SDK::FText*, SDK::UFGRecipe*) = nullptr;
			if (!getn) getn = (SDK::FText*(*)(SDK::FText*, SDK::UFGRecipe*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGRecipe::GetRecipeName");

			if (args < 1) {
				lua_pop(L, args);
				return 0;
			}
			auto r = getClassInstance<SDK::UFGRecipe>(L, 1);
			std::wstring name;
			if (!r && lua_isstring(L, -args)) {
				std::string n = lua_tostring(L, -args);
				name = std::wstring(n.begin(), n.end());
			}
			lua_pop(L, args);
			TArray<UClass*> recipes;
			get((SDK::AFGBuildableManufacturer*)o, &recipes);
			SDK::FText t;
			for (auto i : recipes) {
				if ((UClass*)r == i || (name.size() > 0 && name == getn(&t, (SDK::UFGRecipe*)i)->Get())) {
					((SDK::AFGBuildableManufacturer*)o)->SetRecipe((SDK::UClass*)i);
					return 0;
				}
			}
			((SDK::AFGBuildableManufacturer*)o)->SetRecipe(nullptr);
			return 0;
		}},
	};
	classes[(SDK::UClass*)UFileSystem::staticClass()] = {
		{"open", [](auto L, auto args, auto o) {
			std::string mode = "r";
			if (lua_isstring(L, 2)) mode = lua_tostring(L, 2);
			luaFile(L, ((UFileSystem*)o)->manager->open(luaL_checkstring(L, 1), mode));
			return 1;
		}},
		{"createDir", [](auto L, auto args, auto o) {
			auto path = luaL_checkstring(L, 1);
			auto all = lua_toboolean(L, 2);
			((UFileSystem*)o)->manager->createDir(path, all);
			return 0;
		}},
		{"remove", [](auto L, auto args, auto o) {
			auto path = luaL_checkstring(L, 1);
			bool all = lua_toboolean(L, 2);
			((UFileSystem*)o)->manager->remove(path, all);
			return 0;
		}},
		{"move", [](auto L, auto args, auto o) {
			auto from = luaL_checkstring(L, 1);
			auto to = luaL_checkstring(L, 2);
			((UFileSystem*)o)->manager->move(from, to);
			return 0;
		}},
		{"exists", [](auto L, auto args, auto o) {
			auto path = luaL_checkstring(L, 1);
			lua_pushboolean(L, ((UFileSystem*)o)->manager->exists(path));
			return 1;
		}},
		{"isFile", [](auto L, auto args, auto o) {
			auto path = luaL_checkstring(L, 1);
			lua_pushboolean(L, ((UFileSystem*)o)->manager->isFile(path));
			return 1;
		}},
		{"isDir", [](auto L, auto args, auto o) {
			auto path = luaL_checkstring(L, 1);
			lua_pushboolean(L, ((UFileSystem*)o)->manager->isDir(path));
			return 1;
		}},
		{"doFile", [](auto L, auto args, auto o) {
			auto p = ((UFileSystem*)o)->manager->getPath(luaL_checkstring(L, 1), true, false, 1);
			if (std::filesystem::is_regular_file(p)) return luaL_argerror(L, 1, "path is no valid file");
			int n = lua_gettop(L);
			luaL_dofile(L, p.string().c_str());
			return lua_gettop(L) - n;
		}},
		{"loadFile", [](auto L, auto args, auto o) {
			std::filesystem::path p;
			p = ((UFileSystem*)o)->manager->getPath(luaL_checkstring(L, 1), true, false, 1);
			if (std::filesystem::is_regular_file(p)) return luaL_argerror(L, 1, "path is no valid file");
			luaL_loadfile(L, p.string().c_str());
			return 1;
		}},
	};
	classClasses[SDK::UFGRecipe::StaticClass()] = {
		{"getName", [=](lua_State* L, long args, UObject* o) {
			static SDK::FText*(*get)(SDK::FText*, SDK::UFGRecipe*) = nullptr;
			if (!get) get = (SDK::FText*(*)(SDK::FText*, SDK::UFGRecipe*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGRecipe::GetRecipeName");
			
			lua_pop(L, args);
			SDK::FText n;
			get(&n, (SDK::UFGRecipe*)o);
			std::wstring name = n.Get();
			lua_pushstring(L, std::string(name.begin(), name.end()).c_str());
			return 1;
		}},
		{"getProducts", [=](lua_State* L, long args, UObject* o) {
			static TArray<SDK::FItemAmount>*(*get)(TArray<SDK::FItemAmount>*, SDK::UFGRecipe*, bool) = nullptr;
			if (!get) get = (TArray<SDK::FItemAmount>*(*)(TArray<SDK::FItemAmount>*, SDK::UFGRecipe*, bool)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGRecipe::GetProducts");
			
			bool childs = true;
			if (args > 0) childs = lua_toboolean(L, -args);
			lua_pop(L, args);
			TArray<SDK::FItemAmount> products;
			get(&products, (SDK::UFGRecipe*)o, childs);
			lua_newtable(L);
			int in = 1;
			for (auto i : products) {
				lua_newtable(L);
				lua_pushinteger(L, i.amount); lua_setfield(L, -2, "count");
				newInstance(L, i.ItemClass); lua_setfield(L, -2, "item");
				lua_seti(L, -2, in++);
			}
			return 1;
		}},
		{"getIngredients", [=](lua_State* L, long args, UObject* o) {
			static TArray<SDK::FItemAmount>*(*get)(TArray<SDK::FItemAmount>*, SDK::UFGRecipe*) = nullptr;
			if (!get) get = (TArray<SDK::FItemAmount>*(*)(TArray<SDK::FItemAmount>*, SDK::UFGRecipe*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGRecipe::GetIngredients");

			lua_pop(L, args);
			TArray<SDK::FItemAmount> ingredients;
			get(&ingredients, (SDK::UFGRecipe*)o);
			lua_newtable(L);
			int in = 1;
			for (auto i : ingredients) {
				lua_newtable(L);
				lua_pushinteger(L, i.amount); lua_setfield(L, -2, "count");
				newInstance(L, i.ItemClass); lua_setfield(L, -2, "item");
				lua_seti(L, -2, in++);
			}
			return 1;
		}},
		{"getDuration", [=](lua_State* L, long args, UObject* o) {
			lua_pop(L, args);
			lua_pushnumber(L, recipe->GetManufacturingDuration((SDK::UClass*)o));
			return 1;
		}}
	};
	classClasses[SDK::UFGItemDescriptor::StaticClass()] = {
		{"getName", [=](auto L, auto args, auto o) {
			lua_pop(L, args);
			SDK::FText*(*get)(SDK::FText*, UObject*) = nullptr;
			if (!get) get = (SDK::FText*(*)(SDK::FText*, UObject*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGItemDescriptor::GetItemName");
			SDK::FText n;
			get(&n, o);
			std::wstring name = n.Get();
			lua_pushstring(L, std::string(name.begin(), name.end()).c_str());
			return 1;
		}},
	};
}

// --- instance/component system --- //

LuaDataType propertyToLua(lua_State* L, UProperty* p, void* data) {
	auto c = p->clazz->castFlags;
	if (c & EClassCastFlags::CAST_UBoolProperty) {
		lua_pushboolean(L, *p->getValue<bool>(data));
		return LuaDataType::LUA_BOOL;
	} else if (c & EClassCastFlags::CAST_UIntProperty) {
		lua_pushinteger(L, *p->getValue<std::int32_t>(data));
		return LuaDataType::LUA_INT;
	} else if (c & EClassCastFlags::CAST_UFloatProperty) {
		lua_pushnumber(L, *p->getValue<float>(data));
		return LuaDataType::LUA_NUM;
	} else if (c & EClassCastFlags::CAST_UStrProperty) {
		lua_pushstring(L, p->getValue<FString>(data)->toStr().c_str());
		return LuaDataType::LUA_STR;
	} else if (c & EClassCastFlags::CAST_UObjectProperty) {
		return newInstance(L, *p->getValue<SDK::UObject*>(data)) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
	} else {
		lua_pushnil(L);
		return LuaDataType::LUA_NIL;
	}
}

LuaDataType luaToProperty(lua_State* L, UProperty* p, void* data, int i) {
	auto c = p->clazz->castFlags;
	if (c & EClassCastFlags::CAST_UBoolProperty) {
		*p->getValue<bool>(data) = lua_toboolean(L, i);
		return LuaDataType::LUA_BOOL;
	} else if (c & EClassCastFlags::CAST_UIntProperty) {
		*p->getValue<std::int32_t>(data) = lua_tointeger(L, i);
		return LuaDataType::LUA_INT;
	} else if (c & EClassCastFlags::CAST_UFloatProperty) {
		*p->getValue<float>(data) = lua_tonumber(L, i);
		return LuaDataType::LUA_NUM;
	} else if (c & EClassCastFlags::CAST_UStrProperty) {
		auto s = lua_tostring(L, i);
		if (!s) throw std::exception("string");
		auto o = p->getValue<FString>(data);
		if (o->getData()) *o = FString(s);
		else new (o) FString(s);
		return LuaDataType::LUA_STR;
	} else if (c & EClassCastFlags::CAST_UObjectProperty) {
		if (((UObjectProperty*)c)->objClass->isChild(SML::Objects::UClass::staticClass())) {
			auto o = getClassInstance(L, i, ((UObjectProperty*)c)->objClass);
			*p->getValue<UObject*>(data) = o;
			return (o) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
		} else {
			auto o = getObjInstance(L, i, ((UObjectProperty*)c)->objClass);
			*p->getValue<UObject*>(data) = o;
			return (o) ? LuaDataType::LUA_OBJ : LuaDataType::LUA_NIL;
		}
	} else {
		lua_pushnil(L);
		return LuaDataType::LUA_NIL;
	}
}

int componentFunc(lua_State * L) {
	int args = lua_gettop(L);

	// get data from closure
	auto& ud = *(LuaClassUFunc*)lua_touserdata(L, lua_upvalueindex(1));
	UObject* comp = ud.ptr->getObject();
	UFunction* func = ud.func;

	// check object validity
	if (!comp) {
		luaL_error(L, "component is invalid");
		return 0;
	}

	// allocate parameter space
	void* params = malloc(func->parmsSize);
	memset(params, 0, func->parmsSize);

	// init and set parameter values
	std::string error = "";
	int i = 1;
	for (auto f : *func) {
		auto p = (UProperty*)f;
		if (p->propFlags & Prop_Parm && !(p->propFlags & (Prop_OutParm | Prop_ReturnParm))) {
			try {
				luaToProperty(L, p, params, i++);
			} catch (std::exception e) {
				error = "argument #" + std::to_string(i) + " is not of type " + e.what();
				break;
			}
		}
	}

	// execute native function only if no error
	if (error.length() <= 0) func->invoke(comp, params);

	// free parameter space
	for (auto f : *func) {
		auto p = (UProperty*)f;
		if (p->propFlags & Prop_Parm && !(p->propFlags & (Prop_OutParm | Prop_ReturnParm))) {
			if (--i > 0) break;
			if (p->clazz->castFlags & CAST_UStrProperty) {
				p->getValue<FString>(params)->~FString();
			}
		}
	}
	if (error.length() > 0) {
		free(params);
		return luaL_error(L, std::string("Error at ").append(std::to_string(i).c_str()).append("# parameter: ").append(error).c_str());
	}

	// free parameter space and eventualy push return falues to lua
	i = 0;
	for (auto f : *func) {
		auto p = (UProperty*)f;
		if (p->propFlags & (Prop_OutParm | Prop_ReturnParm)) {
			propertyToLua(L, p, params);
			auto c = p->clazz->castFlags;
			if (c & CAST_UStrProperty) {
				p->getValue<FString>(params)->~FString();
			}
			++i;
		}
	}
	free(params);

	return i;
}

int luaClassFunc(lua_State * L) {
	int args = lua_gettop(L);

	auto& data = *(LuaClassFunc*)lua_touserdata(L, lua_upvalueindex(1));
	auto o = data.ptr->getObject();
	auto so = (SDK::UObject*)o;

	if (!o) return luaL_error(L, "component is invalid");

	int j = 0;
	for (auto clazz : classes) {
		if (!so->IsA(clazz.first)) continue;
		std::vector<LuaFunc> lc = clazz.second;
		auto fi = data.func - j;
		LuaFunc f;
		try {
			f = lc.at(fi);
		} catch (...) {
			j += lc.size();
			continue;
		}
		try {
			return f.func(L, args, o);
		} catch (LuaException& e) {
			return e.lua(L);
		}
	}
	return luaL_error(L, "invalid native function ptr");
}

int luaClassClassFunc(lua_State* L) {
	int args = lua_gettop(L);

	void** data = (void**)lua_touserdata(L, lua_upvalueindex(1));
	auto o = (UObject*)data[0];
	auto so = (SDK::UObject*)o;
	if (!o->isValid()) return 0;

	int fj = 0;
	auto super = (SDK::UClass*)((SDK::UClass*)o)->SuperField;
	while (super) {
		std::vector<LuaFunc>* lc = nullptr;
		try {
			lc = &classClasses.at(super);
		} catch (...) {
			lc = nullptr;
		}
		if (lc) {
			auto fi = (size_t)data[1] - fj;
			try {
				return lc->at(fi).func(L, args, o);
			} catch (LuaException& e) {
				return e.lua(L);
			} catch (...) {
				fj += lc->size();
			}
		}
		super = (SDK::UClass*)super->SuperField;
	}
	return luaL_error(L, "invalid native function ptr");
}

void addCompFuncs(lua_State * L, UObject * comp, UObject* boundComp) {
	if (!comp->clazz->implements(ULuaImplementation::staticClass())) return;
	ULuaContext* ctx = ULuaContext::ctx;
	comp->findFunction(L"luaSetup")->invoke(comp, &ctx);
	for (auto f : *comp->clazz) {
		if (!(f->getName()._Starts_with("lua_") && f->getName().length() > 4)) continue;

		auto comp_ud = (LuaClassUFunc*)lua_newuserdata(L, sizeof(LuaClassUFunc));
		new (comp_ud) LuaClassUFunc(comp, (UFunction*)f, boundComp);
		luaL_setmetatable(L, "ClassPtr");
		lua_pushcclosure(L, componentFunc, 1);
		lua_setfield(L, -2, f->getName().erase(0, 4).c_str());
	}
}

void addPreFuncs(lua_State* L, SDK::UObject* obj, SML::Objects::UObject* boundComp) {
	int j = 0;
	for (auto clazz : classes) {
		if (!obj->IsA(clazz.first)) continue;
		for (int i = 0; i < clazz.second.size(); ++i) {
			auto f = clazz.second[i];
			auto comp_ud = (LuaClassFunc*)lua_newuserdata(L, sizeof(LuaClassFunc));
			new (comp_ud) LuaClassFunc((UObject*)obj, j++, boundComp);
			luaL_setmetatable(L, "ClassPtr");
			lua_pushcclosure(L, luaClassFunc, 1);
			lua_setfield(L, -2, f.name.c_str());
		}
	}
}

bool newInstance(lua_State* L, SDK::UObject* obj, SDK::UObject* component) {
	if (!obj) {
		lua_pushnil(L);
		return false;
	}
	
	lua_newtable(L);
	luaL_setmetatable(L, "Instance");

	auto ud_o = (LuaObjectPtr*) lua_newuserdata(L, (component) ? sizeof(LuaObjectPtr) : sizeof(LuaComponentPtr));
	luaL_setmetatable(L, "WeakObjPtr");
	if (component) new (ud_o) LuaComponentPtr((UObject*)obj, (UObject*)component);
	else new (ud_o) LuaObjectPtr((UObject*)obj);
	lua_setfield(L, -2, "__object");

	if (!obj->IsA(SDK::UClass::StaticClass())) {
		addPreFuncs(L, obj, (UObject*)component);
		addCompFuncs(L, (UObject*)obj, (UObject*)component);

		if (((UObject*)obj)->clazz->implements(ULuaImplementation::staticClass())) {
			((UObject*)obj)->findFunction(L"luaSetup")->invoke((UObject*)obj, &ULuaContext::ctx);
		}

		if (((UObject*)obj)->clazz->implements(UNetworkComponent::staticClass())) {
			struct Params {
				TArray<UObject*> merged;
			};
			Params p;
			((UObject*)obj)->findFunction(L"getMerged")->invoke((UObject*)obj, &p);
			for (auto m : p.merged) {
				if (!m) continue;
				addPreFuncs(L, (SDK::UObject*)m, (UObject*)component);
				addCompFuncs(L, m, (UObject*)component);

				if (m->clazz->implements(ULuaImplementation::staticClass())) {
					m->findFunction(L"luaSetup")->invoke(m, &ULuaContext::ctx);
				}
			}
		}
	} else {
		int j = 0;
		for (auto clazz : classClasses) {
			auto super = (SDK::UClass*)((UClass*)obj)->super;
			while (super) {
				if (super == clazz.first) {
					for (int i = 0; i < clazz.second.size(); ++i) {
						auto f = clazz.second[i];
						void** comp_ud = (void**)lua_newuserdata(L, sizeof(void*) * 2);
						comp_ud[0] = obj;
						comp_ud[1] = (void*)(size_t)j++;
						lua_pushcclosure(L, luaClassClassFunc, 1);
						lua_setfield(L, -2, f.name.c_str());
					}
				}
				super = (SDK::UClass*)super->SuperField;
			}
		}
	}
	return true;
}

void registerClass(SDK::UClass* clazz, std::vector<LuaFunc> functions) {
	classes[clazz] = functions;
}

// --- lua structs/libs --- //

// static funcs //

int luaPrint(lua_State * L) {
	int args = lua_gettop(L);
	for (int i = 1; i <= args; ++i) {
		auto str = lua_tostring(L, i);
		if (str) ULuaContext::ctx->Log(str);
	}
	return 0;
}

int luaComponentProxy(lua_State * L) {
	int args = lua_gettop(L);

	for (int i = 1; i <= args; ++i) {
		auto s = lua_tostring(L, i);
		if (!s) return luaL_error(L, ("argument #" + std::to_string(i) + " is not a string").c_str());
		auto comp = ULuaContext::ctx->getComponent(s);
		if (!comp) lua_pushnil(L);
		else newInstance(L, (SDK::UObject*)comp, (SDK::UObject*)comp);
	}
	return args;
}

int luaFindItem(lua_State * L) {
	static SDK::FText(*gin)(SDK::UFGItemDescriptor*, SDK::FText*, SDK::UClass*) = nullptr;
	if (!gin) gin = (SDK::FText(*)(SDK::UFGItemDescriptor*, SDK::FText*, SDK::UClass*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGItemDescriptor::GetItemName");

	int nargs = lua_gettop(L);
	if (nargs < 1) return 0;
	const char* str = lua_tostring(L, -1);
	if (!str) return luaL_error(L, "argument #1 needs to be string or number");

	TArray<SDK::UClass*> items;
	((SDK::UFGBlueprintFunctionLibrary*)SDK::UFGBlueprintFunctionLibrary::StaticClass()->CreateDefaultObject())->Cheat_GetAllDescriptors((SDK::TArray<SDK::UClass*>*)&items);
	for (auto itemc : items) {
		auto item = (SDK::UFGItemDescriptor*)itemc->CreateDefaultObject();
		if (FString(item->mDisplayName.Get()).toStr() == str) {
			newInstance(L, itemc);
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

void luaListen(UObject* o) {
	if (o && o->clazz->implements(ULuaImplementation::staticClass())) {
		for (auto f : *o->clazz) {
			if (!f->getName()._Starts_with("luaSig_")) continue;
			if (f->clazz->castFlags & EClassCastFlags::CAST_UFunction) {
				auto signalName = f->getName().substr(7);
				if (signalName.length() < 1) continue;

				auto func = (UFunction*)f;

				((UFunction*)f)->func = ULuaContext::execSignalSlot;
				((UFunction*)f)->flags = (EFunctionFlags)(((UFunction*)f)->flags | EFunctionFlags::FUNC_Native);
				ULuaContext* ctx = ULuaContext::ctx;
				o->findFunction(L"luaAddSignalListener")->invoke(o, &ctx);
				ctx->signalSources.push_back(std::unique_ptr<SignalSource>(new SignalSourceProperty(ctx, o)));
			}

		}
	}
	if (o && o->clazz->implements(UNetworkComponent::staticClass())) {
		TArray<UObject*> merged;
		o->findFunction(L"getMerged")->invoke(o, &merged);
		for (auto m : merged) {
			luaListen((UObject*)m);
		}
	}
}

int luaListen(lua_State * L) {
	int args = lua_gettop(L);

	for (int i = 1; i <= args; ++i) {
		auto o = (UObject*)getObjInstance<SDK::UObject>(L, i);
		luaListen(o);
	}
	return 0;
}

int luaPullContinue(lua_State* L, int status, lua_KContext ctx) {
	int a = lua_gettop(L);

	return a;
}

int luaPull(lua_State * L) {
	int args = lua_gettop(L);
	auto t = 0;
	if (args > 0 && !lua_isnil(L, 1)) t = lua_tointeger(L, 1);

	int a = ULuaContext::ctx->doSignal(L);
	if (!a) {
		ULuaContext::ctx->timeout = t;
		ULuaContext::ctx->pullStart = std::chrono::high_resolution_clock::now();

		lua_yieldk(L, 0, 0, luaPullContinue);
		return 0;
	}
	return a;
}

// member funcs //

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

	auto c1 = *(FWeakObjectPtr*)luaL_checkudata(L, -2, "WeakObjPtr");
	auto c2 = *(FWeakObjectPtr*)luaL_checkudata(L, -1, "WeakObjPtr");
	lua_pushboolean(L, *c1 == *c2);
	return 1;
}

int luaFileClose(lua_State* L) {
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	try {
		f->file->close();
	} catch (LuaException& e) { return e.lua(L); }
	return 0;
}

int luaFileWrite(lua_State* L) {
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	auto s = lua_gettop(L);
	for (int i = 2; i <= s; ++i) {
		std::string str = luaL_checkstring(L, i);
		try {
			f->file->write(str);
		} catch (LuaExceptionArg& e) {
			e.arg(i);
			return e.lua(L);
		} catch (LuaException& e) { return e.lua(L); }
	}
	return 0;
}

int luaFileFlush(lua_State* L) {
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	try {
		f->file->flush();
	} catch (LuaException& e) { return e.lua(L); }
	return 0;
}

int luaFileRead(lua_State* L) {
	auto args = lua_gettop(L);
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	for (int i = 2; i <= args; ++i) {
		try {
			if (lua_isnumber(L, i)) {
				if (f->file->isEOF()) lua_pushnil(L);
				int n = lua_tointeger(L, i);
				lua_pushstring(L, f->file->readChars(n).c_str());
			} else {
				char fo = 'l';
				if (lua_isstring(L, i)) {
					std::string s = lua_tostring(L, i);
					if (s.size() != 2 || s[0] != '*') fo = '\0';
					fo = s[1];
				}
				switch (fo) {
				case 'n':
				{
					lua_pushnumber(L, f->file->readNumber());
					break;
				} case 'a':
				{
					lua_pushstring(L, f->file->readAll().c_str());
					break;
				} case 'l':
				{
					if (!f->file->isEOF()) lua_pushstring(L, f->file->readLine().c_str());
					else lua_pushnil(L);
					break;
				}
				default:
					luaL_argerror(L, i, "no valid format");
				}
			}
		} catch (LuaExceptionArg& e) {
			e.arg(i);
			return e.lua(L);
		} catch (LuaException& e) {
			return e.lua(L);
		}
	}
	return args-1;
}

int luaFileReadLine(lua_State* L) {
	auto f = (LuaFile*)luaL_checkudata(L, lua_upvalueindex(1), "File");
	std::string s;
	if (f->file->isEOF()) lua_pushnil(L);
	else try {
		lua_pushstring(L, f->file->readLine().c_str());
	} catch (LuaException& e) { e.lua(L); }
	return 1;
}

int luaFileLines(lua_State* L) {
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	lua_pushcclosure(L, luaFileReadLine, 1);
	return 1;
}

int luaFileSeek(lua_State* L) {
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	std::string w = "cur";
	int off = 0;
	if (lua_isstring(L, 2)) w = lua_tostring(L, 2);
	if (lua_isinteger(L, 3)) off = lua_tointeger(L, 3);
	try {
		lua_pushinteger(L, f->file->seek(w, off));
	} catch (LuaException& e) { return e.lua(L); }
	return 1;
}

int luaFileString(lua_State* L) {
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	try {
		lua_pushstring(L, f->file->readAll().c_str());
	} catch (LuaException& e) { return e.lua(L); }
	return 1;
}

int luaFileGC(lua_State* L) {
	auto f = (LuaFile*) luaL_checkudata(L, 1, "File");
	try {
		f->file->close();
	} catch (LuaException& e) { return e.lua(L); }
	f->~LuaFile();
	return 0;
}

int luaFactoryHookGC(lua_State* L) {
	auto f = (FWeakObjectPtr*)luaL_checkudata(L, 1, "FactoryHook");
	UObject* o = nullptr;
	if (!(o = f->get())) return 0;
	try {
		auto& h = hooks.at(o);
		--h.refs;
		if (h.refs < 1) hooks.erase(o);
	} catch (...) {}
	return 0;
}

int luaFactoryHookGetIperM(lua_State* L) {
	auto f = (FWeakObjectPtr*)luaL_checkudata(L, 1, "FactoryHook");
	if (!f->isValid()) return luaL_error(L, "invalid component");
	lua_pushinteger(L, hooks[*f].iperm.size());
	return 1;
}

int luaFactoryHookListen(lua_State* L) {
	auto f = (FWeakObjectPtr*)luaL_checkudata(L, 1, "FactoryHook");
	if (!f->isValid()) return luaL_error(L, "invalid component");
	hooks[*f].deleg.insert(ULuaContext::ctx);
	return 1;
}

int luaItemStackEQ(lua_State* L) {
	lua_getfield(L, 1, "count");
	lua_getfield(L, 2, "count");
	lua_getfield(L, 1, "item");
	lua_getfield(L, 2, "item");
	lua_pushboolean(L, lua_compare(L, 3, 4, LUA_OPEQ) && lua_compare(L, 5, 6, LUA_OPEQ));
	return 1;
}

int luaItemEQ(lua_State* L) {
	lua_getfield(L, 1, "type");
	lua_getfield(L, 2, "type");
	lua_pushboolean(L, lua_compare(L, 3, 4, LUA_OPEQ));
	return 1;
}

int luaClassGC(lua_State* L) {
	auto c = (LuaClass*)luaL_checkudata(L, 1, "ClassPtr");
	c->~LuaClass();
	return 0;
}

// constructors //

void luaItemStack(lua_State * L, SDK::FInventoryStack stack) {
	lua_newtable(L);
	luaL_setmetatable(L, "ItemStack");
	lua_pushinteger(L, stack.NumItems);
	lua_setfield(L, -2, "count");
	luaItem(L, stack.Item);
	lua_setfield(L, -2, "item");
}

void luaItem(lua_State* L, SDK::FInventoryItem item) {
	lua_newtable(L);
	luaL_setmetatable(L, "Item");
	newInstance(L, item.ItemClass);
	lua_setfield(L, -2, "type");
	//luaItemState(L, item.ItemState);
	//lua_setfield(L, -2, "state");
}

void luaFactoryHook(lua_State* L, SDK::UFGFactoryConnectionComponent * con) {
	auto p = (FWeakObjectPtr*)lua_newuserdata(L, sizeof(FWeakObjectPtr));
	luaL_setmetatable(L, "FactoryHook");
	*p = (UObject*)con;
}

void luaFile(lua_State * L, std::unique_ptr<FileSystemFileStream> file) {
	auto f = (LuaFile*)lua_newuserdata(L, sizeof(LuaFile));
	luaL_setmetatable(L, "File");
	new (f) LuaFile{std::move(file)};
}

// libs

static const luaL_Reg luaInstanceLib[] = {
	{"__eq", luaInstanceEQ},
	{NULL,NULL}
};

static const luaL_Reg luaFileLib[] = {
	{"close", luaFileClose},
	{"write", luaFileWrite},
	{"flush", luaFileFlush},
	{"read", luaFileRead},
	{"lines", luaFileLines},
	{"seek", luaFileSeek},
	{"__tostring", luaFileString},
	{"__gc", luaFileGC},
	{NULL, NULL}
};

static const luaL_Reg luaFactoryHookLib[] = {
	{"getIlastM", luaFactoryHookGetIperM},
	{"listen", luaFactoryHookListen},
	{"__gc", luaFactoryHookGC},
	{NULL, NULL}
};

static const luaL_Reg luaItemStackLib[] = {
	{"__eq", luaItemStackEQ},
	{NULL, NULL}
};

static const luaL_Reg luaItemLib[] = {
	{"__eq", luaItemEQ},
	{NULL, NULL}
};

static const luaL_Reg luaClassLib[] = {
	{"__gc", luaClassGC},
	{NULL, NULL}
};

// metatables

void loadLibs(lua_State* L) {
	luaL_openlibs(L);

	lua_register(L, "log", luaPrint);
	lua_register(L, "proxy", luaComponentProxy);
	lua_register(L, "findItem", luaFindItem);
	lua_register(L, "listen", luaListen);
	lua_register(L, "pull", luaPull);

	luaL_newmetatable(L, "WeakObjPtr");
	lua_pop(L, 1);

	luaL_newmetatable(L, "Instance");
	luaL_setfuncs(L, luaInstanceLib, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, "FactoryHook");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, luaFactoryHookLib, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, "File");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, luaFileLib, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, "ItemStack");
	luaL_setfuncs(L, luaItemStackLib, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, "Item");
	luaL_setfuncs(L, luaItemLib, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, "ClassPtr");
	luaL_setfuncs(L, luaClassLib, 0);
	lua_pop(L, 1);
}
