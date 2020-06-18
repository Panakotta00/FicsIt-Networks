#include "LuaStructs.h"

#include "CoreMinimal.h"
#include "FGBuildableTrainPlatform.h"
#include "FGRailroadSubsystem.h"
#include "FGRailroadTimeTable.h"
#include "FGTrain.h"
#include "FGTrainStationIdentifier.h"
#include "FGBuildableRailroadStation.h"
#include "LuaInstance.h"

namespace FicsItKernel {
	namespace Lua {
		int luaRetNull(lua_State* L) {
			return 0;
		}

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

		void luaStruct(lua_State* L, FInventoryItem item) {
			lua_newtable(L);
			newInstance(L, item.ItemClass);
			lua_setfield(L, -2, "type");
			// TODO: Add Item state to struct
			//luaItemState(L, item.ItemState);
			//lua_setfield(L, -2, "state");
			luaL_setmetatable(L, "Item");
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
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{NULL, NULL}
		};

		void luaStruct(lua_State* L, FItemAmount amount) {
			lua_newtable(L);
			lua_pushinteger(L, amount.Amount);
			lua_setfield(L, -2, "count");
			newInstance(L, amount.ItemClass);
			lua_setfield(L, -2, "item");
			luaL_setmetatable(L, "ItemAmount");
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
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{NULL, NULL}
		};

		void luaStruct(lua_State* L, FInventoryStack stack) {
			lua_newtable(L);
			lua_pushinteger(L, stack.NumItems);
			lua_setfield(L, -2, "count");
			luaStruct(L, stack.Item);
			lua_setfield(L, -2, "item");
			luaL_setmetatable(L, "ItemStack");
		}

		// End FInventoryStack

		// Begin TrackGraph

		struct LuaTrackGraph {
			Network::NetworkTrace trace;
			int trackID;

			static int getTrackID(UObject* obj) {
				if (AFGBuildableTrainPlatform* platform = Cast<AFGBuildableTrainPlatform>(obj)) {
					return platform->GetTrackGraphID();
				}
				if (AFGRailroadVehicle* vehicle = Cast<AFGRailroadVehicle>(obj)) {
					return vehicle->GetTrackGraphID();
				}
				if (AFGTrain* train = Cast<AFGTrain>(obj)) {
					return train->GetTrackGraphID();
				}
				return -1;
			}

			bool isValid() {
				return getTrackID(*trace) >= 0;
			}

			static LuaTrackGraph& getAndCheck(lua_State* L, AFGRailroadSubsystem*& subSys) {
				LuaTrackGraph& track = *static_cast<LuaTrackGraph*>(luaL_checkudata(L, 1, "TrackGraph"));
				if (!track.isValid()) luaL_argerror(L, 1, "TrackGraph is invalid");
				subSys = AFGRailroadSubsystem::Get(track.trace.getUnderlyingPtr().Get());
				return track;
			}
		};

		int luaTrackGraphGetStations(lua_State* L) {
			AFGRailroadSubsystem* subSys = nullptr;
			LuaTrackGraph& graph = LuaTrackGraph::getAndCheck(L, subSys);
			TArray<AFGTrainStationIdentifier*> stations;
			subSys->GetTrainStations(graph.trackID, stations);
			lua_newtable(L);
			for (int i = 0; i < stations.Num(); ++i) {
				int trackID = graph.trackID;
				newInstance(L, (graph.trace / Network::ObjTraceStep(Cast<UObject>(stations[i]->mStation), [trackID](UObject* o1, UObject* o2) { 
					return trackID == LuaTrackGraph::getTrackID(o1) && trackID == LuaTrackGraph::getTrackID(o2);
				})));
				lua_seti(L, -2, i+1);
			}
			return 1;
		}

