#include "FINScriptNode.h"

#include "FindInBlueprintManager.h"

void FFINScriptPin::GetAllConnected(TArray<TSharedPtr<FFINScriptPin>>& Searches) {
	for (const TSharedPtr<FFINScriptPin>& Pin : GetConnections()) {
		if (Searches.Contains(Pin)) continue;
		Searches.Add(Pin);
		Pin->GetAllConnected(Searches);
	}
}

FFINScriptPin::~FFINScriptPin() {
	for (const TSharedPtr<FFINScriptPin>& Pin : ConnectedPins) {
		Pin->RemoveConnection(Pin);
	}
}

void FFINScriptPin::AddConnection(const TSharedPtr<FFINScriptPin>& Pin) {
	if (!CanConnect(Pin)) return;
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
		
		if (!(PinPinType & FIVS_PIN_INPUT)) {
			TArray<TSharedPtr<FFINScriptPin>> Pins = GetConnections();
			for (const TSharedPtr<FFINScriptPin>& ConPin : Pins) {
				if (ConPin->GetPinType() & FIVS_PIN_OUTPUT) {
					return false;
				}
			}
		}
		if (!(PinPinType & FIVS_PIN_OUTPUT)) {
			TArray<TSharedPtr<FFINScriptPin>> Pins = Pin->GetConnections();
			for (const TSharedPtr<FFINScriptPin>& ConPin : Pins) {
				if (ConPin->GetPinType() & FIVS_PIN_OUTPUT) {
					return false;
				}
			}
		}
	} else if (ThisPinType & FIVS_PIN_EXEC) {
		if (!(PinPinType & FIVS_PIN_EXEC)) return false;
		if (PinPinType & FIVS_PIN_INPUT) {
			TArray<TSharedPtr<FFINScriptPin>> Pins = GetConnections();
			for (const TSharedPtr<FFINScriptPin>& ConPin : Pins) {
				if (ConPin->GetPinType() & FIVS_PIN_INPUT) {
					return false;
				}
			}
		}
		if (PinPinType & FIVS_PIN_OUTPUT) {
			TArray<TSharedPtr<FFINScriptPin>> Pins = GetConnections();
			for (const TSharedPtr<FFINScriptPin>& ConPin : Pins) {
				if (ConPin->GetPinType() & FIVS_PIN_INPUT) {
					return false;
				}
			}
		}
	}
	return true;
}

EFINNetworkValueType FFINScriptPin::GetPinDataType() {
	return DataType;
}

FFINScriptWildcardPin::FFINScriptWildcardPin() {
	PinType = (EFINScriptPinType)(FIVS_PIN_INPUT | FIVS_PIN_OUTPUT);
}

EFINScriptPinType FFINScriptWildcardPin::GetPinType() {
	TArray<TSharedPtr<FFINScriptPin>> Connected = GetConnections();
	if (IgnoreNext) Connected.Remove(IgnoreNext);
	IgnoreNext = nullptr;
	bool hasInput = false;
	bool hasOutput = false;
	for (const TSharedPtr<FFINScriptPin>& Pin : Connected) {
		FFINScriptWildcardPin* WPin = dynamic_cast<FFINScriptWildcardPin*>(Pin.Get());
		if (WPin) WPin->IgnoreNext = AsShared();
		EFINScriptPinType Type = Pin->GetPinType();
		if (Type & FIVS_PIN_OUTPUT) hasInput = true;
		if (Type & FIVS_PIN_INPUT) hasOutput = true;
		if (Type & FIVS_PIN_DATA) {
			if (hasInput) {
				return (EFINScriptPinType)(FIVS_PIN_DATA | FIVS_PIN_OUTPUT);
			}
			return (EFINScriptPinType)(FIVS_PIN_DATA | FIVS_PIN_OUTPUT | FIVS_PIN_INPUT);
		}
		if (Type & FIVS_PIN_EXEC) {
			if (!hasOutput) {
				return (EFINScriptPinType)(FIVS_PIN_EXEC | FIVS_PIN_INPUT | FIVS_PIN_OUTPUT);
			}
			return (EFINScriptPinType)(FIVS_PIN_EXEC | FIVS_PIN_INPUT);
		}
		return FIVS_PIN_NONE;
	}
	return (EFINScriptPinType)(FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT);
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
	EFINScriptPinType PinType = GetPinType();
	if ((PinType & FIVS_PIN_DATA) && (PinType & FIVS_PIN_EXEC)) return true;
	const FFINScriptWildcardPin* WPin = dynamic_cast<const FFINScriptWildcardPin*>(Pin.Get());
	if (WPin) {
		PinType = Pin->GetPinType();
		if ((PinType & FIVS_PIN_DATA) && (PinType & FIVS_PIN_EXEC)) return true;
	}
	if (ConnectedPins.Num() > 0) {
		return FFINScriptPin::CanConnect(Pin);
	}
	return Pin->CanConnect(AsShared());
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

