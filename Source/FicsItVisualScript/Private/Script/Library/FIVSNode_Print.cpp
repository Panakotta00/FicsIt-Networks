#include "Script/Library/FIVSNode_Print.h"

#include "FILLogContainer.h"
#include "Kernel/FIVSRuntimeContext.h"

void FFIVSNodeStatement_Print::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(MessageIn);
}

void FFIVSNodeStatement_Print::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	FString Message = Context.TryGetRValue(MessageIn)->GetString();
	Context.GetKernelContext()->GetLog()->PushLogEntry(FIL_Verbosity_Info, Message);
	Context.Push_ExecPin(ExecOut);
}

UFIVSNode_Print::UFIVSNode_Print() {
	DisplayName = FText::FromString(TEXT("Print"));

	ExecIn = CreateDefaultPin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString("Out"));
	MessageIn = CreateDefaultPin(FIVS_PIN_DATA_INPUT, TEXT("Message"), FText::FromString("Message"), FFIVSPinDataType(FIR_STR));
}

void UFIVSNode_Print::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	Actions.Add(
		{
			UFIVSNode_Print::StaticClass(),
			FText::FromString(TEXT("Print")),
			FText::FromString(TEXT("General")),
			FText::FromString(TEXT("Print")),
			{
				FIVS_PIN_EXEC_INPUT,
				{FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIR_STR)},
				FIVS_PIN_EXEC_OUTPUT,
			}
		}
	);
}
