#include "FINComputerProcessorLua.h"
#include "FINLuaProcessor.h"

UFINKernelProcessor* AFINComputerProcessorLua::CreateProcessor() {
	UFINLuaProcessor* Processor = NewObject<UFINLuaProcessor>(this);
	Processor->DebugInfo = this->GetName();
	return Processor;
}
