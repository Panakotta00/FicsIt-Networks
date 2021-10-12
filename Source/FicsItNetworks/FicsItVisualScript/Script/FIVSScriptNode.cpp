#include "FIVSScriptNode.h"

UFIVSPin* UFIVSScriptNode::CreatePin(EFIVSPinType PinType, FText Name, FFIVSPinDataType DataType) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->ParentNode = this;
	Pin->PinType = PinType;
	Pin->Name = Name;
	Pin->PinDataType = DataType;
		
	Pins.Add(Pin);
	return Pin;
}
