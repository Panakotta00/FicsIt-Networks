#include "Script/FIVSNode_Branch.h"

void UFIVSNode_Branch::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	Actions.Add(
		FFIVSNodeAction{
			UFIVSNode_Branch::StaticClass(),
			FText::FromString(TEXT("Branch")),
			FText::FromString(TEXT("General")),
			FText::FromString(TEXT("Branch")),
			{
				FIVS_PIN_EXEC_INPUT,
				{FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIN_BOOL)},
				FIVS_PIN_EXEC_OUTPUT,
				FIVS_PIN_EXEC_OUTPUT
			}
		}
	);
}

void UFIVSNode_Branch::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecTrue = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("True"), FText::FromString("True"));
	ExecFalse = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("False"), FText::FromString("False"));
	Condition = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Condition"), FText::FromString("Condition"), FFIVSPinDataType(FIN_BOOL));
}

UFIVSPin* UFIVSNode_Branch::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	bool bCondition = Context.GetValue(Condition)->GetBool();
	return bCondition ? ExecTrue : ExecFalse;
}
