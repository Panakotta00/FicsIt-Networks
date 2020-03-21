#include "NetworkController.h"

#include "Network/FINNetworkComponent.h"

#include <map>

namespace FicsItKernel {
	namespace Network {
		void NetworkController::addListener(NetworkTrace listener) {
			signalListeners.insert(listener);
		}

		void NetworkController::removeListener(NetworkTrace listener) {
			signalListeners.erase(listener);
		}

		UObject* NetworkController::getComponent() const {
			return component;
		}

		void NetworkController::handleSignal(std::shared_ptr<Signal> signal, NetworkTrace sender) {
			pushSignal(signal, sender);
		}

		std::shared_ptr<Signal> NetworkController::popSignal(NetworkTrace& sender) {
			if (getSignalCount() < 1) return nullptr;
			auto sig = signals.back();
			signals.pop();
			sender = sig.second;
			return sig.first;
		}

		void NetworkController::pushSignal(std::shared_ptr<Signal> signal, NetworkTrace sender) {
			if (signals.size() >= maxSignalCount) return;
			signals.push({std::move(signal), std::move(sender)});
		}

		size_t NetworkController::getSignalCount() {
			return signals.size();
		}

		NetworkTrace NetworkController::getComponentByID(const std::string& id) {
			FGuid guid;
			if (FGuid::Parse(id.c_str(), guid)) if (auto comp = Cast<IFINNetworkComponent>(component)) return NetworkTrace(component) / *comp->FindComponent(guid);
			return NetworkTrace(nullptr);
		}

		std::set<NetworkTrace> NetworkController::getComponentByNick(const std::string& nick) {
			if (auto comp = Cast<IFINNetworkComponent>(component)) {
				std::set<NetworkTrace> outComps;
				auto comps = comp->GetCircuit()->FindComponentsByNick(nick.c_str());
				for (auto& c : comps) {
					outComps.insert(NetworkTrace(component) / c);
				}
				return outComps;
			};
			return std::set<NetworkTrace>();
		}
	}
}