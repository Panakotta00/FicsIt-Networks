#include "FIVSNode.h"

void UFIVSPin::GetAllConnected(TArray<UFIVSPin*>& Searches) {
	if (Searches.Contains(this)) return;
	Searches.Add(this);
	for (UFIVSPin* Pin : GetConnections()) {
		Pin->GetAllConnected(Searches);
	}
}

UFIVSPin* UFIVSPin::FindConnected() {
	EFIVSPinType PinType = (GetPinType() & FIVS_PIN_DATA) != 0 ? FIVS_PIN_OUTPUT : FIVS_PIN_INPUT;
	for (UFIVSPin* Pin :  GetAllConnected()) {
		if (Pin->IsA<UFIVSWildcardPin>()) continue;
		if ((Pin->GetPinType() & PinType) > 0) return Pin;
	}
	return nullptr;
}

void UFIVSPin::AddConnection(UFIVSPin* Pin) {
	if (!CanConnect(Pin) || !Pin->CanConnect(this)) return;
	ConnectedPins.Add(Pin);
	Pin->ConnectedPins.Add(this);
}

void UFIVSPin::RemoveConnection(UFIVSPin* Pin) {
	if (ConnectedPins.Contains(Pin)) ConnectedPins.Remove(Pin);
	if (Pin->ConnectedPins.Contains(this)) Pin->ConnectedPins.Remove(this);
}

EFIVSPinType UFIVSPin::GetPinType() {
	return FIVS_PIN_NONE;
}

EFINNetworkValueType UFIVSPin::GetPinDataType() {
	return FIN_NIL;
}

const TArray<UFIVSPin*>& UFIVSPin::GetConnections() {
	return ConnectedPins;
}

FText UFIVSPin::GetName() {
	return FText::FromString("");
}

bool UFIVSPin::CanConnect(UFIVSPin* Pin) {
	EFIVSPinType ThisPinType = GetPinType();
	EFIVSPinType PinPinType = Pin->GetPinType();
	EFINNetworkValueType ThisPinDataType = GetPinDataType();
	EFINNetworkValueType PinPinDataType = Pin->GetPinDataType();
	if (ConnectedPins.Contains(Pin) || Pin == this) return false;
	if (!((PinPinType & FIVS_PIN_INPUT && ThisPinType & FIVS_PIN_OUTPUT) || (PinPinType & FIVS_PIN_OUTPUT && ThisPinType & FIVS_PIN_INPUT))) return false;

	bool bWouldFail = false;
	if (ThisPinType & FIVS_PIN_DATA) {
		bWouldFail = false;
		if (!(PinPinType & FIVS_PIN_DATA)) bWouldFail = true;
		else if (!((PinPinDataType == ThisPinDataType) ||
            (PinPinDataType == FIN_ANY) ||
            (ThisPinDataType == FIN_ANY))) bWouldFail = true;
	}
	if (ThisPinType & FIVS_PIN_EXEC) {
		bWouldFail = false;
		if (!(PinPinType & FIVS_PIN_EXEC)) bWouldFail = true;
	}
	if (bWouldFail) return false;

	bool bThisHasInput = false;
	bool bPinHasInput = false;
	bool bThisHasOutput = false;
	bool bPinHasOutput = false;
	TArray<UFIVSPin*> Connections;
	GetAllConnected(Connections);
	for (UFIVSPin* Connection : Connections) {
		if (Cast<UFIVSWildcardPin>(Connection)) continue;
		if (Connection->GetPinType() & FIVS_PIN_INPUT) {
			bThisHasOutput = true;
		}
		if (Connection->GetPinType() & FIVS_PIN_OUTPUT) {
			bThisHasInput = true;
		}
	}
	Connections.Empty();
	Pin->GetAllConnected(Connections);
	for (UFIVSPin* Connection : Connections) {
		if (Cast<UFIVSWildcardPin>(Connection)) continue;
		if (Connection->GetPinType() & FIVS_PIN_INPUT) {
			bPinHasOutput = true;
		}
		if (Connection->GetPinType() & FIVS_PIN_OUTPUT) {
			bPinHasInput = true;
		}
	}

	if (ThisPinType & FIVS_PIN_DATA) {
		if (bThisHasInput && bPinHasInput) return false;
	} else if (ThisPinType & FIVS_PIN_EXEC) {
		if (bThisHasOutput && bPinHasOutput) return false;
	}
	return true;
}

void UFIVSPin::RemoveAllConnections() {
	TArray<UFIVSPin*> Connections = GetConnections();
	for (UFIVSPin* Connection : Connections) {
		RemoveConnection(Connection);
	}
}

EFIVSPinType UFIVSGenericPin::GetPinType() {
	return PinType;
}

EFINNetworkValueType UFIVSGenericPin::GetPinDataType() {
	return PinDataType;
}

FText UFIVSGenericPin::GetName() {
	return Name;
}

UFIVSGenericPin* UFIVSGenericPin::Create(EFINNetworkValueType DataType, EFIVSPinType PinType, const FString& Name) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->Name = FText::FromString(Name);
	Pin->PinDataType = DataType;
	Pin->PinType = PinType;
	return Pin;
}

EFIVSPinType UFIVSWildcardPin::GetPinType() {
	TArray<UFIVSPin*> Connected;
	GetAllConnected(Connected);
	EFIVSPinType Type = (EFIVSPinType)(FIVS_PIN_EXEC | FIVS_PIN_DATA);
	for (UFIVSPin* Pin : Connected) {
		if (Cast<UFIVSWildcardPin>(Pin)) continue;
		EFIVSPinType ConnectedPinType = Pin->GetPinType();
		if (ConnectedPinType & FIVS_PIN_DATA) {
			if (ConnectedPinType & FIVS_PIN_OUTPUT) {
				return (EFIVSPinType)(FIVS_PIN_DATA | FIVS_PIN_OUTPUT);
			}
			Type = FIVS_PIN_DATA;
		}
		if (ConnectedPinType & FIVS_PIN_EXEC) {
			if (ConnectedPinType & FIVS_PIN_INPUT) {
				return (EFIVSPinType)(FIVS_PIN_EXEC | FIVS_PIN_INPUT);
			}
			Type = FIVS_PIN_EXEC;
		}
	}
	return (EFIVSPinType)(Type | FIVS_PIN_INPUT | FIVS_PIN_OUTPUT);
}

EFINNetworkValueType UFIVSWildcardPin::GetPinDataType() {
	TArray<UFIVSPin*> Connected;
	GetAllConnected(Connected);
	EFINNetworkValueType Type = FIN_ANY;
	for (UFIVSPin* Pin : Connected) {
		if (Cast<UFIVSWildcardPin>(Pin)) continue;
		Type = Pin->GetPinDataType();
		break;
	}
	return Type;
}

bool UFIVSWildcardPin::CanConnect(UFIVSPin* Pin) {
	return UFIVSPin::CanConnect(Pin);
}

void UFIVSNode::RemoveAllConnections() {
	for (UFIVSPin* Pin : GetNodePins()) {
		Pin->RemoveAllConnections();
	}
}

UFIVSRerouteNode::UFIVSRerouteNode() {
	Pin = CreateDefaultSubobject<UFIVSWildcardPin>("Pin");
	Pin->ParentNode = this;
}

TArray<UFIVSPin*> UFIVSRerouteNode::GetNodePins() const {
	return {Pin};
}
