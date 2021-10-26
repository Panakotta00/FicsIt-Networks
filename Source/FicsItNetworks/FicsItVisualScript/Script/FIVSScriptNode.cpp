#include "FIVSScriptNode.h"

UFIVSPin* UFIVSScriptNode::CreatePin(EFIVSPinType PinType, const FString& InternalName, const FText& DisplayName, FFIVSPinDataType DataType) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->ParentNode = this;
	Pin->PinType = PinType;
	Pin->Name = InternalName;
	Pin->DisplayName = DisplayName;
	Pin->PinDataType = DataType;
		
	Pins.Add(Pin);
	return Pin;
}
