#pragma once

#include "CoreMinimal.h"
#include "Signal.h"
#include <memory>

namespace FicsItKernel {
	namespace Network {
		/**
		 * Allows the implementer to recieve network signals
		 */
		class SignalListener {
		public:
			/**
			 * Handle the given signal.
			 *
			 * @param	signal	the signal you want to handle
			 */
			virtual void handleSignal(std::shared_ptr<Signal> signal, NetworkTrace sender) = 0;
		};
	}
}