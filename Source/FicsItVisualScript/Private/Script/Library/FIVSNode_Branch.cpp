#include "Script/Library/FIVSNode_Branch.h"

#include "Kernel/FIVSRuntimeContext.h"

void FFIVSNodeStatement_Branch::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(Condition);
}

void FFIVSNodeStatement_Branch::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	bool bCondition = Context.TryGetRValue(Condition)->GetBool();
	if (bCondition) {
		Context.Push_ExecPin(ExecTrue);
	} else {
		Context.Push_ExecPin(ExecFalse);
	}
}

UFIVSNode_Branch::UFIVSNode_Branch() {
	DisplayName = FText::FromString(TEXT("Branch"));

	ExecIn = CreateDefaultPin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecTrue = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("True"), FText::FromString("True"));
	ExecFalse = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("False"), FText::FromString("False"));
	Condition = CreateDefaultPin(FIVS_PIN_DATA_INPUT, TEXT("Condition"), FText::FromString("Condition"), FFIVSPinDataType(FIN_BOOL));
}

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
