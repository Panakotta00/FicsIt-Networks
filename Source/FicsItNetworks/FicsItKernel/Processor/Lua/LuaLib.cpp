#include "LuaLib.h"


#include "FGBuildableRailroadStation.h"
#include "FGBuildableTrainPlatformCargo.h"
#include "LuaStructs.h"
#include "LuaHooks.h"

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

using namespace FicsItKernel;
using namespace FicsItKernel::Lua;

class RegisterObjectFunc {
public:
	RegisterObjectFunc(UClass* clazz, std::string funcName, LuaLibFunc func) {
		FicsItKernel::Lua::instanceClasses()[clazz][funcName] = func;
	}
};

class RegisterClassFunc {
public:
	RegisterClassFunc(UClass* clazz, std::string funcName, LuaLibClassFunc func) {
		FicsItKernel::Lua::instanceSubclasses()[clazz][funcName] = func;
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
#define LuaLibFuncEnd }

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
		LuaLibFuncEnd
		
		LuaLibFunc(AActor, getFactoryConnectors)
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		LuaLibFuncEnd
		
		LuaLibFunc(AActor, getInventories)
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFGInventoryComponent::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		LuaLibFuncEnd
		
		LuaLibFunc(AActor, getNetworkConnectors)
			lua_newtable(L);
			int i = 1;
			auto connectors = self->GetComponentsByClass(UFINNetworkConnector::StaticClass());
			for (auto connector : connectors) {
				newInstance(L, obj / connector);
				lua_seti(L, -2, i++);
			}
			return 1;
		LuaLibFuncEnd

		// End AActor

		// Begin UFGInventoryComponent
		
		LuaLibFunc(UFGInventoryComponent, getStack)
			FInventoryStack stack;
			for (int i = 1; i <= args; ++i) {
				if (self->GetStackFromIndex((int)lua_tointeger(L, i), stack)) {
					luaStruct(L, stack);
				} else lua_pushnil(L);
			}
			return args;
		LuaLibFuncEnd

		LuaLibFunc(UFGInventoryComponent, getItemCount)
			lua_pushinteger(L, self->GetNumItems(nullptr));
			return 1;
		LuaLibFuncEnd

		LuaLibFuncGetInt(UFGInventoryComponent, getSize, GetSizeLinear)

		LuaLibFunc(UFGInventoryComponent, sort)
			self->SortInventory();
			return 0;
		LuaLibFuncEnd

		// End UFGInventoryComponent

		// Begin UFGPowerConnectionComponent

		LuaLibFuncGetInt(UFGPowerConnectionComponent, getConnections, GetNumConnections)
		LuaLibFuncGetInt(UFGPowerConnectionComponent, getMaxConnections, GetMaxNumConnections)

		LuaLibFunc(UFGPowerConnectionComponent, getPower)
			newInstance(L, obj / self->GetPowerInfo());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(UFGPowerConnectionComponent, getCircuit)
			newInstance(L, obj / self->GetPowerCircuit());
			return 1;
		LuaLibFuncEnd

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
		LuaLibFuncEnd

		LuaLibFunc(UFGPowerInfoComponent, hasPower)
			lua_pushboolean(L, self->HasPower());
			return 1;
		LuaLibFuncEnd
		
		// End UFGPowerInfoComponent

		// Begin UFGPowerCircuit

		LuaLibFunc(UFGPowerCircuit, getProduction)
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerProduced);
			return 1;
		LuaLibFuncEnd
		
