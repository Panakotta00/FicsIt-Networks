#include "Signal.h"

namespace FicsItKernel {
	namespace Network {
		Signal::Signal(std::string name) : name(name) {}

		std::string Signal::getName() const {
			return name;
		}

		void SignalReader::operator<<(const NetworkTrace& obj) {
			*this << *obj;
		}
	}
}
