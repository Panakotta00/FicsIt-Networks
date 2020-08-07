#include "LuaStructs.h"

#include "CoreMinimal.h"
#include "FGBuildableTrainPlatform.h"
#include "FGRailroadSubsystem.h"
#include "FGRailroadTimeTable.h"
#include "FGTrain.h"
#include "FGTrainStationIdentifier.h"
#include "FGBuildableRailroadStation.h"
#include "FINGlobalRegisterHelper.h"
#include "LuaInstance.h"
#include "LuaProcessor.h"
#include "LuaProcessorStateStorage.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Network/FINFuture.h"
#include "Utils/FINTimeTableStop.h"
#include "Utils/FINTrackGraph.h"

#define PersistParams \
	const std::string& _persist_namespace, \
	const int _persist_permTableIdx, \
	const int _persist_upermTableIdx

namespace FicsItKernel {
	namespace Lua {
		int luaRetNull(lua_State* L) {
			return 0;
		}

		FFINLuaStructRegistry FFINLuaStructRegistry::Instance;

		void FFINLuaStructRegistry::RegisterStructType(UScriptStruct* Type, FString Name, StructSetupFunc Setup, StructConstructorFunc Constructor, StructGetterFunc Getter) {
			RegisteredStructTypes.Add(Type, FFINLuaStructRegisterData{Setup, Constructor, Getter});
			RegisteredStructTypeNames.Add(Type, Name);
			RegisteredNamesOfStructTypes.Add(Name, Type);
		}

		void FFINLuaStructRegistry::Setup(lua_State* L) {
			PersistSetup("Structs", -2);
			for (const TTuple<UScriptStruct*, FFINLuaStructRegisterData>& Data : RegisteredStructTypes) {
				Data.Value.Setup(L, _persist_namespace, _persist_permTableIdx, _persist_upermTableIdx);
			}
		}

		FString FFINLuaStructRegistry::GetName(UScriptStruct* Type) {
			FString* Name = RegisteredStructTypeNames.Find(Type);
			if (Name) return *Name;
			return TEXT("");
		}
		
		UScriptStruct* FFINLuaStructRegistry::GetType(FString Name) {
			UScriptStruct** Type = RegisteredNamesOfStructTypes.Find(Name);
			if (Type) return *Type;
			return nullptr;
		}

		bool FFINLuaStructRegistry::FindStructType(UScriptStruct* Type, StructConstructorFunc* Constructor, StructGetterFunc* Getter) {
			FFINLuaStructRegisterData* TypeData = nullptr;
			do {
				TypeData = RegisteredStructTypes.Find(Type);
				if (!TypeData) Type = Cast<UScriptStruct>(Type->GetSuperStruct());
			} while (!TypeData && Type);
			if (!TypeData) return false;
			if (Constructor) *Constructor = TypeData->Constructor;
			if (Getter) *Getter = TypeData->Getter;
			return true;
		}

		void luaStruct(lua_State* L, const FINStruct& Struct) {
			UScriptStruct* Type = Struct.GetStruct();

			FFINLuaStructRegistry::StructConstructorFunc Constructor;
			if (FFINLuaStructRegistry::Get().FindStructType(Type, &Constructor, nullptr)) {
				Constructor(L, Struct);
			} else {
				lua_pushnil(L);
			}
		}

		void luaGetStruct(lua_State* L, int i, FFINDynamicStructHolder& Struct) {
			FFINLuaStructRegistry::StructGetterFunc Getter;
			FString TypeName = FFINLuaStructRegistry::Get().GetName(Struct.GetStruct());
			i = lua_absindex(L, i);
			if (FString(luaL_typename(L, i)) != TypeName) luaL_argerror(L, i, (std::string("'") + TCHAR_TO_UTF8(*TypeName) + "'" + " expected, got '" + luaL_typename(L, i) + "'").c_str());
			if (!FFINLuaStructRegistry::Get().FindStructType(Struct.GetStruct(), nullptr, &Getter)) return;
			Getter(L, i, Struct);
		}

