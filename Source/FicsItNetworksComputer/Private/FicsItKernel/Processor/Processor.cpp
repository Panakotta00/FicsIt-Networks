#include "FicsItKernel/Processor/Processor.h"

void UFINKernelProcessor::SetKernel(UFINKernelSystem* InKernel) {
	Kernel = InKernel;
}

UFINKernelSystem* UFINKernelProcessor::GetKernel() {
	return Kernel;
}
