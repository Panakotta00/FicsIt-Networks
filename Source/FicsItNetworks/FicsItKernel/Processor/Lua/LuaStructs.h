#pragma once

#include "FGBuildableRailroadTrack.h"
#include "FGInventoryLibrary.h"
#include "FGRailroadTimeTable.h"

#include "Lua.h"
#include "LuaProcessorStateStorage.h"
#include "FicsItKernel/Processor/FicsItFuture.h"
#include "Network/FINNetworkValues.h"

namespace FicsItKernel {
	namespace Lua {
		typedef int(*FutureRetrieveFunc)(lua_State*, TSharedRef<FFINDynamicStructHolder>);
		typedef void(*FutureResolveFunc)(TSharedRef<FFINDynamicStructHolder>, TSharedRef<FFINDynamicStructHolder>);

#define MakeLuaFuture(...) MakeShared<LuaFutureStruct>(__VA_ARGS__)
		
		class LuaFutureStruct : public FicsItFuture {
		public:
			TSharedPtr<FFINDynamicStructHolder> InData;
			TSharedPtr<FFINDynamicStructHolder> OutData;
			FutureResolveFunc ResolveFunc;
			FutureRetrieveFunc RetrieveFunc;
			bool valid = false;

			//LuaFutureStruct(TSharedPtr<FDynamicStructHolder> InDat, UStruct* OutDataStruct, FutureResolveFunc ResolveFunc, FutureRetrieveFunc RetrieveFunc);
			LuaFutureStruct(TSharedPtr<FFINDynamicStructHolder> InDat, TSharedPtr<FFINDynamicStructHolder> OutDataStruct, FutureResolveFunc ResolveFunc, FutureRetrieveFunc RetrieveFunc);
			~LuaFutureStruct();

			// Begin FicsItFuture
			void Execute() override;
			// Eng FicsItFuture

			bool IsValid();
			int Retrieve(lua_State* L);
		};

		typedef TSharedPtr<LuaFutureStruct> LuaFuture;

		void luaStruct(lua_State* L, const FINStruct& Struct);
		void luaStruct(lua_State* L, FInventoryItem item);
		void luaStruct(lua_State* L, FItemAmount amount);
		void luaStruct(lua_State* L, FInventoryStack stack);
		void luaTrackGraph(lua_State* L, const FFINNetworkTrace& trace ,int trackID);
		void luaTimeTableStop(lua_State* L, const FFINNetworkTrace& station, float duration);
		void luaFuture(lua_State* L, LuaFuture future);
		
		FTimeTableStop luaGetTimeTableStop(lua_State* L, int index);

		void setupStructs(lua_State* L);
	}
}