		FFINDynamicStructHolder luaGetStruct(lua_State* L, int i) {
			FString TypeName = luaL_typename(L, i);
			UScriptStruct* Type = FFINLuaStructRegistry::Get().GetType(TypeName);
			if (!Type) return FFINDynamicStructHolder();
			FFINDynamicStructHolder Struct(Type);
			luaGetStruct(L, i, Struct);
			return Struct;
		}

		template<typename T>
		struct TFINLuaStructRegisterer {
			TFINLuaStructRegisterer(const FString& Name, const FFINLuaStructRegistry::StructSetupFunc& Setup, const FFINLuaStructRegistry::StructConstructorFunc& Constructor, const FFINLuaStructRegistry::StructGetterFunc& Getter) {
				FFINGlobalRegisterHelper::AddFunction([=]() {
					FFINLuaStructRegistry::Get().RegisterStructType(T::StaticStruct(), Name, Setup, Constructor, Getter);
				});
			}
		};

		// Begin FInventoryItem

		int luaItemEQ(lua_State* L) {
			lua_getfield(L, 1, "type");
			lua_getfield(L, 2, "type");
			lua_pushboolean(L, lua_compare(L, 3, 4, LUA_OPEQ));
			return 1;
		}

		static const luaL_Reg luaItemLib[] = {
			{"__eq", luaItemEQ},
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{NULL, NULL}
		};

		TFINLuaStructRegisterer<FInventoryItem> GLuaInventoryItem(TEXT("Item"), [](lua_State* L, PersistParams) {
			luaL_newmetatable(L, "Item");
            luaL_setfuncs(L, luaItemLib, 0);
            PersistTable("Item", -1);
            lua_pop(L, 1);
		}, [](lua_State* L, const FFINDynamicStructHolder& Struct) {
			const FInventoryItem& item = Struct.Get<FInventoryItem>();
			if (!item.IsValid()) {
                lua_pushnil(L);
                return;
            }
            lua_newtable(L);
            newInstance(L, item.ItemClass);
            lua_setfield(L, -2, "type");
            // TODO: Add Item state to struct
            //luaItemState(L, item.ItemState);
            //lua_setfield(L, -2, "state");
            luaL_setmetatable(L, "Item");
		}, [](lua_State* L, int i, FFINDynamicStructHolder& Struct) {
			FInventoryItem& Item = Struct.Get<FInventoryItem>();
			lua_getfield(L, i, "type");
			Item.ItemClass = getClassInstance<UFGItemDescriptor>(L, -1);
			lua_pop(L, 1);
			return true;
		});

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
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{NULL, NULL}
		};
		
