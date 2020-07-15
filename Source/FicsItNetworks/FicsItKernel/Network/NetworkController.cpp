#include "NetworkController.h"

#include "Network/FINNetworkComponent.h"

#include <map>

namespace FicsItKernel {
	namespace Network {
		void NetworkController::addListener(NetworkTrace listener) {
			std::lock_guard<std::mutex> m(mutexSignalListeners);
			signalListeners.insert(listener);
		}

		void NetworkController::removeListener(NetworkTrace listener) {
			std::lock_guard<std::mutex> m(mutexSignalListeners);
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
			mutexSignals.lock();
			auto sig = signals.front();
			signals.pop_front();
			mutexSignals.unlock();
			sender = sig.second;
			return sig.first;
		}

#pragma optimize("", off)
		void NetworkController::pushSignal(std::shared_ptr<Signal> signal, NetworkTrace sender) {
			std::lock_guard<std::mutex> m(mutexSignals);
			if (signals.size() >= maxSignalCount || lockSignalRecieving) return;
			signals.push_back({std::move(signal), std::move(sender)});
		}
#pragma optimize("", on)

		void NetworkController::clearSignals() {
			std::lock_guard<std::mutex> m(mutexSignals);
			signals.clear();
		}

		size_t NetworkController::getSignalCount() {
			std::lock_guard<std::mutex> m(mutexSignals);
			return signals.size();
		}

		NetworkTrace NetworkController::getComponentByID(const std::string& id) {
			FGuid guid;
			if (FGuid::Parse(id.c_str(), guid)) {
				if (auto comp = Cast<IFINNetworkComponent>(component)) {
					return NetworkTrace(component) / *comp->Execute_FindComponent(component, guid);
				}
			}
			return NetworkTrace(nullptr);
		}

		std::set<NetworkTrace> NetworkController::getComponentByNick(const std::string& nick) {
			if (component->Implements<UFINNetworkComponent>()) {
				std::set<NetworkTrace> outComps;
				auto comps = IFINNetworkComponent::Execute_GetCircuit(component)->FindComponentsByNick(nick.c_str());
				for (auto& c : comps) {
					outComps.insert(NetworkTrace(component) / c);
				}
				return outComps;
			}
			return std::set<NetworkTrace>();
		}

		void NetworkController::PreSerialize(bool load) {
			lockSignalRecieving = true;
		}

		void NetworkController::Serialize(FArchive& Ar) {
			// serialize signal listeners
			TArray<FFINNetworkTrace> networkTraces;
			if (Ar.IsSaving()) for (const Network::NetworkTrace& trace : signalListeners) {
				networkTraces.Add(trace);
			}
			Ar << networkTraces;
			if (Ar.IsLoading()) for (FFINNetworkTrace& trace : networkTraces) {
				signalListeners.insert(trace);
			}

			// serialize signals
			int32 signalCount = signals.size();
			Ar << signalCount;
			if (Ar.IsSaving()) for (auto& signal : signals) {
				bool valid = signal.first.get();
				Ar << valid;
				if (!valid) continue;
					
				// save signal
				FFINNetworkTrace trace = signal.second;
				Ar << trace;
				FString typeName = signal.first->getTypeName().c_str();
				Ar << typeName;
				signal.first->Serialize(Ar);
			}
			if (Ar.IsLoading()) for (int i = 0; i < signalCount; ++i) {
				bool valid = false;
				Ar << valid;
				if (!valid) continue;
				
				// load signal
				FFINNetworkTrace trace;
				Ar << trace;
				FString typeName;
				Ar << typeName;
				std::shared_ptr<Signal> signal = Signal::deserializeSignal(TCHAR_TO_UTF8(*typeName), Ar);
				signals.push_back({signal, trace});
			}

			// serialize signal senders
			Ar << signalSenders;
		}

		void NetworkController::PostSerialize(bool load) {
			lockSignalRecieving = false;
		}
	}
}