		int luaTrackGraphGetTrains(lua_State* L) {
			AFGRailroadSubsystem* subSys = nullptr;
			LuaTrackGraph& graph = LuaTrackGraph::getAndCheck(L, subSys);
			TArray<AFGTrain*> trains;
			subSys->GetTrains(graph.trackID, trains);
			lua_newtable(L);
			for (int i = 0; i < trains.Num(); ++i) {
				int trackID = graph.trackID;
				newInstance(L, (graph.trace / Network::ObjTraceStep(Cast<UObject>(trains[i]), [trackID](UObject* o1, UObject* o2) { 
                    return trackID == LuaTrackGraph::getTrackID(o1) && trackID == LuaTrackGraph::getTrackID(o2);
                })));
				lua_seti(L, -2, i+1);
			}
			return 1;
		}

		int luaTrackGraphEQ(lua_State* L) {
			LuaTrackGraph* track1 = static_cast<LuaTrackGraph*>(luaL_checkudata(L, 1, "TrackGraph"));
			LuaTrackGraph* track2 = static_cast<LuaTrackGraph*>(luaL_checkudata(L, 2, "TrackGraph"));
			lua_pushboolean(L, track1->trackID == track2->trackID);
			return 1;
		}

		int luaTrackGraphGC(lua_State* L) {
			LuaTrackGraph* track = static_cast<LuaTrackGraph*>(luaL_checkudata(L, 1, "TrackGraph"));
			track->~LuaTrackGraph();
			return 0;
		}
		
		static const luaL_Reg luaTrackGraphMetaLib[] = {
			{"__eq", luaTrackGraphEQ},
			{"__index", luaRetNull},
			{"__newindex", luaRetNull},
			{"__gc", luaTrackGraphGC},
			{NULL,NULL}
		};

		static const luaL_Reg luaTrackGraphLib[] = {
			{"getStations", luaTrackGraphGetStations},
			{"getTrains", luaTrackGraphGetTrains},
			{NULL, NULL}
		};

		void luaTrackGraph(lua_State* L, const Network::NetworkTrace& trace, int trackID) {
			LuaTrackGraph* trackGrpah = static_cast<LuaTrackGraph*>(lua_newuserdata(L, sizeof(LuaTrackGraph)));
			new (trackGrpah) LuaTrackGraph{trace, trackID};
			luaL_setmetatable(L, "TrackGraph");
		}

		// End TrackGraph

		// Begin TimeTableStop

		static const luaL_Reg luaTimeTableStopLib[] = {
			{"__newindex", luaRetNull},
			{"__gc", luaTrackGraphGC},
			{NULL,NULL}
		};

		void luaTimeTableStop(lua_State* L, const Network::NetworkTrace& station, float duration) {
			lua_newtable(L);
			newInstance(L, station);
			lua_setfield(L, -2, "station");
			lua_pushnumber(L, duration);
			lua_setfield(L, -2, "duration");
			luaL_setmetatable(L, "TimeTableStop");
		}

		FTimeTableStop luaGetTimeTableStop(lua_State* L, int index) {
			index = lua_absindex(L, index);
			if (!lua_istable(L, index)) luaL_error(L, "table contains non table value");
			lua_getfield(L, index, "station");
			AFGBuildableRailroadStation* station = getObjInstance<AFGBuildableRailroadStation>(L, index);
			if (!IsValid(station)) luaL_error(L, "table contains value with no valid station");
			lua_pop(L, 1);
			lua_getfield(L, index, "duration");
			if (!lua_isnumber(L, index)) luaL_error(L, "table contains value with no valid duration");
			float duration = lua_tonumber(L, index);
			lua_pop(L, 1);
			return FTimeTableStop{station->GetStationIdentifier(), duration};
		}
		
		// End TimeTableStop
		
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

			luaL_newmetatable(L, "TrackGraph");
			luaL_setfuncs(L, luaTrackGraphMetaLib, 0);
			PersistTable("TrackGraph", -1);
			lua_newtable(L);
			luaL_setfuncs(L, luaTrackGraphLib, 0);
			PersistTable("TrackGraph", -1);
			lua_setfield(L, -2, "__index");
			lua_pop(L, 1);

			luaL_newmetatable(L, "TimeTableStop");
			luaL_setfuncs(L, luaTimeTableStopLib, 0);
			PersistTable("TimeTableStop", -1);
			lua_pop(L, 1);
		}
	}
}
