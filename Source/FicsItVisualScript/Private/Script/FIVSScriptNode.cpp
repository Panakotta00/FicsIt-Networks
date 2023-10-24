#include "Script/FIVSScriptNode.h"

UFIVSPin* UFIVSScriptNode::CreatePin(EFIVSPinType PinType, const FString& InternalName, const FText& DisplayName, FFIVSPinDataType DataType) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->ParentNode = this;
	Pin->PinType = PinType;
	Pin->Name = InternalName;
	Pin->DisplayName = DisplayName;
	Pin->PinDataType = DataType;
		
	Pins.Add(Pin);
	OnPinChanged.Broadcast(FIVS_PinChange_Added, Pin);
	
	return Pin;
}

void UFIVSScriptNode::ReconstructPins() {
	TMap<FString, TArray<UFIVSPin*>> Connected;
	for (UFIVSPin* Pin : Pins) {
		TArray<UFIVSPin*>& Connections = Connected.FindOrAdd(Pin->GetName());
		for (UFIVSPin* Connection : Pin->GetConnections()) Connections.Add(Connection);
		Pin->RemoveAllConnections();
		OnPinChanged.Broadcast(FIVS_PinChange_Removed, Pin);
	}
	Pins.Empty();
	Super::ReconstructPins();
	for (TPair<FString, TArray<UFIVSPin*>> Connection : Connected) {
		UFIVSPin* From = FindPinByName(Connection.Key);
		if (From) for (UFIVSPin* To : Connection.Value) {
			From->AddConnection(To);
		}
	}
}
