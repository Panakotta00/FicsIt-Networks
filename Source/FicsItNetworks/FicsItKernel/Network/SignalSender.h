#pragma once

#include "CoreMinimal.h"
#include "Signal.h"
#include "NetworkTrace.h"

namespace FicsItKernel {
	namespace Network {
		/**
		 * Allows the implementer to send signals by f.e. managing the listeners.
		 * It is also used to identify the senders in code.
		 */
		class SignalSender {
			/**
			 * Allows the given listener to listen to the signals this objects emits.
			 */
			virtual void addListener(NetworkTrace listener) = 0;

			/**
			 * Allows the given listener to stop listening to the signals this object emits.
			 */
			virtual void removeListener(NetworkTrace listener) = 0;

			/**
			 * Returns the sender as component, nullptr if it is not a component.
			 * It is not a component f.e. if the kernel has send the signal itself.
			 */
			virtual UObject* getComponent() const = 0;
		};
	}
}