#include "Script/FIVSScriptNode.h"

UFIVSPin* UFIVSScriptNode::CreatePin(EFIVSPinType PinType, const FString& InternalName, const FText& InDisplayName, FFIVSPinDataType DataType) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->ParentNode = this;
	Pin->PinType = PinType;
	Pin->Name = InternalName;
	Pin->DisplayName = InDisplayName;
	Pin->PinDataType = DataType;

	Pins.Add(Pin);
	OnPinChanged.Broadcast(FIVS_PinChange_Added, Pin);

	return Pin;
}

UFIVSPin* UFIVSScriptNode::CreateDefaultPin(EFIVSPinType PinType, const FName& Name, const FText& InDisplayName, FFIVSPinDataType DataType) {
	UFIVSGenericPin* Pin = CreateDefaultSubobject<UFIVSGenericPin>(Name);
	Pin->ParentNode = this;
	Pin->PinType = PinType;
	Pin->Name = Name.ToString();
	Pin->DisplayName = InDisplayName;
	Pin->PinDataType = DataType;

	Pins.Add(Pin);

	return Pin;
}

void UFIVSScriptNode::DeletePin(UFIVSPin* Pin) {
	if (Pins.Remove(Pin) > 0) {
		OnPinChanged.Broadcast(FIVS_PinChange_Removed, Pin);
	}
}

void UFIVSScriptNode::DeletePins(TArrayView<UFIVSPin*> InPins) {
	for (UFIVSPin* Pin : InPins) {
		DeletePin(Pin);
	}
}

void UFIVSScriptNode::RecreatePin(UFIVSPin*& Pin, EFIVSPinType PinType, const FString& Name, const FText& InDisplayName, FFIVSPinDataType DataType) {
	TArray<UFIVSPin*> Connections = Pin->RemoveAllConnections();
	DeletePin(Pin);
	Pin = CreatePin(PinType, Name, InDisplayName, DataType);
	Pin->AddConnections(Connections);
}
