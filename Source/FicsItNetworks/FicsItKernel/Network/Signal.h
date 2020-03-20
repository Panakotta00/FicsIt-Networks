#pragma once

#include <string>

#include "NetworkTrace.h"

class UObject;

/**
 * Holds information like the paramters of signal
 */
namespace FicsItKernel {
	namespace Network {
		class SignalReader {
		public:
			/** Writes a string to the signal reader */
			virtual void operator<<(const std::string& str) = 0;

			/** Writes a double to the signal reader */
			virtual void operator<<(double num) = 0;

			/** Writes a integer to the signal reader */
			virtual void operator<<(int num) = 0;

			/** Writes a boolean to the signal reader */
			virtual void operator<<(bool b) = 0;

			/** Writes a object instance to the signal reader */
			virtual void operator<<(UObject* obj) = 0;

			/** Writes a object referenced by a network trace to the signal reader */
			virtual void operator<<(const NetworkTrace& obj);
		};

		class Signal {
		private:
			std::string name;

		public:
			Signal(std::string name);
			virtual ~Signal() {}

			/**
			 * Returns the signal name so the user can differentiate between different types of signals.
			 * Each signal "type" should only have one distinct declaration of signal arguments.
			 */
			std::string getName() const;

			/**
			 * Writes all signal arguments in order to the signal reader.
			 * Allowing to convert the signal arguments to the values each processor supports.
			 */
			virtual int operator>>(SignalReader& reader) const = 0;
		};

		class NoSignal : public Signal {
		public:
			NoSignal() : Signal("None") {}

			virtual int operator>>(SignalReader& reader) const { return 0; }
		};
	}
}