#include "Processor.h"

namespace FicsItKernel {
	void Processor::setKernel(KernelSystem* newKernel) {
		kernel = newKernel;
	}

	KernelSystem * Processor::getKernel() {
		return kernel;
	}
}
