#include "FINScriptNode.h"

void UFINScriptPin::GetAllConnected(TArray<UFINScriptPin*>& Searches) {
	if (Searches.Contains(this)) return;
	Searches.Add(this);
	for (UFINScriptPin* Pin : GetConnections()) {
		Pin->GetAllConnected(Searches);
	}
}

void UFINScriptPin::AddConnection(UFINScriptPin* Pin) {
	if (!CanConnect(Pin) || !Pin->CanConnect(this)) return;
	ConnectedPins.Add(Pin);
	Pin->ConnectedPins.Add(this);
}

void UFINScriptPin::RemoveConnection(UFINScriptPin* Pin) {
	if (ConnectedPins.Contains(Pin)) ConnectedPins.Remove(Pin);
	if (Pin->ConnectedPins.Contains(this)) Pin->ConnectedPins.Remove(this);
}

EFINScriptPinType UFINScriptPin::GetPinType() {
	return FIVS_PIN_NONE;
}

EFINNetworkValueType UFINScriptPin::GetPinDataType() {
	return FIN_NIL;
}

const TArray<UFINScriptPin*>& UFINScriptPin::GetConnections() {
	return ConnectedPins;
}

FText UFINScriptPin::GetName() {
	return FText::FromString("");
}

bool UFINScriptPin::CanConnect(UFINScriptPin* Pin) {
	EFINScriptPinType ThisPinType = GetPinType();
	EFINScriptPinType PinPinType = Pin->GetPinType();
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
	TArray<UFINScriptPin*> Connections;
	GetAllConnected(Connections);
	for (UFINScriptPin* Connection : Connections) {
		if (Cast<UFINScriptWildcardPin>(Connection)) continue;
		if (Connection->GetPinType() & FIVS_PIN_INPUT) {
			bThisHasOutput = true;
		}
		if (Connection->GetPinType() & FIVS_PIN_OUTPUT) {
			bThisHasInput = true;
		}
	}
	Connections.Empty();
	Pin->GetAllConnected(Connections);
	for (UFINScriptPin* Connection : Connections) {
		if (Cast<UFINScriptWildcardPin>(Connection)) continue;
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

void UFINScriptPin::RemoveAllConnections() {
	TArray<UFINScriptPin*> Connections = GetConnections();
	for (UFINScriptPin* Connection : Connections) {
		RemoveConnection(Connection);
	}
}

EFINScriptPinType UFINScriptGenericPin::GetPinType() {
	return PinType;
}

EFINNetworkValueType UFINScriptGenericPin::GetPinDataType() {
	return PinDataType;
}

FText UFINScriptGenericPin::GetName() {
	return Name;
}

UFINScriptGenericPin* UFINScriptGenericPin::Create(EFINNetworkValueType DataType, EFINScriptPinType PinType, const FString& Name) {
	UFINScriptGenericPin* Pin = NewObject<UFINScriptGenericPin>();
	Pin->Name = FText::FromString(Name);
	Pin->PinDataType = DataType;
	Pin->PinType = PinType;
	return Pin;
}

EFINScriptPinType UFINScriptWildcardPin::GetPinType() {
	TArray<UFINScriptPin*> Connected;
	GetAllConnected(Connected);
	EFINScriptPinType Type = (EFINScriptPinType)(FIVS_PIN_EXEC | FIVS_PIN_DATA);
	for (UFINScriptPin* Pin : Connected) {
		if (Cast<UFINScriptWildcardPin>(Pin)) continue;
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

EFINNetworkValueType UFINScriptWildcardPin::GetPinDataType() {
	TArray<UFINScriptPin*> Connected;
	GetAllConnected(Connected);
	EFINNetworkValueType Type = FIN_ANY;
	for (UFINScriptPin* Pin : Connected) {
		if (Cast<UFINScriptWildcardPin>(Pin)) continue;
		Type = Pin->GetPinDataType();
		break;
	}
	return Type;
}

bool UFINScriptWildcardPin::CanConnect(UFINScriptPin* Pin) {
	return UFINScriptPin::CanConnect(Pin);
}

EFINScriptPinType UFINScriptReflectionPin::GetPinType() {
	return Property->GetPropertyFlags() & (FIN_Prop_OutParam | FIN_Prop_RetVal) ? FIVS_PIN_DATA_OUTPUT : FIVS_PIN_DATA_INPUT;
}

EFINNetworkValueType UFINScriptReflectionPin::GetPinDataType() {
	if (Property) {
		return Property->GetType();
	}
	return Super::GetPinDataType();
}

FText UFINScriptReflectionPin::GetName() {
	return Property->GetDisplayName();
}

void UFINScriptReflectionPin::SetProperty(UFINProperty* Prop) {
	Property = Prop;
}

void UFINScriptNode::RemoveAllConnections() {
	for (UFINScriptPin* Pin : GetNodePins()) {
		Pin->RemoveAllConnections();
	}
}

UFINScriptRerouteNode::UFINScriptRerouteNode() {
	Pin = CreateDefaultSubobject<UFINScriptWildcardPin>("Pin");
	Pin->ParentNode = this;
}

TArray<UFINScriptPin*> UFINScriptRerouteNode::GetNodePins() const {
	return {Pin};
}

int UFINScriptFuncNode::AddNodePin(UFINScriptPin* Pin) {
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

TArray<UFINScriptPin*> UFINScriptFuncNode::GetNodePins() const {
	return Pins;
}

FString UFINScriptReflectedFuncNode::GetNodeName() const {
	return Function->GetInternalName();
}

void UFINScriptReflectedFuncNode::SetFunction(UFINFunction* inFunction) {
	if (Function) {
		for (int i = 0; i < GetNodePins().Num(); ++i) {
			RemoveNodePin(i);
		}
	}
	Function = inFunction;
	UFINScriptGenericPin* ExecIn = NewObject<UFINScriptGenericPin>(this);
	ExecIn->PinDataType = FIN_NIL;
	ExecIn->PinType = FIVS_PIN_EXEC_INPUT;
	ExecIn->Name = FText::FromString("Exec");
	AddNodePin(ExecIn);
	if (Function->GetFunctionFlags() & FIN_Func_MemberFunc) {
		UFINScriptGenericPin* ReferenceIn = NewObject<UFINScriptGenericPin>(this);
		ReferenceIn->PinDataType = FIN_TRACE;
		ReferenceIn->PinType = FIVS_PIN_DATA_INPUT;
		ReferenceIn->Name = FText::FromString("Ref");
		AddNodePin(ReferenceIn);
	} else if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) {
		UFINScriptGenericPin* ReferenceIn = NewObject<UFINScriptGenericPin>(this);
		ReferenceIn->PinDataType = FIN_CLASS;
		ReferenceIn->PinType = FIVS_PIN_DATA_INPUT;
		ReferenceIn->Name = FText::FromString("Ref");
		AddNodePin(ReferenceIn);
	}
	UFINScriptGenericPin* ExecOut = NewObject<UFINScriptGenericPin>(this);
	ExecOut->PinDataType = FIN_NIL;
	ExecOut->PinType = FIVS_PIN_EXEC_OUTPUT;
	ExecOut->Name = FText::FromString("Return");
	AddNodePin(ExecOut);
	for (UFINProperty* Param : Function->GetParameters()) {
		UFINScriptReflectionPin* Pin = NewObject<UFINScriptReflectionPin>(this);
		Pin->SetProperty(Param);
		AddNodePin(Pin);
	}
}

UFINFunction* UFINScriptReflectedFuncNode::GetFunction() const {
	return Function;
}
