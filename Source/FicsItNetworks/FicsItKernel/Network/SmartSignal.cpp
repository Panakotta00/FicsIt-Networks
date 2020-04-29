#include "SmartSignal.h"

namespace FicsItKernel {
	namespace Network {
		typedef VariaDicSignalElem::Type Type;

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
	}
}
