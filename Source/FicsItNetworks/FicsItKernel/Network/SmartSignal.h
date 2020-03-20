#pragma once

#include "SignalSender.h"

#include <vector>

namespace FicsItKernel {
	namespace Network {
		struct VariaDicSignalElem {
			enum Type {
				Int,
				Float,
				Bool,
				String,
				Object
			};

			VariaDicSignalElem(int e) {
				data.INT = e;
				type = Int;
			}

			VariaDicSignalElem(float e) {
				data.FLOAT = e;
				type = Float;
			}

			VariaDicSignalElem(bool e) {
				data.BOOL = e;
				type = Bool;
			}

			VariaDicSignalElem(FString e) {
				data.STRING = new std::string(TCHAR_TO_ANSI(*e));
				type = String;
			}

			VariaDicSignalElem(std::string e) {
				data.STRING = new std::string(e);
				type = String;
			}

			VariaDicSignalElem(UObject* e) {
				data.OBJECT = e;
				type = Object;
			}

			~VariaDicSignalElem() {
				switch (type) {
				case String:
					delete data.STRING;
					break;
				}
			}

			Type getType() const {
				return type;
			}

			int getInt() const {
				return data.INT;
			}

			float getFloat() const {
				return data.FLOAT;
			}

			bool getBool() const {
				return data.BOOL;
			}

			std::string getString() const {
				return *data.STRING;
			}

			UObject* getObject() const {
				return data.OBJECT;
			}
		private:
			Type type;
			union {
				int   INT;
				float FLOAT;
				bool BOOL;
				std::string* STRING;
				UObject* OBJECT;
			} data;
		};

		class SmartSignal : public Signal {
		protected:
			std::vector<VariaDicSignalElem> args;

		public:
			SmartSignal(std::string name, const std::vector<VariaDicSignalElem>& args) : Signal(name), args(args) {}
			
			template<typename... Ts>
			SmartSignal(std::string signalName, Ts&&... args) : SmartSignal(signalName, {args}) {}

			virtual int operator>>(SignalReader& reader) const;
		};
	}
}