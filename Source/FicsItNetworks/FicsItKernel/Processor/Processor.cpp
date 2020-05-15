#include "Processor.h"

namespace FicsItKernel {
	void Processor::setKernel(KernelSystem * kernel) {
		this->kernel = kernel;
	}

	KernelSystem * Processor::getKernel() {
		return kernel;
	}
}
