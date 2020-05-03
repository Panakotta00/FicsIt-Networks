#include "LuaProcessor.h"
#include "../../FicsItKernel.h"

#include "LuaInstance.h"
#include "LuaLib.h"
#include "LuaStructs.h"
#include "LuaHooks.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "LuaFileSystemAPI.h"
#include "LuaComputerAPI.h"

#include "SML/util/Logging.h"

namespace FicsItKernel {
	namespace Lua {
		LuaSignalReader::LuaSignalReader(lua_State* L) : L(L) {}

		void LuaSignalReader::operator<<(const std::string& str) {
			lua_pushstring(L, str.c_str());
		}

		void LuaSignalReader::operator<<(double num) {
			lua_pushnumber(L, num);
		}

		void LuaSignalReader::operator<<(int num) {
			lua_pushinteger(L, num);
		}

		void LuaSignalReader::operator<<(bool b) {
			lua_pushboolean(L, b);
		}

		void LuaSignalReader::operator<<(UObject* obj) {
			newInstance(L, Network::NetworkTrace(obj));
		}

		void LuaSignalReader::operator<<(const Network::NetworkTrace& obj) {
			newInstance(L, obj);
		}

		void LuaSignalReader::WriteAbstract(const void* obj, const std::string& id) {
			if (id == "InventoryItem") {
				luaStruct(L, *(const FInventoryItem*)obj);
			} else if (id == "ItemAmount") {
				luaStruct(L, *(const FItemAmount*)obj);
			} else if (id == "InventoryStack") {
				luaStruct(L, *(const FInventoryStack*)obj);
			}
		}

		LuaProcessor* LuaProcessor::currentProcessor = nullptr;

		LuaProcessor* LuaProcessor::getCurrentProcessor() {
			return currentProcessor;
		}

		void luaHook(lua_State *L, lua_Debug* ar) {
			lua_gc(L, LUA_GCCOLLECT, 0);
			auto kernel = LuaProcessor::getCurrentProcessor()->getKernel();
			kernel->recalculateResources(KernelSystem::PROCESSOR);
			lua_yield(L, 0);
		}

		LuaProcessor::LuaProcessor(int speed) : speed(speed) {
			
		}

		int LuaProcessor::doSignal(lua_State* L) {
			auto net = getKernel()->getNetwork();
			if (!net || net->getSignalCount() < 1) return 0;
			int props = 2;
			Network::NetworkTrace sender;
			auto signal = net->popSignal(sender);
			lua_pushstring(L, signal->getName().c_str());
			newInstance(L, sender);
			LuaSignalReader reader(L);
			props += *signal >> reader;
			return props;
		}

		void LuaProcessor::tick(double delta) {
			currentProcessor = this;

			if (!luaState) return;

			int status = 0;
			if (timeout > -1) {
				if (getKernel()->getNetwork()->getSignalCount() > 0) {
					timeout = -1;
					auto sigArgs = doSignal(luaState);
					if (sigArgs < 1) status = LUA_ERRRUN;
					else status = lua_resume(luaState, nullptr, sigArgs);
				} else if (timeout == 0 || timeout > std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - pullStart).count()) return;
				else status = lua_resume(luaState, nullptr, 0);
			} else status = lua_resume(luaState, nullptr, 0);
			if (status == LUA_YIELD) {
			} else if (status == LUA_OK) {
				kernel->stop();
			} else {
				kernel->crash({ lua_tostring(luaState, -1) });
			}

			currentProcessor = nullptr;
		}

		void LuaProcessor::reset() {
			if (getKernel()->getState() != RUNNING) return;
			
			timeout = -1;

			if (luaState) {
				lua_close(luaState);
			}

			luaState = luaL_newstate();
			luaSetup(luaState);

			luaL_loadstring(luaState, code.c_str());
			lua_sethook(luaState, &luaHook, LUA_MASKCOUNT, speed);
			// lua_gc(luaState, LUA_GCSETPAUSE, 100);
			// TODO: Check if we actually want to use this or the manual gc call
		}

		std::int64_t LuaProcessor::getMemoryUsage(bool recalc) {
			return lua_gc(luaState, LUA_GCCOUNT, 0) * 1000;
		}

		int luaPrint(lua_State* L) {
			int args = lua_gettop(L);
			std::string log;
			for (int i = 1; i <= args; ++i) {
				const char* s = luaL_tolstring(L, i, 0);
				if (!s) luaL_argerror(L, i, "is not valid type");
				log += s;
			}
			
			try {
				auto stdio = LuaProcessor::getCurrentProcessor()->getKernel()->getDevDevice()->getStdio()->open(FileSystem::APPEND);
				if (stdio) {
					*stdio << log.c_str() << "\n";
					stdio->close();
				}
			} catch (std::exception ex) {
				luaL_error(L, ex.what());
			}

			SML::Logging::error("LuaPrint: ", log.c_str());

			return 0;
		}

		void LuaProcessor::luaSetup(lua_State* L) {
			luaL_openlibs(L); // TODO: restrict to only we rly want to use

			lua_register(L, "print", luaPrint);
			
			setupInstanceSystem(L);
			setupStructs(L);
			setupHooks(L);
			setupComponentAPI(L);
			setupEventAPI(L);
			setupFileSystemAPI(getKernel(), L);
			setupComputerAPI(L, getKernel());
		}

		void LuaProcessor::setCode(const std::string& code) {
			this->code = code;
			reset();
		}
	}
}
