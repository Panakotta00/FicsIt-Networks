#include "LuaLib.h"

#include <vector>


#include "FGBuildableDockingStation.h"
#include "FGBuildablePipeReservoir.h"
#include "FGBuildableRailroadStation.h"
#include "FGBuildableTrainPlatformCargo.h"
#include "FGBuildableRailroadSignal.h"
#include "LuaStructs.h"

#include "Network/FINNetworkConnectionComponent.h"

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
#include "FGPipeSubsystem.h"
#include "FINGlobalRegisterHelper.h"
#include "Network/FINNetworkComponent.h"
#include "Network/FINNetworkCustomType.h"
#include "Utils/FINTimeTableStop.h"
#include "Utils/FINTrackGraph.h"

#define LuaLibTypeRegName(ClassName) ClassName ## _Reg
#define LuaLibFuncName(ClassName, FuncName) ClassName ## _ ## FuncName
#define LuaLibFuncRegName(ClassName, FuncName) ClassName ## _ ## FuncName ## _Reg
#define LuaLibPropSetName(ClassName, PropName) ClassName ## _ ## PropName ## _Set
#define LuaLibPropGetName(ClassName, PropName) ClassName ## _ ## PropName ## _Get
#define LuaLibHookRegName(ClassName, HookClass) ClassName ## _ ## HookName ## _Reg
#define LuaLibTypeDecl(ClassName, TypeName) \
	LuaLibType<ClassName>::RegisterData LuaLibTypeRegName(ClassName) (#TypeName);
#define LuaLibFunc(ClassName, FuncName, Code) \
	int LuaLibFuncName(ClassName, FuncName) (lua_State* L, int args, LuaInstance* instance) { \
		FFINNetworkTrace obj = instance->Trace;\
		ClassName* self = Cast<ClassName>(*obj); \
		Code \
	} \
	typename LuaLibType<ClassName>::RegisterFunc LuaLibFuncRegName(ClassName, FuncName) (#FuncName, & LuaLibFuncName(ClassName, FuncName) );
#define LuaLibProp(ClassName, PropName, Get, Set) \
	int LuaLibPropGetName(ClassName, PropName) (lua_State* L, const FFINNetworkTrace& obj) { \
		ClassName* self = Cast<ClassName>(*obj); \
		Get \
	} \
	int LuaLibPropSetName(ClassName, PropName) (lua_State* L, const FFINNetworkTrace& obj) { \
		ClassName* self = Cast<ClassName>(*obj); \
		Set \
	} \
	typename LuaLibType<ClassName>::RegisterProperty LuaLibFuncRegName(ClassName, PropName) (#PropName, LuaLibProperty{ & LuaLibPropGetName(ClassName, PropName) , false, & LuaLibPropSetName(ClassName, PropName) } );
#define LuaLibPropReadonly(ClassName, PropName, Get) \
	int LuaLibPropGetName(ClassName, PropName) (lua_State* L, const FFINNetworkTrace& obj) { \
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
			TArray<TPair<FString, LuaLibFunc>> funcs;
			TArray<TPair<FString, LuaLibProperty>> props;
			FString name;
			TFunction<void(TSubclassOf<UFINHook>&)> hook = [](TSubclassOf<UFINHook>& H) { H = nullptr; };

			LuaLibType() {
				this->hook = [](TSubclassOf<UFINHook>& hook){ hook = nullptr; };
				FFINGlobalRegisterHelper::Get().AddFunction([this]() {
					LuaInstanceRegistry* reg = LuaInstanceRegistry::get();
					reg->registerType(T::StaticClass(), this->name, false);
					for (const TPair<FString, LuaLibFunc>& Func : this->funcs) reg->registerFunction(T::StaticClass(), Func.Key, Func.Value);
					for (const TPair<FString, LuaLibProperty>& Prop : this->props) reg->registerProperty(T::StaticClass(), Prop.Key, Prop.Value);
					if (hook) {
						TSubclassOf<UFINHook> HookType;
						hook(HookType);
						if (HookType) AFINHookSubsystem::RegisterHook(T::StaticClass(), HookType);
					}
				});
			}
			
		public:
			static LuaLibType* get() {
				static LuaLibType* instance = nullptr;
				if (!instance) instance = new LuaLibType();
				return instance;
			}

			struct RegisterFunc {
				RegisterFunc(const FString& name, const LuaLibFunc& func) {
					LuaLibType::get()->funcs.Add(TPair<FString, LuaLibFunc>{name, func});
				}
			};

			struct RegisterData {
				RegisterData(const FString& name) {
					LuaLibType::get()->name = name;
				}
			};

			struct RegisterHook {
				RegisterHook(std::function<void(TSubclassOf<UFINHook>& hook)> hookFunc) {
					LuaLibType::get()->hook = hookFunc;
				}
			};

			struct RegisterProperty {
				RegisterProperty(const FString& name, const LuaLibProperty& prop) {
					LuaLibType::get()->props.Add(TPair<FString, LuaLibProperty>{name, prop});
				}
			};
		};

		template<typename T>
		class LuaLibClassType {
		private:
			TArray<TPair<FString, LuaLibClassFunc>> funcs;
			FString name;

			LuaLibClassType() {
				FFINGlobalRegisterHelper::AddFunction([this]() {
					LuaInstanceRegistry::get()->registerType(T::StaticClass(), this->name, true);
					for (const TPair<FString, LuaLibClassFunc>& Func : funcs) {
						LuaInstanceRegistry::get()->registerClassFunction(T::StaticClass(), Func.Key, Func.Value);
					}
				});
			}
			
		public:
			static LuaLibClassType* get() {
				static LuaLibClassType* instance = nullptr;
				if (!instance) instance = new LuaLibClassType();
				return instance;
			}

			struct RegisterFunc {
				RegisterFunc(const FString& name, const LuaLibClassFunc& func) {
					LuaLibClassType::get()->funcs.Add(TPair<FString, LuaLibClassFunc>{name, func});
				}
			};

			struct RegisterData {
				RegisterData(const FString& name) {
					LuaLibClassType::get()->name = name;
				}
			};
		};

		/* #################### */
		/* # Object Instances # */
		/* #################### */

		LuaLibTypeDecl(UObject, Object)

		LuaLibFunc(UObject, getMembers, {
			LuaInstanceRegistry* reg = LuaInstanceRegistry::get();

			bool withType = false;
			if (lua_isboolean(L, 1)) withType = lua_toboolean(L, 1);
			
			lua_newtable(L);
            int i = 0;

            if (self->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
                lua_pushstring(L, "id");
                lua_seti(L, -2, ++i);
                lua_pushstring(L, "nick");
                lua_seti(L, -2, ++i);
            }
			
            UClass* type = self->GetClass();
			
            for (const TTuple<FString, int>& func : reg->getMembers(type)) {
                lua_pushstring(L, TCHAR_TO_UTF8(*func.Key));
            	if(withType) {
            		lua_pushinteger(L, func.Value);
					lua_settable(L, -3);
            	} else lua_seti(L, -2, ++i);
            }
            for (TFieldIterator<UFunction> func = TFieldIterator<UFunction>(type); func; ++func) {
                FString funcName = func->GetName();
                if (!(funcName.RemoveFromStart("netFunc_") && funcName.Len() > 0)) continue;
                lua_pushstring(L, TCHAR_TO_UTF8(*funcName));
				if (withType) {
					lua_pushinteger(L, 0);
					lua_settable(L, -3);
				} else lua_seti(L, -2, ++i);
            }

			return 1;
		})

		LuaLibFunc(UObject, getTypes, {
			lua_newtable(L);
			int i = 0;
			if (self->Implements<UFINNetworkCustomType>()) {
				lua_pushstring(L, TCHAR_TO_UTF8(*IFINNetworkCustomType::Execute_GetCustomTypeName(self)));
				lua_seti(L, -2, ++i);
			}
			UClass* Type = self->GetClass();
			FString lastType = "";
			while (Type) {
                FString typeName = LuaInstanceRegistry::get()->findTypeName(Type);
				if (typeName.Len() > 0 && lastType != typeName) {
					lastType = typeName;
					lua_pushstring(L, TCHAR_TO_UTF8(*typeName));
                	lua_seti(L, -2, ++i);
				}
                if (Type == UObject::StaticClass()) Type = nullptr;
                else Type = Type->GetSuperClass();
            }
			return 1;
		})
		
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
			const TSet<UActorComponent*>& Components = self->GetComponents();
			for (TFieldIterator<UObjectProperty> prop(self->GetClass()); prop; ++prop) {
                if (!prop->PropertyClass->IsChildOf(UFGPowerConnectionComponent::StaticClass())) continue;
                UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
				if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
                newInstance(L, obj / Connector);
                lua_seti(L, -2, i++);
            }
			return 1;
		})
		
		LuaLibFunc(AActor, getFactoryConnectors, {
			lua_newtable(L);
			int i = 1;
			const TSet<UActorComponent*>& Components = self->GetComponents();
			for (TFieldIterator<UObjectProperty> prop(self->GetClass()); prop; ++prop) {
                if (!prop->PropertyClass->IsChildOf(UFGFactoryConnectionComponent::StaticClass())) continue;
                UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
				if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
                newInstance(L, obj / Connector);
                lua_seti(L, -2, i++);
            }
			return 1;
		})
		
		LuaLibFunc(AActor, getInventories, {
			lua_newtable(L);
			int i = 1;
			const TSet<UActorComponent*>& Components = self->GetComponents();
			for (TFieldIterator<UObjectProperty> prop(self->GetClass()); prop; ++prop) {
				if (!prop->PropertyClass->IsChildOf(UFGInventoryComponent::StaticClass())) continue;
				UObject* inventory = *prop->ContainerPtrToValuePtr<UObject*>(self);
				if (!Components.Contains(Cast<UActorComponent>(inventory))) continue;
		        newInstance(L, obj / inventory);
		        lua_seti(L, -2, i++);
			}
			return 1;
		})
		
		LuaLibFunc(AActor, getNetworkConnectors, {
			lua_newtable(L);
			int i = 1;
			const TSet<UActorComponent*>& Components = self->GetComponents();
			for (TFieldIterator<UObjectProperty> prop(self->GetClass()); prop; ++prop) {
                if (!prop->PropertyClass->IsChildOf(UFINNetworkConnectionComponent::StaticClass())) continue;
                UObject* connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
				if (!Components.Contains(Cast<UActorComponent>(connector))) continue;
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

		LuaLibFunc(AFGBuildableManufacturer, setRecipe, {
			if (args < 1) {
				return 0;
			}
			TSubclassOf<UFGRecipe> recipe = getClassInstance<UFGRecipe>(L,1);
			luaStruct(L, FFINManufacturerSetRecipeFuture(self, recipe));
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
			luaStruct(L, FFINTrackGraph{obj, self->GetTrackGraphID()});
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
			luaStruct(L, FFINTrackGraph{obj, self->GetTrackGraphID()});
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
			luaStruct(L, FFINTrackGraph{obj, self->GetTrackGraphID()});
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
				luaStruct(L, FFINTimeTableStop{obj / stop.Station->GetStation(), stop.Duration});
			}
			return 1;
		})

		LuaLibFunc(AFGRailroadTimeTable, setStops, {
			luaL_argcheck(L, lua_istable(L, 1), 1, "is not of type table");
			TArray<FTimeTableStop> stops;
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				stops.Add(luaGetStruct<FFINTimeTableStop>(L, -1));
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
				luaStruct(L, FFINTimeTableStop{obj / stop.Station->GetStation(), stop.Duration});
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
			luaStruct(L, FFINTrackGraph{obj, self->GetTrackGraphID()});
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

/**
 * Futures
 */

void FFINManufacturerSetRecipeFuture::Execute() {
	TArray<TSubclassOf<UFGRecipe>> recipes;
	AFGBuildableManufacturer* self = Manufacturer.Get();
	self->GetAvailableRecipes(recipes);
	if (recipes.Contains(Recipe)) {
		TArray<FInventoryStack> stacks;
		self->GetInputInventory()->GetInventoryStacks(stacks);
		self->GetOutputInventory()->AddStacks(stacks);
		self->SetRecipe(Recipe);
		bGotSet = true;
	} else {
		bGotSet = false;
	}
	bDone = true;
}


