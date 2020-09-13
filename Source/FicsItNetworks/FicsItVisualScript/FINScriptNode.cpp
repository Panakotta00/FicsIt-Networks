#include "FINScriptNode.h"

#include "FindInBlueprintManager.h"

void FFINScriptPin::GetAllConnected(TArray<TSharedPtr<FFINScriptPin>>& Searches) {
	if (Searches.Contains(SharedThis(this))) return;
	Searches.Add(SharedThis(this));
	for (const TSharedPtr<FFINScriptPin>& Pin : GetConnections()) {
		Pin->GetAllConnected(Searches);
	}
}

FFINScriptPin::~FFINScriptPin() {
	for (const TSharedPtr<FFINScriptPin>& Pin : ConnectedPins) {
		Pin->RemoveConnection(Pin);
	}
}

void FFINScriptPin::AddConnection(const TSharedPtr<FFINScriptPin>& Pin) {
	if (!CanConnect(Pin) || !Pin->CanConnect(SharedThis(this))) return;
	ConnectedPins.Add(Pin);
	Pin->ConnectedPins.Add(AsShared());
}

void FFINScriptPin::RemoveConnection(const TSharedPtr<FFINScriptPin>& Pin) {
	if (ConnectedPins.Contains(Pin)) ConnectedPins.Remove(Pin);
	if (Pin->ConnectedPins.Contains(AsShared())) Pin->ConnectedPins.Remove(AsShared());
}

const TArray<TSharedPtr<FFINScriptPin>>& FFINScriptPin::GetConnections() {
	return ConnectedPins;
}

EFINScriptPinType FFINScriptPin::GetPinType() {
	return PinType;
}

bool FFINScriptPin::CanConnect(const TSharedPtr<FFINScriptPin>& Pin) {
	EFINScriptPinType ThisPinType = GetPinType();
	EFINScriptPinType PinPinType = Pin->GetPinType();
	EFINNetworkValueType ThisPinDataType = GetPinDataType();
	EFINNetworkValueType PinPinDataType = Pin->GetPinDataType();
	if (ConnectedPins.Contains(Pin) || Pin == SharedThis(this)) return false;
	if (!((PinPinType & FIVS_PIN_INPUT && ThisPinType & FIVS_PIN_OUTPUT) || (PinPinType & FIVS_PIN_OUTPUT && ThisPinType & FIVS_PIN_INPUT))) return false;
	
	if (ThisPinType & FIVS_PIN_DATA) {
		if (!(PinPinType & FIVS_PIN_DATA)) return false;
		if (!((PinPinDataType == ThisPinDataType) ||
            (PinPinDataType == FIN_ANY) ||
            (ThisPinDataType == FIN_ANY))) return false;
	} else if (ThisPinType & FIVS_PIN_EXEC) {
		if (!(PinPinType & FIVS_PIN_EXEC)) return false;
	}

	bool bThisHasInput = false;
	bool bPinHasInput = false;
	bool bThisHasOutput = false;
	bool bPinHasOutput = false;
	TArray<TSharedPtr<FFINScriptPin>> Connections;
	GetAllConnected(Connections);
	for (const TSharedPtr<FFINScriptPin>& Connection : Connections) {
		if (dynamic_cast<FFINScriptWildcardPin*>(Connection.Get())) continue;
		if (Connection->GetPinType() & FIVS_PIN_INPUT) {
			bThisHasOutput = true;
		}
		if (Connection->GetPinType() & FIVS_PIN_OUTPUT) {
			bThisHasInput = true;
		}
	}
	Connections.Empty();
	Pin->GetAllConnected(Connections);
	for (const TSharedPtr<FFINScriptPin>& Connection : Connections) {
		if (dynamic_cast<FFINScriptWildcardPin*>(Connection.Get())) continue;
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

EFINNetworkValueType FFINScriptPin::GetPinDataType() {
	return DataType;
}

void FFINScriptPin::RemoveAllConnections() {
	TArray<TSharedPtr<FFINScriptPin>> Connections = GetConnections();
	for (const TSharedPtr<FFINScriptPin>& Connection : Connections) {
		RemoveConnection(Connection);
	}
}

FFINScriptWildcardPin::FFINScriptWildcardPin() {
	PinType = (EFINScriptPinType)(FIVS_PIN_INPUT | FIVS_PIN_OUTPUT);
}

EFINScriptPinType FFINScriptWildcardPin::GetPinType() {
	TArray<TSharedPtr<FFINScriptPin>> Connected;
	GetAllConnected(Connected);
	EFINScriptPinType Type = (EFINScriptPinType)(FIVS_PIN_EXEC | FIVS_PIN_DATA);
	for (const TSharedPtr<FFINScriptPin>& Pin : Connected) {
		if (dynamic_cast<FFINScriptWildcardPin*>(Pin.Get())) continue;
		EFINScriptPinType PinType = Pin->GetPinType();
		if (PinType & FIVS_PIN_DATA) {
			if (PinType & FIVS_PIN_OUTPUT) {
				return (EFINScriptPinType)(FIVS_PIN_DATA | FIVS_PIN_OUTPUT);
			}
			Type = FIVS_PIN_DATA;
		}
		if (PinType & FIVS_PIN_EXEC) {
			if (PinType & FIVS_PIN_INPUT) {
				return (EFINScriptPinType)(FIVS_PIN_EXEC | FIVS_PIN_INPUT);
			}
			Type = FIVS_PIN_EXEC;
		}
	}
	return (EFINScriptPinType)(Type | FIVS_PIN_INPUT | FIVS_PIN_OUTPUT);
}

EFINNetworkValueType FFINScriptWildcardPin::GetPinDataType() {
	TArray<TSharedPtr<FFINScriptPin>> Connected;
	GetAllConnected(Connected);
	EFINNetworkValueType Type = FIN_ANY;
	for (const TSharedPtr<FFINScriptPin>& Pin : Connected) {
		if (dynamic_cast<FFINScriptWildcardPin*>(Pin.Get())) continue;
		Type = Pin->GetPinDataType();
		break;
	}
	return Type;
}

bool FFINScriptWildcardPin::CanConnect(const TSharedPtr<FFINScriptPin>& Pin) {
	return FFINScriptPin::CanConnect(Pin);
}

void UFINScriptNode::RemoveAllConnections() {
	for (const TSharedRef<FFINScriptPin>& Pin : GetNodePins()) {
		Pin->RemoveAllConnections();
	}
}

UFINScriptRerouteNode::UFINScriptRerouteNode() : Pin(MakeShared<FFINScriptWildcardPin>()) {
	Pin->ParentNode = this;
}

TArray<TSharedRef<FFINScriptPin>> UFINScriptRerouteNode::GetNodePins() const {
	return {Pin};
}

int UFINScriptFuncNode::AddNodePin(const TSharedRef<FFINScriptPin>& Pin) {
	int idx = Pins.Add(Pin);
	if (idx >= 0) {
		Pin->ParentNode = this;
		OnPinChanged.Broadcast(1, idx);
	}
	return idx;
}

void UFINScriptFuncNode::RemoveNodePin(int index) {
	Pins.RemoveAt(index);
	OnPinChanged.Broadcast(1, index);
}

TArray<TSharedRef<FFINScriptPin>> UFINScriptFuncNode::GetNodePins() const {
	return Pins;
}

