#include "SmartSignal.h"

#include "FicsItKernel/FicsItFS/Serial.h"

namespace FicsItKernel {
	namespace Network {
		typedef VariaDicSignalElem::Type Type;

		SignalTypeRegisterer regSmartSignal("SmartSignal", [](FArchive& Ar) {
			return std::shared_ptr<Signal>(new SmartSignal(Ar));
		});

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
