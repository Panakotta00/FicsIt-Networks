#pragma once

#include "FicsItKernel/Processor/Processor.h"
#include "FicsItKernel/Network/Signal.h"

#include <chrono>

struct lua_State;
struct lua_Debug;

namespace FicsItKernel {
	namespace Lua {
		class FLuaProcessorStateStorage {
		private:
			TArray<FFINNetworkTrace> Traces;
            TArray<UObject*> References;

		public:
			bool Serialize(FArchive& Ar);
			
			int32 Add(const FFINNetworkTrace& Trace);

			int32 Add(UObject* Ref);

			FFINNetworkTrace GetTrace(int32 id);

			UObject* GetRef(int32 id);
		};
		
		class LuaSignalReader : public Network::SignalReader {
		private:
			lua_State* L;

		public:
			LuaSignalReader(lua_State* L);

			virtual void operator<<(const std::string& str) override;
			virtual void operator<<(double num) override;
			virtual void operator<<(int num) override;
			virtual void operator<<(bool b) override;
			virtual void operator<<(UObject* obj) override;
			virtual void operator<<(const Network::NetworkTrace& obj) override;
			virtual void WriteAbstract(const void* obj, const std::string& id) override;
		};

		class LuaProcessor : public Processor {
			friend int luaPull(lua_State* L);

		private:
			int speed = 0;

			std::string code = "";

			lua_State* luaState = nullptr;
			lua_State* luaThread = nullptr;
			int luaThreadIndex = 0;
			bool endOfTick = false;
			
			int timeout = -1;
			std::chrono::time_point<std::chrono::high_resolution_clock> pullStart;
			
		public:
			static LuaProcessor* luaGetProcessor(lua_State* L);
			
			LuaProcessor(int speed = 1);

			// Begin Processor
			virtual void tick(float delta) override;
			virtual void reset() override;
			virtual std::int64_t getMemoryUsage(bool recalc = false) override;
			virtual void Serialize(FArchive& Ar) override;
			// End Processor

			/**
			* Sets up the lua environment.
			* Adds the Computer API in global namespace and adds the FileSystem API in global namespace.
			*
			* @param[in]	ctx		the lua context you want to setup
			*/
			void luaSetup(lua_State* L);

			/**
			 * Sets the code you want to execute.
			 * Might cause the processor to reset.
			 */
			void setCode(const std::string& code);

			int doSignal(lua_State* L);

			static void luaHook(lua_State* L, lua_Debug* ar);
			static int luaAPIReturn(lua_State* L, int args);
			
			void persist(FString& Thread, FString& Globals, FLuaProcessorStateStorage& Storage);
			void unpersist(const FString& Thread, const FString& Globals, FLuaProcessorStateStorage& Storage);
		};
	}
}
