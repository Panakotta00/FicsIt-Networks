#include "Script/Library/FIVSNode_Variable.h"

void UFIVSNode_Variable::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (EFINNetworkValueType DataType : TEnumRange<EFINNetworkValueType>()) {
		// Input FIN_ANY is excluded from conversion because it may fail or not and needs its own node
		if (DataType >= FIN_OBJ || DataType == FIN_NIL) continue;
		FString Name = FINGetNetworkValueTypeName(DataType);
		
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_Variable::StaticClass();
		Action.Title = FText::FromString(TEXT("Declare Local Variable (") + Name + TEXT(")"));
		Action.Category = FText::FromString(TEXT("General|Local"));
		Action.SearchableText = Action.Title;
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType)));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(DataType).AsRef()));
		Action.OnExecute.BindLambda([DataType](UFIVSNode* Node) {
			Cast<UFIVSNode_Variable>(Node)->SetType(FFIVSPinDataType(DataType), false);
		});
		Actions.Add(Action);

		Action.Title = FText::FromString(TEXT("Assign value to Reference (") + Name + TEXT(")"));
		Action.SearchableText = Action.Title;
		Action.Pins.Empty();
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_INPUT));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_OUTPUT));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType).AsRef()));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType)));
		Action.OnExecute.BindLambda([DataType](UFIVSNode* Node) {
			Cast<UFIVSNode_Variable>(Node)->SetType(FFIVSPinDataType(DataType), true);
		});
		Actions.Add(Action);
	}
}

TArray<UFIVSPin*> UFIVSNode_Variable::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<UFIVSPin*> InputPins;
	if (DataInput) InputPins.Add(DataInput);
	return InputPins;
}

TArray<UFIVSPin*> UFIVSNode_Variable::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	if (bAssignment) {
		FFIVSValue Variable = Context.GetValue(VarPin);
		FFIVSValue Value = Context.GetValue(DataInput);
		*Variable = *Value;
		return {ExecOutput};
	} else {
		FFINAnyNetworkValue* Value = Context.GetLocalVariable(GetFullName());
		if (!Value) {
			FFIVSValue InitValue = Context.GetValue(DataInput);
			Value = &Context.SetLocalVariable(GetFullName(), *InitValue); // TODO: Use different var name, current one is not persistable since Object it self doesnt get persisted but recreated
		}
		Context.SetValue(VarPin, FFIVSValue(Value));
		return {};
	}
}

void UFIVSNode_Variable::SetType(const FFIVSPinDataType& InType, bool bIsAssignment) {
	Type = InType;
	bAssignment = bIsAssignment;

	if (bAssignment) {
		DisplayName = FText::FromString(TEXT("Assign Reference"));
	} else {
		DisplayName = FText::FromString(TEXT("Declare Local Variable"));
	}

	DeletePin(ExecInput);
	DeletePin(ExecOutput);
	DeletePin(VarPin);
	DeletePin(DataInput);

	if (bAssignment) {
		ExecInput = CreatePin(FIVS_PIN_EXEC_INPUT,TEXT("Exec"), FText::FromString(TEXT("Exec")));
		ExecOutput = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString(TEXT("Out")));
		VarPin = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Var"), FText::FromString(TEXT("Var")), Type.AsRef());
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Value"), FText::FromString(TEXT("Value")), Type.AsVal());
	} else {
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Init Value"), FText::FromString(TEXT("Init Value")), Type.AsVal());
		VarPin = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Var"), FText::FromString(TEXT("Var")), Type.AsRef());
	}
}
