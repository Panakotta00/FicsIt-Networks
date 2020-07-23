#include "NetworkController.h"

#include "Network/FINNetworkComponent.h"

#include <map>

#include "Network/FINDynamicStructHolder.h"

namespace FicsItKernel {
	namespace Network {
		void NetworkController::handleSignal(TSharedPtr<FFINSignal> signal, const FFINNetworkTrace& sender) {
			pushSignal(signal, sender);
		}

		TSharedPtr<FFINSignal> NetworkController::popSignal(FFINNetworkTrace& sender) {
			if (getSignalCount() < 1) return nullptr;
			mutexSignals.lock();
			auto sig = signals.front();
			signals.pop_front();
			mutexSignals.unlock();
			sender = sig.second;
			return sig.first;
		}

		void NetworkController::pushSignal(TSharedPtr<FFINSignal> signal, const FFINNetworkTrace& sender) {
			std::lock_guard<std::mutex> m(mutexSignals);
			if (signals.size() >= maxSignalCount || lockSignalRecieving) return;
			signals.push_back({std::move(signal), std::move(sender)});
		}

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
			if (Ar.IsSaving()) for (const FFINNetworkTrace& trace : signalListeners) {
				networkTraces.Add(trace);
			}
			Ar << networkTraces;
			if (Ar.IsLoading()) for (FFINNetworkTrace& trace : networkTraces) {
				signalListeners.insert(trace);
			}

			// serialize signals
			int32 signalCount = signals.size();
			Ar << signalCount;
			if (Ar.IsLoading()) {
				signals.clear();
			}
			for (int i = 0; i < signalCount; ++i) {
				TFINDynamicStruct<FFINSignal> Signal;
				FFINNetworkTrace Trace;
				if (Ar.IsSaving()) {
					const auto& sig = signals[i];
					Signal = FFINDynamicStructHolder::Copy(sig.first->GetStruct(), sig.first.Get());
					Trace = sig.second;
				}
				bool valid = Trace.getTrace().isValid();
				Ar << valid;
				if (!valid) continue;
				
				// save/save signal
				Signal.Serialize(Ar);
				Trace.Serialize(Ar);
				
				if (Ar.IsLoading()) {
					signals.push_back({Signal.SharedCopy(), Trace});
				}
			}

			// serialize signal senders
			Ar << signalSenders;
		}

		void NetworkController::PostSerialize(bool load) {
			lockSignalRecieving = false;
		}
	}
}
