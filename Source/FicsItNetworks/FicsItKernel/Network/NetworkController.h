#pragma once

#include "CoreMinimal.h"

#include "SmartSignal.h"
#include "SignalSender.h"
#include "SignalListener.h"

#include <string>
#include <queue>
#include <vector>
#include <set>
#include <memory>

namespace FicsItKernel {
	namespace Network {
		/**
		 * Allows to control and manage network connection of a system.
		 * Also manages the network signals.
		 */
		class NetworkController : public SignalSender, public SignalListener {
		protected:
			std::set<NetworkTrace> signalListeners;
			std::queue<std::pair<std::shared_ptr<Signal>, NetworkTrace>> signals;

		public:
			virtual ~NetworkController() {}

			/**
			 * Underlying Computer Network Component used for interacting with the network.
			 * Needs to be a Signal Sender and a Signal Listener which redirect the interaction of both to this.
			 */
			UObject* component = nullptr;

			/**
			 * The maximum amount of signals the signal queue can hold
			 */
			uint32 maxSignalCount = 32;

			// Begin SignalSender
			virtual void addListener(NetworkTrace listener) override;
			virtual void removeListener(NetworkTrace listener) override;
			virtual UObject* getComponent() const override;
			// End SignalSender

			// Begin SignalListener
			virtual void handleSignal(std::shared_ptr<Signal> signal, NetworkTrace sender) override;
			// End SignalListener

			/**
			 * pops a signal form the queue.
			 * returns nullptr if there is no signal left.
			 *
			 * @param sender - out put paramter for the sender of the signal
			 * @return	singal from the queue
			 */
			std::shared_ptr<Signal> popSignal(NetworkTrace& sender);

			/**
			 * pushes a signal to the queue.
			 * signal gets dropped if the queue is already full.
			 *
			 * @param	signal	the singal you want to push
			 */
			void pushSignal(std::shared_ptr<Signal> signal, NetworkTrace sender);

			/**
			 * gets the amount of signals in the queue
			 *
			 * @return	amount of signals
			 */
			size_t getSignalCount();

			/**
			 * trys to find a component with the given ID.
			 *
			 * @return	the component you searched for, nullptr if it was not able to find the component
			 */
			NetworkTrace getComponentByID(const std::string& id);

			/**
			 * returns the components in the network with the given nick.
			 */
			std::set<NetworkTrace> getComponentByNick(const std::string& nick);

			/**
			 * pushes a signal to the queue.
			 * uses the arguments to construct a signal struct.
			 * Should only get used by the kernel modules to emit signals.
			 */
			template<typename... Ts>
			void pushSignalKernel(const std::string& signalName, Ts&&... args) {
				pushSignal(std::shared_ptr<Signal>(new SmartSignal(signalName, args...)), NetworkTrace(component));
			}
		};
	}
}