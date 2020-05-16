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
			virtual ~SignalReader();

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

			/** Trys to write the given object identified by the given type id to the signal reader */
			virtual void WriteAbstract(const void* obj, const std::string& id);
		};

		class Signal {
		public:
			typedef std::function<std::shared_ptr<Signal>(FArchive&)> SignalDeserializeFunc;
			
		private:
			static std::map<std::string, SignalDeserializeFunc> SignalRegistry;

		protected:
			std::string name;

		public:
			/**
			 * Adds a signal type to the signal registry
			 *
			 * @param[in]	name 	the signal type name of the new signal
			 * @param[in]	func	the signal deserialization function
			 */
			static void registerSignalType(const std::string& name, SignalDeserializeFunc func);

			/**
			 * Serialize signal via type
			 *
			 * @param[in]	name	the signal type name of the signal you want to serialize
			 * @aaram[in]	ar		the archive from where you want to read the data
			 * @return	the created and deserialized signal
			 */
			static std::shared_ptr<Signal> deserializeSignal(const std::string& name, FArchive& ar);
			
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

			/**
			 * De/Serialize the signal to an archive
			 *
			 * @param[in]	the archive which stores the signal information
			 */
			virtual void Serialize(FArchive& Ar) = 0;

			/**
			 * Returns the signal type name
			 *
			 * @return	the signal type name
			 */
			virtual std::string getTypeName() = 0;
		};
		
		struct SignalTypeRegisterer {
			SignalTypeRegisterer(const std::string& name, Signal::SignalDeserializeFunc func) {
				Signal::registerSignalType(name, func);
			}
		};

		class NoSignal : public Signal {
		public:
			NoSignal() : Signal("None") {}

			virtual int operator>>(SignalReader& reader) const { return 0; }

			virtual std::string getTypeName() { return "NoSignal"; };

			virtual void Serialize(FArchive& Ar) {};
		};
	}
}