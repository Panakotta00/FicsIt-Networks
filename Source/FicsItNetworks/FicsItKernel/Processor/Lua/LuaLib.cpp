#include "LuaLib.h"

#include "LuaStructs.h"
#include "LuaHooks.h"

#include "Network/FINNetworkConnector.h"

#include "FGPowerConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "FGPowerCircuit.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableManufacturer.h"

using namespace FicsItKernel;
using namespace FicsItKernel::Lua;

class RegisterObjectFunc {
public:
	RegisterObjectFunc(UClass* clazz, std::string funcName, LuaLibFunc func) {
		FicsItKernel::Lua::instanceClasses[clazz][funcName] = func;
	}
};

class RegisterClassFunc {
public:
	RegisterClassFunc(UClass* clazz, std::string funcName, LuaLibClassFunc func) {
		FicsItKernel::Lua::instanceSubclasses[clazz][funcName] = func;
	}
};

#define LuaLibFuncName(ClassName, FuncName) ClassName ## _ ## FuncName
#define LuaLibFuncSig(ClassName, FuncName) int LuaLibFuncName(ClassName, FuncName) (lua_State* L, int args, const FicsItKernel::Network::NetworkTrace& obj)
#define LuaLibClassFuncSig(ClassName, FuncName) int LuaLibFuncName(ClassName, FuncName) (lua_State* L, int args, UClass* clazz)
#define LuaLibRegNamePrefix(FullFuncName) Register_ ## FullFuncName
#define LuaLibRegNameFinal(FullFuncName) LuaLibRegNamePrefix(FullFuncName)
#define LuaLibRegName(ClassName, FuncName) LuaLibRegNameFinal(LuaLibFuncName(ClassName, FuncName))
#define LuaLibFunc(ClassName, FuncName) \
	LuaLibFuncSig(ClassName, FuncName); \
	RegisterObjectFunc LuaLibRegName(ClassName, FuncName) (ClassName::StaticClass(), #FuncName, &LuaLibFuncName(ClassName, FuncName)); \
	LuaLibFuncSig(ClassName, FuncName) { \
		auto self = Cast<ClassName>(*obj);
#define LuaLibClassFunc(ClassName, FuncName) \
	LuaLibClassFuncSig(ClassName, FuncName); \
	RegisterClassFunc LuaLibRegName(ClassName, FuncName) (ClassName::StaticClass(), #FuncName, &LuaLibFuncName(ClassName, FuncName)); \
	LuaLibClassFuncSig(ClassName, FuncName) { \
		auto self = TSubclassOf<ClassName>(clazz);

#define LuaLibFuncGetNum(ClassName, FuncName, RealFuncName) \
	LuaLibFunc(ClassName, FuncName) \
		lua_pushnumber(L, (lua_Number) self-> RealFuncName ()); \
		return 1; \
	}
#define LuaLibFuncGetInt(ClassName, FuncName, RealFuncName) \
	LuaLibFunc(ClassName, FuncName) \
		lua_pushinteger(L, (lua_Integer) self-> RealFuncName ()); \
		return 1; \
	}
#define LuaLibFuncGetBool(ClassName, FuncName, RealFuncName) \
	LuaLibFunc(ClassName, FuncName) \
		lua_pushboolean(L, (int) self-> RealFuncName ()); \
		return 1; \
	}

namespace FicsItKernel {
	namespace Lua {

		/* #################### */
		/* # Object Instances # */
		/* #################### */

		// Begin AActor

		LuaLibFunc(AActor, getPowerConnectors)
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGPowerConnectionComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		}
		
		LuaLibFunc(AActor, getFactoryConnectors)
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		}
		
		LuaLibFunc(AActor, getInventories)
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGInventoryComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		}
		
		LuaLibFunc(AActor, getNetworkConnectors)
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFINNetworkConnector::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		}

		// End AActor

		// Begin UFGInventoryComponent
		
		LuaLibFunc(UFGInventoryComponent, getStack)
			FInventoryStack* stack = new FInventoryStack();
			for (int i = 1; i <= args; ++i) {
				if (self->GetStackFromIndex((int)lua_tointeger(L, i), *stack)) {
					luaStruct(L, *stack);
				} else lua_pushnil(L);
			}
			delete stack;

			return args;
		}

		LuaLibFunc(UFGInventoryComponent, getItemCount)
			lua_pushinteger(L, self->GetNumItems(nullptr));
			return 1;
		}

		LuaLibFuncGetInt(UFGInventoryComponent, getSize, GetSizeLinear)

		LuaLibFunc(UFGInventoryComponent, sort)
			self->SortInventory();
			return 0;
		}

		// End UFGInventoryComponent

		// Begin UFGPowerConnectionComponent

		LuaLibFuncGetInt(UFGPowerConnectionComponent, getConnections, GetNumConnections)
		LuaLibFuncGetInt(UFGPowerConnectionComponent, getMaxConnections, GetMaxNumConnections)

		LuaLibFunc(UFGPowerConnectionComponent, getPower)
			newInstance(L, obj / self->GetPowerInfo());
			return 1;
		}

		LuaLibFunc(UFGPowerConnectionComponent, getCircuit)
			newInstance(L, obj / self->GetPowerCircuit());
			return 1;
		}

		// End UFGPowerConnectionComponent

		// Begin UFGPowerInfoComponent

		LuaLibFuncGetNum(UFGPowerInfoComponent, getDynProduction,		GetRegulatedDynamicProduction)
		LuaLibFuncGetNum(UFGPowerInfoComponent, getBaseProduction,		GetBaseProduction)
		LuaLibFuncGetNum(UFGPowerInfoComponent, getMaxDynProduction,	GetDynamicProductionCapacity)
		LuaLibFuncGetNum(UFGPowerInfoComponent, getTargetConsumption,	GetTargetConsumption)
		LuaLibFuncGetNum(UFGPowerInfoComponent, getConsumption,			GetBaseProduction)
		
		LuaLibFunc(UFGPowerInfoComponent, getCircuit)
			newInstance(L, obj / self->GetPowerCircuit());
			return 1;
		}

		LuaLibFunc(UFGPowerInfoComponent, hasPower)
			lua_pushboolean(L, self->HasPower());
			return 1;
		}
		
		// End UFGPowerInfoComponent

		// Begin UFGPowerCircuit

		LuaLibFunc(UFGPowerCircuit, getProduction)
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerProduced);
			return 1;
		}
		
		LuaLibFunc(UFGPowerCircuit, getConsumption)
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerConsumed);
			return 1;
		}
		
		LuaLibFunc(UFGPowerCircuit, getProductionCapacity)
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerProductionCapacity);
			return 1;
		}

		LuaLibFunc(UFGPowerCircuit, isFuesed)
			lua_pushboolean(L, self->IsFuseTriggered());
			return 1;
		}

		// End UFGPowerCircuit

		// Begin UFGFactoryConnectionComponent
		
		LuaLibFuncGetInt(UFGFactoryConnectionComponent, getType,		GetConnector)
		LuaLibFuncGetInt(UFGFactoryConnectionComponent, getDirection,	GetDirection)
		LuaLibFuncGetInt(UFGFactoryConnectionComponent, isConnected,	IsConnected)

		LuaLibFunc(UFGFactoryConnectionComponent, getInventory)
			newInstance(L, obj / self->GetInventory());
			return 1;
		}

		LuaLibFunc(UFGFactoryConnectionComponent, hook)
			luaHook(L, obj);
			return 1;
		}

		// End UFGFactoryConnectionComponent

		// Begin AFGBuildableFactory

		LuaLibFuncGetNum(AFGBuildableFactory, getProgress,				GetProductionProgress)
		LuaLibFuncGetNum(AFGBuildableFactory, getPowerConsumProducing,	GetProducingPowerConsumption)
		LuaLibFuncGetNum(AFGBuildableFactory, getProductivity,			GetProductivity)
		LuaLibFuncGetNum(AFGBuildableFactory, getCycleTime,				GetProductionCycleTime)
		LuaLibFuncGetNum(AFGBuildableFactory, getPotential,				GetPendingPotential)
		LuaLibFuncGetNum(AFGBuildableFactory, getMaxPotential,			GetMaxPossiblePotential)
		LuaLibFuncGetNum(AFGBuildableFactory, getMinPotential,			GetMinPotential)
		
		LuaLibFunc(AFGBuildableFactory, setPotential)
			if (args < 1) {
				return 0;
			}
			auto p = (float)lua_tonumber(L, -args);
			float min = self->GetMinPotential();
			float max = self->GetMaxPossiblePotential();
			self->SetPendingPotential((min > p) ? min : ((max < p) ? max : p));
			return 0;
		}
		
		// End AFGBuildableFactory

		// Begin AFGBuildableManufacturer

		LuaLibFunc(AFGBuildableManufacturer, getRecipe)
			newInstance(L, self->GetCurrentRecipe());
			return 1;
		}
		
		LuaLibFunc(AFGBuildableManufacturer, getRecipes)
			TArray<TSubclassOf<UFGRecipe>> recipes;
			self->GetAvailableRecipes(recipes);
			lua_newtable(L);
			int i = 1;
			for (auto recipe : recipes) {
				newInstance(L, recipe);
				lua_seti(L, -2, i++);
			}
			return 1;
		}
		
		LuaLibFunc(AFGBuildableManufacturer, setRecipe)
			if (args < 1) {
				return 0;
			}
			auto r = getClassInstance<UFGRecipe>(L, 1);
			FString name;
			if (!r && lua_isstring(L, -args)) {
				name = lua_tostring(L, -args);
			}
			TArray<TSubclassOf<UFGRecipe>> recipes;
			self->GetAvailableRecipes(recipes);
			FText t;
			for (auto recipe : recipes) {
				if ((UClass*)r == recipe || (name == UFGRecipe::GetRecipeName(recipe).ToString())) {
					self->SetRecipe(recipe);
					return 0;
				}
			}
			self->SetRecipe(nullptr);
			return 0;
		}

		// End AFGBuildableManufacturer

		/* ################### */
		/* # Class Instances # */
		/* ################### */

		// Begin UFGRecipe

		LuaLibClassFunc(UFGRecipe, getName)
			FText name = UFGRecipe::GetRecipeName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		}

		LuaLibClassFunc(UFGRecipe, getProducts)
			auto products = UFGRecipe::GetProducts(self);
			lua_newtable(L);
			int in = 1;
			for (auto& product : products) {
				luaStruct(L, product);
				lua_seti(L, -2, in++);
			}
			return 1;
		}
		LuaLibClassFunc(UFGRecipe, getIngredients)
			auto ingredients = UFGRecipe::GetIngredients(self);
			lua_newtable(L);
			int in = 1;
			for (auto& ingredient : ingredients) {
				luaStruct(L, ingredient);
				lua_seti(L, -2, in++);
			}
			return 1;
		}
		LuaLibClassFunc(UFGRecipe, getDuration)
			lua_pushnumber(L, UFGRecipe::GetManufacturingDuration(self));
			return 1;
		}

		// End UFGRecipe

		// Begin UFGItemDescriptor
		
		LuaLibClassFunc(UFGItemDescriptor, getName)
			auto name = UFGItemDescriptor::GetItemName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		}
	}
}