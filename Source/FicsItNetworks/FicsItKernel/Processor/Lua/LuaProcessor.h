#pragma once

#include <chrono>
#include <set>

#include "FicsItKernel/Processor/Processor.h"
#include "LuaFileSystemAPI.h"

class AFINStateEEPROMLua;
struct lua_State;
struct lua_Debug;

namespace FicsItKernel {
	namespace Lua {
		class LuaValueReader : public FFINValueReader {
		private:
			lua_State* L = nullptr;

		public:
			LuaValueReader(lua_State* L);

			virtual void nil() override;
			virtual void operator<<(FINBool b) override;
			virtual void operator<<(FINInt num) override;
			virtual void operator<<(FINFloat num) override;
			virtual void operator<<(FINClass clazz) override;
			virtual void operator<<(const FINStr& str) override;
			virtual void operator<<(const FINObj& obj) override;
			virtual void operator<<(const FINTrace& obj) override;
			virtual void operator<<(const FINStruct& struc) override;
		};

		class LuaFileSystemListener : public FileSystem::Listener {
		private:
			class LuaProcessor* parent = nullptr;
		public:
			LuaFileSystemListener(class LuaProcessor* parent) : parent(parent) {}
			
			virtual void onUnmounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) override;
			virtual void onNodeRemoved(FileSystem::Path path, FileSystem::NodeType type) override;
		};

		class LuaProcessor : public Processor {
			friend int luaPull(lua_State* L);

		private:
			int speed = 0;

			TWeakObjectPtr<AFINStateEEPROMLua> eeprom;

			lua_State* luaState = nullptr;
			lua_State* luaThread = nullptr;
			int luaThreadIndex = 0;
			bool endOfTick = false;

			int pullState = 0; // 0 = not pulling, 1 = pulling with timeout, 2 = pull indefinetly
			double timeout = 0.0;
			std::chrono::time_point<std::chrono::high_resolution_clock> pullStart;
			std::set<LuaFile> fileStreams;
			FileSystem::SRef<LuaFileSystemListener> fileSystemListener;
			
		public:
			static LuaProcessor* luaGetProcessor(lua_State* L);
			
			LuaProcessor(int speed = 1);

			// Begin Processor
			virtual void setKernel(KernelSystem* kernel) override;
			virtual void tick(float delta) override;
			virtual void reset() override;
			virtual std::int64_t getMemoryUsage(bool recalc = false) override;
			virtual void PreSerialize(UProcessorStateStorage* Storage, bool bLoading) override;
			virtual void Serialize(UProcessorStateStorage* Storage, bool bLoading) override;
			virtual void PostSerialize(UProcessorStateStorage* Storage, bool bLoading) override;
			virtual UProcessorStateStorage* CreateSerializationStorage() override;
			virtual void setEEPROM(AFINStateEEPROM* eeprom) override;
			// End Processor

			/**
			* Sets up the lua environment.
			* Adds the Computer API in global namespace and adds the FileSystem API in global namespace.
			*
			* @param[in]	L	the lua context you want to setup
			*/
			void luaSetup(lua_State* L);

			/**
			 * Allows to access the the eeprom used by the processor.
			 * Nullptr if no eeprom is currently set.
			 *
			 * @return	the eeprom used by the processor.
			 */
			AFINStateEEPROMLua* getEEPROM();

			/**
			 * Trys to pop a signal from the signal queue in the network controller
			 * and pushes the resulting values to the given lua stack.
			 *
			 * @param[in]	L	the stack were the values should get pushed to.
			 * @return	the count of values we have pushed.
			 */
			int doSignal(lua_State* L);
			
			void clearFileStreams();
			std::set<LuaFile> getFileStreams() const;

			static void luaHook(lua_State* L, lua_Debug* ar);

			/**
			 * Access the lua processor in the given state registry and checks
			 * if the tick process is in the second stage of execution.
			 * If this is the case yields the runtime with a continuation function
			 * which will return values of the count of args on top of the stack prior
			 * to the yield.
			 * If the tick is in the first stage, we just return.
			 *
			 * @param[in]	L		the lua state all of this should occur
			 * @param[in]	args	the count of arguments we should copy and the continuation should return
			 * @return	if it even returns, returns the same as args
			 */
			static int luaAPIReturn(lua_State* L, int args);
		};
	}
}
