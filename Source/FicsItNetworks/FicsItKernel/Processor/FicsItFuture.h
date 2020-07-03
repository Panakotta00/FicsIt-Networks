#pragma once
#include <functional>

#include "ThirdParty/lua.h"

namespace FicsItKernel {
	/**
	 * Base class for all Kernel Futures which will retrieve data in the main thread.
	 */
	class FicsItFuture {
	public:
		virtual ~FicsItFuture();

		/**
		 * This function will get called from the FicsIt-Kernel in the main thread if it is added to the
		 * future queue of the kernel.
		 */
		virtual void Excecute() = 0;
    };
}