		LuaLibFunc(UFGPowerCircuit, getConsumption)
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerConsumed);
			return 1;
		LuaLibFuncEnd
		
		LuaLibFunc(UFGPowerCircuit, getProductionCapacity)
			FPowerCircuitStats stats;
			self->GetStats(stats);
			lua_pushnumber(L, stats.PowerProductionCapacity);
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(UFGPowerCircuit, isFuesed)
			lua_pushboolean(L, self->IsFuseTriggered());
			return 1;
		LuaLibFuncEnd

		// End UFGPowerCircuit

		// Begin UFGFactoryConnectionComponent
		
		LuaLibFuncGetInt(UFGFactoryConnectionComponent, getType,		GetConnector)
		LuaLibFuncGetInt(UFGFactoryConnectionComponent, getDirection,	GetDirection)
		LuaLibFuncGetInt(UFGFactoryConnectionComponent, isConnected,	IsConnected)

		LuaLibFunc(UFGFactoryConnectionComponent, getInventory)
			newInstance(L, obj / self->GetInventory());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(UFGFactoryConnectionComponent, hook)
			luaHook(L, obj);
			return 1;
		LuaLibFuncEnd

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
		LuaLibFuncEnd
		
		// End AFGBuildableFactory

		// Begin AFGBuildableManufacturer

		LuaLibFunc(AFGBuildableManufacturer, getRecipe)
			newInstance(L, self->GetCurrentRecipe());
			return 1;
		LuaLibFuncEnd
		
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
		LuaLibFuncEnd
		
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
		LuaLibFuncEnd

		// End AFGBuildableManufacturer

		// Begin AFGBuildableTrainPlatform

		LuaLibFunc(AFGBuildableTrainPlatform, getTrack)
			luaTrackGraph(L, obj, self->GetTrackGraphID());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGBuildableTrainPlatform, getConnectedPlatform)
			int direction = lua_tointeger(L, 1);
			newInstance(L, obj / self->GetConnectedPlatformInDirectionOf(direction));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGBuildableTrainPlatform, getDockedVehicle)
            newInstance(L, obj / FReflectionHelper::GetObjectPropertyValue<UObject>(self, TEXT("mDockedRailroadVehicle")));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGBuildableTrainPlatform, getMaster)
            newInstance(L, obj / FReflectionHelper::GetObjectPropertyValue<UObject>(self, TEXT("mStationDockingMaster")));
			return 1;
		LuaLibFuncEnd

		LuaLibFuncGetInt(AFGBuildableTrainPlatform, getStatus, GetDockingStatus)
		LuaLibFuncGetBool(AFGBuildableTrainPlatform, isReversed, IsOrientationReversed)
		
		// End AFGBuildableTrainPlatform

		// Begin AFGBuildableRailroadStation

		LuaLibFunc(AFGBuildableRailroadStation, getName)
           	lua_pushstring(L, TCHAR_TO_UTF8(*self->GetStationIdentifier()->GetStationName().ToString()));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGBuildableRailroadStation, setName)
			self->GetStationIdentifier()->SetStationName(FText::FromString(luaL_checkstring(L, 1)));
			return 0;
		LuaLibFuncEnd

		LuaLibFunc(AFGBuildableTrainPlatform, getDockedLocomotive)
            newInstance(L, obj / FReflectionHelper::GetObjectPropertyValue<UObject>(self, TEXT("mDockingLocomotive")));
			return 1;
		LuaLibFuncEnd

		LuaLibFuncGetNum(AFGBuildableRailroadStation, getDockedOffset, GetDockedVehicleOffset)

		// End AFGBuildableRailroadStation

		// Begin AFGBuildableTrainPlatformCargo
		
		LuaLibFuncGetBool(AFGBuildableTrainPlatformCargo, isLoading, GetIsInLoadMode)
		LuaLibFuncGetBool(AFGBuildableTrainPlatformCargo, isUnloading, IsLoadUnloading)
		LuaLibFuncGetNum(AFGBuildableTrainPlatformCargo, getDockedOffset, GetDockedVehicleOffset)
		LuaLibFuncGetNum(AFGBuildableTrainPlatformCargo, getOutputFlow, GetOutflowRate)
		LuaLibFuncGetNum(AFGBuildableTrainPlatformCargo, getInputFlow, GetInflowRate)
		LuaLibFuncGetBool(AFGBuildableTrainPlatformCargo, getFullLoad, IsFullLoad)
		LuaLibFuncGetBool(AFGBuildableTrainPlatformCargo, getFullUnload, IsFullUnload)
		
		// End AFGBuildableTrainPlatformCargo

		// Begin AFGRailroadVehicle

		LuaLibFunc(AFGRailroadVehicle, getTrain)
            newInstance(L, obj / Cast<UObject>(self->GetTrain()));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadVehicle, isCoupled)
            lua_pushboolean(L, self->IsCoupledAt(static_cast<ERailroadVehicleCoupler>(lua_tointeger(L, 1))));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadVehicle, getCoupled)
            newInstance(L, obj / self->GetCoupledVehicleAt(static_cast<ERailroadVehicleCoupler>(lua_tointeger(L, 1))));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadVehicle, getTrack)
            luaTrackGraph(L, obj, self->GetTrackGraphID());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadVehicle, getMovement)
			newInstance(L, obj / self->GetRailroadVehicleMovementComponent());
			return 1;
		LuaLibFuncEnd

		LuaLibFuncGetNum(AFGRailroadVehicle, getLength, GetLength)
		LuaLibFuncGetBool(AFGRailroadVehicle, isDocked, IsDocked)
		LuaLibFuncGetBool(AFGRailroadVehicle, isReversed, IsOrientationReversed)
		
		// End AFGRailroadVehicle

		// Begin UFGRailroadVehicleMovementComponent
		
		LuaLibFunc(UFGRailroadVehicleMovementComponent, getVehicle)
			newInstance(L, obj / self->GetOwningRailroadVehicle());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(UFGRailroadVehicleMovementComponent, getWheelsetRotation)
			FVector rot = self->GetWheelsetRotation(luaL_checkinteger(L, 1));
			lua_pushnumber(L, rot.X);
			lua_pushnumber(L, rot.Y);
			lua_pushnumber(L, rot.Z);
			return 3;
		LuaLibFuncEnd

		LuaLibFunc(UFGRailroadVehicleMovementComponent, getWheelsetOffset)
			lua_pushnumber(L, self->GetWheelsetOffset(luaL_checkinteger(L, 1)));
			return 1;
		LuaLibFuncEnd
		
		LuaLibFunc(UFGRailroadVehicleMovementComponent, getCouplerRotationAndExtention)
			float extension;
			FVector rotation = self->GetCouplerRotationAndExtention(luaL_checkinteger(L, 1), extension);
			lua_pushnumber(L, rotation.X);
			lua_pushnumber(L, rotation.Y);
			lua_pushnumber(L, rotation.Z);
			lua_pushnumber(L, extension);
			return 4;
		LuaLibFuncEnd
		
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getOrientation, GetOrientation)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getMass, GetMass)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getTareMass, GetTareMass)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getPayloadMass, GetPayloadMass)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getSpeed, GetForwardSpeed)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getRelativeSpeed, GetRelativeForwardSpeed)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getMaxSpeed, GetMaxForwardSpeed)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getGravitationalForce, GetGravitationalForce)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getTractiveForce, GetTractiveForce)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getResistiveForce, GetResistiveForce)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getGradientForce, GetGradientForce)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getBrakingForce, GetBrakingForce)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getAirBrakingForce, GetAirBrakingForce)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getDynamicBrakingForce, GetDynamicBrakingForce)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getMaxTractiveEffort, GetMaxTractiveEffort)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getMaxDynamicBrakingEffort, GetMaxDynamicBrakingEffort)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getMaxAirBrakingEffort, GetMaxAirBrakingEffort)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getTrackGrade, GetTrackGrade)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getTrackCurvature, GetTrackCurvature)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getWheelsetAngle, GetWheelsetAngle)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getRollingResistance, GetRollingResistance)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getCurvatureResistance, GetCurvatureResistance)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getAirResistance, GetAirResistance)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getGradientResistance, GetGradientResistance)
		LuaLibFuncGetNum(UFGRailroadVehicleMovementComponent, getWheelRotation, GetWheelRotation)
		LuaLibFuncGetInt(UFGRailroadVehicleMovementComponent, getNumWheelsets, GetNumWheelsets)
		LuaLibFuncGetBool(UFGRailroadVehicleMovementComponent, isMoving, IsMoving)
		
		// End UFGRailroadVehicleMovementComponent

		// Begin AFGTrain

		LuaLibFunc(AFGTrain, getName)
			lua_pushstring(L, TCHAR_TO_UTF8(*self->GetTrainName().ToString()));
			return 1;
		LuaLibFuncEnd
		
		LuaLibFunc(AFGTrain, setName)
			self->SetTrainName(FText::FromString(luaL_checkstring(L, 1)));
			return 0;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, getTrack)
            luaTrackGraph(L, obj, self->GetTrackGraphID());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, setSelfDriving)
			self->SetSelfDrivingEnabled(lua_toboolean(L, 1));
			return 0;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, getMaster)
			newInstance(L, obj / self->GetMultipleUnitMaster());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, getTimeTable)
            newInstance(L, obj / self->GetTimeTable());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, newTimeTable)
            newInstance(L, obj / self->NewTimeTable());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, getFirst)
            newInstance(L, obj / self->GetFirstVehicle());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, getLast)
            newInstance(L, obj / self->GetLastVehicle());
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, dock)
			self->Dock();
			return 0;
		LuaLibFuncEnd

		LuaLibFunc(AFGTrain, getVehicles)
			lua_newtable(L);
			int i = 1;
			for (AFGRailroadVehicle* vehicle : self->mSimulationData.SimulatedVehicles) {
				newInstance(L, obj / vehicle);
				lua_seti(L, -2, ++i);
			}
			return 1;
		LuaLibFuncEnd

		LuaLibFuncGetBool(AFGTrain, isPlayerDriven, IsPlayerDriven)
		LuaLibFuncGetBool(AFGTrain, isSelfDriving, IsSelfDrivingEnabled)
		LuaLibFuncGetInt(AFGTrain, getSelfDrivingError, GetSelfDrivingError)
		LuaLibFuncGetBool(AFGTrain, hasTimeTable, HasTimeTable)
		LuaLibFuncGetInt(AFGTrain, getDockState, GetDockingState)
		LuaLibFuncGetBool(AFGTrain, isDocked, IsDocked)
		
		// End AFGTrain

		// Begin AFGRailroadTimeTable

		LuaLibFunc(AFGRailroadTimeTable, addStop)
			int stopIndex = luaL_checkinteger(L, 1);
			FTimeTableStop stop {
				getObjInstance<AFGBuildableRailroadStation>(L, 2)->GetStationIdentifier(),
				luaL_checknumber(L, 3)
			};
			lua_pushboolean(L, self->AddStop(stopIndex, stop));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadTimeTable, removeStop)
			self->RemoveStop(luaL_checkinteger(L, 1));
			return 0;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadTimeTable, getStops)
			lua_newtable(L);
			TArray<FTimeTableStop> stops;
			self->GetStops(stops);
			for (int i = 0; i < stops.Num(); ++i) {
				const FTimeTableStop& stop = stops[i];
				luaTimeTableStop(L, obj / stop.Station->GetStation(), stop.Duration);
			}
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadTimeTable, setStops)
            luaL_argcheck(L, lua_istable(L, 1), 1, "is not of type table");
			TArray<FTimeTableStop> stops;
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				stops.Add(luaGetTimeTableStop(L, -1));
				lua_pop(L, 1);
			}
			lua_pushboolean(L, self->SetStops(stops));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadTimeTable, isValidStop)
			lua_pushboolean(L, self->IsValidStop(luaL_checkinteger(L, 1)));
			return 1;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadTimeTable, getStop)
        	FTimeTableStop stop = self->GetStop(luaL_checkinteger(L, 1));
			if (IsValid(stop.Station)) {
				luaTimeTableStop(L, obj / stop.Station->GetStation(), stop.Duration);
			} else {
				lua_pushnil(L);
			}
        	return 1;
        LuaLibFuncEnd

		LuaLibFunc(AFGRailroadTimeTable, setCurrentStop)
			self->SetCurrentStop(luaL_checkinteger(L, 1));
			return 0;
		LuaLibFuncEnd

		LuaLibFunc(AFGRailroadTimeTable, incrementCurrentStop)
			self->IncrementCurrentStop();
			return 0;
		LuaLibFuncEnd

		LuaLibFuncGetInt(AFGRailroadTimeTable, getNumStops, GetNumStops)
		LuaLibFuncGetInt(AFGRailroadTimeTable, getCurrentStop, GetCurrentStop)
		
		// End AFGRailroadTimeTable

		/* ################### */
		/* # Class Instances # */
		/* ################### */

		// Begin UFGRecipe

		LuaLibClassFunc(UFGRecipe, getName)
			FText name = UFGRecipe::GetRecipeName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		LuaLibFuncEnd

		LuaLibClassFunc(UFGRecipe, getProducts)
			auto products = UFGRecipe::GetProducts(self);
			lua_newtable(L);
			int in = 1;
			for (auto& product : products) {
				luaStruct(L, product);
				lua_seti(L, -2, in++);
			}
			return 1;
		LuaLibFuncEnd
		
		LuaLibClassFunc(UFGRecipe, getIngredients)
			auto ingredients = UFGRecipe::GetIngredients(self);
			lua_newtable(L);
			int in = 1;
			for (auto& ingredient : ingredients) {
				luaStruct(L, ingredient);
				lua_seti(L, -2, in++);
			}
			return 1;
		LuaLibFuncEnd
		
		LuaLibClassFunc(UFGRecipe, getDuration)
			lua_pushnumber(L, UFGRecipe::GetManufacturingDuration(self));
			return 1;
		LuaLibFuncEnd

		// End UFGRecipe

		// Begin UFGItemDescriptor
		
		LuaLibClassFunc(UFGItemDescriptor, getName)
			FText name = UFGItemDescriptor::GetItemName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		LuaLibFuncEnd

		LuaLibClassFunc(UFGItemDescriptor, __tostring)
		    FText name = UFGItemDescriptor::GetItemName(self);
			lua_pushstring(L, TCHAR_TO_UTF8(*name.ToString()));
			return 1;
		LuaLibFuncEnd

		// End UFGItemDescriptor
	}
}