#pragma once

#include "FicsItKernel/Processor/Lua/LuaStructs.h"

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
				Object,
				Trace,
				Class,
				Item,
				ItemAmount,
				Stack
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
				if (Cast<UClass>(e)) type = Class;
				else type = Object;
			}

			VariaDicSignalElem(const NetworkTrace& e) {
				data.TRACE = new NetworkTrace(e);
				type = Trace;
			}

			VariaDicSignalElem(const FInventoryItem& e) {
				data.ITEM = new FInventoryItem(e);
				type = Item;
			}

			VariaDicSignalElem(const FItemAmount& e) {
				data.ITEM_AMOUNT = new FItemAmount(e);
				type = ItemAmount;
			}

			VariaDicSignalElem(const FInventoryStack& e) {
				data.STACK = new FInventoryStack(e);
				type = Stack;
			}

			VariaDicSignalElem(const VariaDicSignalElem& other) {
				*this = other;
			}

			VariaDicSignalElem(VariaDicSignalElem&& other) {
				type = other.type;
				data = other.data;
			}

			VariaDicSignalElem& operator=(const VariaDicSignalElem& other) {
				type = other.type;
				switch (type) {
				case String:
					data.STRING = new std::string(*other.data.STRING);
					break;
				case Trace:
					data.TRACE = new Network::NetworkTrace(*other.data.TRACE);
					break;
				case Item:
					data.ITEM = new FInventoryItem(*other.data.ITEM);
					break;
				case ItemAmount:
					data.ITEM_AMOUNT = new FItemAmount(*other.data.ITEM_AMOUNT);
					break;
				case Stack:
					data.STACK = new FInventoryStack(*other.data.STACK);
					break;
				default:
					data = other.data;
					break;
				}
				return *this;
			}

			VariaDicSignalElem& operator=(VariaDicSignalElem&& other) {
				type = other.type;
				data = other.data;
				return *this;
			}

			~VariaDicSignalElem() {
				switch (type) {
				case String:
					delete data.STRING;
					break;
				case Trace:
					delete data.TRACE;
					break;
				case Item:
					delete data.ITEM;
					break;
				case ItemAmount:
					delete data.ITEM_AMOUNT;
					break;
				case Stack:
					delete data.STACK;
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
			SmartSignal(std::string signalName, Ts&&... args) : SmartSignal(signalName, {args}) {}

			virtual int operator>>(SignalReader& reader) const override;
		};
	}
}