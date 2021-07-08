#include "FIVSNode.h"

void UFIVSPin::GetAllConnected(TArray<UFIVSPin*>& Searches) {
	if (Searches.Contains(this)) return;
	Searches.Add(this);
	for (UFIVSPin* Pin : GetConnections()) {
		Pin->GetAllConnected(Searches);
	}
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

EFIVSPinType UFIVSReflectionPin::GetPinType() {
	return Property->GetPropertyFlags() & (FIN_Prop_OutParam | FIN_Prop_RetVal) ? FIVS_PIN_DATA_OUTPUT : FIVS_PIN_DATA_INPUT;
}

EFINNetworkValueType UFIVSReflectionPin::GetPinDataType() {
	if (Property) {
		return Property->GetType();
	}
	return Super::GetPinDataType();
}

FText UFIVSReflectionPin::GetName() {
	return Property->GetDisplayName();
}

void UFIVSReflectionPin::SetProperty(UFINProperty* Prop) {
	Property = Prop;
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

int UFIVSFuncNode::AddNodePin(UFIVSPin* Pin) {
	int idx = Pins.Add(Pin);
	if (idx >= 0) {
		Pin->ParentNode = this;
		OnPinChanged.Broadcast(1, idx);
	}
	return idx;
}

void UFIVSFuncNode::RemoveNodePin(int index) {
	Pins.RemoveAt(index);
	OnPinChanged.Broadcast(1, index);
}

TArray<UFIVSPin*> UFIVSFuncNode::GetNodePins() const {
	return Pins;
}

FString UFIVSReflectedFuncNode::GetNodeName() const {
	return Function->GetInternalName();
}

void UFIVSReflectedFuncNode::SetFunction(UFINFunction* inFunction) {
	if (Function) {
		for (int i = 0; i < GetNodePins().Num(); ++i) {
			RemoveNodePin(i);
		}
	}
	Function = inFunction;
	UFIVSGenericPin* ExecIn = NewObject<UFIVSGenericPin>(this);
	ExecIn->PinDataType = FIN_NIL;
	ExecIn->PinType = FIVS_PIN_EXEC_INPUT;
	ExecIn->Name = FText::FromString("Exec");
	AddNodePin(ExecIn);
	if (Function->GetFunctionFlags() & FIN_Func_MemberFunc) {
		UFIVSGenericPin* ReferenceIn = NewObject<UFIVSGenericPin>(this);
		ReferenceIn->PinDataType = FIN_TRACE;
		ReferenceIn->PinType = FIVS_PIN_DATA_INPUT;
		ReferenceIn->Name = FText::FromString("Ref");
		AddNodePin(ReferenceIn);
	} else if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) {
		UFIVSGenericPin* ReferenceIn = NewObject<UFIVSGenericPin>(this);
		ReferenceIn->PinDataType = FIN_CLASS;
		ReferenceIn->PinType = FIVS_PIN_DATA_INPUT;
		ReferenceIn->Name = FText::FromString("Ref");
		AddNodePin(ReferenceIn);
	}
	UFIVSGenericPin* ExecOut = NewObject<UFIVSGenericPin>(this);
	ExecOut->PinDataType = FIN_NIL;
	ExecOut->PinType = FIVS_PIN_EXEC_OUTPUT;
	ExecOut->Name = FText::FromString("Return");
	AddNodePin(ExecOut);
	for (UFINProperty* Param : Function->GetParameters()) {
		UFIVSReflectionPin* Pin = NewObject<UFIVSReflectionPin>(this);
		Pin->SetProperty(Param);
		AddNodePin(Pin);
	}
}

UFINFunction* UFIVSReflectedFuncNode::GetFunction() const {
	return Function;
}