		TFINLuaStructRegisterer<FItemAmount> GLuaItemAmount(TEXT("ItemAmount"), [](lua_State* L, PersistParams) {
			luaL_newmetatable(L, "ItemAmount");
            luaL_setfuncs(L, luaItemAmountLib, 0);
            PersistTable("ItemAmount", -1);
            lua_pop(L, 1);
		}, [](lua_State* L, const FFINDynamicStructHolder& Struct) {
			const FItemAmount& amount = Struct.Get<FItemAmount>();
			lua_newtable(L);
            lua_pushinteger(L, amount.Amount);
            lua_setfield(L, -2, "count");
            newInstance(L, amount.ItemClass);
            lua_setfield(L, -2, "item");
            luaL_setmetatable(L, "ItemAmount");
        }, [](lua_State* L, int i, FFINDynamicStructHolder& Struct) {
            FItemAmount& Amount = Struct.Get<FItemAmount>();
            lua_getfield(L, i, "item");
            Amount.ItemClass = getClassInstance<UFGItemDescriptor>(L, -1);
        	lua_getfield(L, i, "count");
        	Amount.Amount = lua_tointeger(L, -1);
            lua_pop(L, 2);
            return true;
        });

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
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{NULL, NULL}
		};

		TFINLuaStructRegisterer<FInventoryStack> GLuaItemStack(TEXT("ItemStack"), [](lua_State* L, PersistParams) {
			luaL_newmetatable(L, "ItemStack");
            luaL_setfuncs(L, luaItemStackLib, 0);
            PersistTable("ItemStack", -1);
            lua_pop(L, 1);
		}, [](lua_State* L, const FFINDynamicStructHolder& Struct) {
			const FInventoryStack& stack = Struct.Get<FInventoryStack>();
			lua_newtable(L);
            lua_pushinteger(L, stack.NumItems);
            lua_setfield(L, -2, "count");
            luaStruct(L, stack.Item);
            lua_setfield(L, -2, "item");
            luaL_setmetatable(L, "ItemStack");
        }, [](lua_State* L, int i, FFINDynamicStructHolder& Struct) {
            FInventoryStack& Stack = Struct.Get<FInventoryStack>();
            lua_getfield(L, i, "item");
            Stack.Item = luaGetStruct<FInventoryItem>(L, -1);
            lua_getfield(L, i, "count");
            Stack.NumItems = lua_tointeger(L, -1);
            lua_pop(L, 2);
            return true;
        });

		// End FInventoryStack

		// Begin TrackGraph

		static FFINTrackGraph& luaTrackGraphGetAndCheck(lua_State* L, AFGRailroadSubsystem*& subSys) {
			FFINTrackGraph& track = *static_cast<FFINTrackGraph*>(luaL_checkudata(L, 1, "TrackGraph"));
			if (!track.IsValid()) luaL_argerror(L, 1, "TrackGraph is invalid");
			subSys = AFGRailroadSubsystem::Get(track.Trace.GetUnderlyingPtr().Get());
			return track;
		}

		int luaTrackGraphGetStations(lua_State* L) {
			AFGRailroadSubsystem* subSys = nullptr;
			FFINTrackGraph& graph = luaTrackGraphGetAndCheck(L, subSys);
			TArray<AFGTrainStationIdentifier*> stations;
			subSys->GetTrainStations(graph.TrackID, stations);
			lua_newtable(L);
			for (int i = 0; i < stations.Num(); ++i) {
				int trackID = graph.TrackID;
				newInstance(L, (graph.Trace / stations[i]->mStation));
				lua_seti(L, -2, i+1);
			}
			return 1;
		}

		int luaTrackGraphGetTrains(lua_State* L) {
			AFGRailroadSubsystem* subSys = nullptr;
			FFINTrackGraph& graph = luaTrackGraphGetAndCheck(L, subSys);
			TArray<AFGTrain*> trains;
			subSys->GetTrains(graph.TrackID, trains);
			lua_newtable(L);
			for (int i = 0; i < trains.Num(); ++i) {
				int trackID = graph.TrackID;
				newInstance(L, graph.Trace / trains[i]);
				lua_seti(L, -2, i+1);
			}
			return 1;
		}

		int luaTrackGraphEQ(lua_State* L) {
			FFINTrackGraph* track1 = static_cast<FFINTrackGraph*>(luaL_checkudata(L, 1, "TrackGraph"));
			FFINTrackGraph* track2 = static_cast<FFINTrackGraph*>(luaL_checkudata(L, 2, "TrackGraph"));
			lua_pushboolean(L, track1->TrackID == track2->TrackID);
			return 1;
		}

		int luaTrackGraphUnpersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));
			
			FFINTrackGraph* trackGrpah = static_cast<FFINTrackGraph*>(lua_newuserdata(L, sizeof(FFINTrackGraph)));
			new (trackGrpah) FFINTrackGraph(storage->GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)))->Get<FFINTrackGraph>());
			luaL_setmetatable(L, "TrackGraph");
			return 1;
		}

		int luaTrackGraphPersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));
	
			FFINTrackGraph* track = static_cast<FFINTrackGraph*>(luaL_checkudata(L, 1, "TrackGraph"));
			
			lua_pushinteger(L, storage->Add(MakeShared<FFINDynamicStructHolder>(*track)));
			lua_pushcclosure(L, luaTrackGraphUnpersist, 1);
			return 1;
		}

		int luaTrackGraphGC(lua_State* L) {
			FFINTrackGraph* track = static_cast<FFINTrackGraph*>(luaL_checkudata(L, 1, "TrackGraph"));
			track->~FFINTrackGraph();
			return 0;
		}
		
		static const luaL_Reg luaTrackGraphMetaLib[] = {
			{"__eq", luaTrackGraphEQ},
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{"__persist", luaTrackGraphPersist},
			{"__gc", luaTrackGraphGC},
			{NULL,NULL}
		};

		static const luaL_Reg luaTrackGraphLib[] = {
			{"getStations", luaTrackGraphGetStations},
			{"getTrains", luaTrackGraphGetTrains},
			{NULL, NULL}
		};

		TFINLuaStructRegisterer<FFINTrackGraph> GLuaTrackGraph(TEXT("TrackGraph"), [](lua_State* L, PersistParams) {
			luaL_newmetatable(L, "TrackGraph");
            luaL_setfuncs(L, luaTrackGraphMetaLib, 0);
            PersistTable("TrackGraph", -1);
            lua_newtable(L);
            luaL_setfuncs(L, luaTrackGraphLib, 0);
            PersistTable("TrackGraphLib", -1);
            lua_setfield(L, -2, "__index");
            lua_pop(L, 1);
            lua_pushcfunction(L, luaTrackGraphUnpersist);
            PersistValue("TrackGraphUnpersist");
		}, [](lua_State* L, const FINStruct& Struct) {
			FFINTrackGraph* trackGraph = static_cast<FFINTrackGraph*>(lua_newuserdata(L, sizeof(FFINTrackGraph)));
            new (trackGraph) FFINTrackGraph(Struct.Get<FFINTrackGraph>());
            luaL_setmetatable(L, "TrackGraph");
		}, [](lua_State* L, int i, FINStruct& Struct) {
			FFINTrackGraph* trackGraph = static_cast<FFINTrackGraph*>(luaL_checkudata(L, i, "TrackGraph"));
			Struct = *trackGraph;
			return true;
		});

		// End TrackGraph

		// Begin TimeTableStop

		static const luaL_Reg luaTimeTableStopLib[] = {
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{NULL,NULL}
		};

		void luaTimeTableStop(lua_State* L, const FFINNetworkTrace& station, float duration) {
			lua_newtable(L);
			newInstance(L, station);
			lua_setfield(L, -2, "station");
			lua_pushnumber(L, duration);
			lua_setfield(L, -2, "duration");
			luaL_setmetatable(L, "TimeTableStop");
		}

		TFINLuaStructRegisterer<FFINTimeTableStop> GLuaStructTrackGraphRegisterer(TEXT("TimeTableStop"), [](lua_State* L, PersistParams) {
			luaL_newmetatable(L, "TimeTableStop");
            luaL_setfuncs(L, luaTimeTableStopLib, 0);
            PersistTable("TimeTableStop", -1);
            lua_pop(L, 1);
        }, [](lua_State* L, const FINStruct& Struct) {
        	const FFINTimeTableStop& Stop = Struct.Get<FFINTimeTableStop>();
        	lua_newtable(L);
            newInstance(L, Stop.Station);
            lua_setfield(L, -2, "station");
            lua_pushnumber(L, Stop.Duration);
            lua_setfield(L, -2, "duration");
            luaL_setmetatable(L, "TimeTableStop");
        }, [](lua_State* L, int i, FINStruct& Struct) {
        	i = lua_absindex(L, i);
            if (!lua_istable(L, i)) luaL_error(L, "table contains non table value");
            lua_getfield(L, i, "station");
        	FFINNetworkTrace trace;
            AFGBuildableRailroadStation* station = getObjInstance<AFGBuildableRailroadStation>(L, i, &trace);
            if (!IsValid(station)) luaL_error(L, "table contains value with no valid station");
            lua_pop(L, 1);
            lua_getfield(L, i, "duration");
            if (!lua_isnumber(L, i)) luaL_error(L, "table contains value with no valid duration");
            float duration = lua_tonumber(L, i);
            lua_pop(L, 1);
            Struct = FFINTimeTableStop{trace, duration};
        });
		
		// End TimeTableStop

		// Begin LuaFuture

		typedef TSharedPtr<TFINDynamicStruct<FFINFuture>> LuaFuture;

		int luaFutureAwaitContinue(lua_State* L, int code, lua_KContext ctx) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			if ((*future)->IsDone()) {
				LuaValueReader reader(L);
				return ***future >> reader;
			}
			return lua_yieldk(L, LUA_MULTRET, NULL, luaFutureAwaitContinue);
		}
		
		int luaFutureAwait(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			return lua_yieldk(L, LUA_MULTRET, NULL, luaFutureAwaitContinue);
		}

		int luaFutureGet(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			luaL_argcheck(L, (*future)->IsDone(), 1, "Future is not ready");
			LuaValueReader reader(L);
			return ***future >> reader;
		}

		int luaFutureCanGet(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			lua_pushboolean(L, (*future)->IsDone());
			return 1;
		}

		int luaFutureGC(lua_State* L) {
			LuaFuture* future = static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			future->~LuaFuture();
			return 0;
		}

		int luaFutureUnpersist(lua_State* L) {
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));

			LuaFuture* future = static_cast<LuaFuture*>(lua_newuserdata(L, sizeof(LuaFuture)));
			new (future) LuaFuture(new TFINDynamicStruct<FFINFuture>(*storage->GetStruct(lua_tointeger(L, lua_upvalueindex(1)))));
			if (!(**future)->IsDone()) {
				KernelSystem* kernel = LuaProcessor::luaGetProcessor(L)->getKernel();
				kernel->pushFuture(*future);
			}
			return 1;
		}

		int luaFuturePersist(lua_State* L) {
			LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
			
			// get persist storage
			lua_getfield(L, LUA_REGISTRYINDEX, "PersistStorage");
			ULuaProcessorStateStorage* storage = static_cast<ULuaProcessorStateStorage*>(lua_touserdata(L, -1));
			
			lua_pushinteger(L, storage->Add(future));

			lua_pushcclosure(L, luaFutureUnpersist, 1);
			
			return 1;
		}

		static const luaL_Reg luaFutureLib[] = {
			{"await", luaFutureAwait},
			{"get", luaFutureGet},
			{"canGet", luaFutureCanGet},
			{NULL,NULL}
		};

		static const luaL_Reg luaFutureMetaLib[] = {
			{"__newindex", luaRetNull},
			{"__gc", luaFutureGC},
			{"__persist", luaFuturePersist},
			{NULL,NULL}
		};

		TFINLuaStructRegisterer<FFINFuture> GLuaFutureRegisterer(TEXT("Future"), [](lua_State* L, PersistParams) {
			luaL_newmetatable(L, "Future");
            luaL_setfuncs(L, luaFutureMetaLib, 0);
            PersistTable("Future", -1);
            lua_newtable(L);
            luaL_setfuncs(L, luaFutureLib, 0);
            PersistTable("FutureLib", -1);
            lua_setfield(L, -2, "__index");
            lua_pop(L, 1);
            lua_pushcfunction(L, luaFutureUnpersist);
            PersistValue("FutureUnpersist");
        }, [](lua_State* L, const FINStruct& Struct) {
            LuaFuture* future = static_cast<LuaFuture*>(lua_newuserdata(L, sizeof(LuaFuture)));
            new (future) LuaFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(Struct));
        	KernelSystem* kernel = LuaProcessor::luaGetProcessor(L)->getKernel();
        	kernel->pushFuture(*future);
            luaL_setmetatable(L, "Future");
        }, [](lua_State* L, int i, FINStruct& Struct) {
        	LuaFuture& future = *static_cast<LuaFuture*>(luaL_checkudata(L, 1, "Future"));
        	Struct = *future;
        });
		
		// End LuaFuture
	}
}
