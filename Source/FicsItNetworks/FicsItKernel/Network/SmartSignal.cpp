#include "SmartSignal.h"

#include "FicsItKernel/FicsItFS/Serial.h"
#include "util/Logging.h"

namespace FicsItKernel {
	namespace Network {
		typedef VariaDicSignalElem::Type Type;

		SignalTypeRegisterer regSmartSignal("SmartSignal", [](FArchive& Ar) {
			return std::shared_ptr<Signal>(new SmartSignal(Ar));
		});

		VariaDicSignalElem::VariaDicSignalElem(int e) {
			data.INT = e;
			type = Int;
		}

		VariaDicSignalElem::VariaDicSignalElem(float e) {
			data.FLOAT = e;
			type = Float;
		}

		VariaDicSignalElem::VariaDicSignalElem(bool e) {
			data.BOOL = e;
			type = Bool;
		}

		VariaDicSignalElem::VariaDicSignalElem(FString e) {
			data.STRING = new std::string(TCHAR_TO_UTF8(*e), e.Len());
			type = String;
		}

		VariaDicSignalElem::VariaDicSignalElem(std::string e) {
			data.STRING = new std::string(e);
			type = String;
		}

		VariaDicSignalElem::VariaDicSignalElem(UObject* e) {
			data.OBJECT = e;
			if (Cast<UClass>(e)) type = Class;
			else type = Object;
		}

		VariaDicSignalElem::VariaDicSignalElem(const NetworkTrace& e) {
			data.TRACE = new NetworkTrace(e);
			type = Trace;
		}

		VariaDicSignalElem::VariaDicSignalElem(const FInventoryItem& e) {
			data.ITEM = new FInventoryItem(e);
			type = Item;
		}

		VariaDicSignalElem::VariaDicSignalElem(const FItemAmount& e) {
			data.ITEM_AMOUNT = new FItemAmount(e);
			type = ItemAmount;
		}

		VariaDicSignalElem::VariaDicSignalElem(const FInventoryStack& e) {
			data.STACK = new FInventoryStack(e);
			type = Stack;
		}

		VariaDicSignalElem::VariaDicSignalElem(const VariaDicSignalElem& other) {
			*this = other;
		}

		VariaDicSignalElem::VariaDicSignalElem(FArchive& Ar) {
			int t;
			Ar << t;
			type = static_cast<Type>(t);

			switch (type) {
			case String:
				data.STRING = new std::string();
				SML::Logging::error("Smart string");
				break;
			case Trace:
				data.TRACE = new Network::NetworkTrace();
				break;
			case Item:
				data.ITEM = new FInventoryItem();
				break;
			case ItemAmount:
				data.ITEM_AMOUNT = new FItemAmount();
				break;
			case Stack:
				data.STACK = new FInventoryStack();
				break;
			}

			serialize(Ar);
		}

		VariaDicSignalElem& VariaDicSignalElem::operator=(const VariaDicSignalElem& other) {
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

		VariaDicSignalElem::~VariaDicSignalElem() {
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

		void VariaDicSignalElem::serialize(FArchive& Ar) {
			if (Ar.IsSaving()) {
				int t = type;
				Ar << t;
			}

			switch (type) {
			case Int:
				Ar << data.INT;
				break;
			case Float:
				Ar << data.FLOAT;
				break;
			case Bool:
				Ar << data.BOOL;
				break;
			case String: {
				FString str = FString(data.STRING->c_str(), data.STRING->length());
				Ar << str;
				*data.STRING = std::string(TCHAR_TO_UTF8(*str), str.Len());
				break;
			}
			case Object:
				Ar << data.OBJECT;
				break;
			case Trace: {
				FFINNetworkTrace trace = *data.TRACE;
				Ar << trace;
				*data.TRACE = trace;
				break;
			}
			case Class:
				Ar << data.OBJECT;
				break;
			case Item:
				data.ITEM->Serialize(Ar);
				break;
			case ItemAmount:
				Ar << data.ITEM_AMOUNT->Amount;
				Ar << data.ITEM_AMOUNT->ItemClass;
				break;
			case Stack:
				data.STACK->Item.Serialize(Ar);
				Ar << data.STACK->NumItems;
				break;
			}
		}

		SmartSignal::SmartSignal(FArchive& Ar) : Signal("") {
			Serialize(Ar);
		}

		int SmartSignal::operator>>(SignalReader& reader) const {
			for (auto& arg : args) {
				switch (arg.getType()) {
				case Type::Int:
					reader << arg.getInt();
					break;
				case Type::Float:
					reader << arg.getFloat();
					break;
				case Type::Bool:
					reader << arg.getBool();
					break;
				case Type::String:
					reader << arg.getString();
					break;
				case Type::Object:
					reader << arg.getObject();
					break;
				case Type::Item:
					reader.WriteAbstract(arg.getItem(), "InventoryItem");
					break;
				case Type::ItemAmount:
					reader.WriteAbstract(arg.getItemAmount(), "ItemAmount");
					break;
				case Type::Stack:
					reader.WriteAbstract(arg.getStack(), "InventoryStack");
					break;
				}
			}
			return args.size();
		}

		void SmartSignal::Serialize(FArchive& Ar) {
			FString name = FString(getName().c_str(), getName().length());
			Ar << name;
			this->name = std::string(TCHAR_TO_UTF8(*name), name.Len());
			
			int count = args.size();
			Ar << count;
			if (Ar.IsSaving()) for (VariaDicSignalElem& arg : args) {
				arg.serialize(Ar);
			}
			if (Ar.IsLoading()) for (int i = 0; i < count; ++i) {
				args.push_back(VariaDicSignalElem(Ar));
			}
		}

		std::string SmartSignal::getTypeName() {
			return "SmartSignal";
		}
	}
}
