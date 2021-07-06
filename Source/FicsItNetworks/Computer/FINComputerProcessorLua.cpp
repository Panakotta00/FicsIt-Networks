#include "FINComputerProcessorLua.h"
#include "FicsItNetworks/FicsItKernel/Processor/Lua/LuaProcessor.h"

UFINKernelProcessor* AFINComputerProcessorLua::CreateProcessor() {
	UFINLuaProcessor* Processor = NewObject<UFINLuaProcessor>(this);
	Processor->DebugInfo = this->GetName();
	return Processor;
}
