#include "FINComputerProcessorLua.h"
#include "FicsItKernel/Processor/Lua/LuaProcessor.h"

FicsItKernel::Processor* AFINComputerProcessorLua::CreateProcessor() {
	return new FicsItKernel::Lua::LuaProcessor();
}
