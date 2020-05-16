#include "Signal.h"

namespace FicsItKernel {
	namespace Network {
		std::map<std::string, Signal::SignalDeserializeFunc> Signal::SignalRegistry;
		
		void Signal::registerSignalType(const std::string& name, SignalDeserializeFunc func) {
			SignalRegistry[name] = func;
		}

		std::shared_ptr<Signal> Signal::deserializeSignal(const std::string& name, FArchive& ar) {
			return SignalRegistry[name](ar);
		}

		Signal::Signal(std::string name) : name(name) {}

		std::string Signal::getName() const {
			return name;
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
