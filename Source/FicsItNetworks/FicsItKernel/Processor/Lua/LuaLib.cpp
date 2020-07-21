#include "LuaLib.h"


#include "FGBuildableDockingStation.h"
#include "FGBuildablePipeReservoir.h"
#include "FGBuildableRailroadStation.h"
#include "FGBuildableTrainPlatformCargo.h"
#include "LuaStructs.h"

#include "Network/FINNetworkConnector.h"

#include "FGPowerConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "FGPowerCircuit.h"
#include "FGFactoryConnectionComponent.h"
#include "FGRailroadSubsystem.h"
#include "FGRailroadTimeTable.h"
#include "FGTrain.h"
#include "FGTrainStationIdentifier.h"
#include "LuaInstance.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "util/ReflectionHelper.h"
#include "FGLocomotive.h"
#include "FGBuildableRailroadSwitchControl.h"
#include "FGBuildableRailroadSignal.h"
#include "FGPipeSubsystem.h"
#include "FicsItKernel/Network/SmartSignal.h"

#define LuaLibTypeRegName(ClassName) ClassName ## _Reg
#define LuaLibFuncName(ClassName, FuncName) ClassName ## _ ## FuncName
#define LuaLibFuncRegName(ClassName, FuncName) ClassName ## _ ## FuncName ## _Reg
#define LuaLibPropSetName(ClassName, PropName) ClassName ## _ ## PropName ## _Set
#define LuaLibPropGetName(ClassName, PropName) ClassName ## _ ## PropName ## _Get
#define LuaLibHookRegName(ClassName, HookClass) ClassName ## _ ## HookName ## _Reg
#define LuaLibTypeDecl(ClassName, TypeName) \
	LuaLibType<ClassName>::RegisterData LuaLibTypeRegName(ClassName) (#TypeName);
#define LuaLibFunc(ClassName, FuncName, Code) \
	int LuaLibFuncName(ClassName, FuncName) (lua_State* L, int args, const Network::NetworkTrace& obj) { \
		ClassName* self = Cast<ClassName>(*obj); \
		Code \
	} \
	typename LuaLibType<ClassName>::RegisterFunc LuaLibFuncRegName(ClassName, FuncName) (#FuncName, & LuaLibFuncName(ClassName, FuncName) );
#define LuaLibProp(ClassName, PropName, Get, Set) \
	int LuaLibPropGetName(ClassName, PropName) (lua_State* L, const Network::NetworkTrace& obj) { \
		ClassName* self = Cast<ClassName>(*obj); \
		Get \
	} \
	int LuaLibPropSetName(ClassName, PropName) (lua_State* L, const Network::NetworkTrace& obj) { \
		ClassName* self = Cast<ClassName>(*obj); \
		Set \
	} \
	typename LuaLibType<ClassName>::RegisterProperty LuaLibFuncRegName(ClassName, PropName) (#PropName, LuaLibProperty{ & LuaLibPropGetName(ClassName, PropName) , false, & LuaLibPropSetName(ClassName, PropName) } );
#define LuaLibPropReadonly(ClassName, PropName, Get) \
	int LuaLibPropGetName(ClassName, PropName) (lua_State* L, const Network::NetworkTrace& obj) { \
		ClassName* self = Cast<ClassName>(*obj); \
		Get \
	} \
	typename LuaLibType<ClassName>::RegisterProperty LuaLibFuncRegName(ClassName, PropName) (#PropName, LuaLibProperty{ & LuaLibPropGetName(ClassName, PropName) } );
#define LuaLibPropReadonlyInt(ClassName, PropName, Get) \
	LuaLibPropReadonly(ClassName, PropName, { \
		lua_pushinteger(L, (lua_Integer) self-> Get); \
		return 1; \
	})
#define LuaLibPropReadonlyNum(ClassName, PropName, Get) \
	LuaLibPropReadonly(ClassName, PropName, { \
		lua_pushnumber(L, (lua_Number) self-> Get); \
		return 1; \
	})
#define LuaLibPropReadonlyBool(ClassName, PropName, Get) \
	LuaLibPropReadonly(ClassName, PropName, { \
		lua_pushboolean(L, (int) self-> Get); \
		return 1; \
	})

#define LuaLibHook(ClassName, HookName) \
	LuaLibType<ClassName>::RegisterHook LuaLibHookRegName(ClassName, HookName) ( [](TSubclassOf<UFINHook>& hook) { \
		hook = HookName ::StaticClass(); \
	});
#define LuaLibClassTypeDecl(ClassName, TypeName) \
	LuaLibClassType<ClassName>::RegisterData LuaLibTypeRegName(ClassName) (#TypeName);
#define LuaLibClassFunc(ClassName, FuncName, Code) \
	int LuaLibFuncName(ClassName, FuncName) (lua_State* L, int args, UClass* clazz) { \
		TSubclassOf<ClassName> self = clazz; \
		Code \
	} \
	typename LuaLibClassType<ClassName>::RegisterFunc LuaLibFuncRegName(ClassName, FuncName) (#FuncName, & LuaLibFuncName(ClassName, FuncName) );

#define LuaLibFuncGetNum(ClassName, FuncName, RealFuncName) \
	LuaLibFunc(ClassName, FuncName, { \
		lua_pushnumber(L, (lua_Number) self-> RealFuncName); \
		return 1; \
	})
#define LuaLibFuncGetInt(ClassName, FuncName, RealFuncName) \
	LuaLibFunc(ClassName, FuncName, { \
		lua_pushinteger(L, (lua_Integer) self-> RealFuncName); \
		return 1; \
	})
#define LuaLibFuncGetBool(ClassName, FuncName, RealFuncName) \
	LuaLibFunc(ClassName, FuncName, { \
		lua_pushboolean(L, (int) self-> RealFuncName); \
		return 1; \
	})

TSet<FWeakObjectPtr> UFINFactoryConnectorHook::Senders;
bool UFINFactoryConnectorHook::registered = false;
FCriticalSection UFINFactoryConnectorHook::MutexFactoryGrab;
TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> UFINFactoryConnectorHook::FactoryGrabsRunning;

TSet<FWeakObjectPtr> UFINPowerCircuitHook::Senders;
bool UFINPowerCircuitHook::registered = false;
FCriticalSection UFINPowerCircuitHook::Mutex;
TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> UFINPowerCircuitHook::FactoryGrabsRunning;

namespace FicsItKernel {
	namespace Lua {
		template<typename T>
		class LuaLibType {
		private:
			std::vector<std::pair<std::string, LuaLibFunc>> funcs;
			std::vector<std::pair<std::string, LuaLibProperty>> props;
			std::string name;
			std::function<void(TSubclassOf<UFINHook>&)> hook;

			LuaLibType() {
				this->hook = [](TSubclassOf<UFINHook>& hook){ hook = nullptr; };
				LuaLib::get()->registerRegFunc([this](UClass*& type, std::string& name, std::vector<std::pair<std::string, LuaLibFunc>>& funcs, std::vector<std::pair<std::string, LuaLibProperty>>& props, TSubclassOf<UFINHook>& hook) {
					type = T::StaticClass();
					name = this->name;
					funcs = this->funcs;
					props = this->props;
					this->hook(hook);
				});
			}
			
		public:
			static LuaLibType* get() {
				static LuaLibType* instance = nullptr;
				if (!instance) instance = new LuaLibType();
				return instance;
			}

			struct RegisterFunc {
				RegisterFunc(const std::string& name, const LuaLibFunc& func) {
					LuaLibType::get()->funcs.push_back({name, func});
				}
			};

			struct RegisterData {
				RegisterData(const std::string& name) {
					LuaLibType::get()->name = name;
				}
			};

			struct RegisterHook {
				RegisterHook(std::function<void(TSubclassOf<UFINHook>& hook)> hookFunc) {
					LuaLibType::get()->hook = hookFunc;
				}
			};

			struct RegisterProperty {
				RegisterProperty(const std::string& name, const LuaLibProperty& prop) {
					LuaLibType::get()->props.push_back({name, prop});
				}
			};
		};

		template<typename T>
		class LuaLibClassType {
		private:
			std::vector<std::pair<std::string, LuaLibClassFunc>> funcs;
			std::string name;

			LuaLibClassType() {
				LuaLib::get()->registerRegFunc([this](UClass*& type, std::string& name, std::vector<std::pair<std::string, LuaLibClassFunc>>& funcs) {
					type = T::StaticClass();
					name = this->name;
					funcs = this->funcs;
				});
			}
			
		public:
			static LuaLibClassType* get() {
				static LuaLibClassType* instance = nullptr;
				if (!instance) instance = new LuaLibClassType();
				return instance;
			}

			struct RegisterFunc {
				RegisterFunc(const std::string& name, const LuaLibClassFunc& func) {
					LuaLibClassType::get()->funcs.push_back({name, func});
				}
			};

			struct RegisterData {
				RegisterData(const std::string& name) {
					LuaLibClassType::get()->name = name;
				}
			};
		};
		
		LuaLib* LuaLib::get() {
			static LuaLib* instance = nullptr;
			if (!instance) instance = new LuaLib();
			return instance;
		}

		void LuaLib::registerLib() {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
			for (const ToRegisterFunc& regfunc : toRegister) {
				UClass* type;
				std::string typeName;
				std::vector<std::pair<std::string, LuaLibFunc>> funcs;
				std::vector<std::pair<std::string, LuaLibProperty>> props;
				TSubclassOf<UFINHook> hook = nullptr;
				regfunc(type, typeName, funcs, props, hook);
				reg->registerType(type, typeName, false);
				for (const std::pair<std::string, LuaLibFunc> func : funcs) {
					reg->registerFunction(type, func.first, func.second);
				}
				for (const std::pair<std::string, LuaLibProperty> prop : props) {
					reg->registerProperty(type, prop.first, prop.second);
				}
				if (hook) AFINHookSubsystem::RegisterHook(type, hook);
			}
			for (const ToRegisterClassFunc& regfunc : toRegisterClasses) {
				UClass* type;
				std::string typeName;
				std::vector<std::pair<std::string, LuaLibClassFunc>> funcs;
				regfunc(type, typeName, funcs);
				reg->registerType(type, typeName, true);
				for (const std::pair<std::string, LuaLibClassFunc> func : funcs) {
					reg->registerClassFunction(type, func.first, func.second);
				}
			}
		}

		void LuaLib::registerRegFunc(const ToRegisterFunc& func) {
			toRegister.push_back(func);
		}

		void LuaLib::registerRegFunc(const ToRegisterClassFunc& func) {
			toRegisterClasses.push_back(func);
		}

		/* #################### */
		/* # Object Instances # */
		/* #################### */

		LuaLibTypeDecl(UObject, Object)
		
		// Begin AActor

		LuaLibTypeDecl(AActor, Actor)
		
		LuaLibFunc(AActor, getLocation, {
			FVector loc = self->GetActorLocation();
			lua_pushnumber(L, loc.X);
			lua_pushnumber(L, loc.Y);
			lua_pushnumber(L, loc.Z);
			return 3;
		})

		LuaLibFunc(AActor, getRotation, {
			FRotator rot = self->GetActorRotation();
			lua_pushnumber(L, rot.Pitch);
			lua_pushnumber(L, rot.Yaw);
			lua_pushnumber(L, rot.Roll);
			return 3;
		})

		LuaLibFunc(AActor, getPowerConnectors, {
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGPowerConnectionComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		})
		
		LuaLibFunc(AActor, getFactoryConnectors, {
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		})
		
		LuaLibFunc(AActor, getInventories, {
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGInventoryComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		})
		
		LuaLibFunc(AActor, getNetworkConnectors, {
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFINNetworkConnector::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		})

		// End AActor

		// Begin UFGInventoryComponent

		LuaLibTypeDecl(UFGInventoryComponent, Inventory)
		
		LuaLibFunc(UFGInventoryComponent, getStack, {
			FInventoryStack stack;
			for (int i = 1; i <= args; ++i) {
				if (self->GetStackFromIndex((int)lua_tointeger(L, i), stack)) {
					luaStruct(L, stack);
				} else lua_pushnil(L);
			}
			return args;
		})

		LuaLibPropReadonlyInt(UFGInventoryComponent, itemCount, GetNumItems(nullptr))
		LuaLibPropReadonlyInt(UFGInventoryComponent, size, GetSizeLinear())

		LuaLibFunc(UFGInventoryComponent, sort, {
			self->SortInventory();
			return 0;
		})

		LuaLibFunc(UFGInventoryComponent, flush, {
			TArray<FInventoryStack> stacks;
			self->GetInventoryStacks(stacks);
			self->Empty();
			for (const FInventoryStack& stack : stacks) {
				if (stack.HasItems() && stack.Item.IsValid() && !UFGItemDescriptor::CanBeDiscarded(stack.Item.ItemClass)) {
					self->AddStack(stack);
				}
			}
			return 0;
		})

		// End UFGInventoryComponent

		// Begin UFGPowerConnectionComponent

		LuaLibTypeDecl(UFGPowerConnectionComponent, PowerConnection)

		LuaLibPropReadonlyInt(UFGPowerConnectionComponent, connections, GetNumConnections())
		LuaLibPropReadonlyInt(UFGPowerConnectionComponent, maxConnections, GetMaxNumConnections())

		LuaLibFunc(UFGPowerConnectionComponent, getPower, {
			newInstance(L, obj / self->GetPowerInfo());
			return 1;
		})

		LuaLibFunc(UFGPowerConnectionComponent, getCircuit, {
			newInstance(L, obj / self->GetPowerCircuit());
			return 1;
		})

		// End UFGPowerConnectionComponent

		// Begin UFGPowerInfoComponent

		LuaLibTypeDecl(UFGPowerInfoComponent, PowerInfo)

		LuaLibPropReadonlyNum(UFGPowerInfoComponent, dynProduction,		GetRegulatedDynamicProduction())
		LuaLibPropReadonlyNum(UFGPowerInfoComponent, baseProduction,		GetBaseProduction())
		LuaLibPropReadonlyNum(UFGPowerInfoComponent, maxDynProduction,	GetDynamicProductionCapacity())
		LuaLibPropReadonlyNum(UFGPowerInfoComponent, targetConsumption,	GetTargetConsumption())
		LuaLibPropReadonlyNum(UFGPowerInfoComponent, consumption,			GetBaseProduction())
		LuaLibPropReadonlyBool(UFGPowerInfoComponent, hasPower, HasPower())
		
		LuaLibFunc(UFGPowerInfoComponent, getCircuit, {
			newInstance(L, obj / self->GetPowerCircuit());
			return 1;
		})
		
		// End UFGPowerInfoComponent

		// Begin UFGPowerCircuit

		LuaLibTypeDecl(UFGPowerCircuit, PowerCircuit)

		LuaLibHook(UFGPowerCircuit, UFINPowerCircuitHook)

		LuaLibPropReadonly(UFGPowerCircuit, production, {
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerProduced);
			return 1;
		})
		
		LuaLibPropReadonly(UFGPowerCircuit, consumption, {
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerConsumed);
			return 1;
		})
		
		LuaLibPropReadonly(UFGPowerCircuit, capacity, {
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerProductionCapacity);
			return 1;
		})

		LuaLibPropReadonlyBool(UFGPowerCircuit, isFuesed, IsFuseTriggered())

		// End UFGPowerCircuit

		// Begin UFGFactoryConnectionComponent

		LuaLibTypeDecl(UFGFactoryConnectionComponent, FactoryConnection)

		LuaLibHook(UFGFactoryConnectionComponent, UFINFactoryConnectorHook)
		
		LuaLibPropReadonlyInt(UFGFactoryConnectionComponent, type,			GetConnector())
		LuaLibPropReadonlyInt(UFGFactoryConnectionComponent, direction,		GetDirection())
		LuaLibPropReadonlyBool(UFGFactoryConnectionComponent, isConnected,	IsConnected())

		LuaLibFunc(UFGFactoryConnectionComponent, getInventory, {
			newInstance(L, obj / self->GetInventory());
			return 1;
		})

		// End UFGFactoryConnectionComponent

		// Begin AFGBuildableFactory

		LuaLibTypeDecl(AFGBuildableFactory, Factory)

		LuaLibPropReadonlyNum(AFGBuildableFactory, progress,				GetProductionProgress())
		LuaLibPropReadonlyNum(AFGBuildableFactory, powerConsumProducing,	GetProducingPowerConsumption())
		LuaLibPropReadonlyNum(AFGBuildableFactory, productivity,			GetProductivity())
		LuaLibPropReadonlyNum(AFGBuildableFactory, cycleTime,			GetProductionCycleTime())
		LuaLibPropReadonlyNum(AFGBuildableFactory, maxPotential,			GetMaxPossiblePotential())
		LuaLibPropReadonlyNum(AFGBuildableFactory, minPotential,			GetMinPotential())

		LuaLibProp(AFGBuildableFactory, standby, {
			lua_pushboolean(L, self->IsProductionPaused());
			return 1;
		}, {
			self->SetIsProductionPaused(lua_toboolean(L, 1));
			return 0;
		})

		LuaLibProp(AFGBuildableFactory, potential, {
			lua_pushnumber(L, self->GetPendingPotential());
			return 1;
		}, {
			float p = static_cast<float>(luaL_checknumber(L, 1));
			float min = self->GetMinPotential();
			float max = self->GetMaxPossiblePotential();
			self->SetPendingPotential((min > p) ? min : ((max < p) ? max : p));
			return 0;
		})
		
		// End AFGBuildableFactory

		// Begin AFGBuildableManufacturer

		LuaLibTypeDecl(AFGBuildableManufacturer, Manufacturer)

		LuaLibFunc(AFGBuildableManufacturer, getRecipe, {
			newInstance(L, self->GetCurrentRecipe());
			return 1;
		})
		
		LuaLibFunc(AFGBuildableManufacturer, getRecipes, {
			TArray<TSubclassOf<UFGRecipe>> recipes;
			self->GetAvailableRecipes(recipes);
			lua_newtable(L);
			int i = 1;
			for (auto recipe : recipes) {
				newInstance(L, recipe);
				lua_setfield(L, -2, TCHAR_TO_UTF8(*UFGRecipe::GetRecipeName(recipe).ToString()));
			}
			return 1;
		})

		void ManufacturerSetRecipeResolve(TSharedRef<FFINDynamicStructHolder> In, TSharedRef<FFINDynamicStructHolder> Out) {
			TArray<TSubclassOf<UFGRecipe>> recipes;
			FFINManufacturerSetRecipeInData& InData = In->Get<FFINManufacturerSetRecipeInData>();
			AFGBuildableManufacturer* self = InData.Manufacturer.Get();
			self->GetAvailableRecipes(recipes);
			if (recipes.Contains(InData.Recipe)) {
				TArray<FInventoryStack> stacks;
				self->GetInputInventory()->GetInventoryStacks(stacks);
				self->GetOutputInventory()->AddStacks(stacks);
				self->SetRecipe(InData.Recipe);
				Out->Get<FFINBoolData>().Data = true;
			} else {
				Out->Get<FFINBoolData>().Data = false;
			}
		}

		RegisterFuturePointer(ManufacturerSetRecipeResolve, &ManufacturerSetRecipeResolve)

		int ManufacturerSetRecipeRetrieve(lua_State* L, TSharedRef<FFINDynamicStructHolder> In) {
			lua_pushboolean(L, In->Get<FFINBoolData>().Data);
			return 1;
		}
		RegisterFuturePointer(ManufacturerSetRecipeRetrieve, &ManufacturerSetRecipeRetrieve)
		
		LuaLibFunc(AFGBuildableManufacturer, setRecipe, {
			if (args < 1) {
				return 0;
			}
			TSubclassOf<UFGRecipe> recipe = getClassInstance<UFGRecipe>(L,1);
			TSharedPtr<FFINDynamicStructHolder> holder1;
			TSharedPtr<FFINDynamicStructHolder> holder2;
			luaFuture(L, MakeShared<LuaFutureStruct>(holder1 = MakeShared<FFINDynamicStructHolder>(FFINManufacturerSetRecipeInData::StaticStruct()), holder2 = MakeShared<FFINDynamicStructHolder>(FFINBoolData::StaticStruct()), ManufacturerSetRecipeResolve, ManufacturerSetRecipeRetrieve));
			holder1->Get<FFINManufacturerSetRecipeInData>().Manufacturer = self;
			holder1->Get<FFINManufacturerSetRecipeInData>().Recipe = recipe;
			return 1;
		})

		LuaLibFunc(AFGBuildableManufacturer, getInputInv, {
			newInstance(L, obj / self->GetInputInventory());
			return 1;
		})

		LuaLibFunc(AFGBuildableManufacturer, getOutputInv, {
			newInstance(L, obj / self->GetOutputInventory());
			return 1;
		})

		// End AFGBuildableManufacturer

		// Begin AFGBuildableTrainPlatform

		LuaLibTypeDecl(AFGBuildableTrainPlatform, TrainPlatform)

		LuaLibFunc(AFGBuildableTrainPlatform, getTrackGraph, {
			luaTrackGraph(L, obj, self->GetTrackGraphID());
			return 1;
		})

		LuaLibFunc(AFGBuildableTrainPlatform, getTrackPos, {
			FRailroadTrackPosition pos = self->GetTrackPosition();
			if (!pos.IsValid()) return 0;
			newInstance(L, obj(pos.Track.Get()));
			lua_pushnumber(L, pos.Offset);
			lua_pushnumber(L, pos.Forward);
			return 3;
		})

		LuaLibFunc(AFGBuildableTrainPlatform, getConnectedPlatform, {
			int direction = lua_tointeger(L, 1);
			newInstance(L, obj / self->GetConnectedPlatformInDirectionOf(direction));
			return 1;
		})

		LuaLibFunc(AFGBuildableTrainPlatform, getDockedVehicle, {
			newInstance(L, obj / FReflectionHelper::GetObjectPropertyValue<UObject>(self, TEXT("mDockedRailroadVehicle")));
			return 1;
		})

		LuaLibFunc(AFGBuildableTrainPlatform, getMaster, {
			newInstance(L, obj / FReflectionHelper::GetObjectPropertyValue<UObject>(self, TEXT("mStationDockingMaster")));
			return 1;
		})

		LuaLibFunc(AFGBuildableTrainPlatform, getDockedLocomotive, {
            newInstance(L, obj / FReflectionHelper::GetObjectPropertyValue<UObject>(self, TEXT("mDockingLocomotive")));
            return 1;
        })

		LuaLibPropReadonlyInt(AFGBuildableTrainPlatform, status, GetDockingStatus())
		LuaLibPropReadonlyBool(AFGBuildableTrainPlatform, isReversed, IsOrientationReversed())
		
		// End AFGBuildableTrainPlatform

		// Begin AFGBuildableRailroadStation

		LuaLibTypeDecl(AFGBuildableRailroadStation, RailroadStation)

		LuaLibProp(AFGBuildableRailroadStation, name, {
		   	lua_pushstring(L, TCHAR_TO_UTF8(*self->GetStationIdentifier()->GetStationName().ToString()));
			return 1;
		},{
			self->GetStationIdentifier()->SetStationName(FText::FromString(luaL_checkstring(L, 1)));
			return 0;
		})

		LuaLibPropReadonlyNum(AFGBuildableRailroadStation, dockedOffset, GetDockedVehicleOffset())

		// End AFGBuildableRailroadStation

		// Begin AFGBuildableTrainPlatformCargo

		LuaLibTypeDecl(AFGBuildableTrainPlatformCargo, TrainPlatformCargo)
		
		LuaLibPropReadonlyBool(AFGBuildableTrainPlatformCargo, isLoading, GetIsInLoadMode())
		LuaLibPropReadonlyBool(AFGBuildableTrainPlatformCargo, isUnloading, IsLoadUnloading())
		LuaLibPropReadonlyNum(AFGBuildableTrainPlatformCargo, dockedOffset, GetDockedVehicleOffset())
		LuaLibPropReadonlyNum(AFGBuildableTrainPlatformCargo, outputFlow, GetOutflowRate())
		LuaLibPropReadonlyNum(AFGBuildableTrainPlatformCargo, inputFlow, GetInflowRate())
		LuaLibPropReadonlyBool(AFGBuildableTrainPlatformCargo, fullLoad, IsFullLoad())
		LuaLibPropReadonlyBool(AFGBuildableTrainPlatformCargo, fullUnload, IsFullUnload())
		
		// End AFGBuildableTrainPlatformCargo

		// Begin AFGRailroadVehicle

		LuaLibTypeDecl(AFGRailroadVehicle, RailroadVehicle)

		LuaLibFunc(AFGRailroadVehicle, getTrain, {
			newInstance(L, obj / Cast<UObject>(self->GetTrain()));
			return 1;
		})

		LuaLibFunc(AFGRailroadVehicle, isCoupled, {
			lua_pushboolean(L, self->IsCoupledAt(static_cast<ERailroadVehicleCoupler>(lua_tointeger(L, 1))));
			return 1;
		})

		LuaLibFunc(AFGRailroadVehicle, getCoupled, {
			newInstance(L, obj / self->GetCoupledVehicleAt(static_cast<ERailroadVehicleCoupler>(lua_tointeger(L, 1))));
			return 1;
		})

		LuaLibFunc(AFGRailroadVehicle, getTrackGraph, {
			luaTrackGraph(L, obj, self->GetTrackGraphID());
			return 1;
		})

		LuaLibFunc(AFGRailroadVehicle, getTrackPos, {
			FRailroadTrackPosition pos = self->GetTrackPosition();
			if (!pos.IsValid()) return 0;
			newInstance(L, obj(pos.Track.Get()));
			lua_pushnumber(L, pos.Offset);
			lua_pushnumber(L, pos.Forward);
			return 3;
		})

		LuaLibFunc(AFGRailroadVehicle, getMovement, {
			newInstance(L, obj / self->GetRailroadVehicleMovementComponent());
			return 1;
		})

		LuaLibPropReadonlyNum(AFGRailroadVehicle, length, GetLength())
		LuaLibPropReadonlyBool(AFGRailroadVehicle, isDocked, IsDocked())
		LuaLibPropReadonlyBool(AFGRailroadVehicle, isReversed, IsOrientationReversed())
		
		// End AFGRailroadVehicle

		// Begin UFGRailroadVehicleMovementComponent

		LuaLibTypeDecl(UFGRailroadVehicleMovementComponent, RailroadVehicleMovement)
		
		LuaLibFunc(UFGRailroadVehicleMovementComponent, getVehicle, {
			newInstance(L, obj / self->GetOwningRailroadVehicle());
			return 1;
		})

		LuaLibFunc(UFGRailroadVehicleMovementComponent, getWheelsetRotation, {
			FVector rot = self->GetWheelsetRotation(luaL_checkinteger(L, 1));
			lua_pushnumber(L, rot.X);
			lua_pushnumber(L, rot.Y);
			lua_pushnumber(L, rot.Z);
			return 3;
		})

		LuaLibFunc(UFGRailroadVehicleMovementComponent, getWheelsetOffset, {
			lua_pushnumber(L, self->GetWheelsetOffset(luaL_checkinteger(L, 1)));
			return 1;
		})
		
		LuaLibFunc(UFGRailroadVehicleMovementComponent, getCouplerRotationAndExtention, {
			float extension;
			FVector rotation = self->GetCouplerRotationAndExtention(luaL_checkinteger(L, 1), extension);
			lua_pushnumber(L, rotation.X);
			lua_pushnumber(L, rotation.Y);
			lua_pushnumber(L, rotation.Z);
			lua_pushnumber(L, extension);
			return 4;
		})
		
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, orientation, GetOrientation())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, mass, GetMass())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, tareMass, GetTareMass())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, payloadMass, GetPayloadMass())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, speed, GetForwardSpeed())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, relativeSpeed, GetRelativeForwardSpeed())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, maxSpeed, GetMaxForwardSpeed())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, gravitationalForce, GetGravitationalForce())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, tractiveForce, GetTractiveForce())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, resistiveForce, GetResistiveForce())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, gradientForce, GetGradientForce())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, brakingForce, GetBrakingForce())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, airBrakingForce, GetAirBrakingForce())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, dynamicBrakingForce, GetDynamicBrakingForce())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, maxTractiveEffort, GetMaxTractiveEffort())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, maxDynamicBrakingEffort, GetMaxDynamicBrakingEffort())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, maxAirBrakingEffort, GetMaxAirBrakingEffort())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, trackGrade, GetTrackGrade())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, trackCurvature, GetTrackCurvature())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, wheelsetAngle, GetWheelsetAngle())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, rollingResistance, GetRollingResistance())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent,  curvatureResistance, GetCurvatureResistance())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, airResistance, GetAirResistance())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, gradientResistance, GetGradientResistance())
		LuaLibPropReadonlyNum(UFGRailroadVehicleMovementComponent, wheelRotation, GetWheelRotation())
		LuaLibPropReadonlyInt(UFGRailroadVehicleMovementComponent, numWheelsets, GetNumWheelsets())
		LuaLibPropReadonlyBool(UFGRailroadVehicleMovementComponent, isMoving, IsMoving())
		
		// End UFGRailroadVehicleMovementComponent

		// Begin AFGTrain

		LuaLibTypeDecl(AFGTrain, Train)

		LuaLibHook(AFGTrain, UFINTrainHook);

		LuaLibFunc(AFGTrain, getName, {
			lua_pushstring(L, TCHAR_TO_UTF8(*self->GetTrainName().ToString()));
			return 1;
		})
		
		LuaLibFunc(AFGTrain, setName, {
			self->SetTrainName(FText::FromString(luaL_checkstring(L, 1)));
			return 0;
		})

		LuaLibFunc(AFGTrain, getTrackGraph, {
			luaTrackGraph(L, obj, self->GetTrackGraphID());
			return 1;
		})

		LuaLibFunc(AFGTrain, setSelfDriving, {
			self->SetSelfDrivingEnabled(lua_toboolean(L, 1));
			return 0;
		})

		LuaLibFunc(AFGTrain, getMaster, {
			newInstance(L, obj / self->GetMultipleUnitMaster());
			return 1;
		})

		LuaLibFunc(AFGTrain, getTimeTable, {
			newInstance(L, obj / self->GetTimeTable());
			return 1;
		})

		LuaLibFunc(AFGTrain, newTimeTable, {
			newInstance(L, obj / self->NewTimeTable());
			return 1;
		})

		LuaLibFunc(AFGTrain, getFirst, {
			newInstance(L, obj / self->GetFirstVehicle());
			return 1;
		})

		LuaLibFunc(AFGTrain, getLast, {
			newInstance(L, obj / self->GetLastVehicle());
			return 1;
		})

		LuaLibFunc(AFGTrain, dock, {
			self->Dock();
			return 0;
		})

		LuaLibFunc(AFGTrain, getVehicles, {
			lua_newtable(L);
			int i = 1;
			for (AFGRailroadVehicle* vehicle : self->mSimulationData.SimulatedVehicles) {
				newInstance(L, obj / vehicle);
				lua_seti(L, -2, ++i);
			}
			return 1;
		})

		LuaLibFuncGetBool(AFGTrain, isPlayerDriven, IsPlayerDriven())
		LuaLibFuncGetBool(AFGTrain, isSelfDriving, IsSelfDrivingEnabled())
		LuaLibFuncGetInt(AFGTrain, getSelfDrivingError, GetSelfDrivingError())
		
		LuaLibPropReadonlyBool(AFGTrain, hasTimeTable, HasTimeTable())
		LuaLibPropReadonlyInt(AFGTrain, getDockState, GetDockingState())
		LuaLibPropReadonlyBool(AFGTrain, isDocked, IsDocked())
		
		// End AFGTrain

		// Begin AFGRailroadTimeTable

		LuaLibTypeDecl(AFGRailroadTimeTable, TimeTable)

		LuaLibFunc(AFGRailroadTimeTable, addStop, {
			int stopIndex = luaL_checkinteger(L, 1);
			FTimeTableStop stop;
			stop.Station = getObjInstance<AFGBuildableRailroadStation>(L, 2)->GetStationIdentifier();
			stop.Duration = luaL_checknumber(L, 3);
			lua_pushboolean(L, self->AddStop(stopIndex, stop));
			return 1;
		})

		LuaLibFunc(AFGRailroadTimeTable, removeStop, {
			self->RemoveStop(luaL_checkinteger(L, 1));
			return 0;
		})

		LuaLibFunc(AFGRailroadTimeTable, getStops, {
			lua_newtable(L);
			TArray<FTimeTableStop> stops;
			self->GetStops(stops);
			for (int i = 0; i < stops.Num(); ++i) {
				const FTimeTableStop& stop = stops[i];
				luaTimeTableStop(L, obj / stop.Station->GetStation(), stop.Duration);
			}
			return 1;
		})

		LuaLibFunc(AFGRailroadTimeTable, setStops, {
			luaL_argcheck(L, lua_istable(L, 1), 1, "is not of type table");
			TArray<FTimeTableStop> stops;
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				stops.Add(luaGetTimeTableStop(L, -1));
				lua_pop(L, 1);
			}
			lua_pushboolean(L, self->SetStops(stops));
			return 1;
		})

		LuaLibFunc(AFGRailroadTimeTable, isValidStop, {
			lua_pushboolean(L, self->IsValidStop(luaL_checkinteger(L, 1)));
			return 1;
		})

		LuaLibFunc(AFGRailroadTimeTable, getStop, {
			FTimeTableStop stop = self->GetStop(luaL_checkinteger(L, 1));
			if (IsValid(stop.Station)) {
				luaTimeTableStop(L, obj / stop.Station->GetStation(), stop.Duration);
			} else {
				lua_pushnil(L);
			}
			return 1;
		})

		LuaLibFunc(AFGRailroadTimeTable, setCurrentStop, {
			self->SetCurrentStop(luaL_checkinteger(L, 1));
			return 0;
		})

		LuaLibFunc(AFGRailroadTimeTable, incrementCurrentStop, {
			self->IncrementCurrentStop();
			return 0;
		})
		
		LuaLibFuncGetInt(AFGRailroadTimeTable, getCurrentStop, GetCurrentStop())

		LuaLibPropReadonlyInt(AFGRailroadTimeTable, numStops, GetNumStops())
		
		// End AFGRailroadTimeTable

		// Begin AFGBuildableRailroadTrack

		LuaLibTypeDecl(AFGBuildableRailroadTrack, RailroadTrack)

		LuaLibFunc(AFGBuildableRailroadTrack, getClosestTrackPosition, {
			FRailroadTrackPosition pos = self->FindTrackPositionClosestToWorldLocation(FVector(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
			if (!pos.IsValid()) return 0;
			newInstance(L, obj(pos.Track.Get()));
			lua_pushnumber(L, pos.Offset);
			lua_pushnumber(L, pos.Forward);
			return 3;
		})

		LuaLibFunc(AFGBuildableRailroadTrack, getWorldLocAndRotAtPos, {
			FRailroadTrackPosition pos(getObjInstance<AFGBuildableRailroadTrack>(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
			FVector loc;
			FVector rot;
			self->GetWorldLocationAndDirectionAtPosition(pos, loc, rot);
			lua_pushnumber(L, loc.X);
			lua_pushnumber(L, loc.Y);
			lua_pushnumber(L, loc.Z);
			lua_pushnumber(L, rot.X);
			lua_pushnumber(L, rot.Y);
			lua_pushnumber(L, rot.Z);
			return 6;
		})

		LuaLibFunc(AFGBuildableRailroadTrack, getConnection, {
			newInstance(L, obj / self->GetConnection(luaL_checkinteger(L, 1)));
			return 1;
		})

		LuaLibFunc(AFGBuildableRailroadTrack, getTrackGraph, {
			luaTrackGraph(L, obj, self->GetTrackGraphID());
			return 1;
		})
		
		LuaLibPropReadonlyNum(AFGBuildableRailroadTrack, length, GetLength())
		LuaLibPropReadonlyBool(AFGBuildableRailroadTrack, isOwnedByPlatform, GetIsOwnedByPlatform())
		
		// End AFGBuildableRailroadTrack

		// Begin UFGRailroadTrackConnectionComponent

		LuaLibTypeDecl(UFGRailroadTrackConnectionComponent, RailroadTrackConnection)

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getConnectorLocation, {
			FVector loc = self->GetConnectorLocation();
			lua_pushnumber(L, loc.X);
			lua_pushnumber(L, loc.Y);
			lua_pushnumber(L, loc.Z);
			return 3;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getConnectorNormal, {
			FVector loc = self->GetConnectorNormal();
			lua_pushnumber(L, loc.X);
			lua_pushnumber(L, loc.Y);
			lua_pushnumber(L, loc.Z);
			return 3;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getConnection, {
			if (lua_isinteger(L, 1)) {
				newInstance(L, obj / self->GetConnection(lua_tointeger(L, 1)));
			} else {
				newInstance(L, obj / self->GetConnection());
			}
			return 1;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getConnections, {
			lua_newtable(L);
			int i = 1;
			for (UFGRailroadTrackConnectionComponent* conn : self->GetConnections()) {
				newInstance(L, obj / conn);
				lua_seti(L, -2, i++);
			}
			return 1;
		})
		
		LuaLibFunc(UFGRailroadTrackConnectionComponent, getTrackPosition, {
			FRailroadTrackPosition pos = self->GetTrackPosition();
			if (!pos.IsValid()) return 0;
			newInstance(L, obj(pos.Track.Get()));
			lua_pushnumber(L, pos.Offset);
			lua_pushnumber(L, pos.Forward);
			return 3;
		})
		
		LuaLibFunc(UFGRailroadTrackConnectionComponent, getTrack, {
			newInstance(L, obj / self->GetTrack());
			return 1;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getSwitchControl, {
			newInstance(L, obj / self->GetSwitchControl());
			return 1;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getStation, {
			newInstance(L, obj / self->GetStation());
			return 1;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getSignal, {
			newInstance(L, obj / self->GetSignal());
			return 1;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getOpposite, {
			newInstance(L, obj / self->GetOpposite());
			return 1;
		})

		LuaLibFunc(UFGRailroadTrackConnectionComponent, getNext, {
			newInstance(L, obj / self->GetNext());
			return 1;
		})
		
		LuaLibFunc(UFGRailroadTrackConnectionComponent, setSwitchPosition, {
			if (lua_isinteger(L, 1)) self->SetSwitchPosition(luaL_checkinteger(L, 1));
			else self->SetSwitchPosition(getObjInstance<AFGBuildableRailroadTrack>(L, 1));
			return 0;
		})
		LuaLibFuncGetInt(UFGRailroadTrackConnectionComponent, getSwitchPosition, GetSwitchPosition())
		
		LuaLibPropReadonlyBool(UFGRailroadTrackConnectionComponent, isConnected, IsConnected())
		LuaLibPropReadonlyBool(UFGRailroadTrackConnectionComponent, isFacingSwitch, IsFacingSwitch())
		LuaLibPropReadonlyBool(UFGRailroadTrackConnectionComponent, isTrailingSwitch, IsTrailingSwitch())
		LuaLibPropReadonlyInt(UFGRailroadTrackConnectionComponent, numSwitchPositions, GetNumSwitchPositions())
		
		// End UFGRailroadTrackConnectionComponent

		// Begin AFGBuildableRailroadSwitchControl

		LuaLibTypeDecl(AFGBuildableRailroadSwitchControl, RailroadSwitchControl)

		LuaLibFunc(AFGBuildableRailroadSwitchControl, toggleSwitch, {
			self->ToggleSwitchPosition();
			return 0;
		})
		
		LuaLibPropReadonlyInt(AFGBuildableRailroadSwitchControl, switchPosition, GetSwitchPosition())
		
		// End AFGBuildableRailroadSwitchControl

		// Begin AFGBuildableRailroadSignal

		// End AFGBuildableRailroadSignal

		// Begin AFGBuildableDockingStation

		LuaLibTypeDecl(AFGBuildableDockingStation, DockingStation)

		LuaLibFunc(AFGBuildableDockingStation, getFuelInv, {
			newInstance(L, obj / self->GetFuelInventory());
			return 1;
		})

		LuaLibFunc(AFGBuildableDockingStation, getInv, {
			newInstance(L, obj / self->GetInventory());
			return 1;
		})

		LuaLibFunc(AFGBuildableDockingStation, getDocked, {
			newInstance(L, obj / self->GetDockedActor());
			return 1;
		})

		LuaLibFunc(AFGBuildableDockingStation, undock, {
			self->Undock();
			return 0;
		})

		LuaLibProp(AFGBuildableDockingStation, isLoadMode, {
			lua_pushboolean(L, self->GetIsInLoadMode());
			return 1;
		}, {
			self->SetIsInLoadMode(lua_toboolean(L, 1));
			return 0;
		})
		
		LuaLibPropReadonlyBool(AFGBuildableDockingStation, isLoadUnloading, IsLoadUnloading())

		// End AFGBuildableDockingStation
		
		// Begin AFGBuildablePipeReservoir

		LuaLibTypeDecl(AFGBuildablePipeReservoir, "PipeReservoir")

		LuaLibFunc(AFGBuildablePipeReservoir, flush, {
			AFGPipeSubsystem::Get(self->GetWorld())->FlushIntegrant(self);
			return 0;
		})

		LuaLibFunc(AFGBuildablePipeReservoir, getFluidType, {
			newInstance(L, self->GetFluidDescriptor());
			return 1;
        })

		LuaLibPropReadonlyNum(AFGBuildablePipeReservoir, fluidContent, GetFluidBox()->Content)
		LuaLibPropReadonlyNum(AFGBuildablePipeReservoir, maxFluidContent, GetFluidBox()->MaxContent)
		LuaLibPropReadonlyNum(AFGBuildablePipeReservoir, flowFill, GetFluidBox()->FlowFill)
		LuaLibPropReadonlyNum(AFGBuildablePipeReservoir, flowDrain, GetFluidBox()->FlowDrain)
		LuaLibPropReadonlyNum(AFGBuildablePipeReservoir, flowLimit, GetFluidBox()->FlowLimit)

		// End AFGBuildablePipeReservoir
		
		/* ################### */
		/* # Class Instances # */
		/* ################### */

		// Begin UFGRecipe

		LuaLibClassTypeDecl(UFGRecipe, Recipe)

		LuaLibClassFunc(UFGRecipe, getName, {
			FText name = UFGRecipe::GetRecipeName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		})

		LuaLibClassFunc(UFGRecipe, getProducts, {
			TArray<FItemAmount> products = UFGRecipe::GetProducts(self);
			lua_newtable(L);
			int in = 1;
			for (auto& product : products) {
				luaStruct(L, product);
				lua_seti(L, -2, in++);
			}
			return 1;
		})
		
		LuaLibClassFunc(UFGRecipe, getIngredients, {
			auto ingredients = UFGRecipe::GetIngredients(self);
			lua_newtable(L);
			int in = 1;
			for (auto& ingredient : ingredients) {
				luaStruct(L, ingredient);
				lua_seti(L, -2, in++);
			}
			return 1;
		})
		
		LuaLibClassFunc(UFGRecipe, getDuration, {
			lua_pushnumber(L, UFGRecipe::GetManufacturingDuration(self));
			return 1;
		})

		// End UFGRecipe

		// Begin UFGItemDescriptor

		LuaLibClassTypeDecl(UFGItemDescriptor, ItemType)
		
		LuaLibClassFunc(UFGItemDescriptor, getName, {
			FText name = UFGItemDescriptor::GetItemName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		})

		LuaLibClassFunc(UFGItemDescriptor, __tostring, {
			FText name = UFGItemDescriptor::GetItemName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		})

		// End UFGItemDescriptor
	}
}
