#include "Signal.h"

namespace FicsItKernel {
	namespace Network {
		std::map<std::string, Signal::SignalDeserializeFunc>& Signal::SignalRegistry() {
            static std::map<std::string, Signal::SignalDeserializeFunc>* reg = nullptr;
			if (reg == nullptr) reg = new std::map<std::string, Signal::SignalDeserializeFunc>();
            return *reg;
		}
		
		void Signal::registerSignalType(const std::string& name, SignalDeserializeFunc func) {
			SignalRegistry()[name] = func;
		}

		std::shared_ptr<Signal> Signal::deserializeSignal(const std::string& name, FArchive& ar) {
			return SignalRegistry()[name](ar);
		}

		Signal::Signal(std::string name) : name(name) {}

		std::string Signal::getName() const {
			return name;
		}

		SignalTypeRegisterer::SignalTypeRegisterer(const std::string& name, Signal::SignalDeserializeFunc func) {
			Signal::registerSignalType(name, func);
		}

		SignalReader::~SignalReader() {}

		void SignalReader::operator<<(const NetworkTrace& obj) {
			*this << *obj;
		}
		
		void SignalReader::WriteAbstract(const void* obj, const std::string& id) {}

		SignalTypeRegisterer regNoSignal("NoSignal", [](FArchive& Ar) {
			return std::make_shared<NoSignal>();
		});
	}
}
