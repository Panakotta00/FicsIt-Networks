#include "FIVSNode_Branch.h"

TArray<FFIVSNodeAction> UFIVSNode_Branch::GetNodeActions() const {
	return {
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
	};
}

void UFIVSNode_Branch::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
	ExecTrue = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("True"));
	ExecFalse = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("False"));
	Condition = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Condition"), FFIVSPinDataType(FIN_BOOL));
}

UFIVSPin* UFIVSNode_Branch::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	bool bCondition = Context.GetValue(Condition)->GetBool();
	return bCondition ? ExecTrue : ExecFalse;
}
