#include "FINScriptNode.h"

#include "FindInBlueprintManager.h"

FFINScriptPin::~FFINScriptPin() {
	for (FFINScriptPin* Pin : ConnectedPins) {
		Pin->RemoveConnection(Pin);
	}
}

void FFINScriptPin::AddConnection(FFINScriptPin* Pin) {
	if (ConnectedPins.Contains(Pin)) return;
	if (Pin == this) return;
	if (!((Pin->PinType & FIVS_PIN_INPUT && PinType & FIVS_PIN_OUTPUT) || (Pin->PinType & FIVS_PIN_OUTPUT && PinType & FIVS_PIN_INPUT))) return;
	if (Pin->PinType & FIVS_PIN_DATA && (!(PinType & FIVS_PIN_DATA) || (Pin->DataType != DataType))) return;
	ConnectedPins.Add(Pin);
	Pin->AddConnection(this);
}

void FFINScriptPin::RemoveConnection(FFINScriptPin* Pin) {
	if (!ConnectedPins.Contains(Pin)) return;
	ConnectedPins.Remove(Pin);
	Pin->RemoveConnection(this);
}

const TArray<FFINScriptPin*>& FFINScriptPin::GetConnections() const {
	return ConnectedPins;
}

int UFINScriptNode::AddPin(const TSharedPtr<FFINScriptPin>& Pin) {
	int idx = Pins.Add(Pin);
	if (idx >= 0) {
		Pin->ParentNode = this;
		OnPinChanged.Broadcast(1, idx);
	}
	return idx;
}

void UFINScriptNode::RemovePin(int index) {
	Pins.RemoveAt(index);
	OnPinChanged.Broadcast(1, index);
}

const TArray<TSharedPtr<FFINScriptPin>> UFINScriptNode::GetPins() const {
	return Pins;
}

