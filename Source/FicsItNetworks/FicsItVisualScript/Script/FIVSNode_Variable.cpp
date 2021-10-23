#include "FIVSNode_Variable.h"

void UFIVSNode_Variable::InitPins() {
	if (bAssignment) {
		CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString(TEXT("Exec")));
		ExecOutput = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString(TEXT("Out")));
		VarPin = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("Var")), Type.AsRef());
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("Value")), Type.AsVal());
	} else {
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("InitValue")), Type.AsVal());
		VarPin = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString(TEXT("Var")), Type.AsRef());
	}
}

TArray<FFIVSNodeAction> UFIVSNode_Variable::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for (EFINNetworkValueType DataType : TEnumRange<EFINNetworkValueType>()) {
		// Input FIN_ANY is excluded from conversion because it may fail or not and needs its own node
		if (DataType >= FIN_OBJ || DataType == FIN_NIL) continue;
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_Variable::StaticClass();
		Action.Title = FText::FromString(TEXT("Declare Local Var"));
		Action.Category = FText::FromString(TEXT("General|Local"));
		Action.SearchableText = Action.Title;
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType)));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(DataType).AsRef()));
		Action.OnExecute.BindLambda([DataType](UFIVSNode* Node) {
			Cast<UFIVSNode_Variable>(Node)->Type = FFIVSPinDataType(DataType);
		});
		Actions.Add(Action);

		Action.Title = FText::FromString(TEXT("Assign value to Var"));
		Action.SearchableText = Action.Title;
		Action.Pins.Empty();
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_INPUT));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_OUTPUT));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType).AsRef()));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType)));
		Action.OnExecute.BindLambda([DataType](UFIVSNode* Node) {
			Cast<UFIVSNode_Variable>(Node)->Type = FFIVSPinDataType(DataType);
			Cast<UFIVSNode_Variable>(Node)->bAssignment = true;
		});
		Actions.Add(Action);
	}
	return Actions;
}

FString UFIVSNode_Variable::GetNodeName() const {
	return TEXT("Test");
}
