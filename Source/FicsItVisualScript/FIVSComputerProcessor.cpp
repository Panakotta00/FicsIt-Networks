#include "FIVSComputerProcessor.h"

#include "Kernel/FIVSProcessor.h"

UFINKernelProcessor* AFINScriptProcessor::CreateProcessor() {
	return NewObject<UFIVSProcessor>(this);
}
