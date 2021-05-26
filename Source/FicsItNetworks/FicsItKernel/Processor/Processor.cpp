#include "Processor.h"

void UFINKernelProcessor::SetKernel(UFINKernelSystem* InKernel) {
	Kernel = InKernel;
}

UFINKernelSystem* UFINKernelProcessor::GetKernel() {
	return Kernel;
}