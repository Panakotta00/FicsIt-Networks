#include "Script/FIVSPin.h"


void UFIVSPin::GetAllConnected(TArray<UFIVSPin*>& Searches) {
	if (Searches.Contains(this)) return;
	Searches.Add(this);
	for (UFIVSPin* Pin : GetConnections()) {
		Pin->GetAllConnected(Searches);
	}
}

UFIVSPin* UFIVSPin::FindConnected() {
	EFIVSPinType PinType = (GetPinType() & FIVS_PIN_DATA) != 0 ? FIVS_PIN_OUTPUT : FIVS_PIN_INPUT;
	for (UFIVSPin* Pin : GetAllConnected()) {
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

void UFIVSPin::AddConnections(TArray<UFIVSPin*> Pins) {
	for (UFIVSPin* pin : Pins) {
		AddConnection(pin);
	}
}

void UFIVSPin::RemoveConnection(UFIVSPin* Pin) {
	if (ConnectedPins.Contains(Pin)) ConnectedPins.Remove(Pin);
	if (Pin->ConnectedPins.Contains(this)) Pin->ConnectedPins.Remove(this);
}

bool FFIVSFullPinType::CanConnect(const FFIVSFullPinType& Other) const {
	const FFIVSFullPinType* Output = this;
	const FFIVSFullPinType* Input = &Other;
	if (Output->PinType & FIVS_PIN_INPUT && Input->PinType & FIVS_PIN_OUTPUT) {
		Swap(Output, Input);
	}
	if (!(Output->PinType & FIVS_PIN_OUTPUT && Input->PinType & FIVS_PIN_INPUT)) return false;

	bool bWouldFail = true;
	if (Output->PinType & FIVS_PIN_DATA) {
		bWouldFail = false;
		if (!(Input->PinType & FIVS_PIN_DATA)) bWouldFail = true;
		else if (!Output->DataType.IsA(Input->DataType) && Output->PinType != (FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT) && Input->PinType != (FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT) /* Exclude wildcard pins with no yet defined state */) bWouldFail = true;
	}
	if (bWouldFail && Output->PinType & FIVS_PIN_EXEC) {
		bWouldFail = false;
		if (!(Input->PinType & FIVS_PIN_EXEC)) bWouldFail = true;
	}
	if (bWouldFail) return false;
	return true;
}

EFIVSPinType UFIVSPin::GetPinType() {
	return FIVS_PIN_NONE;
}

FFIVSPinDataType UFIVSPin::GetPinDataType() {
	return FFIVSPinDataType(FIR_NIL);
}

const TArray<UFIVSPin*>& UFIVSPin::GetConnections() {
	return ConnectedPins;
}

bool UFIVSPin::CanConnect(UFIVSPin* Pin) {
	if (ConnectedPins.Contains(Pin) || Pin == this) return false;

	UFIVSPin* OutputPin = this;
	UFIVSPin* InputPin = Pin;
	EFIVSPinType OutputPinType = GetPinType();
	EFIVSPinType InputPinType = Pin->GetPinType();
	FFIVSPinDataType OutputPinDataType = GetPinDataType();
	FFIVSPinDataType InputPinDataType = Pin->GetPinDataType();

	// Swap Input and Output Pins if it makes sense to swap them
	if (OutputPinType & FIVS_PIN_INPUT && InputPinType & FIVS_PIN_OUTPUT) {
		Swap(OutputPinType, InputPinType);
		Swap(OutputPinDataType, InputPinDataType);
		Swap(OutputPin, InputPin);
	}
	if (!(OutputPinType & FIVS_PIN_OUTPUT && InputPinType & FIVS_PIN_INPUT)) return false;

	if (!FFIVSFullPinType(OutputPinType, OutputPinDataType).CanConnect(FFIVSFullPinType(InputPinType, InputPinDataType))) return false;

	bool bThisHasInput = false;
	bool bPinHasInput = false;
	bool bThisHasOutput = false;
	bool bPinHasOutput = false;
	TArray<UFIVSPin*> Connections;
	OutputPin->GetAllConnected(Connections);
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
	InputPin->GetAllConnected(Connections);
	for (UFIVSPin* Connection : Connections) {
		if (Cast<UFIVSWildcardPin>(Connection)) continue;
		if (Connection->GetPinType() & FIVS_PIN_INPUT) {
			bPinHasOutput = true;
		}
		if (Connection->GetPinType() & FIVS_PIN_OUTPUT) {
			bPinHasInput = true;
		}
	}

	if (OutputPinType & FIVS_PIN_DATA) {
		if (bThisHasInput && bPinHasInput) return false;
	} else if (OutputPinType & FIVS_PIN_EXEC) {
		if (bThisHasOutput && bPinHasOutput) return false;
	}
	return true;
}

TArray<UFIVSPin*> UFIVSPin::RemoveAllConnections() {
	TArray<UFIVSPin*> Connections = GetConnections();
	for (UFIVSPin* Connection : Connections) {
		RemoveConnection(Connection);
	}
	return Connections;
}

EFIVSPinType UFIVSGenericPin::GetPinType() {
	return PinType;
}

FFIVSPinDataType UFIVSGenericPin::GetPinDataType() {
	return PinDataType;
}

UFIVSGenericPin* UFIVSGenericPin::Create(FFIVSPinDataType DataType, EFIVSPinType PinType, const FString& Name, const FText& DisplayName) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->Name = Name;
	Pin->DisplayName = DisplayName;
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

FFIVSPinDataType UFIVSWildcardPin::GetPinDataType() {
	TArray<UFIVSPin*> Connected;
	GetAllConnected(Connected);
	FFIVSPinDataType Type = FFIVSPinDataType(FFIRExtendedValueType(FIR_ANY));
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
