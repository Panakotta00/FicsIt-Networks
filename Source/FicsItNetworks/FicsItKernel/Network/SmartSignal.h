#pragma once

#include "FicsItKernel/Processor/Lua/LuaStructs.h"

#include "SignalSender.h"

#include <vector>

#include "Network/FINNetworkTrace.h"

namespace FicsItKernel {
	namespace Network {
		struct VariaDicSignalElem {
			enum Type {
				Int,
				Float,
				Bool,
				String,
				Object,
				Trace,
				Class,
				Item,
				ItemAmount,
				Stack
			};

			VariaDicSignalElem(int e);

			VariaDicSignalElem(float e);

			VariaDicSignalElem(bool e);

			VariaDicSignalElem(FString e);

			VariaDicSignalElem(std::string e);

			VariaDicSignalElem(UObject* e);

			VariaDicSignalElem(const NetworkTrace& e);

			VariaDicSignalElem(const FInventoryItem& e);

			VariaDicSignalElem(const FItemAmount& e);

			VariaDicSignalElem(const FInventoryStack& e);

			VariaDicSignalElem(const VariaDicSignalElem& other);

			VariaDicSignalElem(FArchive& Ar);

			VariaDicSignalElem& operator=(const VariaDicSignalElem& other);

			~VariaDicSignalElem();

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

			UClass* getUClass() const {
				return Cast<UClass>(data.OBJECT);
			}

			const FInventoryItem* getItem() const {
				return data.ITEM;
			}

			const FItemAmount* getItemAmount() const {
				return data.ITEM_AMOUNT;
			}

			const FInventoryStack* getStack() const {
				return data.STACK;
			}

			void serialize(FArchive& Ar);

		private:
			Type type;
			union {
				int					INT;
				float				FLOAT;
				bool				BOOL;
				std::string*		STRING;
				UObject*			OBJECT;
				NetworkTrace*		TRACE;
				FInventoryItem*		ITEM;
				FItemAmount*		ITEM_AMOUNT;
				FInventoryStack*	STACK;
			} data;
		};

		class SmartSignal : public Signal {
		protected:
			std::vector<VariaDicSignalElem> args;

		public:
			SmartSignal(std::string name, const std::vector<VariaDicSignalElem>& args) : Signal(name), args(args) {}
			
			template<typename... Ts>
			SmartSignal(std::string signalName, Ts&&... args) : SmartSignal(signalName, {VariaDicSignalElem(args)...}) {}

			SmartSignal(FArchive& Ar);
			
			virtual int operator>>(SignalReader& reader) const override;

			virtual void Serialize(FArchive& Ar) override;

			virtual std::string getTypeName() override;
		};
	}
}
