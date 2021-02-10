#include "FIVSComputerProcessor.h"

#include "Kernel/FIVSProcessor.h"

FicsItKernel::Processor* AFINScriptProcessor::CreateProcessor() {
	return new FicsItKernel::FIVS::FIVSProcessor();
}
